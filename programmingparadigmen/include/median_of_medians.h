#ifndef MEDIAN_OF_MEDIANS_H
#define MEDIAN_OF_MEDIANS_H

#include <vector>
#include <algorithm>
#include <stdexcept> // For std::runtime_error
#include <cassert>   // For assert

// Helper function to check if a vector is partitioned around a pivot
template<typename T>
bool is_partitioned(const std::vector<T>& vec, const T& pivot) {
    auto it = std::find(vec.begin(), vec.end(), pivot);
    if (it == vec.end()) return false; // pivot not found

    auto left = vec.begin();
    auto right = vec.end();
    while (left != it && right != it) {
        if (*left > pivot || *std::prev(right) < pivot) return false;
        ++left; --right;
    }
    return true;
}
/**
 * @brief Calculates the median of three values.
 * 
 * @tparam T The type of the elements to compare
 * @param a First element
 * @param b Second element
 * @param c Third element
 * @return T The median of the three elements
 */
template <typename T>
inline T median_of_three(const T& a, const T& b, const T& c) {
    return std::max(std::min(a, b), std::min(std::max(a, b), c));
}

// Three-way partitioning based on Dijkstra's algorithm with a comparator
template <typename RandomIt, typename Compare>
inline RandomIt partition(RandomIt first, RandomIt last, RandomIt pivot, Compare comp) {
    assert(first <= last);
    assert(pivot >= first && pivot < last);

    auto pivot_value = *pivot;
    RandomIt i = first, j = last - 1;

    // Move pivot to the end to simplify the algorithm
    std::iter_swap(pivot, j);

    RandomIt p = first;  // Pointer for elements equal to pivot
    RandomIt q = first;  // Pointer for elements less than pivot

    while (p <= j) {
        if (comp(*p, pivot_value)) {
            std::iter_swap(p++, q++);
        } else if (comp(pivot_value, *p)) {
            std::iter_swap(p, j--);
        } else {
            ++p;  // If equal, just move p forward
        }

        // Additional checks for debugging
        assert(p <= last);
        assert(q <= p);
        assert(j >= first);
    }

    // Move pivot back to its final position
    std::iter_swap(j, q);

    return q; // Return the position where the partition ended
}

// nth_element_median_of_medians with comparator
template <typename RandomIt, typename Compare>
inline void nth_element_median_of_medians(RandomIt first, RandomIt nth, RandomIt last, Compare comp) {
    assert(first <= last);
    assert(nth >= first && nth < last);

    auto distance = std::distance(first, last);
    if (distance <= 5) {
        std::sort(first, last, comp);
        return;
    }

    auto pivot = first + (distance / 2);
    auto partition_point = partition(first, last, pivot, comp);
    
    auto left_size = std::distance(first, partition_point);
    auto right_size = std::distance(partition_point, last);
    
    if (std::max(left_size, right_size) <= 3 * std::min(left_size, right_size)) {
        if (nth < partition_point) {
            nth_element_median_of_medians(first, nth, partition_point, comp);
        } else if (nth > partition_point) {
            nth_element_median_of_medians(std::next(partition_point), nth, last, comp);
        } 
        return;
    }

    auto median_start = first + (distance / 10); // Roughly 1/10th of the way from the start

    for (auto it = first; it < last; it = std::next(it, 5)) {
        auto group_end = std::min(std::next(it, 5), last);
        std::sort(it, group_end, comp);
        auto median_index = std::distance(it, group_end) / 2;
        
        assert(median_index < std::distance(it, group_end)); // Check if median index is valid

        if (median_start < last) {
            std::iter_swap(it + median_index, median_start);
            ++median_start;
        }
    }

    // Use the median of medians as pivot for quickselect
    pivot = first + (distance / 2);
    nth_element_median_of_medians(first, pivot, last, comp);

    partition_point = partition(first, last, pivot, comp);

    assert(partition_point >= first && partition_point <= last); // Check if partition point is valid

    if (nth < partition_point) {
        nth_element_median_of_medians(first, nth, partition_point, comp);
    } else if (nth > partition_point) {
        nth_element_median_of_medians(std::next(partition_point), nth, last, comp);
    }
}

