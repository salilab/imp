import IMP
import IMP.domino
import math

#parameters
from membrane_parameters import *

def create_states(protein,phase):
    pst=IMP.domino.ParticleStatesTable()
    rot0=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), math.pi/2.0)
    trs=[]
    for dx in range(-grid_ix,grid_ix+1):
        xx=float(dx)*grid_Dx
        for dy in range(-grid_iy,grid_iy+1):
            yy=float(dy)*grid_Dx
            rg=math.sqrt(xx**2+yy**2)
            if ( rg > diameter_/2.0 ): continue
            for dz in range(-grid_iz,grid_iz+1):
                zz=float(dz)*grid_Dx
                for i in range(grid_irot):
                    rotz=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,0,1), float(i)*grid_Drot)
                    for t in range(grid_itilt+1):
                        tilt=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), float(t)*grid_Dtilt)
                        rot1=IMP.algebra.compose(tilt,rotz)
                        for s in range(grid_iswing):
                            if ( t == 0 ) and ( s != 0 ) : break
                            swing=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,0,1), float(s)*grid_Dswing)
                            rot2=IMP.algebra.compose(swing,rot1)
                            frame=IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.compose(rot2,rot0),
                                    IMP.algebra.Vector3D(xx,yy,zz)))
                            trs.append(frame)
    rbs=IMP.domino.RigidBodyStates(trs)
    for h in TM_res:
        s0=IMP.atom.Selection(protein, atom_type = IMP.atom.AT_CA, residue_index = h[0])
        rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        pst.set_particle_states(rb,rbs)
    return pst

def create_sampler(m, rset, pst):
    s=IMP.domino.DominoSampler(m, pst)
    #ig= IMP.domino.get_interaction_graph(rset, pst)
    #jt= IMP.domino.get_junction_tree(ig)
    #print ig,jt
    #IMP.show_graphviz(ig)
    #IMP.show_graphviz(jt)
    filters=[]
    filters.append(IMP.domino.ExclusionSubsetFilterTable(pst))
    filters.append(IMP.domino.RestraintScoreSubsetFilterTable(m, pst))
    states= IMP.domino.BranchAndBoundAssignmentsTable(pst, filters)
    s.set_assignments_table(states)
    s.set_subset_filter_tables(filters)
    return s
