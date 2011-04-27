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

# TM regions
TMH= [[24,48], [75,94], [220,238]]#, [254,276]]

# define TMH sequences
seq0=("M","L","I","H","N","W","I","L","T","F","S","I","F","R","E","H","P","S","T","V","F","Q","I","F","T","K","C","I","L","V","S","S","S","F","L","L","F","Y","T","L","L","P","H","G","L","L","E","D","L","M","R","R","V","G","D","S","L","V","D","L","I","V","I","C","E","D","S","Q","G","Q","H","L","S","S","F","C","L","F","V","A","T","L","Q","S","P","F","S","A","G","V","S","G","L","C","K","A","I","L","L","P","S","K","Q","I","H","V","M","I","Q","S","V","D","L","S","I","G","I","T","N","S","L","T","N","E","Q","L","C","G","F","G","F","F","L","N","V","K","T","N","L","H","C","S","R","I","P","L","I","T","N","L","F","L","S","A","R","H","M","S","L","D","L","E","N","S","V","G","S","Y","H","P","R","M","I","W","S","V","T","W","Q","W","S","N","Q","V","P","A","F","G","E","T","S","L","G","F","G","M","F","Q","E","K","G","Q","R","H","Q","N","Y","E","F","P","C","R","C","I","G","T","C","G","R","G","S","V","Q","C","A","G","L","I","S","L","P","I","A","I","E","F","T","Y","Q","L","T","S","S","P","T","C","I","V","R","P","W","R","F","P","N","I","F","P","L","I","A","C","I","L","L","L","S","M","N","S","T","L","S","L","F","S","F","S","G","G","R","S","G","Y","V","L","M","L","S","S","K","Y","Q","D","S","F","T","S","K","T","R","N","K","R","E","N","S","I","F","F","L","G","L","N","T","F","T","D","F","R","H","T","I","N","G","P","I","S","P","L","M","R","S","L","T","R","S","T","V","E")

# storing sequences of TMH
seq=[]
for h in TMH:
    tmp=[]
    for j in range(h[0],h[1]+1):
        tmp.append(seq0[j-1])
    seq.append(tmp)

# define the topology
topo=[1.0,-1.0,1.0]#,-1.0]

# name of the TMH
names=["TM0","TM1","TM2"]#,"TM3"]

m=IMP.Model()

print "creating representation"
(chain,tbr,rot0)=create_representation(m,seq,names,TMH,topo)

print "creating restraints"
rset=create_restraints(m,chain,tbr,TMH,rot0,topo)

print "creating sampler"
kt=1.0
mc=setup_MonteCarlo(m,chain,TMH,kt)

# preparing hdf5 file
tfn= "traj.hdf5"
rh = IMP.rmf.RootHandle(tfn, True)
#rh = IMP.rmf.RootHandle(tfn, False)
for hs in chain.get_children():
    IMP.rmf.add_hierarchy(rh, hs)
#IMP.rmf.set_hierarchies(rh, chain.get_children())

print "sampling"
for steps in range(100):
    mc.optimize(100)
    #print steps. m.evaluate(False)
    #display(m,chain,TMH,"conf_"+str(steps)+".score_"+str(score)+".pym")
    for hs in chain.get_children():
        IMP.rmf.save_frame(rh, steps+1, hs)
    #    IMP.rmf.load_frame(rh, steps+1, hs)

# close file
del rh
