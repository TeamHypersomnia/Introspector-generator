#pragma once
#define FIELD(x) f(#x, _t_.x...)

// Forward declarations
enum class EN1;
enum EN2;
struct c;
class a;
template <class id_type>
struct base;
template <class id_type>
struct derived;

namespace myn {
	enum class EN3;
}

namespace augs {
	template <class... T>
	const char* enum_to_string(const EN1 e, T...) {
		switch(e) {
		case EN1::A: return "A";
		case EN1::B: return "B";
		case EN1::C: return "C";
		default: return "Invalid";
		}
	}

	template <class... T>
	const char* enum_to_string(const EN2 e, T...) {
		switch(e) {
		case EN2::DASD: return "DASD";
		case EN2::f34879078: return "f34879078";
		case EN2::EF: return "EF";
		default: return "Invalid";
		}
	}

	template <class... T>
	const char* enum_to_string(const myn::EN3 e, T...) {
		switch(e) {
		case myn::EN3::asdad: return "asdad";
		case myn::EN3::fdsfsfds: return "fdsfsfds";
		case myn::EN3::EfsdfdsfsF: return "EfsdfdsfsF";
		default: return "Invalid";
		}
	}

	struct introspection_access {
		/* Generated introspectors begin here */

		template <class F, class... Instances>
		static void introspect_body(
			const c* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(super_important_field);
			FIELD(another_important_field);
			FIELD(foobar);
		}

		template <class F, class... Instances>
		static void introspect_body(
			const a* const,
			F f,
			Instances&&... _t_
		) {

			FIELD(field);
			FIELD(other_field);

#if COSMOS_TRACKS_GUIDS
			FIELD(another_field);
#endif
			FIELD(and_yet_another_field);
		}

		template <class F, class id_type, class... Instances>
		static void introspect_body(
			const base<id_type>* const,
			F f,
			Instances&&... _t_
		) {
			FIELD(base_field);
			FIELD(other_base_field);
		}

		template <class F, class id_type, class... Instances>
		static void introspect_body(
			const derived<id_type>* const,
			F f,
			Instances&&... _t_
		) {
			introspect_body(static_cast<base<id_type>*>(nullptr), f, std::forward<Instances>(_t_)...);
			FIELD(container_entity);
		}

	};
}
