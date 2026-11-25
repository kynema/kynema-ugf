// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS), National Renewable Energy Laboratory, University of Texas Austin,
// Northwest Research Associates. Under the terms of Contract DE-NA0003525
// with NTESS, the U.S. Government retains certain rights in this software.
//
// This software is released under the BSD 3-clause license. See LICENSE file
// for more details.
//

#ifndef TimeIntegrator_h
#define TimeIntegrator_h

#include <Enums.h>
#include <vector>
#include <string>
#include <memory>

namespace YAML {
class Node;
}

namespace sierra {
namespace kynema_ugf {

class Realm;
class Simulation;
class ExtOverset;

class TimeIntegrator
{
public:
  TimeIntegrator();
  TimeIntegrator(Simulation* sim);
  ~TimeIntegrator();

  void load(const YAML::Node& node);

  void breadboard();

  void initialize();

  void integrate_realm();
  void provide_mean_norm();
  bool simulation_proceeds();

  // Modular coupling TODO: export a UGF-owned interface around Simulation and
  // TimeIntegrator so kynema-driver's ModuleAdapter only forwards calls, as
  // for FMB. Keep driver headers/PreparationContext out of UGF; expose native
  // preparation stages, runtime/communicator ownership, identifier, and
  // time/dt/index accessors instead of public-member access from the driver.
  //
  // set_inputs(iteration): wrap prepare_time_step() and apply staged coupling
  // data every iteration, without repeating first-iteration timestep setup.
  // predict_inputs(iteration): wrap pre_realm_advance_stage1(); defer stage 2
  // until after connectivity via prepare_solution_exchange(), including when
  // Tioga is disabled. Keep the pending stage and iteration inside UGF.
  // advance_states(iteration): own the realm loop calling advance_time_step()
  // and process_multi_physics_transfer() in its existing order.
  // calculate_outputs(iteration): refresh exchangeable forces/fields and host
  // data after solves, separately from post_realm_advance()/finalize_time_step().
  //
  // Add initialization_data()/receive_initialization_data() and
  // receive_inputs()/outputs() for JSON coupling independently of overset.
  // Agree on layout, units, frame, and time/version; validate received data,
  // map it into realm fields, and synchronize host/device storage before use.
  // Staging a payload must not advance time. No payload may be represented by
  // null; supported non-null inputs must not be silently discarded.
  //
  // reset_states() requires explicit realm snapshot/restore support, not
  // another swap_states(). Retain received inputs and current overset data.
  // Preserve the current no-op reset and grouped nonlinear_iterations(count)
  // corrections until native rollback is implemented and regression-tested.
  void prepare_for_time_integration();
  void prepare_time_step(size_t inonlin = 0);
  void pre_realm_advance_stage1(size_t inonlin = 0);
  void pre_realm_advance_stage2(size_t inonlin = 0);
  void post_realm_advance();
  void interstep_updates(int nonLinearIterationIndex);

  Simulation* sim_{nullptr};

  double totalSimTime_;
  double currentTime_;
  double timeStepFromFile_;
  double timeStepN_;
  double timeStepNm1_;
  double gamma1_;
  double gamma2_;
  double gamma3_;
  int timeStepCount_;
  int maxTimeStepCount_;
  bool secondOrderTimeAccurate_;
  bool adaptiveTimeStep_;
  bool terminateBasedOnTime_;
  int nonlinearIterations_;

  std::string name_;

  std::vector<std::string> realmNamesVec_;

  std::vector<Realm*> realmVec_;

  void set_time_step(const double dt);
  double
  get_time_step(const KynemaUGFState& theState = KYNEMA_UGF_STATE_N) const;
  double get_current_time() const;
  double get_gamma1() const;
  double get_gamma2() const;
  double get_gamma3() const;
  int get_time_step_count() const;
  double get_time_step_from_file();
  bool get_is_fixed_time_step();
  bool get_is_terminate_based_on_time();
  double get_total_sim_time();
  int get_max_time_step_count();
  void compute_gamma();

  std::unique_ptr<ExtOverset> overset_;
};

} // namespace kynema_ugf
} // namespace sierra

#endif
