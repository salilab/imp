import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.spb

#parameters
from membrane_parameters import *

def setup_MonteCarlo(m,protein):
    if(mc_wte): mc=IMP.spb.MonteCarloWithWte(m,-400.0,0.0,20.0,16.0,0.001)
    else:       mc=IMP.core.MonteCarlo(m)
    mc.set_return_best(False)
    mc.set_kt(mc_kt)
# create Movers
    for i,h in enumerate(TM_res):
        s0=IMP.atom.Selection(protein, atom_type = IMP.atom.AT_CA, residue_index = h[0])
        rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        if ( i == 0 ): mv= IMP.spb.RigidBodyNewMover(rb,   0.0,   0.0, mc_dx, mc_dang)
        if ( i == 1 ): mv= IMP.spb.RigidBodyNewMover(rb, mc_dx,   0.0, mc_dx, mc_dang)
        if ( i > 1 ):  mv= IMP.spb.RigidBodyNewMover(rb, mc_dx, mc_dx, mc_dx, mc_dang)
        mc.add_mover(mv)
    return mc
