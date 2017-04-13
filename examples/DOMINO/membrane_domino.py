import IMP
import IMP.domino
import math

#parameters
from membrane_parameters import *

def create_states(protein):
    pst=IMP.domino.ParticleStatesTable()
    rot0=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), -math.pi/2.0)
    rbs=IMP.spb.RigidBodyGridStates((-diameter_,-diameter_,-5.0,0.0,0.0,0.0),(diameter_,diameter_,5.0,315.0,45.0,315.0),(grid_Dx,grid_Dx,grid_Dx,45.0,15.0,45.0),-math.pi/2.0)
    for i,h in enumerate(TM_res):
        s0=IMP.atom.Selection(protein, atom_type = IMP.atom.AT_CA, residue_index = h[0])
        rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        pst.set_particle_states(rb,rbs)
    return pst

def create_sampler(m, rset, pst):
    s=IMP.domino.DominoSampler(m, pst)
    ig= IMP.domino.get_interaction_graph(rset, pst)
    jt= IMP.domino.get_junction_tree(ig)
    print ig,jt
    exit()
    #IMP.show_graphviz(ig)
    #IMP.show_graphviz(jt)
    filters=[]
    filters.append(IMP.domino.ExclusionSubsetFilterTable(pst))
    filters.append(IMP.domino.RestraintScoreSubsetFilterTable(m, pst))
    states= IMP.domino.BranchAndBoundAssignmentsTable(pst, filters)
    s.set_assignments_table(states)
    s.set_subset_filter_tables(filters)
    return s
