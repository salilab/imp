## \example kernel/chain.py
# This example shows how to set up an optimization involving several
# particles constrained to be connected in a loop. It uses non bonded
# lists and a variety of restraints.

import IMP.kernel
import IMP.atom
import IMP.core
import random
import RMF
import IMP.container
import IMP.rmf
import IMP.base
import sys

IMP.base.setup_from_argv(sys.argv, "Chain example")

# A trivial example that constructs a set of particles which are restrained
# to form a chain via bonds between successive particles. In addition
# the head and the tail of the chain are restrained to be close to one
# another.

IMP.base.set_log_level(IMP.base.TERSE)
m = IMP.kernel.Model()
m.set_log_level(IMP.base.SILENT)
h = IMP.atom.Hierarchy.setup_particle(m, m.add_particle("root"))
# The particles in the chain
ps = IMP.core.create_xyzr_particles(m, 10, 1.0)
for p in ps:
    # build a hierarchy with all the particles
    h.add_child(IMP.atom.Hierarchy.setup_particle(p))
    # things are expected to have mass
    IMP.atom.Mass.setup_particle(p, 1)
chain = IMP.container.ListSingletonContainer(ps, "chain")

restraints = []
# create a spring between successive particles
bonds = IMP.container.ConsecutivePairContainer(ps)
hdps = IMP.core.HarmonicDistancePairScore(2, 1)
chainr = IMP.container.PairsRestraint(hdps, bonds)
chainr.set_name("The chain restraint")
restraints.append(chainr)

# Prevent non-bonded particles from penetrating one another
nbl = IMP.container.ClosePairContainer(chain, 0, 2)
bpc = IMP.container.ConsecutivePairFilter(bonds)  # exclude existing bonds
nbl.add_pair_filter(bpc)
lr = IMP.container.PairsRestraint(IMP.core.SoftSpherePairScore(1), nbl,
                                  "excluded volume")
restraints.append(lr)

# Tie the ends of the chain
tie = IMP.core.PairRestraint(IMP.core.HarmonicDistancePairScore(3, 1),
                             (ps[0], ps[-1]))
tie.set_name("tie ends")
restraints.append(tie)

s = IMP.core.MCCGSampler(m)  # sample using MC and CG
for r in restraints:
    # set a criteria for what makes a good score
    r.set_maximum_score(.1)
s.set_scoring_function(restraints)
s.set_number_of_attempts(10)

confs = s.create_sample()
print "Found", confs.get_number_of_configurations(), "configurations"
fh = RMF.create_rmf_file("solutions.rmfz")
IMP.rmf.add_hierarchy(fh, h)
for i in range(0, confs.get_number_of_configurations()):
    confs.load_configuration(i)
    IMP.rmf.save_frame(fh, str(i))
