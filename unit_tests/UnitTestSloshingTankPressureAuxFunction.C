#include "gtest/gtest.h"

#include "user_functions/SloshingTankPressureAuxFunction.h"

#include <vector>

namespace sierra::kynema_ugf {

namespace {
// water level, amplitude, kappa, interface thickness, domain height, gravity_z
const std::vector<double> flat_tank_params{0.0, 0.0, 0.25, 0.01, 2.4, -9.81};
constexpr double rho_l = 998.6;
constexpr double rho_g = 1.225;
constexpr double g = 9.81;
constexpr double domain_top = 2.4;

double
evaluate(const SloshingTankPressureAuxFunction& func, const double z)
{
  double coords[3] = {0.3, -0.2, z};
  double pressure = 0.0;
  func.do_evaluate(coords, 0., 3, 1, &pressure, 1, 0, 1);
  return pressure;
}
} // namespace

TEST(SloshingTankPressureAuxFunction, hydrostatic_in_liquid_uses_set_densities)
{
  SloshingTankPressureAuxFunction func(flat_tank_params, rho_l, rho_g);

  // far below the smoothed interface: air column above plus water above z
  const double z = -1.0;
  const double expected = g * (rho_g * domain_top + rho_l * (-z));
  EXPECT_NEAR(evaluate(func, z), expected, 1.0e-9 * expected);

  // vertical gradient in the liquid is rho_l g
  EXPECT_NEAR(evaluate(func, -2.0) - evaluate(func, -1.0), rho_l * g, 1.0e-8);
}

TEST(SloshingTankPressureAuxFunction, hydrostatic_in_gas_uses_set_densities)
{
  SloshingTankPressureAuxFunction func(flat_tank_params, rho_l, rho_g);

  const double z = 1.0;
  EXPECT_NEAR(evaluate(func, z), g * rho_g * (domain_top - z), 1.0e-10);
  EXPECT_NEAR(evaluate(func, domain_top), 0.0, 1.0e-10);
}

TEST(SloshingTankPressureAuxFunction, single_phase_density_gives_linear_profile)
{
  SloshingTankPressureAuxFunction func(flat_tank_params, 1.2, 1.2);

  for (const double z : {-1.5, -0.005, 0.0, 0.004, 1.0}) {
    EXPECT_NEAR(evaluate(func, z), g * 1.2 * (domain_top - z), 1.0e-10);
  }
}

} // namespace sierra::kynema_ugf
