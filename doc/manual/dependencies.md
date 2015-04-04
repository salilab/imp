Dependencies {#dependencies}
============

IMP needs to keep track of how various IMP::kernel::Particle, IMP::kernel::ScoreState, IMP::kernel::Restraint and other things related to one another. For example, it needs to know that in order to evaluate a certain IMP::kernel::Restraint, the IMP::kernel::ScoreState managing the IMP::core::RigidBody that one of the restraint's input particles is an IMP::core::RigidMember of needs to be updated.

The set of objects that can be involved in the graph are ones derived from IMP::kernel::ModelObject. The IMP::kernel::ModelObject::get_inputs() and IMP::kernel::ModelObject::get_outputs() return the input and output model objects. These lists of inputs and outputs are used to build an IMP::kernel::DependencyGraph, a DAG that relates the data flow between IMP::kernel::ModelObject instances. As the most important type of IMP::kernel::ModelObject is IMP::kernel::ScoreState, IMP::kernel::ModelObject::get_required_score_states() returns all the score states that are upstream in the DAG from that IMP::kernel::ModelObject.

Dependencies are needed when restraints are evaluated. To get them, call IMP::kernel::Model::set_has_dependencies(true) (undocumented). That goes over all known IMP::kernel::ModelObject, asks them for their inputs and outputs and builds the graph. It then flows over the graph to compute the IMP::kernel::ScoreState that is upstream from each each IMP::kernel::ModelObject. These are stored in each IMP::kernel::ModelObject using the (private)  IMP::kernel::ModelObject::set_has_dependencies(true, score_states) call.

When the graph changes, it must be updated. This is done by simply invalidating the graph when necessary. This invalidation occurs when
- a new IMP::kernel::ModelObject is created that has a non-empty IMP::kernel::ModelObject::get_outputs().
- when IMP::kernel::Model::set_has_dependencies(false) is called to deliberately invalidate all dependencies

Each IMP::kernel::ModelObject has a bit for whether it's dependencies are valid (IMP::kernel::ModelObject::get_has_dependencies()). Its dependencies will be updated any time they are needed (particularly when its IMP::kernel::ModelObject::get_required_score_states() method is called. This occurs by having the IMP::kernel::Model create the dependency graph if needed and then for each input
- add its IMP::kernel::ModelObject::get_required_score_state() to the list
- if it is an IMP::kernel::ScoreState, add it to the list.

As a special case, the IMP::core::IncrementalScoringFunction has to take action after its dependencies are updated. This is because it keeps track of which restraints depend on which of the IMP::kernel::Particle instances that it is keeping track of. To do this, the IMP::kernel::ModelObject::do_set_has_dependencies() is called after dependencies are set up. IMP::core::IncrementalScoringFunction uses that function to compute its dependencies.
