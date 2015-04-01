Frequently asked questions (FAQ) {#faq}
================================

## How do I make my script run faster?
* Use specialized scores that don't take a IMP::UnaryFunction argument, e.g.
  replace the combination of IMP::core::SphereDistancePairScore and
  IMP::core::HarmonicLowerBound with IMP::core::SoftSpherePairScore.
  This reduces the number of virtual function calls during evalution.
* Group similar restraints: e.g. replace a large number of
  IMP::core::PairRestraint objects with a single
  IMP::container::PairsRestraint. This also reduces the number of
  function calls.
* If you are programming in C++, you can use IMP::container::create_restraint()
  to create a 'generic' restraint from a container and a pair score. This
  removes a few more virtual function calls. If you are creating a restraint
  with only a single particle or pair, use IMP::create_restraint() instead.
* Adjusting slack parameters on non-bonded lists: IMP::container::ClosePairContainer and IMP::core::ExcludedVolumeRestraint have "slack" parameters which adjust how often the non-bonded lists that are used are recomputed. Changing this parameter does not change the computed score, just how often the lists are updated. As a rough guideline, you want the slack to be larger than the total amount any particle moves in, say 20 evaluates (e.g. 20 MD steps). Experiment with changing the parameter to see what yields the  best results or try the IMP::container::get_slack_estimate() function to try to estimate the best value to use automatically.
* Turn off or turn down logging: call IMP.base.set_log_level(IMP.base.SILENT)
  or [recompile from the source code](@ref installation_source) passing
  [-DIMP_MAX_LOG=SILENT to cmake](@ref cmake_further) (the latter will be
  more efficient).
* Turn off runtime checks: call IMP.base.set_check_level(IMP.base.USAGE) or
  IMP.base.set_check_level(IMP.base.NONE). Proceed very carefully, since with
  runtime checks turned off, %IMP will not warn you if you do something
  foolish - be sure to test your protocol first! Again, this will be more
  efficient if you
  [recompile from the source code](@ref installation_source) passing
  [the -DIMP_MAX_CHECKS option to cmake](@ref cmake_further).

## Why can't I clone a Particle?
The short answer is: "because no one knows how to do it safely". The complication is that the attributes of various particles may have relationships to one another that would be violated by the cloning process. For example, a Particle attribute in a Particle could be part of a bond, in which case the bond particle must also be cloned and the attribute in the new particle must point to the new bond, or it could be a pointer to a particle which keeps track of all of the atoms in the system, in which case the attribute value should be copied unchanged. A clone function would have no way of knowing which should be done.

We suggest that you either
* call your initialization code more than once (to create two copies of the set of particles):

        protein_0= create_protein("my_protein_name")
        protein_1= create_protein("my_protein_name")

* use the `IMP.atom.create_clone()` function to clone a molecular hierarchy.
