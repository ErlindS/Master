/*
 Minimal helpers for the SYCL Academy exercises.
*/
#pragma once

#include <cstdlib>
#include <iostream>

#define SYCLACADEMY_ASSERT(cond)                                              \
  do {                                                                        \
    if (!(cond)) {                                                            \
      std::cerr << __FILE__ << ":" << __LINE__                                \
                << ": assertion failed: " #cond << std::endl;                 \
      std::exit(EXIT_FAILURE);                                                \
    }                                                                         \
  } while (0)

#define SYCLACADEMY_ASSERT_EQUAL(a, b) SYCLACADEMY_ASSERT((a) == (b))
