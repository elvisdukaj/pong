#ifndef ENTT_CORE_ALGORITHM_HPP
#define ENTT_CORE_ALGORITHM_HPP

#include "utility.hpp"
#if not defined(ENTT_IMPORT_STD)
#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>
#include <vector>
#else
import std;
#endif

namespace vis::ecs {

/**
 * @brief Function object to wrap `std::sort` in a class type.
 *
 * Unfortunately, `std::sort` cannot be passed as template argument to a class
 * template or a function template.<br/>
 * This class fills the gap by wrapping some flavors of `std::sort` in a
 * function object.
 */
struct std_sort {
	/**
	 * @brief Sorts the elements in a range.
	 *
	 * Sorts the elements in a range using the given binary comparison function.
	 *
	 * @tparam It Type of random access iterator.
	 * @tparam Compare Type of comparison function object.
	 * @tparam Args Types of arguments to forward to the sort function.
	 * @param first An iterator to the first element of the range to sort.
	 * @param last An iterator past the last element of the range to sort.
	 * @param compare A valid comparison function object.
	 * @param args Arguments to forward to the sort function, if any.
	 */
	template <typename It, typename Compare = std::less<>, typename... Args>
	void operator()(It first, It last, Compare compare = Compare{}, Args&&... args) const {
		std::sort(std::forward<Args>(args)..., std::move(first), std::move(last), std::move(compare));
	}
};

/*! @brief Function object for performing insertion sort. */
struct insertion_sort {
	/**
	 * @brief Sorts the elements in a range.
	 *
	 * Sorts the elements in a range using the given binary comparison function.
	 *
	 * @tparam It Type of random access iterator.
	 * @tparam Compare Type of comparison function object.
	 * @param first An iterator to the first element of the range to sort.
	 * @param last An iterator past the last element of the range to sort.
	 * @param compare A valid comparison function object.
	 */
	template <typename It, typename Compare = std::less<>>
	void operator()(It first, It last, Compare compare = Compare{}) const {
		if (first < last) {
			for (auto it = first + 1; it < last; ++it) {
				auto value = std::move(*it);
				auto pre = it;

				// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				for (; pre > first && compare(value, *(pre - 1)); --pre) {
					*pre = std::move(*(pre - 1));
				}
				// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

				*pre = std::move(value);
			}
		}
	}
};

/**
 * @brief Function object for performing LSD radix sort.
 * @tparam Bit Number of bits processed per pass.
 * @tparam N Maximum number of bits to sort.
 */
template <std::size_t Bit, std::size_t N> struct radix_sort {
	static_assert((N % Bit) == 0,
								"The maximum number of bits to sort must be a multiple of the number of bits processed per pass");

	/**
	 * @brief Sorts the elements in a range.
	 *
	 * Sorts the elements in a range using the given _getter_ to access the
	 * actual data to be sorted.
	 *
	 * This implementation is inspired by the online book
	 * [Physically Based
	 * Rendering](http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html#RadixSort).
	 *
	 * @tparam It Type of random access iterator.
	 * @tparam Getter Type of _getter_ function object.
	 * @param first An iterator to the first element of the range to sort.
	 * @param last An iterator past the last element of the range to sort.
	 * @param getter A valid _getter_ function object.
	 */
	template <typename It, typename Getter = identity>
	void operator()(It first, It last, Getter getter = Getter{}) const {
		if (first < last) {
			constexpr auto passes = N / Bit;

			using value_type = typename std::iterator_traits<It>::value_type;
			using difference_type = typename std::iterator_traits<It>::difference_type;
			std::vector<value_type> aux(static_cast<std::size_t>(std::distance(first, last)));

			auto part = [getter = std::move(getter)](auto from, auto to, auto out, auto start) {
				constexpr auto mask = (1 << Bit) - 1;
				constexpr auto buckets = 1 << Bit;

				// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays, misc-const-correctness)
				std::size_t count[buckets]{};

				for (auto it = from; it != to; ++it) {
					++count[(getter(*it) >> start) & mask];
				}

				// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
				std::size_t index[buckets]{};

				for (std::size_t pos{}, end = buckets - 1u; pos < end; ++pos) {
					index[pos + 1u] = index[pos] + count[pos];
				}

				for (auto it = from; it != to; ++it) {
					const auto pos = index[(getter(*it) >> start) & mask]++;
					out[static_cast<difference_type>(pos)] = std::move(*it);
				}
			};

			for (std::size_t pass = 0; pass < (passes & ~1u); pass += 2) {
				part(first, last, aux.begin(), pass * Bit);
				part(aux.begin(), aux.end(), first, (pass + 1) * Bit);
			}

			if constexpr (passes & 1) {
				part(first, last, aux.begin(), (passes - 1) * Bit);
				std::move(aux.begin(), aux.end(), first);
			}
		}
	}
};

} // namespace vis::ecs

#endif
