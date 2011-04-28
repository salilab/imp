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
#domino stuff
from membrane_domino     import *

m=IMP.Model()

print "creating representation"
(protein,tbr,rot0)=create_representation(m)

print "creating restraints"
rset=create_restraints(m,protein,tbr,rot0)

print "computing IG and JT"
(ig,jt)=get_graphs(m,protein,rset)
print ig,jt

print "creating sampler"
mc_tilt=setup_MonteCarlo_1(m,protein)
mc=setup_MonteCarlo_2(m,protein)

# short run to adjust the topology
mc_tilt.optimize(500)

# preparing hdf5 file
rh = IMP.rmf.RootHandle(mc_traj_file, True)
#rh = IMP.rmf.RootHandle(mc_traj_file, False)
for hs in protein.get_children():
    IMP.rmf.add_hierarchy(rh, hs)
#IMP.rmf.set_hierarchies(rh, protein.get_children())

print "sampling"
for steps in range(mc_loops):
    mc.optimize(mc_steps)
    #print steps, m.evaluate(False)
    #display(m,protein,TMH,"conf_"+str(steps)+".score_"+str(score)+".pym")
    for hs in protein.get_children():
        IMP.rmf.save_frame(rh, steps+1, hs)
    #    IMP.rmf.load_frame(rh, steps+1, hs)

# close file
del rh
