Close pairs lists {#close_pairs}
=================

There a several pathways by which lists of spatially close pairs of particles are generated. In all cases, the lists are of close balls, that is pairs where the distance between the balls (defined by the coordinates and radius) is smaller than a threshold. 

The three most common points of usage are
- IMP::container::ClosePairContainer
- IMP::core::ExcludedVolumeRestraint
- IMP::container::CloseBipartitePairContainer

They all maintain an invariant which is that they have a list of close pairs that includes all that are within the requested threshold, but can contain others. They use this ability to have extra pairs in order to avoid rebuilding the list every time the model is perturbed. Specifically, they maintain a list that
- has all pairs within threshold of one another
- when initialized has all pairs within `threshold + 2*slack` of one another
That way, until anything moves more than slack the list is still correct.

They all use slightly different code paths but have an overall similar structure:
- A list of pairs that are currently close to one another, given the threshold and the slack
- A data structure that keeps track of which particles have moved more than the slack
- An update procedure for adding new close pairs when particles have moved more than the slack.

The update procedure uses one or more of:
- an IMP::core::ClosePairFinder
- the template-based grid close pairs support in IMP::core::internal
- sphere trees attached to rigid bodies (since there are multiple lists of particles that can be used with each rigid body)
