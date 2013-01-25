## \example restrainer/simple_excluded_volume.py
## This example shows how to create simple IMP::core::ExcludedVolumeRestraint.
##

#-- File: simple_excluded_volume.py --#

import IMP
import IMP.core
import IMP.atom
import IMP.restrainer

m= IMP.Model()

sel = IMP.atom.CAlphaPDBSelector()
p0 = IMP.atom.read_pdb(
         IMP.restrainer.get_example_path("input.pdb"), m, sel)
p1 = IMP.atom.read_pdb(
         IMP.restrainer.get_example_path("input.pdb"), m, sel)

mhs = IMP.atom.Hierarchies()
mhs.append(p0)
mhs.append(p1)

ps = []
for mh in mhs:
    ps.append(mh.get_particle())

rbs = IMP.restrainer.set_rigid_bodies(mhs)

sev = IMP.restrainer.create_simple_excluded_volume_on_rigid_bodies(rbs)

r = sev.get_restraint()

m.add_restraint(r)
