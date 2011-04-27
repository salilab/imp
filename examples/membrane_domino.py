import IMP
import IMP.domino

def get_graphs(m,protein,TMH,rset):
# setup fake pst
    pst=IMP.domino.ParticleStatesTable()
    rbs=IMP.domino.RigidBodyStates([IMP.algebra.ReferenceFrame3D()])
    for i,h in enumerate(TMH):
        s0=IMP.atom.Selection(protein, atom_type = IMP.atom.AT_CA, residue_index = h[0])
        rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        pst.set_particle_states(rb, rbs)
    ig= IMP.domino.get_interaction_graph(rset, pst)
    jt= IMP.domino.get_junction_tree(ig)
    return ig,jt
