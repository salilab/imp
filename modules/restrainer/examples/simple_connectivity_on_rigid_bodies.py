## \example restrainer/simple_connectivity_on_rigid_bodies.py
## This example shows how to create simple IMP::core::ConnectivityRestraint on rigid bodies.
##

#-- File: simple_connectivity_on_rigid_bodies --#

import IMP
import IMP.atom
import IMP.restrainer

m = IMP.Model()
sel = IMP.atom.CAlphaPDBSelector()

mh1 = IMP.atom.read_pdb(IMP.restrainer.get_example_path("input.pdb"), m, sel)
mh2 = IMP.atom.read_pdb(IMP.restrainer.get_example_path("input.pdb"), m, sel)
IMP.atom.add_radii(mh1)
IMP.atom.add_radii(mh2)

mhs = IMP.atom.Hierarchies()
mhs.append(mh1)
mhs.append(mh2)

rbs = IMP.restrainer.set_rigid_bodies(mhs)

sc = IMP.restrainer.create_simple_connectivity_on_rigid_bodies(rbs)

r = sc.get_restraint()
h = sc.get_harmonic_upper_bound()
ps = sc.get_sphere_distance_pair_score()

sc.set_mean(10.0)
sc.set_standard_deviation(3.5)
sc.set_k(0.1)

m.add_restraint(r)
r.show()

m.evaluate(False)
