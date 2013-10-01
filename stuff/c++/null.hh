#ifndef GUARD_AWESOME_NULL_HH_
#define GUARD_AWESOME_NULL_HH_

namespace awesome {
	namespace detail {
		namespace {
			struct null {
				template<typename T> operator T *() const {
					return 0;
				}

				bool operator!() const {
					return true;
				}

				const null &operator&() const { return *this; }
				const null &operator*() const { return *this; }

				static const null instance;

				private:
				null() {}
				null(const null &) {}
				null &operator=(const null &) { return *this; }
			};
			const null null::instance;
		}
	}
	static const detail::null &null = detail::null::instance;
}

#endif /* GUARD_AWESOME_NULL_HH_ */
