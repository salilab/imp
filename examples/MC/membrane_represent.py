import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.spb
import math
import random

#parameters
from membrane_parameters import *

def create_representation(m):

    tbr= IMP.core.TableRefiner()
    all=IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))

    def generate_tm(id,name,res,topo,sfile):
        pm=IMP.Particle(m)
        tm=IMP.atom.Molecule.setup_particle(pm)
        tm.set_name(name)
        nres=res[1]-res[0]+1
        atoms=[]
        if (len(sfile)>0):
            helix=IMP.atom.read_pdb(sfile,m,IMP.atom.CAlphaPDBSelector())
            ps=helix.get_leaves()
            if(len(ps)!=nres): print "Check your pdb!"; exit()
        for i in range(nres):
            if(len(sfile)>0):
                aa=IMP.core.XYZR(ps[i])
                x=aa.get_coordinates()[0]
                y=aa.get_coordinates()[1]
                z=aa.get_coordinates()[2]
            else:
                x=2.3*math.cos(math.radians(100.0)*float(i))
                y=2.3*math.sin(math.radians(100.0)*float(i))
                z=1.51*(float(i)-float((nres-1))/2.0)
            # set up residue
            p=IMP.Particle(m)
            ires=i+res[0]
            r=IMP.atom.Residue.setup_particle(p, IMP.atom.get_residue_type(TM_seq[ires-1]), ires)
            rt=r.get_residue_type()
            if(len(sfile)>0 and IMP.atom.Residue(IMP.atom.Atom(ps[i]).get_parent()).get_residue_type() != rt):
                print "Check your pdb!"; exit()
            vol=IMP.atom.get_volume_from_residue_type(rt)
            rg=IMP.algebra.get_ball_radius_from_volume_3d(vol)
            #rg=2.273
            # set up atom
            p1=IMP.Particle(m)
            a=IMP.atom.Atom.setup_particle(p1, IMP.atom.AT_CA)
            ad=IMP.core.XYZR.setup_particle(p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(x,y,z),rg))
            r.add_child(a)
            tm.add_child(r)
            atoms.append(ad)
        if (len(sfile)>0): IMP.atom.destroy(helix)
        all.add_child(tm)
        prb=IMP.Particle(m)
        rb=IMP.core.RigidBody.setup_particle(prb,atoms)
        rb.set_name(name)
        tbr.add_particle(prb,atoms)
        bb=IMP.core.RigidMember(atoms[0]).get_internal_coordinates()[0]
        ee=IMP.core.RigidMember(atoms[-1]).get_internal_coordinates()[0]
        if ( topo * ( ee - bb ) < 0 ):
            for i,ps in enumerate(atoms):
                coord=IMP.core.RigidMember(ps).get_internal_coordinates()
                rot=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,0,1),math.pi)
                tr=IMP.algebra.Transformation3D(rot,IMP.algebra.Vector3D(0,0,0))
                IMP.core.RigidMember(ps).set_internal_coordinates(tr.get_transformed(coord))
        if ( id == 0 ): xx=0; yy=0;
        if ( id == 1 ): yy=0; xx=random.randrange(0, diameter_)
        if ( id > 1  ): xx=random.randrange(-diameter_, diameter_); yy=random.randrange(-diameter_, diameter_)
        rb.set_reference_frame(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D
        (IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0),-math.pi/2.0),
         IMP.algebra.Vector3D(xx,yy,0))))
        bb=IMP.core.RigidMember(atoms[0]).get_internal_coordinates()[0]
        ee=IMP.core.RigidMember(atoms[-1]).get_internal_coordinates()[0]
        d_rbs=IMP.spb.HelixDecorator.setup_particle(prb,bb,ee)

    for i in range(len(TM_names)):
        generate_tm(i,TM_names[i],TM_res[i],TM_topo[i],TM_struct[i])
    return all,tbr
