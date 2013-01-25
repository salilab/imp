## \example restrainer/simple_connectivity_on_molecules.py
## This example shows how to create simple IMP::core::ConnectivityRestraint on molecules.
##

#-- File: simple_connectivity_on_molecules.py --#

import IMP
import IMP.core
import IMP.restrainer

m = IMP.Model()

IMP.core.create_xyzr_particles(m, 4, 1.0)
ps = m.get_particles()
mhs = IMP.atom.Hierarchies()

for p in ps:
    mh = IMP.atom.Hierarchy.setup_particle(p)
    mhs.append(mh)

sc = IMP.restrainer.create_simple_connectivity_on_molecules(mhs)

r = sc.get_restraint()
h = sc.get_harmonic_upper_bound()
sdps = sc.get_sphere_distance_pair_score()

sc.set_mean(10.0)
sc.set_standard_deviation(3.5)
sc.set_k(0.1)

m.add_restraint(r)
r.show()

m.evaluate(False)
