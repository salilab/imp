import sys
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.helper
import time

class TrivialParticleStates(IMP.domino2.ParticleStates):
    def __init__(self, n):
        IMP.domino2.ParticleStates.__init__(self)
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
        pst= IMP.domino2.ParticleStatesTable()
        pft= IMP.domino2.PermutationSubsetFilterTable(pst)
        dsst= IMP.domino2.DefaultSubsetStatesTable(pst, [pft])
        for p in ps:
            pst.set_particle_states(p, TrivialParticleStates(ns))
        lsc= IMP.domino2.Subset(ps)
        ss= dsst.get_subset_states(lsc)
        self.assertEqual(ss.get_number_of_subset_states(), ns**len(ps))
        all_states=[]
        for i in range(0, ss.get_number_of_subset_states()):
            state= ss.get_subset_state(i)
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
        pst= IMP.domino2.ParticleStatesTable()
        tps=TrivialParticleStates(ns)
        pst.set_particle_states(ps[0], tps)
        pst.set_particle_states(ps[1], tps)
        for p in ps[2:]:
            pst.set_particle_states(p, TrivialParticleStates(ns))
        pft= IMP.domino2.PermutationSubsetFilterTable(pst)
        dsst= IMP.domino2.DefaultSubsetStatesTable(pst, [pft])
        lsc= IMP.domino2.Subset(ps)
        IMP.set_log_level(IMP.VERBOSE)
        ss= dsst.get_subset_states(lsc)
        self.assertEqual(ss.get_number_of_subset_states(), ns**(len(ps)-2)*(ns)*(ns-1))
        all_states=[]
        print "testing"
        for i in range(0, ss.get_number_of_subset_states()):
            state= ss.get_subset_state(i)
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
        pst= IMP.domino2.ParticleStatesTable()
        tps=TrivialParticleStates(ns)
        pst.set_particle_states(ps[0], tps)
        pst.set_particle_states(ps[1], tps)
        for p in ps[2:]:
            pst.set_particle_states(p, TrivialParticleStates(ns))
        pft= IMP.domino2.PermutationSubsetFilterTable([(ps[0], ps[1])])
        dsst= IMP.domino2.DefaultSubsetStatesTable(pst, [pft])
        lsc= IMP.domino2.Subset(ps)
        IMP.set_log_level(IMP.VERBOSE)
        ss= dsst.get_subset_states(lsc)
        self.assertEqual(ss.get_number_of_subset_states(), ns**(len(ps)-2)*(ns)*(ns-1))
        all_states=[]
        print "testing"
        for i in range(0, ss.get_number_of_subset_states()):
            state= ss.get_subset_state(i)
            print state
            #print all_states
            self.assertNotIn(state, all_states)
            all_states.append(state)

if __name__ == '__main__':
    IMP.test.main()
