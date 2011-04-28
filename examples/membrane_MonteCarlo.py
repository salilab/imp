import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.membrane

#parameters
from membrane_parameters import *

def setup_MonteCarlo_1(m,protein):
    mc=IMP.core.MonteCarlo(m)
    mc.set_return_best(False)
    mc.set_kt(mc_kt)
# create Movers and initialize the system
    rot=IMP.algebra.get_identity_rotation_3d()
    for i,h in enumerate(TM_res):
        s0=IMP.atom.Selection(protein, atom_type = IMP.atom.AT_CA, residue_index = h[0])
        rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        mv= IMP.membrane.RigidBodyNewMover(rb, 0.0, 0.0, 0.0, 0.0, 0.2, 0.0)
        mc.add_mover(mv)
        tr=IMP.algebra.Transformation3D(rot,IMP.algebra.Vector3D(i*30.0,0,0))
        IMP.core.transform(rb,tr)
    return mc

def setup_MonteCarlo_2(m,protein):
    mc=IMP.core.MonteCarlo(m)
    mc.set_return_best(False)
    mc.set_kt(mc_kt)
# create Movers
    for i,h in enumerate(TM_res):
        s0=IMP.atom.Selection(protein, atom_type = IMP.atom.AT_CA, residue_index = h[0])
        rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        if ( i == 0 ): mv= IMP.membrane.RigidBodyNewMover(rb, 0.0, 0.0, 1.0, 0.2, 0.2, 0.2)
        if ( i == 1 ): mv= IMP.membrane.RigidBodyNewMover(rb, 1.0, 0.0, 1.0, 0.2, 0.2, 0.2)
        if ( i > 1 ):  mv= IMP.membrane.RigidBodyNewMover(rb, 1.0, 1.0, 1.0, 0.2, 0.2, 0.2)
        mc.add_mover(mv)
    return mc
