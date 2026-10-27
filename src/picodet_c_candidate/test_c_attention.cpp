#include "picodet_c_attention.hpp"
#include <cassert>
#include <cmath>
#include <vector>
int main() {
  eagleeye::PicodetCAttention policy;
  const eagleeye::PersonBox person{0.9f, 100.f, 80.f, 40.f, 80.f};
  auto first = policy.Update(1, {person});
  assert(first.valid && first.target_id != 0 && first.roi.x <= 320 && first.roi.y <= 180);
  auto second = policy.Update(2, {person});
  assert(second.valid && second.target_id == first.target_id);
  assert(!policy.Update(3, {}).valid);
  auto recovered = policy.Update(4, {person});
  assert(recovered.valid && recovered.target_id != first.target_id);
  auto invalid = person; invalid.x = NAN;
  assert(!policy.Update(5, {invalid}).valid);
  assert(!policy.Update(6, std::vector<eagleeye::PersonBox>(17, person)).valid);
  auto last = policy.Update(7, {person});
  assert(last.valid && last.target_id != recovered.target_id);
}