// sycl_triad.cpp – speichergebundener Vergleichskernel (STREAM-Triade)
//
//   ./sycl_triad_dpcpp triad <n> [wiederholungen]      n = Anzahl Elemente
//
// a[i] = b[i] + s * c[i]: pro Element zwei Gleitkommaoperationen bei
// 12 Byte Speicherverkehr. Die Leistung ist damit durch die
// Speicherbandbreite begrenzt, nicht durch die Rechenleistung, und ergänzt
// die rechenintensive Matrixmultiplikation (Repräsentativität).

#include <sycl/sycl.hpp>

#include <exception>

#include "common.hpp"

#ifndef IMPL_NAME
#define IMPL_NAME "sycl"
#endif

namespace {

double event_ms(const sycl::event& e) {
  const auto s = e.get_profiling_info<sycl::info::event_profiling::command_start>();
  const auto t = e.get_profiling_info<sycl::info::event_profiling::command_end>();
  return double(t - s) * 1e-6;
}

}  // namespace

int main(int argc, char** argv) {
  const auto args = va::parse_args(argc, argv, "triad");
  const std::size_t n = args.n;
  const std::size_t bytes = n * sizeof(float);
  const float s = va::kTriadScalar;

  auto async_handler = [](sycl::exception_list l) {
    for (auto& e : l) std::rethrow_exception(e);
  };
  sycl::queue q{sycl::default_selector_v, async_handler,
                sycl::property_list{sycl::property::queue::enable_profiling{}}};
  const sycl::device dev = q.get_device();
  const va::Kontext k{IMPL_NAME,
                      dev.is_gpu() ? "gpu" : (dev.is_cpu() ? "cpu" : "andere"),
                      dev.get_info<sycl::info::device::name>(),
                      "triad",
                      "triad",
                      n};

  std::vector<float> ha(n), hb(n), hc(n);
  va::fill_random(hb.data(), n, 3);
  va::fill_random(hc.data(), n, 4);

  float* a = sycl::malloc_device<float>(n, q);
  float* b = sycl::malloc_device<float>(n, q);
  float* c = sycl::malloc_device<float>(n, q);
  q.memcpy(b, hb.data(), bytes);
  q.memcpy(c, hc.data(), bytes);
  q.wait_and_throw();

  auto triad = [&] {
    return q.parallel_for(sycl::range<1>{n},
                          [=](sycl::id<1> i) { a[i] = b[i] + s * c[i]; });
  };

  va::Timer t0;
  triad().wait_and_throw();
  const double first = t0.ms();
  va::zeile(k, 0, first, va::triad_gbs(n, first), "GB/s", "erster_lauf");

  for (int r = 1; r <= args.reps; ++r) {
    auto e = triad();
    e.wait_and_throw();
    const double ms = event_ms(e);
    va::zeile(k, r, ms, va::triad_gbs(n, ms), "GB/s", "ok");
  }

  q.memcpy(ha.data(), a, bytes).wait_and_throw();
  const bool ok = va::verify_triad(ha.data(), hb.data(), hc.data(), s, n);
  if (!ok) va::zeile(k, 0, NAN, 0.0, "GB/s", "falsch");

  sycl::free(a, q);
  sycl::free(b, q);
  sycl::free(c, q);
  return ok ? 0 : 1;
}
