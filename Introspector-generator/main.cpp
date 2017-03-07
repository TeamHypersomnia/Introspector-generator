#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <limits>

template <typename CharType>
void typesafe_sprintf_detail(size_t, std::basic_string<CharType>&) {

}

template<typename CharType, typename T, typename... A>
void typesafe_sprintf_detail(size_t starting_pos, std::basic_string<CharType>& target_str, T&& val, A&& ...a) {
	starting_pos = target_str.find('%', starting_pos);

	if (starting_pos != std::string::npos) {
		std::basic_ostringstream<CharType> replacement;

		auto opcode = target_str[starting_pos + 1];

		if (opcode == L'f') {
			replacement << std::fixed;
			opcode = target_str[starting_pos + 2];
		}

		if (opcode >= L'0' && opcode <= L'9') {
			replacement.precision(opcode - L'0');
		}
		else if (opcode == L'*') {
			replacement.precision(std::numeric_limits<std::decay_t<T>>::digits10);
		}

		replacement << val;
		target_str.replace(starting_pos, 2, replacement.str());
	}

	typesafe_sprintf_detail(starting_pos, target_str, std::forward<A>(a)...);
}

template<typename CharType, typename... A>
std::basic_string<CharType> typesafe_sprintf(std::basic_string<CharType> f, A&&... a) {
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

template<typename... A>
std::string typesafe_sprintf(const char* const c_str, A&&... a) {
	auto f = std::string(c_str);
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

template<typename... A>
std::wstring typesafe_sprintf(const wchar_t* const c_str, A&&... a) {
	auto f = std::wstring(c_str);
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

#include <experimental\filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <cassert>

namespace fs = std::experimental::filesystem;

std::vector<std::string> get_file_lines(const std::string& filename) {
	std::ifstream input(filename);

	std::vector<std::string> out;

	for (std::string line; std::getline(input, line); ) {
		out.emplace_back(line);
	}

	return std::move(out);
}

template <class T>
void assign_file_contents(const std::string& filename, T& target) {
	std::ifstream t(filename);

	t.seekg(0, std::ios::end);
	target.reserve(static_cast<unsigned>(t.tellg()));
	t.seekg(0, std::ios::beg);

	target.assign((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
}

std::string get_file_contents(std::string filename) {
	std::string result;
	assign_file_contents(filename, result);
	return result;
}

template <class T>
void create_text_file(const T& filename, const T& text) {
	std::ofstream out(filename, std::ios::out);
	out << text;
}

int main() {
	// std::string exts[] = { ".h" };
	const auto dirs = get_file_lines("directories.txt");

	const std::string beginning_sequence = get_file_contents("beginning_sequence.txt");
	const std::string ending_sequence = get_file_contents("ending_sequence.txt");

	const std::string introspector_body_format = get_file_contents("introspector_body_format.txt");
	const std::string introspector_field_format = get_file_contents("introspector_field_format.txt");

	const std::string introspect_file_format_path = get_file_contents("introspect_file_format_path.txt");
	const std::string introspect_file_format = get_file_contents(introspect_file_format_path);
	const std::string output_path = get_file_contents("output_path.txt");
	
	std::string generated_introspectors;

	for (const auto dirpath : dirs) {
		for (fs::recursive_directory_iterator i(dirpath), end; i != end; ++i) {
			if (!is_directory(i->path())) {
				const auto path = i->path();
				
				if (path.extension() == ".h") {

					const auto lines = get_file_lines(path.string());

					size_t current_line = 0;

					if(path.filename() == "config_lua_table.h") {
						current_line = 0;
					}

					while (current_line < lines.size()) {
						const auto found_gen_begin = lines[current_line].find(beginning_sequence);

						if (found_gen_begin != std::string::npos) {
							std::string type_name = lines[current_line].substr(found_gen_begin + beginning_sequence.length());

							std::string generated_fields;

							while (true) {
								++current_line;

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

									assert(found_s != std::string::npos);

									const auto field_name_beginning = new_field_line.rfind(" ", found_s - 1) + 1;

									field_name = new_field_line.substr(
										field_name_beginning, found_s - field_name_beginning
									);
								}
								else {
									const auto found_s = new_field_line.find(";");

									assert(found_s != std::string::npos);

									const auto field_name_beginning = new_field_line.rfind(" ", found_s) + 1;

									field_name = new_field_line.substr(
										field_name_beginning, found_s- field_name_beginning
									);
								}

								assert(field_name.find_first_of("[]") == std::string::npos);

								if (field_name == "pad") {
									continue;
								}
								
								generated_fields += typesafe_sprintf(introspector_field_format, field_name);
							}

							generated_introspectors += typesafe_sprintf(
								introspector_body_format, 
								type_name,
								generated_fields
							);
						}

						++current_line;
					}
				}
			}
		}
	}

	create_text_file(
		output_path,
		typesafe_sprintf(introspect_file_format, generated_introspectors)
	);
}