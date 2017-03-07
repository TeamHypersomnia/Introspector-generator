# Introspector-generator
Small utility that generates introspector functions from comments in the code

Example:

Given suchlike structures:


```cpp
		struct paint_circle_midpoint_command {
			// GEN INTROSPECTOR augs::image::paint_circle_midpoint_command
			unsigned radius;
			unsigned border_width = 1;
			bool scale_alpha = false;
			bool constrain_angle = false;
			std::array<padding_byte, 2> pad;
			float angle_start = 0.f;
			float angle_end = 0.f;
			rgba filling = white;
			// END GEN INTROSPECTOR

			static std::string get_command_name() {
				return "circle_midpoint";
			}
		};
    
		struct paint_circle_filled_command {
			// GEN INTROSPECTOR augs::image::paint_circle_filled_command
			unsigned radius;
			rgba filling = white;
			// END GEN INTROSPECTOR

			static std::string get_command_name() {
				return "circle_filled";
			}
		};    
```

Given this output file format:

```cpp
#include "game/transcendental/types_specification/all_components_declaration.h"
#include "augs/image/image.h"

#define NVP(x) x, #x

namespace augs {
%x
}
 ```

Given this introspector body format:

```cpp
  	template <bool C, class F>
  	void introspect(
  		maybe_const_ref_t<C, %x> t,
  		F f
  	) {
      %x
  	}
```

Given this field format:

```cpp
		f(t.NVP(%x));    
```

The program will generate this file to a given path:

```cpp
#include "augs/image/image.h"

#define NVP(x) x, #x

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, augs::image::paint_circle_midpoint_command> t,
		F f
	) {
		f(t.NVP(radius));
		f(t.NVP(border_width));
		f(t.NVP(scale_alpha));
		f(t.NVP(constrain_angle));
		f(t.NVP(angle_start));
		f(t.NVP(angle_end));
		f(t.NVP(filling));

	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, augs::image::paint_circle_filled_command> t,
		F f
	) {
		f(t.NVP(radius));
		f(t.NVP(filling));

	}
}
```
