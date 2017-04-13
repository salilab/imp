import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import IMP.spb
import IMP.rmf
import math

#parameters
from membrane_parameters import *
#representation
from membrane_represent  import *
#restraints
from membrane_restraints import *
#domino stuff
from membrane_domino     import *

m=IMP.Model()

print "creating representation"
(protein,tbr)=create_representation(m)

print "creating restraints"
rset=create_restraints(m,protein,tbr)

print "creating states"
pst=create_states(protein)

print "creating sampler"
s=create_sampler(m,rset,pst)

print "sampling"
ass=IMP.domino.Subset(pst.get_particles())

cs=s.get_sample_assignments(ass)

print "found ",len(cs)," solutions"

# open the file and clear any existing contents
rt= IMP.rmf.HDF5Group(ass_file, True)
#rt= IMP.rmf.HDF5Group(ass_file, False)

# add a data set to store it in, it must have dimension 2
data_set= rt.add_child_index_data_set("node_1_assignments", 2)
#data_set= rt.get_child_index_data_set("node_1_assignments", 2)

# save assignments to file
IMP.domino.set_assignments(data_set, cs, ass, pst.get_particles())
#cs=IMP.domino.get_assignments(data_set, ass, pst.get_particles())
#print "found ",len(cs)," solutions"
