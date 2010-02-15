import IMP
import IMP.core
import random
import IMP.display

# A trivial example that constructs a set of particles which are restrained
# to form a chain via bonds between successive particles. In addition
# the head and the tail of the chain are restrained to be close to one
# another.

IMP.set_log_level(IMP.TERSE)
m= IMP.Model()
# The particles in the chain
chain= IMP.container.ListSingletonContainer(IMP.core.create_xyzr_particles(m, 2, 1.0))

# create a bond between successive particles
IMP.atom.Bonded.setup_particle(chain.get_particle(0))
bonds= IMP.container.ListSingletonContainer("particles")
for i in range(1, chain.get_number_of_particles()):
    bp= IMP.atom.Bonded(chain.get_particle(i-1))
    bpr= IMP.atom.Bonded.setup_particle(chain.get_particle(i))
    b= IMP.atom.custom_bond(bp, bpr, 1.5, 10)
    bonds.add_particle(b.get_particle())

# If you want to inspect the particles
# Notice that each bond is a particle
for p in m.get_particles():
    p.show()

# Prevent non-bonded particles from penetrating one another
nbl= IMP.container.ClosePairContainer(chain, 0,2)
bpc=IMP.atom.BondedPairFilter() # exclude existing bonds
nbl.add_pair_filter(bpc)
ps= IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1))
m.add_restraint(IMP.container.PairsRestraint(ps, nbl))

# penalize conformations where bond lengths aren't preserved
bss= IMP.atom.BondSingletonScore(IMP.core.Harmonic(0,1))
m.add_restraint(IMP.container.SingletonsRestraint(bss, bonds))

# Tie the ends of the chain
p= IMP.ParticlePair(chain.get_particle(0),
                    chain.get_particle(chain.get_number_of_particles()-1))
pps= IMP.container.ListPairContainer()
pps.add_particle_pair(p)
m.add_restraint(IMP.container.PairsRestraint(
           IMP.core.SphereDistancePairScore(IMP.core.Harmonic(3,1)), pps))

s= IMP.core.MCCGSampler(m) # sample using MC and CG
s.set_number_of_attempts(10)
confs= s.sample()
for i in range(0, confs.get_number_of_configurations()):
    confs.set_configuration(i)
    d=IMP.display.ChimeraWriter("solution"+str(i)+".py")
    for p in chain.get_particles():
        d.add_geometry(IMP.display.XYZRGeometry(p))
