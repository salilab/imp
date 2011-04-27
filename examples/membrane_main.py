import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import IMP.membrane
import IMP.rmf
import math

#representation
from membrane_represent  import *
#restraints
from membrane_restraints import *
#sampler
from membrane_MonteCarlo import *
#domino stuff
from membrane_domino     import *

m=IMP.Model()

print "reading protein data"
(seq,names,TMH,topo,TM_inter)=setup_protein()

print "creating representation"
(chain,tbr,rot0)=create_representation(m,seq,names,TMH,topo)

print "creating restraints"
rset=create_restraints(m,chain,tbr,TMH,rot0,topo,TM_inter)

print "visualizing IG and JT"
(ig,jt)=get_graphs(m,chain,TMH,rset)
print ig,jt

print "creating sampler"
mc=setup_MonteCarlo(m,chain,TMH,mc_kt)

# preparing hdf5 file
rh = IMP.rmf.RootHandle(mc_traj_file, True)
#rh = IMP.rmf.RootHandle(mc_traj_file, False)
for hs in chain.get_children():
    IMP.rmf.add_hierarchy(rh, hs)
#IMP.rmf.set_hierarchies(rh, chain.get_children())

print "sampling"
for steps in range(mc_loops):
    mc.optimize(mc_steps)
    #print steps. m.evaluate(False)
    #display(m,chain,TMH,"conf_"+str(steps)+".score_"+str(score)+".pym")
    for hs in chain.get_children():
        IMP.rmf.save_frame(rh, steps+1, hs)
    #    IMP.rmf.load_frame(rh, steps+1, hs)

# close file
del rh
