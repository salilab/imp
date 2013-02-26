## \example kernel/chain.py
## This example shows how to set up an optimization involving several particles constrained to be connected in a loop. It uses non bonded lists and a variety of restraints.

import IMP
import IMP.core
import random
import IMP.display
import IMP.container

# A trivial example that constructs a set of particles which are restrained
# to form a chain via bonds between successive particles. In addition
# the head and the tail of the chain are restrained to be close to one
# another.

IMP.base.set_log_level(IMP.base.TERSE)
m= IMP.Model()
m.set_log_level(IMP.base.SILENT)
# The particles in the chain
ps=IMP.core.create_xyzr_particles(m, 10, 1.0)
chain= IMP.container.ListSingletonContainer(ps, "chain")

# create a spring between successive particles
bonds= IMP.container.ConsecutivePairContainer(ps)
hdps=IMP.core.HarmonicDistancePairScore(2,1)
chainr= IMP.container.PairsRestraint(hdps,bonds)
chainr.set_name("The chain restraint")
m.add_restraint(chainr)

# If you want to inspect the particles
# Notice that each bond is a particle
for p in m.get_particles():
    p.show()

# Prevent non-bonded particles from penetrating one another
nbl= IMP.container.ClosePairContainer(chain, 0,2)
bpc=IMP.container.ConsecutivePairFilter(bonds) # exclude existing bonds
nbl.add_pair_filter(bpc)
lr=IMP.container.PairsRestraint(IMP.core.SoftSpherePairScore(1), nbl,
                               "excluded volume")
m.add_restraint(lr)

# Tie the ends of the chain
tie=IMP.core.PairRestraint(IMP.core.HarmonicDistancePairScore(3,1),
                                (ps[0], ps[-1]))
tie.set_name("tie ends")
m.add_restraint(tie)

s= IMP.core.MCCGSampler(m) # sample using MC and CG
s.set_number_of_attempts(10)
m.set_maximum_score(1)
confs= s.get_sample()
print "Found", confs.get_number_of_configurations(), "configurations"
for i in range(0, confs.get_number_of_configurations()):
    confs.load_configuration(i)
    d=IMP.display.ChimeraWriter("solution"+str(i)+".py")
    for p in chain.get_particles():
        d.add_geometry(IMP.core.XYZRGeometry(p))

# print out info about used modules so that the versions are known
#IMP.show_used_modules()
