import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.atom

class TrivialParticleStates(IMP.domino.ParticleStates):
    def __init__(self, n):
        IMP.domino.ParticleStates.__init__(self)
        self.n=n
    def get_number_of_particle_states(self):
        return self.n
    def load_state(self, i, p):
        pass
    def do_show(self, stream):
        pass

class DOMINOTests(IMP.test.TestCase):
    def test_global_min1(self):
        """Testing default subset states writing to a file"""
        m= IMP.Model()
        #try:
        #    import IMP.rmf
        #except:
        #    self.skipTest("RMF is disabled")
        ps=[]
        ns=5
        np=4
        for i in range(0,np):
            ps.append(IMP.Particle(m))
        pst= IMP.domino.ParticleStatesTable()
        pft= IMP.domino.ExclusionSubsetFilterTable(pst)
        dsst= IMP.domino.BranchAndBoundAssignmentsTable(pst, [pft])
        for p in ps:
            pst.set_particle_states(p, TrivialParticleStates(ns))
        lsc= IMP.domino.Subset(ps)
        tfn= self.get_tmp_file_name("subsetassignments.hdf5")
        h5= IMP.rmf.create_hdf5_file(tfn)
        pss= IMP.domino.HDF5AssignmentContainer(h5, lsc, ps, "assignments")
        dsst.load_assignments(lsc, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
        self.assertEqual(len(ss), ns**len(ps))
        all_states=[]
        for state in ss:
            print state
            print all_states
            self.assertNotIn(state, all_states)
            all_states.append(state)

    def test_sample(self):
        """Testing default sample container"""
        sac= IMP.domino.SampleAssignmentContainer(10)
        for i in range(0,25):
            ass= IMP.domino.Assignment([i])
            sac.add_assignment(ass)
        self.assertEqual(sac.get_number_of_assignments(), 10)
        print sac.get_assignments()

    def test_cluster(self):
        """Testing the cluster container"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        s= IMP.domino.Subset(ps)
        pst= IMP.domino.ParticleStatesTable()
        ik= IMP.IntKey("hi")
        iss= IMP.domino.IndexStates(10000, ik)
        for p in ps:
            p.add_attribute(ik, 1)
            pst.set_particle_states(p, iss)
        cac= IMP.domino.ClusteredAssignmentContainer(10, s, pst)
        cac.set_log_level(IMP.VERBOSE)
        for i in range(0,6):
            ass=[100*i for j in range(0,10)]
            for j in range(0,6):
                ass[9]=j
                print "adding", ass
                cac.add_assignment(IMP.domino.Assignment(ass))
        got=[]
        all= cac.get_assignments()
        print all
        for a in all:
            print a
            got.append(a[0])
        for i in range(0,6):
            self.assert_(i*100 in got)

    def test_heap_container(self):
        """Testing heap sample container"""

        #create particles
        m= IMP.Model()
        m.set_log_level(IMP.SILENT)
        ps=[]
        for i in range(0,3):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0),5))
            ps.append(p)

        pts=[IMP.algebra.Vector3D(0,0,0),
             IMP.algebra.Vector3D(1,0,0),
             IMP.algebra.Vector3D(2,0,0),
             IMP.algebra.Vector3D(3,0,0)]
        particle_state= IMP.domino.XYZStates(pts)
        pst= IMP.domino.ParticleStatesTable()
        for p in ps:
            pst.set_particle_states(p, particle_state)
        m.add_restraint(IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1), ps[0], ps[1]))
        m.add_restraint(IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1), ps[1], ps[2]))
        print 5
        sampler= IMP.domino.DominoSampler(m, pst)
        rssft= IMP.domino.RestraintScoreSubsetFilterTable(m, pst)
        s=IMP.domino.Subset(pst.get_particles())
        rssf=rssft.get_restraint_score_subset_filter(s,[])
        assignments=sampler.get_sample_assignments(s);
        print "number of assignments:",len(assignments)
        scores=[]
        for i in range(len(assignments)):
            print assignments[i],rssf.get_score(assignments[i])
            scores.append(rssf.get_score(assignments[i]))
        scores.sort()
        hac= IMP.domino.HeapAssignmentContainer(10,rssf)
        for a in assignments:
            hac.add_assignment(a)
        self.assertEqual(hac.get_number_of_assignments(), 10)
        #check that you have the top 10
        print "top ten"
        for a in hac.get_assignments():
            self.assertLess(rssf.get_score(a),scores[9]+0.01)
            print a,rssf.get_score(a)



if __name__ == '__main__':
    IMP.test.main()
