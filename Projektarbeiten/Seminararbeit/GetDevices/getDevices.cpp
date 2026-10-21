#include "common.h"

using namespace sycl;

int main() {

    queue q;

    std::cout << "Selected device: "
              << q.get_device().get_info<info::device::name>()
              << "\n";

    return 0;
}