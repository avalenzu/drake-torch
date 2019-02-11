#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

#include <gflags/gflags.h>

#include <drake/common/drake_copyable.h>
#include <drake/common/eigen_types.h>
#include <drake/common/text_logging_gflags.h>
#include <drake/lcm/drake_lcm.h>
#include <drake/multibody/joints/floating_base_types.h>
#include <drake/multibody/parsers/sdf_parser.h>
#include <drake/multibody/rigid_body_plant/drake_visualizer.h>
#include <drake/multibody/rigid_body_tree.h>
#include <drake/systems/analysis/simulator.h>
#include <drake/systems/framework/context.h>
#include <drake/systems/framework/diagram.h>
#include <drake/systems/framework/diagram_builder.h>
#include <drake/systems/framework/vector_base.h>
#include <drake/systems/primitives/constant_vector_source.h>

#include <drake/common/find_resource.h>
#include <drake/common/nice_type_name.h>
#include <drake/common/drake_assert.h>
#include <drake/common/text_logging_gflags.h>
#include <drake/geometry/geometry_visualization.h>
#include <drake/geometry/scene_graph.h>
#include <drake/lcm/drake_lcm.h>
#include <drake/multibody/parsing/parser.h>
#include <drake/multibody/plant/multibody_plant.h>
#include <drake/multibody/tree/prismatic_joint.h>
#include <drake/multibody/tree/revolute_joint.h>
#include <drake/multibody/tree/uniform_gravity_field_element.h>
#include <drake/systems/analysis/simulator.h>
#include <drake/systems/framework/system.h>
#include <drake/systems/framework/diagram_builder.h>
#include <drake/common/drake_deprecated.h>
#include <drake/geometry/scene_graph.h>
#include <drake/multibody/parsing/package_map.h>
#include <drake/multibody/plant/multibody_plant.h>
#include <drake/multibody/tree/multibody_tree_indexes.h>

// namespace drake_external_examples {
// namespace cart_pole {
// namespace {

using drake::geometry::SceneGraph;
using drake::lcm::DrakeLcm;

// "multibody" namespace is ambiguous here without "drake::".
using drake::multibody::MultibodyPlant;
using drake::multibody::Parser;
using drake::multibody::PrismaticJoint;
using drake::multibody::RevoluteJoint;
using drake::multibody::UniformGravityFieldElement;

DEFINE_double(target_realtime_rate, 1.0,
              "Desired rate relative to real time.  See documentation for "
              "Simulator::set_target_realtime_rate() for details.");

DEFINE_double(simulation_time, 10.0,
              "Desired duration of the simulation in seconds.");

DEFINE_double(time_step, 0,
            "If greater than zero, the plant is modeled as a system with "
            "discrete updates and period equal to this time_step. "
            "If 0, the plant is modeled as a continuous system.");

/// Check if the specified file exists.
/// @param[in] name of the file
/// @return existence (true) or otherwise (false)
bool file_exists(const std::string& name) {
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}

int do_main() {
  drake::systems::DiagramBuilder<double> builder;

  SceneGraph<double>& scene_graph = *builder.AddSystem<SceneGraph>();
  scene_graph.set_name("scene_graph");

  // Make and add the cart_pole model.
  const std::string full_name =
      "drake/examples/multibody/cart_pole/cart_pole.sdf";
  if (!file_exists(full_name)) {
    throw std::runtime_error(std::string("could not find '") +
                             full_name + std::string("'"));
  }
  MultibodyPlant<double>& cart_pole =
      *builder.AddSystem<MultibodyPlant>(FLAGS_time_step);
  Parser(&cart_pole, &scene_graph).AddModelFromFile(full_name);

  // Add gravity to the model.
  cart_pole.AddForceElement<UniformGravityFieldElement>();

  // Now the model is complete.
  cart_pole.Finalize();

  // Sanity check on the availability of the optional source id before using it.
  DRAKE_DEMAND(cart_pole.geometry_source_is_registered());

  builder.Connect(
      cart_pole.get_geometry_poses_output_port(),
      scene_graph.get_source_pose_port(cart_pole.get_source_id().value()));

  drake::geometry::ConnectDrakeVisualizer(&builder, scene_graph);
  auto diagram = builder.Build();

  // Create a context for this system:
  std::unique_ptr<drake::systems::Context<double>> diagram_context =
      diagram->CreateDefaultContext();
  diagram->SetDefaultContext(diagram_context.get());
  drake::systems::Context<double>& cart_pole_context =
      diagram->GetMutableSubsystemContext(cart_pole, diagram_context.get());

  // There is no input actuation in this example for the passive dynamics.
  cart_pole_context.FixInputPort(
      cart_pole.get_actuation_input_port().get_index(), drake::Vector1d(0));

  // Get joints so that we can set initial conditions.
  const PrismaticJoint<double>& cart_slider =
      cart_pole.GetJointByName<PrismaticJoint>("CartSlider");
  const RevoluteJoint<double>& pole_pin =
      cart_pole.GetJointByName<RevoluteJoint>("PolePin");

  // Set initial state.
  cart_slider.set_translation(&cart_pole_context, 0.0);
  pole_pin.set_angle(&cart_pole_context, 2.0);

  drake::systems::Simulator<double> simulator(*diagram, std::move(diagram_context));

  simulator.set_publish_every_time_step(false);
  simulator.set_target_realtime_rate(FLAGS_target_realtime_rate);
  simulator.Initialize();
  simulator.StepTo(FLAGS_simulation_time);

  return 0;
}

// }  // namespace
// }  // namespace cart_pole
// }  // namespace drake_external_examples

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage(
      "A simple cart pole demo using Drake's MultibodyPlant,"
      "with SceneGraph visualization. "
      "Launch drake-visualizer before running this example.");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  drake::logging::HandleSpdlogGflags();
//   return drake_external_examples::cart_pole::do_main();
return do_main();
}