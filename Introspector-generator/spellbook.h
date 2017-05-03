#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <limits>
#include <fstream>
#include <cassert>
#include <experimental\filesystem>

template<class Str, class Repl>
Str replace_all(Str str, Repl _from, Repl _to) {
	const Str& from(_from);
	const Str& to(_to);

	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != Str::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

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

namespace fs = std::experimental::filesystem;

std::vector<std::string> get_file_lines(const std::string& filename) {
	std::ifstream input(filename);

	std::vector<std::string> out;

	for (std::string line; std::getline(input, line); ) {
		out.emplace_back(line);
	}

	return std::move(out);
}

std::vector<std::string> get_file_lines_without_blanks_and_comments(
	const std::string& filename,
	const char comment_begin_character = '%'
) {
	std::ifstream input(filename);

	std::vector<std::string> out;

	for (std::string line; std::getline(input, line); ) {
		const bool should_omit = 
			std::all_of(line.begin(), line.end(), isspace) 
			|| line[0] == comment_begin_character
		;

		if(!should_omit) {
			out.emplace_back(line);
		}
	}

	return std::move(out);
}

void debugbreak() {
	std::getchar();
	exit(0);
}

std::string get_file_contents(std::string path) {
	if (!fs::exists(path)) {
		std::cout << typesafe_sprintf("File %x does not exist!", path);
		debugbreak();
	}

	std::ifstream t(path);
	std::stringstream buffer;
	buffer << t.rdbuf();

	return buffer.str();
}

template <class T>
void create_text_file(const T& filename, const T& text) {
	std::ofstream out(filename, std::ios::out);
	out << text;
}

auto make_get_line_until(
	const std::vector<std::string>& lines,
	size_t& current_line
) {
	return [&lines, &current_line](const std::string delimiter = std::string()) {
		if (!(current_line < lines.size())) {
			return false;
		}
		
		if ((!delimiter.empty() && lines[current_line] == delimiter)) {
			++current_line;
			return false;
		}
		
		return true;
	};
}

template <typename... A>
void LOG(const std::string& f, A&&... a) {
	LOG(typesafe_sprintf(f, std::forward<A>(a)...));
}

template <>
void LOG(const std::string& f) {

}

#define ensure(x) if(!(x))\
{\
    LOG( "ensure(%x) failed\nfile: %x\nline: %x", #x, __FILE__, __LINE__ );\
	std::getchar(); \
}

auto lines_to_string(
	const std::vector<std::string>& lines
) {
	std::string output;

	for(const auto& l : lines) {
		output += l + '\n';
	}

	return output;
}

auto break_lines_by_properties(
	const std::vector<std::string>& lines,
	const std::vector<std::string>& properties
) {
	std::vector<std::vector<std::string>> lines_per_property;

	if (properties.size() > 0 && lines.size() > 0) {
		std::size_t i = 0;
		std::size_t p = 0;

		auto get_line_until = make_get_line_until(lines, i);

		const bool first_property_name_matches = lines[i++] == properties[p++];
		ensure(first_property_name_matches);

		for (; p < properties.size(); ++p) {
			std::vector<std::string> new_property_content;

			while (get_line_until(properties[p])) {
				new_property_content.push_back(lines[i++]);
			}

			lines_per_property.push_back(new_property_content);
		}

		{
			std::vector<std::string> new_property_content;

			while(get_line_until()) {
				new_property_content.push_back(lines[i++]);
			}

			lines_per_property.push_back(new_property_content);
		}

		ensure(lines_per_property.size() == properties.size());
	}

	return lines_per_property;
}