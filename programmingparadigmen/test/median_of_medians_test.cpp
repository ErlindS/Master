#include <gtest/gtest.h>
#include "median_of_medians.h"
#include <vector>
#include <string>
#include <algorithm> 
#include <random>   

#include <gtest/gtest.h>
#include "median_of_medians.h"
#include <vector>
#include <algorithm> // For std::generate, std::iota
#include <random>    // For std::mt19937, std::uniform_int_distribution




// Test for median_of_three with integers
TEST(MedianOfMediansTest, MedianOfThreeInts) {
    EXPECT_EQ(median_of_three(1, 3, 2), 2);
    EXPECT_EQ(median_of_three(3, 1, 2), 2);
    EXPECT_EQ(median_of_three(2, 1, 3), 2);
    EXPECT_EQ(median_of_three(3, 2, 1), 2);
    EXPECT_EQ(median_of_three(1, 2, 3), 2);
    EXPECT_EQ(median_of_three(2, 3, 1), 2);
}

// Test for median_of_three with doubles
TEST(MedianOfMediansTest, MedianOfThreeDoubles) {
    EXPECT_DOUBLE_EQ(median_of_three(1.5, 2.5, 2.0), 2.0);
    EXPECT_DOUBLE_EQ(median_of_three(2.5, 1.5, 2.0), 2.0);
    EXPECT_DOUBLE_EQ(median_of_three(2.0, 1.5, 2.5), 2.0);
    EXPECT_DOUBLE_EQ(median_of_three(2.5, 2.0, 1.5), 2.0);
    EXPECT_DOUBLE_EQ(median_of_three(1.5, 2.0, 2.5), 2.0);
    EXPECT_DOUBLE_EQ(median_of_three(2.0, 2.5, 1.5), 2.0);
}


// Test for nth_element_median_of_medians with integers including exception case
/*
TEST(MedianOfMediansTest, NthElementInts) {
    std::vector<int> nums = {3, 2, 1, 5, 6, 4};

    for (size_t i = 0; i < nums.size(); ++i) {
        auto nth = nums.begin() + i; // Ensure nth is within range
        std::vector<int> nums_copy = nums;
        nth_element_median_of_medians(nums.begin(), nth, nums.end());
        
        // Sort copy to check if nth is correct
        std::sort(nums_copy.begin(), nums_copy.end());
        EXPECT_EQ(*nth, nums_copy[i]); // Check if nth element is in correct position
    }

    // Test for empty vector
    std::vector<int> empty;
    EXPECT_THROW(nth_element_median_of_medians(empty.begin(), empty.begin(), empty.end()), std::runtime_error);
}
*/

// Test for nth_element_median_of_medians with doubles
TEST(MedianOfMediansTest, NthElementDoubles) {
    std::vector<double> nums = {3.1, 2.1, 1.1, 5.1, 6.1, 4.1};
    auto nth = nums.begin() + 2; // Find 3rd smallest element (0-based index)
    nth_element_median_of_medians(nums.begin(), nth, nums.end());
    EXPECT_DOUBLE_EQ(*nth, 3.1);

    std::vector<double> sortedNums = {1.1, 2.1, 3.1, 4.1, 5.1, 6.1};
    nth_element_median_of_medians(sortedNums.begin(), sortedNums.begin() + 3, sortedNums.end());
    EXPECT_DOUBLE_EQ(sortedNums[3], 4.1); // Check if 4th element is still 4.1
}
/*
TEST(MedianOfMediansTest, LargeNthElementInts) {
    const int SIZE = 150;
    std::vector<int> nums(SIZE);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, 1000); // Random numbers between 1 and 1000

    // Fill vector with random numbers
    std::generate(nums.begin(), nums.end(), [&]() { return dist(rng); });
	
    // Check 10 different nth positions
    for (int i = 0; i < 10; ++i) {
        int nth = i * (SIZE / 10); // Testing nth positions from 0 to SIZE-1 in increments
        auto nth_it = nums.begin() + nth;
        std::vector<int> nums_copy = nums; // Copy to preserve original for comparison

        nth_element_median_of_medians(nums.begin(), nth_it, nums.end());

        // Sort a copy to verify correctness
        std::sort(nums_copy.begin(), nums_copy.end());
        EXPECT_EQ(*nth_it, nums_copy[nth]);
    }
}*/