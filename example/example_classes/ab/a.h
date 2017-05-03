#pragma once
namespace augs {
	struct introspection_access;
}

class a {
	friend struct augs::introspection_access;

	struct xy {
		// WARNING: Nested class cannot be introspected.
		int x;
		int y;
	};

	// GEN INTROSPECTOR class a
	friend class cosmos;

	int field = 0xdeadbeef;
	unsigned other_field = 0;

#if COSMOS_TRACKS_GUIDS
	xy another_field = { 0, 1 };
#endif
public:
	double and_yet_another_field;
	// END GEN INTROSPECTOR
};