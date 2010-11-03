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
        score= IMP.core.HarmonicDistancePairScore(1, 1)
        sdps= IMP.core.SphereDistancePairScore(IMP.core.Linear(0,1))
        score= IMP.core.RigidBodyDistancePairScore(
            sdps, IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))

        # the restraint will be broken apart during optimization
        pc= IMP.container.ListPairContainer([(ps[p[0]], ps[p[1]]) for p in pairs],
                                            "Restrained pairs")
        self.pr= IMP.container.PairsRestraint(score, pc)
        m.set_maximum_score(self.pr, 10)
        m.add_restraint(self.pr)
        return m.get_restraints()

    def create(self):
        m= IMP.Model()
        h=IMP.atom.read_pdb("input/1z5s.pdb",m)
        self.mhs=[]
        ps=[]
        self.rbs=[]
        IMP.atom.add_radii(h)
        all_atoms=[]
        for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
            self.mhs.append(c)
            ps.append(c)
            all_atoms+=IMP.core.get_leaves(c)
            self.rbs.append(IMP.atom.setup_as_rigid_body(c))
        r= IMP.core.ExcludedVolumeRestraint(IMP.container.ListSingletonContainer(all_atoms))
        m.add_restraint(r)
        m.set_maximum_score(r, 100)
        return (m, ps)
    def load_transformations(self):
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-5,-5,-5),
                                      IMP.algebra.Vector3D(5,5,5))
        self.trans=[]
        self.trans.append(IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                                       IMP.algebra.Vector3D(0,0,0)))
        for i in range(1):
            self.trans.append(IMP.algebra.Transformation3D(#IMP.algebra.get_identity_rotation_3d(),
                IMP.algebra.get_random_rotation_3d(IMP.algebra.get_identity_rotation_3d(), .4),
                IMP.algebra.get_random_vector_in(bb)))
    def get_display(self):
        return IMP.atom.WritePDBOptimizerState(self.mhs,"temp_%03d.pdb")
    def test_rcos(self):
        """Test domino2 working for multifit"""
        print "===creating"
        (m, ps)= self.create()
        self.load_transformations()
        self.create_restraints(m,ps)
        print "MODEL SCORE:",m.evaluate(None)
        #add restraints
        #setup domino
        #create discrete states
        states=IMP.domino2.RigidBodyStates(self.trans)
        pst= IMP.domino2.ParticleStatesTable()
        for p in self.mhs:
            pst.set_particle_states(IMP.core.RigidBody(p.get_particle()), states)
        #create sampler
        s=IMP.domino2.DominoSampler(m, pst)
        print pst
        filters=[]
        my_states= IMP.domino2.BranchAndBoundSubsetStatesTable(pst)
        s.set_subset_states_table(my_states)
        print "going to sample"
        s.set_maximum_score(700)
        cs=s.get_sample()

        print "found ", cs.get_number_of_configurations(), "solutions"
        print cs
        self.assertGreaterEqual(cs.get_number_of_configurations(),1)

if __name__ == '__main__':
    IMP.test.main()
