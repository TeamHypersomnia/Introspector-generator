#include <tuple>

template <class id_type>
struct temp {
	// GEN INTROSPECTOR struct temp class id_type
	int temp_field;
	int other_temp_field;
	// END GEN INTROSPECTOR
};

template <class T, class... Types>
struct temp_many {
	// GEN INTROSPECTOR struct temp_many class T class... Types
	T temp_field;
	std::tuple<Types...> other_temp_field;
	// END GEN INTROSPECTOR
};