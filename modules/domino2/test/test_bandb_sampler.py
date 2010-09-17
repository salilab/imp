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
        self.key= IMP.IntKey("hi")
        self.n=n
    def get_number_of_particle_states(self):
        return self.n
    def load_particle_state(self, i, p):
        if p.has_attribute(self.key):
            p.set_value(self.key, i)
        else:
            p.add_attribute(self.key, i)
    def do_show(self, stream):
        pass

class DOMINOTests(IMP.test.TestCase):

    def test_global_min1(self):
        """Testing branch and bound sampler"""
        m= IMP.Model()
        m.set_log_level(IMP.SILENT)
        IMP.set_log_level(IMP.VERBOSE)
        ps=[]
        ns=5
        np=4
        for i in range(0,np):
            ps.append(IMP.Particle(m))
        pst= IMP.domino2.ParticleStatesTable()
        print m.get_number_of_score_states()
        print m.get_number_of_restraints()
        dsst= IMP.domino2.BranchAndBoundSampler(m, pst)
        for p in ps:
            pst.set_particle_states(p, TrivialParticleStates(ns))
        cs= dsst.get_sample()
        self.assertEqual(cs.get_number_of_configurations(), ns**len(ps))
        all_states=[]
        for i in range(0, cs.get_number_of_configurations()):
            cs.load_configuration(i)
            s=[]
            for p in ps:
                s.append(p.get_value(IMP.IntKey("hi")))
            ss= IMP.domino2.SubsetState(s)
            #print all_states
            self.assert_(s not in all_states)
            all_states.append(s)

if __name__ == '__main__':
    unittest.main()
