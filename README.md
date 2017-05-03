# Introspector-generator
Small utility that generates introspector functions into a separate file, from comments in the code.

It is currently used to generate type information for my open-source shooter [Hypersomnia][3].

Why choose this approach?

* You don't have to use macro hacks like ```(int) field, (double) other``` so when you suddenly realize that you don't need introspection in your code, you are left  with a handful of comments instead of some crazy syntax. Don't even get me started about the abominations that you need to include every time in order to even get it working.
* The aforementioned macros require boost, don't they? So that's one dependency less.
* Build based on CMake lets you easily incorporate this utility as a pre-build event for your codebase.
* I put all the generated introspectors inside a single struct, instead of putting them inside of introspected classes, in order to:
	1. not have to duplicate the code for const/non-const variations
	2. not involve the introspection code where it is clearly not needed - that should speed up build times. The only compilation units which require introspection logic are mostly related to i/o, and they make up a tiny percentage of source files.
	3. have all introspectors defined within a single struct named ```introspection_access``` that is easy to befriend in the event that some private members require introspection.

Why not choose this approach?
* If you feel bad for having your computer write code for you, well, that's a reason I guess.

# How to build
To build, you will need CMake. Create a ```build/``` folder next to ```CMakeLists.txt``` file. 
Then, use your favorite shell to go into the newly created ```build/``` folder and run:

```
cmake ..
```

If you are on Windows, resultant ```.sln``` and ```.vcxproj``` files should appear in the ```build/``` directory.
Open ```IntrospectorGenerator.sln``` file, select **Release** configuration and hit **F7** to build.

# Usage

The program takes a single command line argument, and that is the path to your input configuration file.
To see an example of a correct configuration file, open ```examples/input.cfg```.

# Example usage in your code:

1. Paste ``` // GEN INTROSPECTOR [struct|class] [type|namespace::type] [template arg1] [template arg name1] [template arg2] [template arg name2] ...``` before the introspected members.
2. Paste ``` // END GEN INTROSPECTOR``` after all the introspected members.

Keywords for the starting and finishing comments can be configured, as seen in ```examples/input.cfg```.

The algorithm will output a message in the console when there is a problem with processing due to bad syntax or something else.

What the algorithm allows between ```// GEN INTROSPECTOR``` and ```// END GEN INTROSPECTOR```:
* Members of exactly this format:
```cpp
type_name member1;
type_name member2 = some_value;
type_name member3 = some_type(some_arguments...);
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

What **cannot** be found between ```// GEN INTROSPECTOR``` and ```// END GEN INTROSPECTOR```:

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

%xstruct introspection_access {
%x};
 ```
where ```%x```  are the places where the generator will put forward declarations and resultant introspectors respectively, and given this introspector body format:
```cpp
	template <class F%x, class... MemberInstances>
	static void introspect_body(
		%x,
		F f,
		MemberInstances&&... _t_
	) {
%x	}


```
where ```%x``` are the places where the generator will put template arguments for the introspected type, dummy pointer to the type name for correct overload resolution, and the generated fields respectively,
and given this field format:

```cpp
		f("%x", _t_.%x...);
```
where ```%x``` are both the places where the field's name will be pasted, the program will generate this exact file to a given path:

```cpp
#pragma once

class cosmos_metadata;
struct cosmos_significant_state;

struct introspection_access {
	template <class F, class... MemberInstances>
	static void introspect_body(
		const cosmos_metadata* const,
		F f,
		MemberInstances&&... _t_
	) {

		f("delta", _t_.delta...);
		f("total_steps_passed", _t_.total_steps_passed...);

#if COSMOS_TRACKS_GUIDS
		f("next_entity_guid", _t_.next_entity_guid...);
#endif
		f("settings", _t_.settings...);

		f("flyweights", _t_.flyweights...);
	}

	template <class F, class... MemberInstances>
	static void introspect_body(
		const cosmos_significant_state* const,
		F f,
		MemberInstances&&... _t_
	) {
		f("meta", _t_.meta...);

		f("pool_for_aggregates", _t_.pool_for_aggregates...);
		f("pools_for_components", _t_.pools_for_components...);
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

Example generated introspector:

```cpp
template <class F, class id_type, class... MemberInstances>
static void introspect_body(
	const basic_inventory_slot_id<id_type>* const,
	F f,
	MemberInstances&&... _t_
) {
	f("type", _t_.type...);
	f("container_entity", _t_.container_entity...);
}
```

You can use more template arguments, just separate each by a space, for example

class T size_t count

for a template with suchlike arguments:
```cpp
<class T, size_t count>
```

Namespaces are respected before type names like that:
```cpp
// GEN INTROSPECTOR struct augs::image
```
They will also be correctly forward-declared, albeit only with one level of depth. Moreover, the algorithm does not recognize if an alleged namespace is actually a nested class, so this won't work with these.  

  [3]: https://github.com/TeamHypersomnia/Hypersomnia
