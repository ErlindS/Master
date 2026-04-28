#include "shading_algorithms.cpp"


// Explicit instantiations to avoid linker errors when used in other translation units
template class ShadingAlgorithms<float, 3>;
template class Color<float>;