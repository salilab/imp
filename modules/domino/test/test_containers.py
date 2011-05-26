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
        h5= IMP.rmf.HDF5Group(tfn, True)
        pss= IMP.domino.HDF5AssignmentContainer(h5, "assignments")
        dsst.load_assignments(lsc, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
        self.assertEqual(len(ss), ns**len(ps))
        all_states=[]
        for state in ss:
            print state
            #print all_states
            self.assertNotIn(state, all_states)
            all_states.append(state)


if __name__ == '__main__':
    IMP.test.main()
