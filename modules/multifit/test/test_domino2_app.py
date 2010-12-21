import IMP
import IMP.test
import IMP.bullet
import IMP.core
import IMP.algebra
import IMP.domino
import StringIO
import math

class MultiFitDominoTests(IMP.test.TestCase):
    """Test domino enumeration of rigid bodies"""
    def local_random(self,rb):
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-5,-5,-5),
                                      IMP.algebra.Vector3D(5,5,5))
        t=IMP.algebra.Transformation3D(#IMP.algebra.get_identity_rotation_3d(),
                                       IMP.algebra.get_random_rotation_3d(IMP.algebra.get_identity_rotation_3d(), .4),
                                       IMP.algebra.get_random_vector_in(bb))
        IMP.core.transform(rb,t)
    def create_restraints(self,m, ps):
        pairs=[[0,1],[0,2],[1,3],[0,3]]
        hsdps= IMP.core.HarmonicSphereDistancePairScore(0, 1)
        sdps= IMP.core.KClosePairsPairScore(hsdps,
                                           IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
        # the restraint will be broken apart during optimization
        pc= IMP.container.ListPairContainer([(ps[p[0]], ps[p[1]]) for p in pairs],
                                            "Restrained pairs")
        self.pr= IMP.container.PairsRestraint(sdps, pc)
        self.pr.set_name("interactions")
        #m.set_maximum_score(self.pr, 10)
        m.add_restraint(self.pr)
        print "done"

    def create(self):
        m= IMP.Model()
        ss= IMP.SetLogState(IMP.SILENT)
        h=IMP.atom.read_pdb(self.get_input_file_name("1z5s.pdb"),m)
        del ss
        ps=[]
        rbs=[]
        all_atoms=[]
        print "creating rbs"
        for cr in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
            c= IMP.atom.create_simplified_along_backbone(IMP.atom.Chain(cr), 10)
            ps.append(c)
            all_atoms+=IMP.core.get_leaves(c)
            rbs.append(IMP.atom.create_rigid_body(c))
        #r= IMP.core.ExcludedVolumeRestraint(IMP.container.ListSingletonContainer(all_atoms))
        #m.add_restraint(r)
        IMP.atom.destroy(h)
        crbs= IMP.container.ListSingletonContainer(ps)
        aprbs= IMP.container.AllPairContainer(crbs)
        rdps= IMP.core.KClosePairsPairScore(IMP.core.SoftSpherePairScore(1),
                                           IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
        r= IMP.container.PairsRestraint(rdps, aprbs)
        r.set_name("EV")
        m.add_restraint(r)
        #m.set_maximum_score(r, 1)
        return (m, ps, rbs)
    def load_transformations(self, rbs):
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-5,-5,-5),
                                      IMP.algebra.Vector3D(5,5,5))
        trans=[]
        for r in rbs:
            trans.append(r.get_reference_frame())
        trans.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                                                              IMP.algebra.Vector3D(0,0,0))))
        for i in range(0):
            trans.append(IMP.algebra.Transformation3D(#IMP.algebra.get_identity_rotation_3d(),
                IMP.algebra.get_random_rotation_3d(IMP.algebra.get_identity_rotation_3d(), .4),
                IMP.algebra.get_random_vector_in(bb)))
        return trans
    def get_display(self, ps):
        return IMP.atom.WritePDBOptimizerState(ps,"temp_%03d.pdb")
    def test_rcos(self):
        """Test domino working for multifit"""
        print "===creating"
        (m, ps, rbs)= self.create()
        m.set_log_level(IMP.SILENT)
        trans=self.load_transformations(rbs)
        print "create restraints"
        self.create_restraints(m,ps)
        print "MODEL SCORE:",
        initial_score= m.evaluate(None)
        print initial_score
        #add restraints
        #setup domino
        #create discrete states
        states=IMP.domino.RigidBodyStates(trans)
        pst= IMP.domino.ParticleStatesTable()
        for p in rbs:
            pst.set_particle_states(IMP.core.RigidBody(p.get_particle()), states)
        #create sampler
        oc= IMP.domino.OptimizeContainers(m.get_root_restraint_set(), pst)
        ocr= IMP.domino.OptimizeRestraints(m.get_root_restraint_set(), pst)
        allrs= IMP.get_restraints(m.get_root_restraint_set())
        for r in allrs:
            print r.get_name()
        for p in pst.get_particles():
            print p.get_name()
        print "interaction graph"
        ig= IMP.domino.get_interaction_graph(m.get_root_restraint_set(), pst)
        ig.show()
        print "restraint graph"
        rg= IMP.domino.get_restraint_graph(m.get_root_restraint_set(), pst)
        rg.show()
        s=IMP.domino.DominoSampler(m, pst)
        s.set_log_level(IMP.TERSE)
        print pst
        print "going to sample"
        s.set_maximum_score(initial_score+1)
        cs=s.get_sample()

        print "found ", cs.get_number_of_configurations(), "solutions"
        #cs.load_configuration(0)
        #m.evaluate(False)
        self.assertEqual(cs.get_number_of_configurations(),1)

if __name__ == '__main__':
    IMP.test.main()
