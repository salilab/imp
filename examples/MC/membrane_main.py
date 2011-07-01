import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import IMP.membrane
import IMP.rmf
import math

#parameters
from membrane_parameters import *
#representation
from membrane_represent  import *
#restraints
from membrane_restraints import *
#sampler
from membrane_MonteCarlo import *

m=IMP.Model()

print "creating representation"
(protein,tbr)=create_representation(m)

print "creating restraints"
rset=create_restraints(m,protein,tbr)

print "creating sampler"
mc=setup_MonteCarlo(m,protein)

# preparing hdf5 file
rh = IMP.rmf.RootHandle(mc_traj_file, True)
#rh = IMP.rmf.RootHandle(mc_traj_file, False)
for hs in protein.get_children():
    IMP.rmf.add_hierarchy(rh, hs)
#IMP.rmf.set_hierarchies(rh, protein.get_children())

print "sampling"
for steps in range(mc_loops):
    mc.optimize(mc_steps)
    print steps, m.evaluate(False), mc_steps, mc.get_number_of_forward_steps()
    for hs in protein.get_children():
        IMP.rmf.save_frame(rh, steps+1, hs)
    #    IMP.rmf.load_frame(rh, steps+1, hs)

# close file
#del rh
