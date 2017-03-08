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

Usage:

1. Paste ``` // GEN INTROSPECTOR [struct|class] [type] [template arg1] [template arg name1] [template arg2] [template arg name2] ...``` before the introspected members.
2. Paste ``` // END GEN INTROSPECTOR``` after all the introspected members.

Keywords for the starting and finishing comments may be modified in ```beginning_sequence.txt``` and ```ending_sequence.txt```.

The algorithm will output a message in the console when there is a problem with processing due to bad syntax or something else.

What the algorithm preserves:
* Members of exactly this format:
```cpp
type_name member1;
type_name member2 = some_initializator();
```
In particular, there must be only one space before the = sign.
* Macros (the first character of the line must be #).
* Lines with only whitespaces.

What the algorithm skips:
* Fields named ```pad``` (for my internal usage).
* Lines with friend declarations.
* Lines with using declarations.
* Lines with typedef declarations.
* Lines with public/private/protected specifiers.

What **cannot** be found between the // GEN and // END GEN comments:

* Members that occupy more than one line, multiple members per line and members separated by a comma. In particular, these:
```cpp
int
member;

int a, b, c;
int 
d,
e,
f;
```
are examples of wrong usage.
* C-style arrays. Use std::array instead.
* Functions of any kind.

Example:

Given suchlike structures:

```cpp
class cosmos_metadata {
	// GEN INTROSPECTOR class cosmos_metadata
	friend class cosmos;

	augs::delta delta;
	unsigned total_steps_passed = 0;

#if COSMOS_TRACKS_GUIDS
	entity_guid next_entity_guid = 1;
#endif
public:
	all_simulation_settings settings;

	cosmos_flyweights_state flyweights;
	// END GEN INTROSPECTOR
};

struct cosmos_significant_state {
	// GEN INTROSPECTOR struct cosmos_significant_state
	cosmos_metadata meta;

	typename cosmos_base::aggregate_pool_type pool_for_aggregates;
	typename cosmos_base::component_pools_type pools_for_components;
	// END GEN INTROSPECTOR

	bool operator==(const cosmos_significant_state&) const;
	bool operator!=(const cosmos_significant_state&) const;
}; 
```

Given this output file format:

```cpp
#pragma once
#include "augs/templates/maybe_const.h"

#define NVP(x) x, #x

%xnamespace augs {
%x}
 ```
where ```%x```  are the places where the generator will put forward declarations and resultant introspectors respectively, and given this introspector body format:
```cpp
	template <bool C, class F%x>
	void introspect(
		maybe_const_ref_t<C, %x> t,
		F f
	) {
%x	}


```
(my ```maybe_const_ref_t<C, T>``` is a shorthand for ```std::conditional_t<C, const T&, T&>```)
where ```%x``` are the places where the generator will put template arguments, type name and the generated fields respectively,
and given this field format:

```cpp
		f(t.NVP(%x));    
```
where ```%x``` is the place where the field's name will be pasted, the program will generate this exact file to a given path:

```cpp
#pragma once
#include "augs/templates/maybe_const.h"

#define NVP(x) x, #x

class cosmos_metadata;
struct cosmos_significant_state;

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, cosmos_metadata> t,
		F f
	) {

		f(t.NVP(delta));
		f(t.NVP(total_steps_passed));

#if COSMOS_TRACKS_GUIDS
		f(t.NVP(next_entity_guid));
#endif
		f(t.NVP(settings));

		f(t.NVP(flyweights));
	}

	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, cosmos_significant_state> t,
		F f
	) {
		f(t.NVP(meta));

		f(t.NVP(pool_for_aggregates));
		f(t.NVP(pools_for_components));
	}
}
```

It also works with templated types.

Example input:

```cpp
template <class id_type>
struct basic_inventory_slot_id {
	// GEN INTROSPECTOR struct basic_inventory_slot_id class id_type
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
