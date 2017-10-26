#include <algorithm>
#include <iostream>
#include <map>
#include <chrono>

#include "spellbook.h"

using namespace std::chrono;

int main(int argc, char** argv) {
	auto start = high_resolution_clock::now();

	std::cout << "------------\nIntrospector-generator run" << std::endl;

	auto end_message = augs::make_scope_guard([&]() {
		std::cout << "Run time: " << duration_cast<duration<double, milliseconds::period>>(high_resolution_clock::now() - start).count() << " ms" << std::endl << "------------\n";
	});

	static_assert("C++17");

	if (const auto cxx17iftest = !(argc >= 2);
		cxx17iftest
	) {
		std::cout << "usage: configuration_file_input_path" << std::endl;
		return 0;
	}

	const std::string configuration_file_input_path = argv[1];

	auto guarded_create_file = [](
		const std::string& path,
		const std::string& new_contents
		) {
		if (!fs::exists(path) || get_file_contents(path) != new_contents) {
			create_text_file(path, new_contents);
		}
	};

	std::string beginning_line;
	std::string ending_line;
	std::string introspect_base_line;
	std::vector<std::string> header_directories;
	std::vector<std::string> header_files;
	std::string generated_file_path;
	std::string introspector_field_format;
	std::string base_introspector_field_format;
	std::string introspector_body_format;
	std::string enum_field_format;
	std::string enum_introspector_body_format;
	std::string enum_arg_format;
	std::string enum_to_args_body_format;
	std::string generated_file_format;

	{
		const auto cfg = get_file_lines(configuration_file_input_path);
		
		try {
			if (cfg.size() == 0) {
				throw std::exception();
			}

			const auto lines_per_prop = break_lines_by_properties(
				cfg,
				{
					"beginning-line:",
					"ending-line:",
					"introspect-base-line:",
					"header-directories:",
					"header-files:",
					"generated-file-path:",
					"introspector-field-format:",
					"base-introspector-field-format:",
					"introspector-body-format:",
					"enum-field-format:",
					"enum-introspector-body-format:",
					"enum-arg-format:",
					"enum-to-args-body-format:",
					"generated-file-format:"
				}
			);

			std::size_t i = 0u;

			beginning_line = lines_per_prop[i++][0];
			ending_line = lines_per_prop[i++][0];
			introspect_base_line = lines_per_prop[i++][0];
			header_directories = lines_per_prop[i++];
			header_files = lines_per_prop[i++];
			generated_file_path = lines_per_prop[i++][0];
			introspector_field_format = lines_to_string(lines_per_prop[i++]);
			base_introspector_field_format = lines_to_string(lines_per_prop[i++]);
			introspector_body_format = lines_to_string(lines_per_prop[i++]);
			enum_field_format = lines_to_string(lines_per_prop[i++]);
			enum_introspector_body_format = lines_to_string(lines_per_prop[i++]);
			enum_arg_format = lines_to_string(lines_per_prop[i++]);
			enum_to_args_body_format = lines_to_string(lines_per_prop[i++]);
			generated_file_format = lines_to_string(lines_per_prop[i++]);
		}
		catch (...) {
			std::cout << "Failure\nError while reading configuration values." << std::endl;
			return 1;
		}
	}


	std::vector<std::string> generated_files_for_inclusion;

	std::string generated_introspectors;
	std::string generated_enums;

	std::map<std::string, std::string> namespaces;

	const auto make_namespaces = [&]() {
		std::string all;

		for (const auto& n : namespaces) {
			if (n.first == "<unnamed>") {
				all += typesafe_sprintf("%x\n", n.second);
			}
			else {
				all += typesafe_sprintf("namespace %x {\n%x}\n\n", n.first, n.second);
			}
		}

		return all;
	};

	for (const auto dirpath : header_directories) {
		for (fs::recursive_directory_iterator i(dirpath), end; i != end; ++i) {
			if (!is_directory(i->path())) {
				const auto path = i->path();

				if (path.extension() == ".h") {
					header_files.push_back(path.string());
				}
			}
		}
	}

	try {
		for(const auto& path : header_files) {
			const auto lines = get_file_lines(path);

			size_t current_line = 0;

			const auto errcheck = [&](const bool flag) {
				if (!flag) {
					const auto error_contents = typesafe_sprintf(
						"A problem in line %x in file %x:\n%x\n",
						current_line,
						path,
						lines[current_line]
					);

					create_text_file(generated_file_path, "#error " + error_contents);

					std::cout << "------------\nIntrospector-generator run failed." << std::endl;
					std::cout << error_contents << std::endl;
					std::cout << "------------\n";

					throw std::exception();
				}
			};

			while (current_line < lines.size()) {
				const auto found_gen_begin = lines[current_line].find(beginning_line);

				if (found_gen_begin != std::string::npos) {
					const auto after_gen = lines[current_line].substr(found_gen_begin + beginning_line.length());
					std::istringstream in(after_gen);

					std::string struct_or_class_or_enum;
					in >> struct_or_class_or_enum;

					const bool is_enum =
						struct_or_class_or_enum == "enum"
						;

					errcheck(
						struct_or_class_or_enum == "struct"
						|| struct_or_class_or_enum == "class"
						|| is_enum
					);

					std::string type_name;

					std::string argument_template_arguments;
					std::string template_template_arguments;

					std::string type_name_without_templates;
					
					if (is_enum) {
						in >> type_name_without_templates;

						if (type_name_without_templates == "class") {
							struct_or_class_or_enum = "enum class";
							in >> type_name_without_templates;
						}
					}
					else {
						in >> type_name_without_templates;
					}

					type_name = type_name_without_templates;

					std::vector <std::pair<std::string, std::string>> template_arguments;

					std::string template_arg_type;
					std::string template_arg_name;

					while (in >> template_arg_type && in >> template_arg_name) {
						template_arguments.push_back({ template_arg_type, template_arg_name });
					}

					if (template_arguments.size() > 0) {
						argument_template_arguments = "<";
						template_template_arguments = ", ";

						for (size_t a = 0; a < template_arguments.size(); ++a) {
							argument_template_arguments += template_arguments[a].second;
							
							if (template_arguments[a].first.find("...") != std::string::npos) {
								argument_template_arguments += "...";
							}

							template_template_arguments += template_arguments[a].first + " " + template_arguments[a].second;

							if (a != template_arguments.size() - 1) {
								argument_template_arguments += ", ";
								template_template_arguments += ", ";
							}
						}

						argument_template_arguments += ">";

						type_name += argument_template_arguments;
					}

					std::string type_without_namespace = type_name_without_templates;
					std::string namespace_of_type = "<unnamed>";

					const auto found_colons = type_name_without_templates.find("::");

					if (found_colons != std::string::npos) {
						const auto& name = type_name_without_templates;

						namespace_of_type = name.substr(0, found_colons);
						type_without_namespace = name.substr(found_colons + 2);
					}

					std::vector<std::string> forward_declaration_lines;

					if (template_arguments.size()) {
						forward_declaration_lines.push_back(
							typesafe_sprintf("template %x\n", "<" + template_template_arguments.substr(2) + ">")
						);
					}
					
					forward_declaration_lines.push_back(
						typesafe_sprintf(
							"%x %x;\n",
							struct_or_class_or_enum,
							type_without_namespace
						)
					);

					const bool should_add_tabulation = namespace_of_type != "<unnamed>";

					if (should_add_tabulation) {
						for (auto& l : forward_declaration_lines) {
							l = "	" + l;
						}
					}

					for (const auto& l : forward_declaration_lines) {
						namespaces[namespace_of_type] += l;
					}

					std::string generated_fields;
					std::string generated_enum_args;

					auto redirect_line = [&](const std::string& line) {
						generated_fields += line + '\n';
						generated_enum_args += line + '\n';
					};

					while (true) {
						++current_line;

						const auto& new_field_line = lines[current_line];

						if (new_field_line.find(ending_line) != std::string::npos) {
							break;
						}
						
						if (new_field_line[0] == '#') {
							redirect_line(new_field_line);
							continue;
						}

						if (std::all_of(new_field_line.begin(), new_field_line.end(), isspace)) {
							redirect_line(new_field_line);
							continue;
						}

						if (is_enum) {
							const auto field_name_beginning = new_field_line.find_first_not_of(" \t\r");
							errcheck(field_name_beginning != std::string::npos);

							auto field_name_ending = new_field_line.find_first_of("=, \t\r", field_name_beginning);

							if (field_name_ending == std::string::npos) {
								field_name_ending = new_field_line.size();
							}

							const auto field_name = new_field_line.substr(field_name_beginning, field_name_ending - field_name_beginning);

							generated_fields += typesafe_sprintf(
								enum_field_format,
								type_name,
								field_name,
								field_name
							);

							generated_enum_args += typesafe_sprintf(
								enum_arg_format,
								type_name,
								field_name + ","
							);
						}
						else {
							const auto found_base_gen_begin = new_field_line.find(introspect_base_line);

							if (found_base_gen_begin != std::string::npos) {
								const auto base_type_name = new_field_line.substr(1 + found_base_gen_begin + introspect_base_line.length());

								generated_fields += typesafe_sprintf(
									base_introspector_field_format,
									base_type_name
								);

								continue;
							}

							static const std::string skip_keywords[] = {
								"private:",
								"protected:",
								"public:",
								"friend ",
								"using ",
								"typedef "
							};

							bool should_skip = false;

							for (const auto& k : skip_keywords) {
								if (new_field_line.find(k) != std::string::npos) {
									should_skip = true;
								}
							}

							if (should_skip) {
								continue;
							}

							std::string field_name;
							std::string field_type;

							std::size_t field_name_beginning = std::string::npos;
							std::size_t field_name_ending = std::string::npos;
							
							const auto found_eq = new_field_line.find("=");

							if (found_eq != std::string::npos) {
								field_name_ending = new_field_line.find(" =");
								errcheck(field_name_ending != std::string::npos);
								field_name_beginning = new_field_line.rfind(" ", field_name_ending - 1) + 1;
							}
							else {
								field_name_ending = new_field_line.find(";");
								errcheck(field_name_ending != std::string::npos);
								field_name_beginning = new_field_line.rfind(" ", field_name_ending) + 1;
							}
							
							field_name = new_field_line.substr(
								field_name_beginning, field_name_ending - field_name_beginning
							);

							const auto field_type_beginning = new_field_line.find_first_not_of(" \t\r");

							field_type = new_field_line.substr(
								field_type_beginning,
								field_name_beginning - field_type_beginning - 1 // peel off the trailing space
							);

							errcheck(field_name.find_first_of("[]") == std::string::npos);

							generated_fields += typesafe_sprintf(
								introspector_field_format,
								field_name,
								field_type
							);
						}
					}

					if (is_enum) {
						generated_enums += typesafe_sprintf(
							enum_introspector_body_format,
							type_name,
							generated_fields
						);

						if (generated_enum_args.size() > 0) {
							generated_enum_args.erase(generated_enum_args.begin() + generated_enum_args.rfind(',')); // peel off the trailing comma
						}

						generated_enums += typesafe_sprintf(
							enum_to_args_body_format,
							type_name,
							generated_enum_args
						);
					}
					else {
						generated_introspectors += typesafe_sprintf(
							introspector_body_format,
							template_template_arguments,
							typesafe_sprintf("const %x* const", type_name),
							//type_name,
							generated_fields
						);
					}
				}

				++current_line;
			}
		}
	}
	catch (...) {
		return 1;
	}

	const auto generated_file = typesafe_sprintf(
		generated_file_format,
		make_namespaces(),
		generated_enums,
		generated_introspectors
	);

	guarded_create_file(
		generated_file_path,
		generated_file
	);

	std::cout << "Success\nWritten the generated introspectors to:\n" << generated_file_path << std::endl;
	std::cout << "Lines: " << std::count(generated_file.begin(), generated_file.end(), '\n') << std::endl;

	return 0;
}