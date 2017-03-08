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

void debugbreak() {
	std::getchar();
	exit(0);
}

template <class T>
void assign_file_contents(const std::string& filename, T& target) {
	if (!fs::exists(filename)) {
		std::cout << typesafe_sprintf("File %x does not exist!", filename);
		debugbreak();
	}

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