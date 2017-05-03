#pragma once
#define FIELD(x) f(#x, _t_.x...)

// Forward declarations

struct c;
class a;
template <class id_type>
struct base;
template <class id_type>
struct derived;

namespace augs {
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
