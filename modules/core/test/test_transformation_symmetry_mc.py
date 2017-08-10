import IMP
import IMP.container
import IMP.atom
import IMP.rmf
import RMF
from math import *
import IMP.test
import itertools

"""
This tests that NonRigidMembers, along with other invariants,
are correctly updated when a TransformationSymmetry is applied in a MonteCarlo trajectory
"""

class RigidBodyTransformation(object):
    '''Utility class to keep track of RB transformations'''
    def __init__(self,rb):
        self.rb=rb
        self.set_initial()
        self.set_final()

    def set_initial(self):
        self.r0=self.rb.get_coordinates()
        self.rot0=self.rb.get_rotation()

    def set_final(self):
        self.r1=self.rb.get_coordinates()
        self.rot1=self.rb.get_rotation()

    def get_translation(self):
        '''Returns the magnitude'''
        return (self.r1-self.r0).get_magnitude()

    def get_rotation(self):
        '''Returns the angle and axis'''
        q=(self.rot1/self.rot0).get_quaternion()
        return get_angle_and_axis(q)

class RigidBodyPairTransformation(object):
    '''Utility class to monitor transformations between RBs'''
    def __init__(self,rb0,rb1):
        self.rb0=rb0
        self.rb1=rb1

    def get_translation(self):
        '''Returns the magnitude'''
        self.r0=self.rb0.get_coordinates()
        self.r1=self.rb1.get_coordinates()
        return (self.r1-self.r0).get_magnitude()

    def get_rotation(self):
        '''Returns the angle and axis'''
        self.rot0=self.rb0.get_rotation()
        self.rot1=self.rb1.get_rotation()
        q=(self.rot1/self.rot0).get_quaternion()
        return get_angle_and_axis(q)

def get_angle_and_axis(q):
    '''get angle and axis from quaternion'''
    try:
        angle = 2 * acos(q[0])
    except:
        return (None,None)
    if angle != 0:
        x = q[1] / sqrt(1-q[0]*q[0])
        y = q[2] / sqrt(1-q[0]*q[0])
        z = q[3] / sqrt(1-q[0]*q[0])
        return (angle,(x,y,z))
    else:
        return (0,None)

def get_rbs_and_beads(hiers):
    """Returns rbs and fbs objects in original order"""
    rbs = set()
    beads = []
    rbs_ordered = []
    if not hasattr(hiers,'__iter__'):
        hiers = [hiers]
    for p in hiers:
        if IMP.core.RigidMember.get_is_setup(p):
            rb = IMP.core.RigidMember(p).get_rigid_body()
            if rb not in rbs:
                rbs.add(rb)
                rbs_ordered.append(rb)
        elif IMP.core.NonRigidMember.get_is_setup(p):
            rb = IMP.core.NonRigidMember(p).get_rigid_body()
            if rb not in rbs:
                rbs.add(rb)
                rbs_ordered.append(rb)
            beads.append(p)
        else:
            beads.append(p)
    return rbs_ordered,beads

def add_symmetry(clones,refs,transform,mdl):
    '''setup the simmetry from the clones'''

    ref_rbs,ref_beads = get_rbs_and_beads(refs)
    clones_rbs,clones_beads = get_rbs_and_beads(clones)

    for ref,clone in zip(ref_rbs,clones_rbs):
        IMP.core.Reference.setup_particle(clone,ref)

    for ref,clone in zip(ref_beads,clones_beads):
        IMP.core.Reference.setup_particle(clone,ref)

    sm = IMP.core.TransformationSymmetry(transform)
    lsc = IMP.container.ListSingletonContainer(mdl,clones_rbs+clones_beads)
    c = IMP.container.SingletonsConstraint(sm, None, lsc)

    mdl.add_score_state(c)

