## \example domino/restraint_cache.py
## Caching restraint scores so that restraints are not evaluated repeatedly for the same configuration is an important part of domino. Without caching, sub assignments that are shared between subsets will be rescored. The IMP::domino::RestraintCache provides a centralized place for this. To use it, one creates one and then adds the restraints you want to use for filtering and scoring to it. You can then pass the cache to the IMP::domino::RestraintScoreFilterTable and it will filter based on those restraints. You can also extract scores from the table directly, using it to manage the loading of particle states.

import IMP.domino
import IMP.algebra
import IMP.container
import IMP.atom
import IMP.rmf
import RMF

resolution=.5

sls=IMP.base.SetLogState(IMP.base.SILENT)

m= IMP.Model()

# create some particles and a restraint that scores based on the
# length of the chain
ps= [IMP.core.XYZR.setup_particle(IMP.Particle(m))
     for i in range(0,int(2.0/resolution))]
eps= [IMP.core.XYZR.setup_particle(IMP.Particle(m)) for i in range(0,2)]
eps[0].set_coordinates(IMP.algebra.Vector3D(-1,0,0))
eps[1].set_coordinates(IMP.algebra.Vector3D(1,0,0))
for p in ps:
    p.set_radius(resolution/2.0)
for p in eps:
    p.set_radius(.4*resolution)
order=[eps[0]]+ps+[eps[1]]
# create a hierarchy with the particles to aid writing them to RMF
h= IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
for i,p in enumerate(order):
    IMP.atom.Mass.setup_particle(p,1)
    h.add_child(IMP.atom.Hierarchy.setup_particle(p))
    color= IMP.display.get_jet_color(float(i)/(len(order)-1))
    IMP.display.Colored.setup_particle(p, color)


r= IMP.container.PairsRestraint(IMP.core.HarmonicUpperBoundSphereDistancePairScore(0,1),
                                IMP.container.ConsecutivePairContainer(order))
# allow a maximum distance of 2
r.set_maximum_score(0.01)

# dumb way to generate points on a gride
g= IMP.algebra.DenseFloatGrid3D(resolution, IMP.algebra.get_unit_bounding_box_3d())
# create some random sites
vs= [g.get_center(i) for i in g.get_all_indexes()]
print "States are", vs
pst= IMP.domino.ParticleStatesTable()
states=IMP.domino.XYZStates(vs)
for p in ps:
    pst.set_particle_states(p, states)


# now create a restraint cache
# cache the most recently used one million scores
rc= IMP.domino.RestraintCache(pst, 1000000)
r.set_log_level(IMP.base.SILENT)
rc.add_restraints([r])

s=IMP.domino.DominoSampler(m, pst)
s.set_check_level(IMP.base.NONE)
ef=IMP.domino.ExclusionSubsetFilterTable(pst)
# pass the cache to the restraint score based filter
# it will use all the restraints in the cache
rssft= IMP.domino.RestraintScoreSubsetFilterTable(rc)
s.set_subset_filter_tables([ef, rssft])

# create a subset with all the particles
# Subsets keep the particles in sorted order and so are different than
# a simple list of particles
alls= IMP.domino.Subset(ps)

# get all the assignments that fit
sts= s.get_sample_assignments(alls)

# create a temporary file to write things to
rmf= RMF.create_rmf_file(IMP.create_temporary_file_name("cache", ".rmf"))
print "saving configurations to", rmf.get_name()

IMP.rmf.add_hierarchy(rmf, h)

# the restraint cache processes the restraints to make them more efficient
# for use with domino. So we want to get back the processed restraints before
# looking at them further
domino_restraints= rc.get_restraints()

for r in domino_restraints:
    IMP.show_restraint_hierarchy(r)

IMP.rmf.add_restraints(rmf, domino_restraints)
# for each assignment load it, and add the configuration to an rmf file
print "found assignments", sts
# we don't care about messages during saving
for i, s in enumerate(sts):
    IMP.domino.load_particle_states(alls, s, pst)
    # go through the restraints and get the score
    # here, we only care about its side effect of setting the last score
    for r in domino_restraints:
        rc.load_last_score(r, alls, s)
    # save the configuration and scores to the rmf
    IMP.rmf.save_frame(rmf, i)