// Overload for when no comparator is specified (uses default comparison)
template <typename RandomIt>
inline void nth_element_median_of_medians(RandomIt first, RandomIt nth, RandomIt last) {
    using T = typename std::iterator_traits<RandomIt>::value_type;
    nth_element_median_of_medians(first, nth, last, std::less<T>());
}
/*
// Three-way partitioning based on Dijkstra's algorithm
template <typename RandomIt>
inline RandomIt partition(RandomIt first, RandomIt last, RandomIt pivot) {
    assert(first <= last);
    assert(pivot >= first && pivot < last);

    auto pivot_value = *pivot;
    RandomIt i = first, j = last - 1;

    // Move pivot to the end to simplify the algorithm
    std::iter_swap(pivot, j);

    RandomIt p = first;  // Pointer for elements equal to pivot
    RandomIt q = first;  // Pointer for elements less than pivot

    while (p <= j) {
        if (*p < pivot_value) {
            std::iter_swap(p++, q++);
        } else if (*p > pivot_value) {
            std::iter_swap(p, j--);
        } else {
            ++p;  // If equal, just move p forward
        }

        // Additional checks for debugging
        assert(p <= last);
        assert(q <= p);
        assert(j >= first);
    }

    // Move pivot back to its final position
    std::iter_swap(j, q);

    return q; // Return the position where the partition ended
}*/
/**
 * @brief Finds the nth element in a sorted sequence using the median-of-medians method.
 * 
 * @tparam RandomIt Random access iterator type for the range of elements
 * @param first Iterator to the first element in the range
 * @param nth Iterator pointing to where the nth element should be placed
 * @param last Iterator pointing one past the last element of the range
 * @throws std::runtime_error if the range [first, last) is empty
 */
 /*
template <typename RandomIt>
inline void nth_element_median_of_medians(RandomIt first, RandomIt nth, RandomIt last) {
    // Ensure first is not after last
    assert(first <= last);

    if (first == last) {
        throw std::runtime_error("Cannot process an empty range");
    }

    // Ensure nth is within range
    assert(nth >= first && nth < last);

    auto distance = std::distance(first, last);
    if (distance <= 5) {
        std::sort(first, last);
        return;
    }
    
    // Optimization: Check if we can skip median-of-medians
    auto pivot = first + (distance / 2);
    auto partition_point = partition(first, last, pivot);

    auto left_size = std::distance(first, partition_point);
    auto right_size = std::distance(partition_point, last);
    
    if (std::max(left_size, right_size) <= 3 * std::min(left_size, right_size)) {
        // The split is balanced enough, continue with quickselect directly
        if (nth < partition_point) {
            nth_element_median_of_medians(first, nth, partition_point);
        } else if (nth > partition_point) {
            nth_element_median_of_medians(std::next(partition_point), nth, last);
        } // else, nth is at the correct position, so we do nothing
        return;
    }

    // If not balanced, proceed with median-of-medians
    auto median_start = first + (distance / 10); // Roughly 1/10th of the way from the start

    for (auto it = first; it < last; it = std::next(it, 5)) {
        auto group_end = std::min(std::next(it, 5), last);
        std::sort(it, group_end);
        auto median_index = std::distance(it, group_end) / 2;
        assert(median_index < std::distance(it, group_end)); // Check if median index is valid

        if (median_start < last) {
            std::iter_swap(it + median_index, median_start);
            ++median_start;
        }
    }

    // Use the median of medians as pivot for quickselect
    pivot = first + (distance / 2);
    nth_element_median_of_medians(first, pivot, last);

    // Partition again after finding median of medians
    partition_point = partition(first, last, pivot);

    if (nth < partition_point) {
        nth_element_median_of_medians(first, nth, partition_point);
    } else if (nth > partition_point) {
        nth_element_median_of_medians(std::next(partition_point), nth, last);
    } // else, nth is at the correct position, so we do nothing
}
*/
#endif // MEDIAN_OF_MEDIANS_H