import IMP
import IMP.test
import IMP.bullet
import IMP.core
import IMP.algebra
import IMP.domino2
import StringIO
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
        hsdps= IMP.core.HarmonicSphereDistancePairScore(0, 1)
        sdps= IMP.core.KClosePairsPairScore(hsdps,
                                           IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
        # the restraint will be broken apart during optimization
        pc= IMP.container.ListPairContainer([(ps[p[0]], ps[p[1]]) for p in pairs],
                                            "Restrained pairs")
        self.pr= IMP.container.PairsRestraint(sdps, pc)
        self.pr.set_name("interactions")
        m.set_maximum_score(self.pr, 10)
        m.add_restraint(self.pr)
        print "done"

    def create(self):
        m= IMP.Model()
        ss= IMP.SetLogState(IMP.SILENT)
        h=IMP.atom.read_pdb(self.get_input_file_name("1z5s.pdb"),m)
        del ss
        self.mhs=[]
        ps=[]
        self.rbs=[]
        all_atoms=[]
        print "creating rbs"
        for cr in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
            c= IMP.atom.create_simplified_along_backbone(IMP.atom.Chain(cr), 10)
            self.mhs.append(c)
            ps.append(c)
            all_atoms+=IMP.core.get_leaves(c)
            self.rbs.append(IMP.atom.create_rigid_body(c))
        #r= IMP.core.ExcludedVolumeRestraint(IMP.container.ListSingletonContainer(all_atoms))
        #m.add_restraint(r)
        rbs= IMP.container.ListSingletonContainer(ps)
        aprbs= IMP.container.AllPairContainer(rbs)
        rdps= IMP.core.KClosePairsPairScore(IMP.core.SoftSpherePairScore(1),
                                           IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
        r= IMP.container.PairsRestraint(rdps, aprbs)
        r.set_name("EV")
        m.add_restraint(r)
        m.set_maximum_score(r, 100)
        return (m, ps)
    def load_transformations(self):
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-5,-5,-5),
                                      IMP.algebra.Vector3D(5,5,5))
        self.trans=[]
        self.trans.append(IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                                       IMP.algebra.Vector3D(0,0,0)))
        for i in range(0):
            self.trans.append(IMP.algebra.Transformation3D(#IMP.algebra.get_identity_rotation_3d(),
                IMP.algebra.get_random_rotation_3d(IMP.algebra.get_identity_rotation_3d(), .4),
                IMP.algebra.get_random_vector_in(bb)))
    def get_display(self):
        return IMP.atom.WritePDBOptimizerState(self.mhs,"temp_%03d.pdb")
    def test_rcos(self):
        """Test domino2 working for multifit"""
        print "===creating"
        (m, ps)= self.create()
        m.set_log_level(IMP.SILENT)
        self.load_transformations()
        print "create restraints"
        self.create_restraints(m,ps)
        print "MODEL SCORE:",
        print m.evaluate(None)
        #add restraints
        #setup domino
        #create discrete states
        states=IMP.domino2.RigidBodyStates(self.trans)
        pst= IMP.domino2.ParticleStatesTable()
        for p in self.rbs:
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
        s.set_log_level(IMP.VERBOSE)
        print pst
        print "going to sample"
        s.set_maximum_score(700)
        cs=s.get_sample()

        print "found ", cs.get_number_of_configurations(), "solutions"
        #cs.load_configuration(0)
        #m.evaluate(False)
        self.assertGreaterEqual(cs.get_number_of_configurations(),0)

if __name__ == '__main__':
    IMP.test.main()
