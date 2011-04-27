import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.membrane

def setup_MonteCarlo(m,chain,TMH,temp):
    mc=IMP.core.MonteCarlo(m)
    mc.set_return_best(False)
    mc.set_kt(temp)
# create Movers and initialize the system
    rot=IMP.algebra.get_identity_rotation_3d()
    for i,h in enumerate(TMH):
        s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
        rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        if ( i == 0 ): mv= IMP.membrane.RigidBodyNewMover(rb, 0.0, 0.0, 0.5, 0.05, 0.05, 0.05)
        if ( i == 1 ): mv= IMP.membrane.RigidBodyNewMover(rb, 0.5, 0.0, 0.5, 0.05, 0.05, 0.05)
        if ( i > 1 ):  mv= IMP.membrane.RigidBodyNewMover(rb, 0.5, 0.5, 0.5, 0.05, 0.05, 0.05)
        mc.add_mover(mv)
        tr=IMP.algebra.Transformation3D(rot,IMP.algebra.Vector3D(i*12.0,0,0))
        IMP.core.transform(rb,tr)
    return mc
