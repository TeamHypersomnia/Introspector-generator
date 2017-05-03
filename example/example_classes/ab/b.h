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