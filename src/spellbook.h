#pragma once
#include <type_traits>
#include <string>
#include <sstream>
#include <fstream>
#include <experimental/filesystem>
#include <limits>
#include <vector>

namespace augs {
	template <class F>
	class scope_guard {
	public:
		scope_guard(F&& exit_function) : 
			exit_function(std::move(exit_function))
		{}

		scope_guard(scope_guard&& f) :
			exit_function(std::move(f.exit_function)),
			execute_on_destruction(f.execute_on_destruction) 
		{
			f.release();
		}

		~scope_guard() {
			if (execute_on_destruction) {
				exit_function();
			}
		}

		void release() {
			execute_on_destruction = false;
		}

		scope_guard(const scope_guard&) = delete;
		scope_guard& operator=(const scope_guard&) = delete;
		scope_guard& operator=(scope_guard&&) = delete;

	private:
		F exit_function;
		bool execute_on_destruction = true;
	};

	template <class F>
	scope_guard<F> make_scope_guard(F&& exit_function) {
		return scope_guard<F>{std::forward<F>(exit_function)};
	}
}

template<class Str, class Repl>
auto replace_all(Str str, Repl _from, Repl _to) {
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
auto typesafe_sprintf(std::basic_string<CharType> f, A&&... a) {
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

template<typename... A>
auto typesafe_sprintf(const char* const c_str, A&&... a) {
	auto f = std::string(c_str);
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

template<typename... A>
auto typesafe_sprintf(const wchar_t* const c_str, A&&... a) {
	auto f = std::wstring(c_str);
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

namespace fs = std::experimental::filesystem;

auto get_file_lines(const std::string& filename) {
	std::ifstream input(filename);

	std::vector<std::string> out;

	for (std::string line; std::getline(input, line); ) {
		out.emplace_back(line);
	}

	return out;
}

void debugbreak() {
	std::getchar();
	exit(0);
}

std::string file_to_string(std::string path) {
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
	std::cout << f;
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

		if (!first_property_name_matches) {
			throw std::exception();
		}

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

		if (lines_per_property.size() != properties.size()) {
			throw std::exception();
		}
	}

	return lines_per_property;
}