#include "flexrayusbinterface/Calibration.hpp"
#include <gtest/gtest.h>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>

/*
 * Setting parameters to disjoint groups of targets will never lead to overrides.
 */
TEST(Calibration, disjoint_targets)
{
  using calibrator_t = Calibration<std::bitset<5>, int>;
  using parameter_group_t = calibrator_t::parameter_group_t;
  using parameters_t = calibrator_t::parameters_t;

  calibrator_t cal;
  cal.set(0b10000, 0);
  cal.set(0b00010, 3);
  cal.set(0b01000, 1);
  cal.set(0b00100, 2);
  auto params = cal.get();

  std::sort(std::begin(params), std::end(params),
            [](parameter_group_t const& left, parameter_group_t const& right) { return left.second < right.second; });

  parameters_t expected{ { 0b10000, 0 }, { 0b01000, 1 }, { 0b00100, 2 }, { 0b00010, 3 } };
  EXPECT_EQ(expected, params);
}

/*
 * Parameters can be overridden by subsequent calls to "set".
 */
TEST(Calibration, override_parameters)
{
  using calibrator_t = Calibration<std::bitset<5>, int>;
  using bitset_t = calibrator_t::bitset_t;
  using parameter_group_t = calibrator_t::parameter_group_t;
  using parameters_t = calibrator_t::parameters_t;

  calibrator_t cal;
  cal.set(0b10101, 0);
  cal.set(0b00111, 1);  // << overrides the parameter for the middle and last target
  auto params = cal.get();

  std::sort(std::begin(params), std::end(params),
            [](parameter_group_t const& left, parameter_group_t const& right) { return left.second < right.second; });

  parameters_t expected{ { 0b10000, 0 }, { 0b00111, 1 } };
  EXPECT_EQ(expected, params);
}

/*
 * Parameters which have been overridden for all the targets they were initially set to are discarded.
 */
TEST(Calibration, no_dangling_parameters)
{
  using calibrator_t = Calibration<std::bitset<5>, int>;
  using bitset_t = calibrator_t::bitset_t;
  using parameter_group_t = calibrator_t::parameter_group_t;
  using parameters_t = calibrator_t::parameters_t;

  calibrator_t cal;
  cal.set(0b10101, 0);
  cal.set(0b00111, 1);  // << overrides the parameter for the middle and last target
  cal.set(0b11000, 2);  // << overrides the parameter for the first target
  auto params = cal.get();

  std::sort(std::begin(params), std::end(params),
            [](parameter_group_t const& left, parameter_group_t const& right) { return left.second < right.second; });

  parameters_t expected{ { 0b00111, 1 }, { 0b11000, 2 } };
  EXPECT_EQ(expected, params);
}

boost::dynamic_bitset<> operator""_db(unsigned long long int mask)
{
    size_t max_bit = 0;
    for (auto m = mask; m; m>>= 1) ++max_bit;
    return boost::dynamic_bitset<>{max_bit, mask};
}

/*
 * Setting parameters to disjoint groups of targets will never lead to overrides.
 */
TEST(Calibration, disjoint_targets_)
{
  using calibrator_t = Calibration<boost::dynamic_bitset<>, int>;
  using parameter_group_t = calibrator_t::parameter_group_t;
  using parameters_t = calibrator_t::parameters_t;

  calibrator_t cal;
  cal.set(0b10000_db, 0);
  cal.set(0b00010_db, 3);
  cal.set(0b01000_db, 1);
  cal.set(0b00100_db, 2);
  cal.set(0b000001_db, 4);
  auto params = cal.get();

  std::sort(std::begin(params), std::end(params),
            [](parameter_group_t const& left, parameter_group_t const& right) { return left.second < right.second; });

  parameters_t expected{ { 0b10000_db, 0 }, { 0b01000_db, 1 }, { 0b00100_db, 2 }, { 0b00010_db, 3 },
      {0b000001_db, 4}};
  EXPECT_EQ(expected, params);
}

/*
 * Parameters can be overridden by subsequent calls to "set".
 */
TEST(Calibration, override_parameters_)
{
  using calibrator_t = Calibration<boost::dynamic_bitset<>, int>;
  using bitset_t = calibrator_t::bitset_t;
  using parameter_group_t = calibrator_t::parameter_group_t;
  using parameters_t = calibrator_t::parameters_t;

  calibrator_t cal;
  cal.set(0b10101_db, 0);
  cal.set(0b00111_db, 1);  // << overrides the parameter for the middle and last target
  auto params = cal.get();

  std::sort(std::begin(params), std::end(params),
            [](parameter_group_t const& left, parameter_group_t const& right) { return left.second < right.second; });

  parameters_t expected{ { 0b10000_db, 0 }, { 0b00111_db, 1 } };
  EXPECT_EQ(expected, params);
}

/*
 * Parameters which have been overridden for all the targets they were initially set to are discarded.
 */
TEST(Calibration, no_dangling_parameters_)
{
  using calibrator_t = Calibration<boost::dynamic_bitset<>, int>;
  using bitset_t = calibrator_t::bitset_t;
  using parameter_group_t = calibrator_t::parameter_group_t;
  using parameters_t = calibrator_t::parameters_t;

  calibrator_t cal;
  cal.set(0b10101_db, 0);
  cal.set(0b00111_db, 1);  // << overrides the parameter for the middle and last target
  cal.set(0b11000_db, 2);  // << overrides the parameter for the first target
  auto params = cal.get();

  std::sort(std::begin(params), std::end(params),
            [](parameter_group_t const& left, parameter_group_t const& right) { return left.second < right.second; });

  parameters_t expected{ { 0b00111_db, 1 }, { 0b11000_db, 2 } };
  EXPECT_EQ(expected, params);
}
