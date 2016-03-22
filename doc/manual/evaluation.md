Evaluation {#evaluation}
==========

Evaluation of a scoring function has three stages

1. Update of IMP::ScoreState objects. These must be done in a partially ordered manner as some depend on one another (e.g., the contents of a non-bonded list depend on the coordinates of a member of a rigid body). The order is computed as part of the [dependencies](@ref dependencies). When OpenMP is used, independent ScoreStates are updated in parallel

1. Evaluation of the IMP::Restraint objects. In principle, all restraint evaluations, done using the IMP::Restraint::add_to_score_and_derivatives() function are independent and so can be done in parallel. They use the ScoreAccumulator and DerivativeAccumulator to add appropriate values to the total score, and when requested, individual variable derivatives.

1. Post-evaluate update of the IMP::ScoreState objects. The purpose of this is mostly so that some amount of processing can occur on derivatives. Primarily, this involves moving derivatives from IMP::core::RigidMember objects to the corresponding IMP::core::RigidBody object.
