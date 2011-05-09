import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.membrane
import math

#parameters
from membrane_parameters import *

def create_representation(m):

    tbr= IMP.core.TableRefiner()
    all=IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))

    def read_tm(id,seq,name,res,sign):
        pm=IMP.Particle(m)
        tm=IMP.atom.Molecule.setup_particle(pm)
        tm.set_name(name)
        nres=len(seq)
        helix=IMP.atom.read_pdb("TM"+str(id)+".pdb",m,IMP.atom.CAlphaPDBSelector())
        atoms=[]
        for i,ps in enumerate(helix.get_leaves()):
            aa=IMP.core.XYZR(ps)
            x=aa.get_coordinates()[0]
            y=aa.get_coordinates()[1]
            z=aa.get_coordinates()[2]
            # set up residue
            p=IMP.Particle(m)
            r=IMP.atom.Residue.setup_particle(p, IMP.atom.get_residue_type(seq[i]), i+res[0])
            rt=r.get_residue_type()
            vol=IMP.atom.get_volume_from_residue_type(rt)
            rg=IMP.algebra.get_ball_radius_from_volume_3d(vol)
            #rg=2.273
            # set up atom
            p1=IMP.Particle(m)
            a=IMP.atom.Atom.setup_particle(p1, IMP.atom.AT_CA)
            ad=IMP.core.XYZR.setup_particle(p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(x,y,z),rg))
            if (i == 0):      begin=p1
            if (i == nres-1): end=p1
            r.add_child(a)
            tm.add_child(r)
            atoms.append(ad)
        IMP.atom.destroy(helix)
        all.add_child(tm)
        rb=IMP.Particle(m)
        rb=IMP.atom.create_rigid_body(atoms,name)
        tbr.add_particle(rb,atoms)
        # initialize helix decorator
        bb=IMP.core.RigidMember(begin).get_internal_coordinates()[0]
        ee=IMP.core.RigidMember(end).get_internal_coordinates()[0]
        d_rbs=IMP.membrane.HelixDecorator.setup_particle(rb,bb,ee)
        if ( sign * ( ee - bb ) > 0 ): rot0= -math.pi/2.0
        else :                         rot0=  math.pi/2.0
        # initialize system to match topology
        if ( sign  < 0 ): rot=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0),math.pi)
        else            : rot=IMP.algebra.get_identity_rotation_3d()
        tr=IMP.algebra.Transformation3D(rot,IMP.algebra.Vector3D(id*15.0,0,0))
        IMP.core.transform(rb,tr)
        return rot0

    def generate_tm(id,seq,name,res,sign):
        pm=IMP.Particle(m)
        tm=IMP.atom.Molecule.setup_particle(pm)
        tm.set_name(name)
        nres=len(seq)
        atoms=[]
        for i in range(nres):
            x=2.3*math.cos(math.radians(100.0)*float(i))
            y=2.3*math.sin(math.radians(100.0)*float(i))
            z=1.51*(float(i)-float((nres-1))/2.0)
            # set up residue
            p=IMP.Particle(m)
            r=IMP.atom.Residue.setup_particle(p, IMP.atom.get_residue_type(seq[i]), i+res[0])
            rt=r.get_residue_type()
            vol=IMP.atom.get_volume_from_residue_type(rt)
            rg=IMP.algebra.get_ball_radius_from_volume_3d(vol)
            #rg=2.273
            # set up atom
            p1=IMP.Particle(m)
            a=IMP.atom.Atom.setup_particle(p1, IMP.atom.AT_CA)
            ad=IMP.core.XYZR.setup_particle(p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(x,y,z),rg))
            if (i == 0):      begin=p1
            if (i == nres-1): end=p1
            r.add_child(a)
            tm.add_child(r)
            atoms.append(ad)
        all.add_child(tm)
        rb=IMP.Particle(m)
        rb=IMP.atom.create_rigid_body(atoms,name)
        tbr.add_particle(rb,atoms)
        # initialize helix decorator
        bb=IMP.core.RigidMember(begin).get_internal_coordinates()[0]
        ee=IMP.core.RigidMember(end).get_internal_coordinates()[0]
        d_rbs=IMP.membrane.HelixDecorator.setup_particle(rb,bb,ee)
        if ( sign * ( ee - bb ) > 0 ): rot0= -math.pi/2.0
        else :                         rot0=  math.pi/2.0
        # initialize system to match topology
        if ( sign  < 0 ): rot=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0),math.pi)
        else            : rot=IMP.algebra.get_identity_rotation_3d()
        tr=IMP.algebra.Transformation3D(rot,IMP.algebra.Vector3D(id*15.0,0,0))
        IMP.core.transform(rb,tr)
        return rot0

    rot0=[]
    for i in range(len(TM_seq)):
        rot=generate_tm(i,TM_seq[i],TM_names[i],TM_res[i],TM_topo[i])
        #rot=read_tm(i,TM_seq[i],TM_names[i],TM_res[i],TM_topo[i])
        rot0.append(rot)
    return all,tbr,rot0
