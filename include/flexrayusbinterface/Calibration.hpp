#pragma once
#include <algorithm>
#include <bitset>
#include <vector>


template <typename Bitset, typename Params>
class Calibration
{
public:
  using bitset_t = Bitset;
  using parameter_group_t = std::pair<bitset_t, Params>;
  using parameters_t = std::vector<parameter_group_t>;

private:
  parameters_t parameter_groups;

public:
  void set(bitset_t target_motors, Params new_parameters)
  {
    if (target_motors.none())
      return;
    for (auto& group : parameter_groups)
      group.first &= ~target_motors;
    auto new_end = std::remove_if(std::begin(parameter_groups), std::end(parameter_groups),
                                  [](parameter_group_t const& group) { return group.first.none(); });
    parameter_groups.erase(new_end, std::end(parameter_groups));
    parameter_groups.emplace_back(target_motors, new_parameters);
  }

  auto get() const& -> parameters_t const&
  {
    return parameter_groups;
  }

  auto get() && -> parameters_t
  {
    return std::move(parameter_groups);
  }
};
