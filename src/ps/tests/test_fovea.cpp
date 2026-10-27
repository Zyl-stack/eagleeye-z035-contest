#include "fovea_roi.hpp"
#include <cassert>
#include <iostream>
#include <vector>

int main() {
  using eagleeye::PersonBox;
  eagleeye::AttentionPolicy policy;
  const PersonBox large{0.95f, 300.f, 140.f, 100.f, 180.f};
  const PersonBox small{0.55f, 560.f, 80.f, 20.f, 40.f};
  const std::vector<PersonBox> people{large, small,
      {0.54f, 561.f, 81.f, 20.f, 40.f}};  // Duplicate, not another candidate.

  auto state = policy.Update(people);
  assert(state.valid && state.switched && state.accepted_players == 2);
  assert(state.score == 1125 && state.zone == 90 && state.small == 255);
  assert(state.roi.x == 320 && state.roi.y == 10);
  const auto selected_id = state.target_id;
  for (int i = 0; i < 4; ++i) {
    state = policy.Update(people);
    assert(state.valid && !state.switched && state.target_id == selected_id);
  }

  state = policy.Update({large});
  assert(state.valid && state.switched && state.target_id != selected_id);
  assert(state.roi.x == 190 && state.roi.y == 140);
  state = policy.Update({});
  assert(!state.valid && policy.Current().x == 190 && policy.Current().y == 140);

  eagleeye::AttentionPolicy moving;
  state = moving.Update({{0.80f, 520.f, 120.f, 20.f, 40.f}});
  const auto motion_id = state.target_id;
  state = moving.Update({{0.80f, 480.f, 120.f, 20.f, 40.f}});
  assert(state.valid && !state.switched && state.target_id == motion_id);
  assert(state.motion > 0 && state.approach > 0);

  eagleeye::AttentionPolicy edge;
  state = edge.Update({{0.90f, 620.f, 300.f, 18.f, 40.f}});
  assert(state.valid && state.roi.x == 320 && state.roi.y == 180);

  eagleeye::RoiTrajectory trajectory;
  trajectory.SetTarget({320, 180});
  assert(trajectory.Advance() && trajectory.Current().x == 172 && trajectory.Current().y == 102);
  for (int i = 0; i < 20; ++i) trajectory.Advance();
  assert(trajectory.Current().x == 320 && trajectory.Current().y == 180);
  std::cout << "PASS attention selection, ROI bounds and smooth trajectory\n";
}
