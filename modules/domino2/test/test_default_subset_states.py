import sys
import unittest
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
    def get_number_of_states(self):
        return self.n
    def load_state(self, i, p):
        pass
    def do_show(self, stream):
        pass

class DOMINOTests(IMP.test.TestCase):

    def _test_global_min1(self):
        """Testing default subset states"""
        m= IMP.Model()
        ps=[]
        ns=5
        np=4
        for i in range(0,np):
            ps.append(IMP.Particle(m))
        pst= IMP.domino2.ParticleStatesTable()
        dsst= IMP.domino2.DefaultSubsetStatesTable(pst)
        for p in ps:
            pst.set_particle_states(p, TrivialParticleStates(ns))
        lsc= IMP.container.ListSingletonContainer(ps)
        ss= dsst.get_subset_states(lsc)
        self.assertEqual(ss.get_number_of_states(), ns**len(ps))
        all_states=[]
        for i in range(0, ss.get_number_of_states()):
            state= ss.get_state(i)
            print state
            #print all_states
            self.assert_(state not in all_states)
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
        dsst= IMP.domino2.DefaultSubsetStatesTable(pst)
        lsc= IMP.container.ListSingletonContainer(ps)
        ss= dsst.get_subset_states(lsc)
        self.assertEqual(ss.get_number_of_states(), ns**(len(ps)-2)*(ns)*(ns-1))
        all_states=[]
        for i in range(0, ss.get_number_of_states()):
            state= ss.get_state(i)
            print state
            #print all_states
            self.assert_(state not in all_states)
            all_states.append(state)

if __name__ == '__main__':
    unittest.main()
