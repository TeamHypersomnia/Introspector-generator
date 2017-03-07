# Introspector-generator
Small utility that generates introspector functions into a separate file, from comments in the code.

Why choose this approach?

* You don't have to use macro hacks like ```(int) field, (double) other``` so when you suddenly realize that you don't need introspection in your code, you are left  with a handful of comments instead of some crazy syntax. Don't even get me started about the abominations that you need to include every time in order to even get it working.
* The aforementioned macros require boost, don't they? So that's one dependency less.
* I don't put the introspectors inside the class definitions in order to:
	1. not have to duplicate the code for const/non-const variations
	2. not involve the introspection code where it is clearly not needed - that should speed up build times. The only compilation units which require introspection logic are mostly related to i/o, and they make up a tiny percentage of source files.

Why not choose this approach?
* If you have classes with mostly private members, well, RIP - it won't save you the typing required for befriending the introspectors, which can get quite annoying. Unless you do the introspection some other way where you befriend something like boost::access.
* If you feel bad for having your computer write code for you, well, that's a reason I guess.

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
#include "augs/image/image.h"

#define NVP(x) x, #x

namespace augs {
%x
}
 ```
where ```%x``` is the place where all resultant introspectors will be pasted, and given this introspector body format:

```cpp
  	template <bool C, class F%x>
  	void introspect(
  		maybe_const_ref_t<C, %x> t,
  		F f
  	) {
      %x
  	}
```

where ```%x``` are the places where the generator will put template arguments, type name and the generated fields respectively,
and given this field format:

```cpp
		f(t.NVP(%x));    
```
where ```%x``` is the place where the field's name will be pasted, the program will generate this exact file to a given path:

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

Notice how the algorihtm skips the fields named ```pad```.

It also works with templated types.

Example input:

```cpp
template <class id_type>
struct basic_inventory_slot_id {
	// GEN INTROSPECTOR basic_inventory_slot_id class id_type
	slot_function type;
	id_type container_entity;
	// END GEN INTROSPECTOR

	basic_inventory_slot_id();
	basic_inventory_slot_id(const slot_function, const id_type);

	void unset();

	bool operator<(const basic_inventory_slot_id b) const;
	bool operator==(const basic_inventory_slot_id b) const;
	bool operator!=(const basic_inventory_slot_id b) const;
};
```

Example output:

```cpp
template <bool C, class F, class id_type>
void introspect(
	maybe_const_ref_t<C, basic_inventory_slot_id<id_type>> t,
	F f
) {
	f(t.NVP(type));
	f(t.NVP(container_entity));

}
```

You can use more template arguments, just separate each by a space, for example

class T size_t count

for a template with suchlike arguments:
```cpp
<class T, size_t count>
```
