import sys
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.restrainer
import IMP.algebra
import time

class DOMINOTests(IMP.test.TestCase):

    def _get_full_list(self, pst, lsc):
        print 'getting'
        dsst= IMP.domino2.DefaultSubsetStatesTable(pst, [])
        ss= dsst.get_subset_states(lsc)
        print "got all"
        all_states=[]
        for state in ss:
            #print all_states
            self.assertNotIn(state, all_states)
            all_states.append(state)
        return all_states

    def _get_positions(self):
        vs=[]
        for i in range(0,6):
            vs.append(IMP.algebra.Vector3D(i,0,0))
        return vs
    def _get_stuff(self):
        IMP.set_log_level(IMP.SILENT)
        m= IMP.Model()
        m.set_log_level(IMP.SILENT)
        ps=[]
        ns=5
        np=4
        for i in range(0,np):
            ps.append(IMP.Particle(m))
            IMP.core.XYZ.setup_particle(ps[-1])
        pst= IMP.domino2.ParticleStatesTable()
        print "creating subset"
        lsc= IMP.domino2.Subset(ps)
        print "returning"
        return (m, pst, lsc)

    def test_global_min1(self):
        """Testing default subset states with filtering"""
        (m, pst, lsc)= self._get_stuff()
        vs= self._get_positions()
        for p in lsc:
            pst.set_particle_states(p, IMP.domino2.XYZStates(vs))
        print lsc[0].get_name()
        print lsc[1].get_name()
        all_states= self._get_full_list(pst, lsc)
        print "There are ", len(all_states), "states"
        r= IMP.core.DistanceRestraint(IMP.core.Harmonic(1,2),
                                      lsc[1],
                                      lsc[2])
        r.set_name("1 2");
        r.set_log_level(IMP.VERBOSE)
        m.add_restraint(r)
        ds= IMP.domino2.DominoSampler(m)
        m.set_maximum_score(.5)
        rssft= IMP.domino2.RestraintScoreSubsetFilterTable(m, pst)
        dsst= IMP.domino2.BranchAndBoundSubsetStatesTable(pst, [rssft])
        IMP.set_log_level(IMP.VERBOSE)
        print "setting"
        ss= dsst.get_subset_states(lsc)
        print len(ss), "states"
        found_states=[]
        print repr(lsc)
        for s in ss:
            print s
            found_states.append(s)
            self.assertEqual((s[1]-s[2])**2, 1)
            self.assertIn(s, all_states)
        for s in all_states:
            if (s[1]-s[2])**2==1:
                print s
                self.assertIn(s, found_states)

    def test_global_min2(self):
        """Testing default subset states with total score filtering"""
        (m, pst, lsc)= self._get_stuff()
        vs= self._get_positions()
        for p in lsc:
            pst.set_particle_states(p, IMP.domino2.XYZStates(vs))
        print lsc[0].get_name()
        print lsc[1].get_name()
        all_states= self._get_full_list(pst, lsc)
        print "There are ", len(all_states), "states"
        r0= IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1),
                                      lsc[1],
                                      lsc[2])
        r0.set_name("1 2");
        r0.set_log_level(IMP.VERBOSE)
        m.add_restraint(r0)
        r1= IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1),
                                      lsc[0],
                                      lsc[1])
        r1.set_name("0 1");
        r1.set_log_level(IMP.VERBOSE)
        m.add_restraint(r1)
        m.set_maximum_score(.6)
        ds= IMP.domino2.DominoSampler(m)
        rssft= IMP.domino2.RestraintScoreSubsetFilterTable(m, pst)
        dsst= IMP.domino2.BranchAndBoundSubsetStatesTable(pst, [rssft])
        IMP.set_log_level(IMP.VERBOSE)
        print "setting"
        ss= dsst.get_subset_states(lsc)
        print len(ss), "states"
        found_states=[]
        print repr(lsc)
        for s in ss:
            print s, (s[1]-s[2])**2, (s[0]-s[1])**2
            found_states.append(s)
            self.assert_((s[1]-s[2])**2 == 1 or (s[0]-s[1])**2 == 1)
            self.assertIn(s, all_states)
        for s in all_states:
            IMP.domino2.load_particle_states(lsc, s, pst)
            if m.evaluate(False) < .6:
                print s
                self.assertIn(s, found_states)

if __name__ == '__main__':
    IMP.test.main()
