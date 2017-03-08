#pragma once
#include <iostream>
#include <map>

#include "spellbook.h"

int main() {
	const auto dirs = get_file_lines("directories.txt");

	const auto beginning_sequence = get_file_contents("beginning_sequence.txt");
	const auto ending_sequence = get_file_contents("ending_sequence.txt");

	const auto introspector_body_format = get_file_contents("introspector_body_format.txt");
	const auto introspector_field_format = get_file_contents("introspector_field_format.txt");

	const auto introspect_file_format_path = get_file_contents("introspect_file_format_path.txt");
	const auto introspect_file_format = get_file_contents(introspect_file_format_path);
	
	const auto output_path = fs::path(get_file_contents("output_path.txt"));

	bool one_introspector_per_type = false;

	if (output_path.filename() == ".") {
		fs::create_directories(output_path);
		one_introspector_per_type = true;
	}

	std::vector<std::string> generated_files_for_inclusion;

	std::string generated_introspectors;

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

	for (const auto dirpath : dirs) {
		for (fs::recursive_directory_iterator i(dirpath), end; i != end; ++i) {
			if (!is_directory(i->path())) {
				const auto path = i->path();
				
				if (path.extension() == ".h") {
					const auto lines = get_file_lines(path.string());

					size_t current_line = 0;

					const auto errcheck = [&](const bool flag) {
						if (!flag) {
							std::cout <<
								typesafe_sprintf(
									"A problem in line %x in file %x:\n%x\n",
									current_line,
									path.string(),
									lines[current_line]
								)
							;

							std::getchar();
						}
					};

					while (current_line < lines.size()) {
						const auto found_gen_begin = lines[current_line].find(beginning_sequence);

						if (found_gen_begin != std::string::npos) {
							const auto after_gen = lines[current_line].substr(found_gen_begin + beginning_sequence.length());
							std::istringstream in(after_gen);

							std::string struct_or_class;
							in >> struct_or_class;

							errcheck(struct_or_class == "struct" || struct_or_class == "class");

							std::string type_name;

							std::string argument_template_arguments;
							std::string template_template_arguments;
							
							std::string type_name_without_templates;
							in >> type_name_without_templates;

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
									struct_or_class,
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

							while (true) {
								++current_line;

								const auto& new_field_line = lines[current_line];

								if (new_field_line.find(ending_sequence) != std::string::npos) {
									break;
								}
								
								if (new_field_line[0] == '#') {
									generated_fields += new_field_line + "\n";
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

								if (std::all_of(new_field_line.begin(), new_field_line.end(), isspace)) {
									generated_fields += new_field_line + "\n";
									continue;
								}

								const auto found_eq = new_field_line.find("=");
								std::string field_name;

								if (found_eq != std::string::npos) {
									const auto found_s = new_field_line.find(" =");

									errcheck(found_s != std::string::npos);

									const auto field_name_beginning = new_field_line.rfind(" ", found_s - 1) + 1;

									field_name = new_field_line.substr(
										field_name_beginning, found_s - field_name_beginning
									);
								}
								else {
									const auto found_s = new_field_line.find(";");

									errcheck(found_s != std::string::npos);

									const auto field_name_beginning = new_field_line.rfind(" ", found_s) + 1;

									field_name = new_field_line.substr(
										field_name_beginning, found_s- field_name_beginning
									);
								}

								errcheck(field_name.find_first_of("[]") == std::string::npos);

								if (field_name == "pad") {
									continue;
								}
								
								generated_fields += typesafe_sprintf(introspector_field_format, field_name);
							}

							generated_introspectors += typesafe_sprintf(
								introspector_body_format,
								template_template_arguments,
								type_name,
								generated_fields
							);

							if (one_introspector_per_type) {
								const auto valid_type_name = replace_all(type_name_without_templates, "::", "_");
								const auto filename = "introspect_" + valid_type_name + ".h";
								const auto final_path = output_path.string() + filename;
								
								generated_files_for_inclusion.push_back(filename);

								create_text_file(
									final_path,
									typesafe_sprintf(
										introspect_file_format, 
										make_namespaces(),
										generated_introspectors
									)
								);

								namespaces.clear();
								generated_introspectors.clear();
							}
						}

						++current_line;
					}
				}
			}
		}
	}

	if (one_introspector_per_type) {
		std::string include_all_file_contents;

		for (const auto& l : generated_files_for_inclusion) {
			include_all_file_contents += typesafe_sprintf("#include \"%x\"\n", l);
		}

		create_text_file(
			output_path.string() + "include_all_introspectors.h",
			include_all_file_contents
		);
	}
	else {
		create_text_file(
			output_path.string(),
			typesafe_sprintf(
				introspect_file_format, 
				make_namespaces(),
				generated_introspectors
			)
		);
	}
}