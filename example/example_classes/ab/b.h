#include <tuple>

template <class id_type>
struct base {
	// GEN INTROSPECTOR struct base class id_type
	int base_field;
	int other_base_field;
	// END GEN INTROSPECTOR
};

template <class id_type>
struct derived : base<id_type> {
	// GEN INTROSPECTOR struct derived class id_type
	// INTROSPECT BASE base<id_type>
	id_type container_entity;
	// END GEN INTROSPECTOR
};

template <class T, class... Types>
struct base_many {
	// GEN INTROSPECTOR struct base_many class T class... Types
	T base_field;
	std::tuple<Types...> other_base_field;
	// END GEN INTROSPECTOR
};

template <class T, class... Types>
struct derived_many : base_many<T, Types...> {
	// GEN INTROSPECTOR struct derived_many class T class... Types
	// INTROSPECT BASE base_many<T, Types...>
	T t;
	// END GEN INTROSPECTOR
};