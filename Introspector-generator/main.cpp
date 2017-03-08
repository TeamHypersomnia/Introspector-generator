#pragma once
#include <iostream>

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

	for (const auto dirpath : dirs) {
		for (fs::recursive_directory_iterator i(dirpath), end; i != end; ++i) {
			if (!is_directory(i->path())) {
				const auto path = i->path();
				
				if (path.extension() == ".h") {
					const auto lines = get_file_lines(path.string());

					size_t current_line = 0;

					while (current_line < lines.size()) {
						const auto found_gen_begin = lines[current_line].find(beginning_sequence);

						if (found_gen_begin != std::string::npos) {
							const auto after_gen = lines[current_line].substr(found_gen_begin + beginning_sequence.length());
							std::istringstream in(after_gen);

							std::string type_name;

							std::string argument_template_arguments;
							std::string template_template_arguments;

							in >> type_name;

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

							std::string generated_fields;

							while (true) {
								++current_line;

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

								const auto& new_field_line = lines[current_line];

								if (new_field_line.find(ending_sequence) != std::string::npos) {
									break;
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
								const auto valid_type_name = replace_all(type_name, "::", "_");
								const auto filename = "introspect_" + valid_type_name + ".h";
								const auto final_path = output_path.string() + filename;
								
								generated_files_for_inclusion.push_back(filename);

								create_text_file(
									final_path,
									typesafe_sprintf(introspect_file_format, generated_introspectors)
								);

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
			typesafe_sprintf(introspect_file_format, generated_introspectors)
		);
	}
}