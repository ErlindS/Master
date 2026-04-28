#include "vector.h"
#include "vector.cpp"

namespace math {

// Explicit instantiations for float types
template Vector<float, 2> operator*<float, 2>(float, const Vector<float, 2>&);
template Vector<float, 3> operator*<float, 3>(float, const Vector<float, 3>&);
template Vector<float, 4> operator*<float, 4>(float, const Vector<float, 4>&);

template std::ostream& operator<<(std::ostream& os, const Vector<float, 2>& vec);
template std::ostream& operator<<(std::ostream& os, const Vector<float, 3>& vec);
template std::ostream& operator<<(std::ostream& os, const Vector<float, 4>& vec);
	
// Explicit instantiation for float types
template class Vector<float, 2>;
template class Vector<float, 3>;
template class Vector<float, 4>;

}