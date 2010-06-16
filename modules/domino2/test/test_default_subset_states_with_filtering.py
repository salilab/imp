import sys
import unittest
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.helper
import IMP.algebra
import time

class DOMINOTests(IMP.test.TestCase):

    def _get_full_list(self, pst, lsc):
        dsst= IMP.domino2.DefaultSubsetStatesTable(pst)
        ss= dsst.get_subset_states(lsc)
        all_states=[]
        for i in range(0, ss.get_number_of_states()):
            state= ss.get_state(i)
            #print all_states
            self.assert_(state not in all_states)
            all_states.append(state)
        return all_states

    def _get_positions(self):
        vs=[]
        for i in range(0,6):
            vs.append(IMP.algebra.Vector3D(i,0,0))
        return vs
    def _get_stuff(self):
        m= IMP.Model()
        ps=[]
        ns=5
        np=4
        for i in range(0,np):
            ps.append(IMP.Particle(m))
            IMP.core.XYZ.setup_particle(ps[-1])
        pst= IMP.domino2.ParticleStatesTable()
        lsc= IMP.container.ListSingletonContainer(ps)
        return (m, pst, lsc)


    def test_global_min1(self):
        """Testing default subset states with filtering"""
        (m, pst, lsc)= self._get_stuff()
        vs= self._get_positions()
        for p in lsc.get_particles():
            pst.set_particle_states(p, IMP.domino2.XYZStates(vs))
        print lsc.get_particle(0).get_name()
        print lsc.get_particle(1).get_name()
        r= IMP.core.DistanceRestraint(IMP.core.Harmonic(1,2),
                                      lsc.get_particle(0),
                                      lsc.get_particle(1))
        all_states= self._get_full_list(pst, lsc)
        print "There are ", len(all_states), "states"
        m.add_restraint(r)
        ds= IMP.domino2.DominoSampler(m)
        ds.set_maximum_score(.5)
        dsst= IMP.domino2.DefaultSubsetStatesTable(pst)
        me= IMP.domino2.ModelSubsetEvaluatorTable(m, pst)
        me.set_sampler(ds)
        dsst.set_subset_evaluator_table(me)
        dsst.set_sampler(ds)
        ss= dsst.get_subset_states(lsc)
        print ss.get_number_of_states(), "states"
        found_states=[]
        for i in range(0, ss.get_number_of_states()):
            s= ss.get_state(i)
            print s
            found_states.append(s)
            self.assert_((s[0]-s[1])**2==1)
            self.assert_(s in all_states)
        for s in all_states:
            if (s[0]-s[1])**2==1:
                self.assert_(s in found_states)

    def _test_global_min2(self):
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
