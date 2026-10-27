#include "fovea_roi.hpp"
#include <cassert>
#include <iostream>

int main() {
  using eagleeye::PersonBox;
  eagleeye::GridPolicy grid;
  const std::vector<PersonBox> left = {
      {0.90f, 20.f, 20.f, 20.f, 40.f},
      {0.80f, 70.f, 20.f, 20.f, 40.f},
      {0.75f, 21.f, 20.f, 20.f, 40.f},  // duplicate, not a third person
      {0.95f, 0.f, 0.f, 350.f, 350.f}, // huge false positive, ignored
  };
  auto state = grid.Update(left);
  assert(state.valid && state.switched && state.row == 0 && state.col == 0);
  assert(state.players == 2 && state.accepted_players == 2);
  assert(state.roi.x == 0 && state.roi.y == 0);

  const std::vector<PersonBox> right = {
      {0.91f, 550.f, 270.f, 20.f, 45.f},
      {0.85f, 590.f, 260.f, 20.f, 45.f},
      {0.82f, 610.f, 290.f, 20.f, 40.f},
  };
  for (int i = 0; i < 3; ++i) {
    state = grid.Update(right);
    assert(state.valid && !state.switched && state.row == 0 && state.col == 0);
  }
  state = grid.Update(right);
  assert(state.valid && state.switched && state.row == 2 && state.col == 2);
  assert(state.players == 3 && state.roi.x == 320 && state.roi.y == 180);
  const auto held = grid.Current();
  state = grid.Update({});
  assert(!state.valid && grid.Current().x == held.x && grid.Current().y == held.y);

  eagleeye::GridPolicy advantage;
  advantage.Update(left);
  std::vector<PersonBox> one_more = left;
  one_more.insert(one_more.end(), right.begin(), right.end());
  for (int i = 0; i < 5; ++i) {
    state = advantage.Update(one_more);
    assert(state.col == 0); // 3 vs 2 is not the required +2 margin.
  }
  one_more.push_back({0.78f, 630.f, 320.f, 15.f, 30.f});
  state = advantage.Update(one_more);
  assert(state.switched && state.col == 2 && state.players == 4);

  eagleeye::RoiTrajectory motion;
  motion.SetTarget({320, 180});
  assert(motion.Advance() && motion.Current().x == 172 && motion.Current().y == 102);
  for (int i = 0; i < 12; ++i) assert(motion.Advance());
  assert(motion.Current().x == 316 && motion.Current().y == 180);
  motion.SetTarget({0, 0});
  assert(motion.Advance() && motion.Current().x == 304 && motion.Current().y == 168);
  for (int i = 0; i < 30; ++i) motion.Advance();
  assert(motion.Current().x == 0 && motion.Current().y == 0);
  assert(!motion.Advance());

  eagleeye::GridPolicy tie;
  state = tie.Update({{0.9f, 250.f, 160.f, 20.f, 40.f},
                      {0.9f, 300.f, 160.f, 20.f, 40.f}});
  assert(state.row == 1 && state.col == 1 && state.players == 2);
  std::cout << "PASS 3x3 selection and 20Hz ROI trajectory\n";
}