class TestSymmetryMC(IMP.test.TestCase):

    def test_complete(self):

        mdl=IMP.Model()
        root=IMP.atom.Hierarchy(IMP.Particle(mdl))

        #need some data structure

        #rigidbody->particles
        rbs_ps={}
        #floppy bodies
        fbs=[]
        #rigidbodies
        rbs=[]
        #particles->movers
        ps_mv={}
        #movers
        mvs=[]
        #particles by clone
        pss=[]

        #setting up Hierarchy and RigidBodies with movers
        for mol in range(10):
            mol=IMP.atom.Hierarchy(IMP.Particle(mdl))
            root.add_child(mol)
            ps=[]
            for c in [(0,0,0),(5,0,0),(0,5,0)]:
                p=IMP.Particle(mdl)
                dr=IMP.core.XYZR.setup_particle(p)
                dr.set_coordinates(c)
                dr.set_radius(1.0)
                IMP.atom.Mass.setup_particle(p,1.0)
                h=IMP.atom.Hierarchy(p)
                mol.add_child(h)
                ps.append(p)
            rb = IMP.atom.create_rigid_body(IMP.atom.get_leaves(mol))
            rb_mover = IMP.core.RigidBodyMover(rb.get_model(),rb,1,0.5)
            mvs.append(rb_mover)
            rbs_ps[rb]=ps
            rbs.append(rb)
            pss.append(ps)
            for p in ps:
                if p in ps_mv:
                    ps_mv[p].append(rb_mover)
                else:
                    ps_mv[p]=[rb_mover]

        #setup NonRigidMembers with movers
        floatkeys = [IMP.FloatKey(4), IMP.FloatKey(5), IMP.FloatKey(6)]
        for rb in rbs_ps:
            for p in rbs_ps[rb][1:2]:
                rb.set_is_rigid_member(p.get_index(),False)
                for fk in floatkeys:
                    p.set_is_optimized(fk,True)
                fbmv=IMP.core.BallMover(p.get_model(),p,IMP.FloatKeys(floatkeys),1.0)
                mvs.append(fbmv)
                fbs.append(p)
                if p in ps_mv:
                    ps_mv[p].append(fbmv)
                else:
                    ps_mv[p]=[fbmv]


        # enforce rotational symmetry
        # while removing movers from the clones
        rotational_axis=IMP.algebra.Vector3D(-0.98477941,0.15168411,-0.08486137)
        rotational_point=IMP.algebra.Vector3D(0,0,0)
        disabled_movers=set()
        for n,c in enumerate(pss[1:]):
            rotation_angle = 2.0 * pi / len(pss) * float(n+1)
            rotation3D = IMP.algebra.get_rotation_about_axis(rotational_axis, rotation_angle)
            transformation3D =IMP.algebra.get_rotation_about_point(rotational_point, rotation3D)
            add_symmetry(c,pss[0],transformation3D,mdl)
            for p in c:
                for mv in ps_mv[p]:
                    disabled_movers.add(mv)

        mdl.update()
        enabled_movers=[]
        for mv in mvs:
            if mv in disabled_movers:
                continue
            else:
                enabled_movers.append(mv)


        #create EV restraint
        ssps = IMP.core.SoftSpherePairScore(1.0)
        lsa = IMP.container.ListSingletonContainer(mdl)
        lsa.add(IMP.get_indexes(IMP.atom.get_leaves(root)))
        rbcpf = IMP.core.RigidClosePairsFinder()
        cpc = IMP.container.ClosePairContainer(lsa, 0.0, rbcpf, 10.0)
        evr = IMP.container.PairsRestraint(ssps, cpc)

        #setup Score, MC, etc
        sf = IMP.core.RestraintsScoringFunction([evr])
        mc = IMP.core.MonteCarlo(mdl)
        mc.set_scoring_function(sf)

        sm = IMP.core.SerialMover(enabled_movers)
        mc.add_mover(sm)
        mc.set_return_best(False)
        mc.set_kt(1.0)

        rh = RMF.create_rmf_file("out.rmf")
        IMP.rmf.add_hierarchies(rh, [root])
        IMP.rmf.save_frame(rh)

        #data structure to store transformation classes
        rb_rbt={}
        rb_rbpt={}
        for rb in rbs:
            rbt=RigidBodyTransformation(rb)
            rb_rbt[rb]=rbt
            if rb != rbs[0]:
                rbpt=RigidBodyPairTransformation(rbs[0],rb)
                rb_rbpt[rb]=rbpt

        #data structure to store all output
        rb_rbpt_values={}
        rb_rbpt_values["trans"]={}
        rb_rbpt_values["angle"]={}
        rb_rbpt_values["axis_x"]={}
        rb_rbpt_values["axis_y"]={}
        rb_rbpt_values["axis_z"]={}

        for i in range(10):
            mc.optimize(10)
            mdl.update()

            #first check that all translations and rotation magnitudes are identical
            trans_list=[]
            rot_list=[]
            for i,(rb,rbt) in enumerate(rb_rbt.items()):
                rbt.set_final()
                trans_list.append(rbt.get_translation())
                rot_list.append(rbt.get_rotation()[0])
                rbt.set_initial()

            for a,b in itertools.combinations(trans_list,2):
                self.assertAlmostEqual(a,b,places=7)
            for a,b in itertools.combinations(rot_list,2):
                self.assertAlmostEqual(a,b,places=7)

            #second store the pairwise transformations
            for i,(rb,rbpt) in enumerate(rb_rbpt.items()):
                trans=rbpt.get_translation()
                (angle,axis)=rbpt.get_rotation()
                for key in rb_rbpt_values:
                    if rb not in rb_rbpt_values[key]: rb_rbpt_values[key][rb]=[]
                    #if key=="trans": rb_rbpt_values[key][rb].append(trans)
                    if key=="angle": rb_rbpt_values[key][rb].append(angle)
                    if key=="axis_x": rb_rbpt_values[key][rb].append(abs(axis[0]))
                    if key=="axis_y": rb_rbpt_values[key][rb].append(abs(axis[1]))
                    if key=="axis_z": rb_rbpt_values[key][rb].append(abs(axis[2]))


            #third check the NonRigidMember internal coordinates:
            ic_xs=[]
            ic_ys=[]
            ic_zs=[]
            for fb in fbs:
                ic=IMP.core.NonRigidMember(fb).get_internal_coordinates()
                ic_xs.append(ic[0])
                ic_ys.append(ic[1])
                ic_zs.append(ic[2])
            for a,b in itertools.combinations(ic_xs,2):
                self.assertAlmostEqual(a,b,places=7)
            for a,b in itertools.combinations(ic_ys,2):
                self.assertAlmostEqual(a,b,places=7)
            for a,b in itertools.combinations(ic_zs,2):
                self.assertAlmostEqual(a,b,places=7)

            IMP.rmf.save_frame(rh)
            rh.flush()

        # last check the consistency of pairwise rotations along the trajectory
        for key in rb_rbpt_values:
            for rb in rb_rbpt_values[key]:
                for a,b in itertools.combinations(rb_rbpt_values[key][rb],2):
                    self.assertAlmostEqual(a,b,places=7)
        del rh


if __name__ == '__main__':
    IMP.test.main()
