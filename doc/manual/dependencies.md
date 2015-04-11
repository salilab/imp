Dependencies {#internal_dependencies}
============

IMP needs to keep track of how various IMP::Particle, IMP::ScoreState, IMP::Restraint and other things related to one another. For example, it needs to know that in order to evaluate a certain IMP::Restraint, the IMP::ScoreState managing the IMP::core::RigidBody that one of the restraint's input particles is an IMP::core::RigidMember of needs to be updated.

The set of objects that can be involved in the graph are ones derived from IMP::ModelObject. The IMP::ModelObject::get_inputs() and IMP::ModelObject::get_outputs() return the input and output model objects. These lists of inputs and outputs are used to build an IMP::DependencyGraph, a DAG that relates the data flow between IMP::ModelObject instances. As the most important type of IMP::ModelObject is IMP::ScoreState, IMP::ModelObject::get_required_score_states() returns all the score states that are upstream in the DAG from that IMP::ModelObject.

Dependencies are needed when restraints are evaluated. To get them, call IMP::Model::set_has_dependencies(true) (undocumented). That goes over all known IMP::ModelObject, asks them for their inputs and outputs and builds the graph. It then flows over the graph to compute the IMP::ScoreState that is upstream from each each IMP::ModelObject. These are stored in each IMP::ModelObject using the (private)  IMP::ModelObject::set_has_dependencies(true, score_states) call.

When the graph changes, it must be updated. This is done by simply invalidating the graph when necessary. This invalidation occurs when
- a new IMP::ModelObject is created that has a non-empty IMP::ModelObject::get_outputs().
- when IMP::Model::set_has_dependencies(false) is called to deliberately invalidate all dependencies

Each IMP::ModelObject has a bit for whether it's dependencies are valid (IMP::ModelObject::get_has_dependencies()). Its dependencies will be updated any time they are needed (particularly when its IMP::ModelObject::get_required_score_states() method is called. This occurs by having the IMP::Model create the dependency graph if needed and then for each input
- add its IMP::ModelObject::get_required_score_state() to the list
- if it is an IMP::ScoreState, add it to the list.

As a special case, the IMP::core::IncrementalScoringFunction has to take action after its dependencies are updated. This is because it keeps track of which restraints depend on which of the IMP::Particle instances that it is keeping track of. To do this, the IMP::ModelObject::do_set_has_dependencies() is called after dependencies are set up. IMP::core::IncrementalScoringFunction uses that function to compute its dependencies.
