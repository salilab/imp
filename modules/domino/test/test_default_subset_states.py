import IMP
import IMP.test
import IMP.domino
import IMP.core

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

class Tests(IMP.test.TestCase):

    def _test_basic(self, nm):
        """Testing default subset states"""
        m= IMP.Model()
        ps=[]
        ns=5
        np=4
        for i in range(0,np):
            ps.append(IMP.Particle(m))
        pst= IMP.domino.ParticleStatesTable()
        pft= IMP.domino.ExclusionSubsetFilterTable(pst)
        dsst= nm(pst, [pft])
        for p in ps:
            pst.set_particle_states(p, TrivialParticleStates(ns))
        lsc= IMP.domino.Subset(ps)
        pss= IMP.domino.PackedAssignmentContainer()
        dsst.load_assignments(lsc, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
        self.assertEqual(len(ss), ns**len(ps))
        all_states=[]
        for state in ss:
            #print state
            #print all_states
            self.assertNotIn(state, all_states)
            all_states.append(state)

    def _test_equivalencies(self, nm):
        """Testing default subset states with equivalencies"""
        m= IMP.Model()
        ps=[]
        ns=5
        np=4
        for i in range(0,np):
            ps.append(IMP.Particle(m))
        pst= IMP.domino.ParticleStatesTable()
        tps=TrivialParticleStates(ns)
        pst.set_particle_states(ps[0], tps)
        pst.set_particle_states(ps[1], tps)
        for p in ps[2:]:
            pst.set_particle_states(p, TrivialParticleStates(ns))
        pft= IMP.domino.ExclusionSubsetFilterTable(pst)
        dsst= nm(pst, [pft])
        lsc= IMP.domino.Subset(ps)
        IMP.base.set_log_level(IMP.base.SILENT)
        pss= IMP.domino.PackedAssignmentContainer()
        dsst.load_assignments(lsc, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
        self.assertEqual(len(ss), ns**(len(ps)-2)*(ns)*(ns-1))
        all_states=[]
        print "testing"
        for state in ss:
            #print state
            #print all_states
            self.assertNotIn(state, all_states)
            all_states.append(state)

    def _test_explicit(self, nm):
        """Testing default subset states with explicit equivalencies"""
        m= IMP.Model()
        ps=[]
        ns=5
        np=4
        for i in range(0,np):
            ps.append(IMP.Particle(m))
        pst= IMP.domino.ParticleStatesTable()
        tps=TrivialParticleStates(ns)
        pst.set_particle_states(ps[0], tps)
        pst.set_particle_states(ps[1], tps)
        for p in ps[2:]:
            pst.set_particle_states(p, TrivialParticleStates(ns))
        pft= IMP.domino.ExclusionSubsetFilterTable()
        pft.add_pair((ps[0], ps[1]))
        dsst= nm(pst, [pft])
        lsc= IMP.domino.Subset(ps)
        IMP.base.set_log_level(IMP.base.SILENT)
        pss= IMP.domino.PackedAssignmentContainer()
        dsst.load_assignments(lsc, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
        self.assertEqual(len(ss), ns**(len(ps)-2)*(ns)*(ns-1))
        all_states=[]
        print "testing"
        for state in ss:
            #print state
            #print all_states
            self.assertNotIn(state, all_states)
            all_states.append(state)

    def test_bandb(self):
        """Test branch and bound subset states"""
        self._test_basic(IMP.domino.BranchAndBoundAssignmentsTable)
        self._test_equivalencies(IMP.domino.BranchAndBoundAssignmentsTable)
        self._test_explicit(IMP.domino.BranchAndBoundAssignmentsTable)
    def test_simple(self):
        """Test simple subset states"""
        self._test_basic(IMP.domino.SimpleAssignmentsTable)
        self._test_equivalencies(IMP.domino.SimpleAssignmentsTable)
        self._test_explicit(IMP.domino.SimpleAssignmentsTable)
    def test_recursive(self):
        """Test recursive subset states"""
        self._test_basic(IMP.domino.RecursiveAssignmentsTable)
        self._test_equivalencies(IMP.domino.RecursiveAssignmentsTable)
        self._test_explicit(IMP.domino.RecursiveAssignmentsTable)
if __name__ == '__main__':
    IMP.test.main()
