import sys
import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.atom
import IMP.restrainer
import time

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
        """Testing default subset states"""
        m= IMP.Model()
        ps=[]
        ns=5
        np=4
        for i in range(0,np):
            ps.append(IMP.Particle(m))
        pst= IMP.domino.ParticleStatesTable()
        pft= IMP.domino.ExclusionSubsetFilterTable(pst)
        dsst= IMP.domino.BranchAndBoundSubsetStatesTable(pst, [pft])
        for p in ps:
            pst.set_particle_states(p, TrivialParticleStates(ns))
        lsc= IMP.domino.Subset(ps)
        ss= dsst.get_subset_states(lsc)
        self.assertEqual(len(ss), ns**len(ps))
        all_states=[]
        for state in ss:
            print state
            #print all_states
            self.assertNotIn(state, all_states)
            all_states.append(state)

    def test_global_min2(self):
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
        dsst= IMP.domino.BranchAndBoundSubsetStatesTable(pst, [pft])
        lsc= IMP.domino.Subset(ps)
        IMP.set_log_level(IMP.VERBOSE)
        ss= dsst.get_subset_states(lsc)
        self.assertEqual(len(ss), ns**(len(ps)-2)*(ns)*(ns-1))
        all_states=[]
        print "testing"
        for state in ss:
            print state
            #print all_states
            self.assertNotIn(state, all_states)
            all_states.append(state)

    def test_global_min3(self):
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
        dsst= IMP.domino.BranchAndBoundSubsetStatesTable(pst, [pft])
        lsc= IMP.domino.Subset(ps)
        IMP.set_log_level(IMP.VERBOSE)
        ss= dsst.get_subset_states(lsc)
        self.assertEqual(len(ss), ns**(len(ps)-2)*(ns)*(ns-1))
        all_states=[]
        print "testing"
        for state in ss:
            print state
            #print all_states
            self.assertNotIn(state, all_states)
            all_states.append(state)

if __name__ == '__main__':
    IMP.test.main()
