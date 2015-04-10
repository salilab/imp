# -*- coding: utf-8 -*-
# <nbformat>3.0</nbformat>

# <codecell>

from __future__ import print_function
import IMP
import os
import IMP.pmi.representation
import IMP.pmi.restraints.basic


psiv=0.01
sigmav=12.0
slope=0.0
length=21
inputx=70

# <codecell>

representations = []

# <codecell>

# define the particles in state 1


m = IMP.Model()
r = IMP.pmi.representation.Representation(m)

r.create_component("particle1", color=0.1)
p11 = r.add_component_beads("particle1", [(1, 10)])
r.create_component("particle2", color=0.5)
p21 = r.add_component_beads("particle2", [(1, 10)])
r.create_component("particle3", color=1.0)
p31 = r.add_component_beads("particle3", [(1, 10)])

representations.append(r)

# define the particles in state 2

r = IMP.pmi.representation.Representation(m)

r.create_component("particle1", color=0.1)
p12 = r.add_component_beads("particle1", [(1, 10)])
r.create_component("particle2", color=0.5)
p22 = r.add_component_beads("particle2", [(1, 10)])
r.create_component("particle3", color=0.3)
p32 = r.add_component_beads("particle3", [(1, 10)])

representations.append(r)

# remove paricles 1 and 2 from the otpmized articles list

representations[0].floppy_bodies.pop(0)
representations[0].floppy_bodies.pop(0)
representations[1].floppy_bodies.pop(0)
representations[1].floppy_bodies.pop(0)

print(representations[0].floppy_bodies)
print(representations[1].floppy_bodies)

# <codecell>

import IMP.core

pp1 = IMP.atom.get_leaves(p11[0])[0]
pp2 = IMP.atom.get_leaves(p21[0])[0]
pp3 = IMP.atom.get_leaves(p31[0])[0]
xyz11 = IMP.core.XYZ(pp1.get_particle())
xyz21 = IMP.core.XYZ(pp2.get_particle())
xyz31 = IMP.core.XYZ(pp3.get_particle())
xyz11.set_coordinates((0, 0, 0))
print(xyz11.get_coordinates())
xyz21.set_coordinates((inputx, 0, 0))
xyz31.set_coordinates((0, 0, 0))

pp1 = IMP.atom.get_leaves(p12[0])[0]
pp2 = IMP.atom.get_leaves(p22[0])[0]
pp3 = IMP.atom.get_leaves(p32[0])[0]
xyz12 = IMP.core.XYZ(pp1.get_particle())
xyz22 = IMP.core.XYZ(pp2.get_particle())
xyz32 = IMP.core.XYZ(pp3.get_particle())
xyz12.set_coordinates((0, 0, 0))
xyz22.set_coordinates((inputx, 0, 0))
xyz32.set_coordinates((inputx, 0, 0))

# <codecell>

eb = IMP.pmi.restraints.basic.ExternalBarrier(representations[0], 1000)
eb.add_to_model()

eb = IMP.pmi.restraints.basic.ExternalBarrier(representations[1], 1000)
eb.add_to_model()

# <codecell>

import IMP.pmi.restraints.crosslinking

restraints = '''#
particle2 particle3 5 5 1 1
particle1 particle3 5 5 1 2 '''

xl = IMP.pmi.restraints.crosslinking.ISDCrossLinkMS(representations,
                                                    restraints,
                                                    length=length,
                                                    slope=0.0,
                                                    inner_slope=slope,
                                                    resolution=1.0)

psi = xl.get_psi(0.05)

psi[0].set_scale(psiv)

sigma = xl.get_sigma(1.0)

sigma[0].set_scale(sigmav)

xl.set_psi_is_sampled(False)
xl.set_sigma_is_sampled(False)

# psi.set_scale(0.1)
# xl.get_sigma(1.0)

out_dict = xl.get_output()
sorted_keys = sorted(out_dict.keys())
for entry in sorted_keys:
    print(entry, out_dict[entry])

print(xyz11.get_coordinates())

xl.add_to_model()
print(m.evaluate(False))

# <codecell>

import IMP.pmi.samplers

import IMP.pmi.output

o = IMP.pmi.output.Output()
o.init_rmf(
    "trajectory.rmf3",
    [representations[0].prot,
     representations[1].prot])

print(o.dictionary_rmfs)

mc = IMP.pmi.samplers.MonteCarlo(m, representations, 1.0)
mc.set_simulated_annealing(min_temp=1.0,
                           max_temp=2.0,
                           min_temp_time=200,
                           max_temp_time=50)

# <codecell>

o.init_stat2("modeling.stat", [mc, xl] + representations)


for i in range(1,20):
    xyz31.set_coordinates((float(i), 0, 0))
    for j in range(1,20):
        xyz32.set_coordinates((float(j), 0, 0))
        print(i,j,m.evaluate(False))
        o.write_stats2()


# <codecell>


po = IMP.pmi.output.ProcessOutput("modeling.stat")

# <codecell>

po.get_keys()

# <codecell>

fs = po.get_fields(
    ['ISDCrossLinkMS_Distance_interrb-State:0-5:particle1_5:particle3-1-1-0.05_None',
     'ISDCrossLinkMS_Distance_interrb-State:0-5:particle2_5:particle3-1-1-0.05_None',
     'ISDCrossLinkMS_Distance_interrb-State:1-5:particle1_5:particle3-1-1-0.05_None',
     'ISDCrossLinkMS_Distance_interrb-State:1-5:particle2_5:particle3-1-1-0.05_None',
     'SimplifiedModel_Total_Score_None',
     'ISDCrossLinkMS_Data_Score_None',
     'ISDCrossLinkMS_Linear_Score_None',
     'ISDCrossLinkMS_Psi_0.05_None'])

print(fs.keys())
o.close_rmf("trajectory.rmf3")

# <codecell>

for output in ['excluded.None.xl.db', 'included.None.xl.db',
               'missing.None.xl.db', 'modeling.stat', 'trajectory.rmf3']:
    os.unlink(output)
