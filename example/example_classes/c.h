#pragma once

enum class EN1 {
	// GEN INTROSPECTOR enum class EN1
	A = 576 + 1,
	B,
	C = 42,
	COUNT
	// END GEN INTROSPECTOR
};

enum EN2 {
	// GEN INTROSPECTOR enum EN2
	DASD = 576 + 8,
	f34879078,
	EF = f34879078 + DASD,
	COUNT
	// END GEN INTROSPECTOR
};

namespace myn {
	enum class EN3 {
		// GEN INTROSPECTOR enum class myn::EN3
		asdad = 1,
		fdsfsfds,
		EfsdfdsfsF,
		COUNT
		// END GEN INTROSPECTOR
	};
}

struct c {
	// GEN INTROSPECTOR struct c
	int super_important_field;
	int another_important_field;
	int foobar;
	// END GEN INTROSPECTOR
};