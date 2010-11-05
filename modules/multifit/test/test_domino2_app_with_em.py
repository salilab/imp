import IMP
import IMP.test
import IMP.bullet
import IMP.core
import IMP.algebra
import IMP.domino2
import StringIO
import IMP.em
import math

class MultiFitDominoTests(IMP.test.TestCase):
    """Test domino2 enumeration of rigid bodies"""
    def local_random(self,rb):
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-5,-5,-5),
                                      IMP.algebra.Vector3D(5,5,5))
        t=IMP.algebra.Transformation3D(#IMP.algebra.get_identity_rotation_3d(),
                                       IMP.algebra.get_random_rotation_3d(IMP.algebra.get_identity_rotation_3d(), .4),
                                       IMP.algebra.get_random_vector_in(bb))
        IMP.core.transform(rb,t)
    def create_restraints(self,m, ps):
        pairs=[[0,1],[0,2],[1,3],[0,3]]
        #hsdps= IMP.core.HarmonicSphereDistancePairScore(0, 1)
        hsdps=IMP.core.SphereDistancePairScore(IMP.core.HarmonicUpperBound(0,1))
        sdps= IMP.core.KClosePairsPairScore(hsdps,
                                           IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
        # the restraint will be broken apart during optimization
        pc= IMP.container.ListPairContainer([(ps[p[0]], ps[p[1]]) for p in pairs],
                                            "Restrained pairs")
        self.pr= IMP.container.PairsRestraint(sdps, pc)
        self.pr.set_name("interactions")
        #m.set_maximum_score(self.pr, 10)
        m.add_restraint(self.pr)
        dmap=IMP.em.read_map(self.get_input_file_name("1z5s_20.mrc"))
        dmap.get_header().set_resolution(20)
        self.r_em=IMP.em.FitRestraint(ps,dmap,IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
        m.add_restraint(self.r_em)
        print "done"


    def create(self):
        m= IMP.Model()
        ss= IMP.SetLogState(IMP.SILENT)
        h=IMP.atom.read_pdb(self.get_input_file_name("1z5s.pdb"),m)
        h_ref=IMP.atom.read_pdb(self.get_input_file_name("1z5s.pdb"),m)
        del ss
        ps=[]
        ps_ref=[]
        rbs=[]
        self.rbs_orig_trans=[]
        all_atoms=[]
        print "creating rbs"
        gs=[]
        for i,cr in enumerate(IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)):
            c= IMP.atom.create_simplified_along_backbone(IMP.atom.Chain(cr),3)
            c1= IMP.atom.create_simplified_along_backbone(IMP.atom.Chain(cr),3)
            ps.append(c)
            ps_ref.append(c1)
            all_atoms+=IMP.core.get_leaves(c)
            rbs.append(IMP.atom.create_rigid_body(c))
            self.rbs_orig_trans.append(rbs[-1].get_transformation())
            color= IMP.display.get_display_color(i)
            for l in IMP.atom.get_leaves(c):
                g= IMP.display.XYZRGeometry(l)
                g.set_color(color)
                # objects with the same name are grouped in pymol
                gs.append(g)
        w= IMP.display.ChimeraWriter("config.chimera.py")
        for g in gs:
            w.add_geometry(g)
        IMP.atom.destroy(h)
        crbs= IMP.container.ListSingletonContainer(ps)
        aprbs= IMP.container.AllPairContainer(crbs)
        rdps= IMP.core.KClosePairsPairScore(IMP.core.SoftSpherePairScore(1),
                                           IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
        #add an excluded volume restraint
        self.r_ev= IMP.container.PairsRestraint(rdps, aprbs)
        self.r_ev.set_name("EV")
        m.add_restraint(self.r_ev)
        m.set_maximum_score(self.r_ev, 50)
        return (m, ps, rbs,ps_ref)
    def load_transformations(self, rb):
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-5,-5,-5),
                                      IMP.algebra.Vector3D(5,5,5))
        trans=[]
        trans.append(rb.get_transformation())
        for i in range(5):
            t=IMP.algebra.Transformation3D(
                IMP.algebra.get_random_rotation_3d(IMP.algebra.get_identity_rotation_3d(), .4),
                IMP.algebra.get_random_vector_in(bb))
            IMP.core.transform(rb,t)
            trans.append(rb.get_transformation())
            IMP.core.transform(rb,t.get_inverse())
        return trans

    def get_display(self, ps):
        return IMP.atom.WritePDBOptimizerState(ps,"temp_%03d.pdb")
    def test_rcos(self):
        """Test domino2 working for multifit"""
        print "===creating"
        (m, ps, rbs,ps_ref)= self.create()
        m.set_log_level(IMP.SILENT)
        print "create restraints"
        self.create_restraints(m,ps)
        print "MODEL SCORE:",
        initial_score= m.evaluate(None)
        print initial_score
        #add restraints
        #setup domino
        #create discrete states
        pst= IMP.domino2.ParticleStatesTable()
        for p in rbs:
            trans=self.load_transformations(p)
            states=IMP.domino2.RigidBodyStates(trans)
            pst.set_particle_states(IMP.core.RigidBody(p.get_particle()), states)
        #create sampler
        oc= IMP.domino2.OptimizeContainers(m.get_root_restraint_set(), pst)
        ocr= IMP.domino2.OptimizeRestraints(m.get_root_restraint_set(), pst)
        allrs= IMP.get_restraints(m.get_root_restraint_set())
        for r in allrs:
            print r.get_name()
        for p in pst.get_particles():
            print p.get_name()
        print "interaction graph"
        ig= IMP.domino2.get_interaction_graph(m.get_root_restraint_set(), pst)
        ig.show()
        print "restraint graph"
        rg= IMP.domino2.get_restraint_graph(m.get_root_restraint_set(), pst)
        rg.show()
        s=IMP.domino2.DominoSampler(m, pst)
        s.set_log_level(IMP.TERSE)
        print pst
        print "going to sample"
        s.set_maximum_score(initial_score)
        cs=s.get_sample()

        print "found ", cs.get_number_of_configurations(), "solutions"
        h=IMP.atom.read_pdb(self.get_input_file_name("1z5s.pdb"),m)
        h_ref=IMP.atom.read_pdb(self.get_input_file_name("1z5s.pdb"),m)
        xyz=IMP.core.XYZs(IMP.core.get_leaves(h))
        xyz_ref=IMP.core.XYZs(IMP.core.get_leaves(h_ref))
        test_rbs=[]
        test_rbs_orig_trans=[]
        for cr in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
            test_rbs.append(IMP.atom.create_rigid_body(cr))
            test_rbs_orig_trans.append(test_rbs[-1].get_transformation())
        best_rmsd=[999,0]
        best_score=[999,0]
        for i in range(cs.get_number_of_configurations()):
            cs.load_configuration(i)
            cur_trans=[]
            for j,rb in enumerate(rbs):
                cur_trans.append(rb.get_transformation()/self.rbs_orig_trans[j]*test_rbs_orig_trans[j])
                test_rbs[j].set_transformation(cur_trans[-1])
            fn="./model_"+str(i)+".pdb"
            #IMP.atom.write_pdb(h,fn)
            rmsd=IMP.atom.get_rmsd(IMP.core.XYZs(xyz),IMP.core.XYZsTemp(xyz_ref))
            if best_rmsd[0]>rmsd:
                best_rmsd=[rmsd,i]
            if best_score[0]>self.r_em.evaluate(False)+self.pr.evaluate(False):
                best_score=[self.r_em.evaluate(False)+self.pr.evaluate(False),i]
            print fn, m.evaluate(False),self.r_em.evaluate(False),self.pr.evaluate(False),self.r_ev.evaluate(False),rmsd

        self.assertGreaterEqual(cs.get_number_of_configurations(),1)
        self.assertLess(best_rmsd[0],3)
        print best_rmsd
        print best_score
        self.assertEqual(best_rmsd[1],best_score[1])

if __name__ == '__main__':
    IMP.test.main()
