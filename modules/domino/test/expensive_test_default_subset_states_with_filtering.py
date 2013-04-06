import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.algebra

class Tests(IMP.test.TestCase):

    def _get_full_list(self, pst, lsc):
        print 'getting'
        dsst= IMP.domino.BranchAndBoundAssignmentsTable(pst, [])
        pss= IMP.domino.PackedAssignmentContainer()
        dsst.load_assignments(lsc, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
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
        IMP.base.set_log_level(IMP.base.SILENT)
        m= IMP.Model()
        m.set_log_level(IMP.base.SILENT)
        ps=[]
        ns=5
        np=4
        for i in range(0,np):
            ps.append(IMP.Particle(m))
            IMP.core.XYZ.setup_particle(ps[-1])
        pst= IMP.domino.ParticleStatesTable()
        print "creating subset"
        lsc= IMP.domino.Subset(ps)
        print "returning"
        return (m, pst, lsc)

    def _test_filtering(self, nm):
        """Testing default subset states with filtering"""
        (m, pst, lsc)= self._get_stuff()
        vs= self._get_positions()
        for p in lsc:
            pst.set_particle_states(p, IMP.domino.XYZStates(vs))
        print lsc[0].get_name()
        print lsc[1].get_name()
        s= IMP.domino.Subset(lsc)
        all_states= self._get_full_list(pst, lsc)
        print "There are ", len(all_states), "states"
        r= IMP.core.DistanceRestraint(IMP.core.Harmonic(1,2),
                                      s[1],
                                      s[2])
        r.set_name("1 2")
        r.set_log_level(IMP.base.VERBOSE)
        m.add_restraint(r)
        ds= IMP.domino.DominoSampler(m)
        m.set_maximum_score(.5)
        rssft= IMP.domino.RestraintScoreSubsetFilterTable(m, pst)
        rssft.set_log_level(IMP.base.SILENT)
        dsst= nm(pst, [rssft])
        IMP.base.set_log_level(IMP.base.VERBOSE)
        print "setting"
        pss= IMP.domino.PackedAssignmentContainer()
        dsst.load_assignments(lsc, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
        print len(ss), "states"
        found_states=[]
        print repr(lsc)
        for s in ss:
            #print s
            found_states.append(s)
            self.assertEqual((s[1]-s[2])**2, 1)
            self.assertIn(s, all_states)
        for s in all_states:
            if (s[1]-s[2])**2==1:
                #print s
                self.assertIn(s, found_states)

    def _test_minimal_filtering(self, nm):
        """Testing MinimumRestraintScoreSubsetFilterTable"""
        (m, pst, lsc)= self._get_stuff()
        vs= self._get_positions()
        for p in lsc:
            pst.set_particle_states(p, IMP.domino.XYZStates(vs))
        print lsc[0].get_name()
        print lsc[1].get_name()
        s= IMP.domino.Subset(lsc)
        all_states= self._get_full_list(pst, lsc)
        print "There are ", len(all_states), "states"
        r= IMP.core.DistanceRestraint(IMP.core.Harmonic(1,2),
                                      s[1],
                                      s[2])
        r.set_name("1 2")
        r.set_log_level(IMP.base.VERBOSE)
        m.add_restraint(r)
        r= IMP.core.DistanceRestraint(IMP.core.Harmonic(1,2),
                                      s[2],
                                      s[3])
        r.set_name("2 3")
        r.set_log_level(IMP.base.VERBOSE)
        m.add_restraint(r)
        r= IMP.core.DistanceRestraint(IMP.core.Harmonic(1,2),
                                      s[0],
                                      s[1])
        r.set_name("0 1")
        r.set_log_level(IMP.base.VERBOSE)
        m.add_restraint(r)
        m.set_maximum_score(.5)
        rc= IMP.domino.RestraintCache(pst)
        rc.add_restraints(m.get_restraints())
        rssft= IMP.domino.MinimumRestraintScoreSubsetFilterTable(m.get_restraints(), rc,1)
        rssft.set_log_level(IMP.base.SILENT)
        dsst= nm(pst, [rssft])
        IMP.base.set_log_level(IMP.base.VERBOSE)
        print "setting"
        pss= IMP.domino.PackedAssignmentContainer()
        dsst.load_assignments(lsc, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
        print len(ss), "states"
        found_states=[]
        print repr(lsc)
        for s in ss:
            #print s
            found_states.append(s)
            #self.assertEqual((s[1]-s[2])**2, 1)
            self.assertIn(s, all_states)
        for s in all_states:
            if (s[1]-s[2])**2==1:
                #print s
                self.assertIn(s, found_states)

    def _test_total_filtering(self, nm):
        """Testing default subset states with total score filtering"""
        (m, pst, lsc)= self._get_stuff()
        vs= self._get_positions()
        for p in lsc:
            pst.set_particle_states(p, IMP.domino.XYZStates(vs))
        print lsc[0].get_name()
        print lsc[1].get_name()
        all_states= self._get_full_list(pst, lsc)
        print "There are ", len(all_states), "states"
        r0= IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1),
                                      lsc[1],
                                      lsc[2])
        r0.set_name("1 2")
        r0.set_log_level(IMP.base.VERBOSE)
        m.add_restraint(r0)
        r1= IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1),
                                      lsc[0],
                                      lsc[1])
        r1.set_name("0 1")
        r1.set_log_level(IMP.base.VERBOSE)
        m.add_restraint(r1)
        m.set_maximum_score(.6)
        rssft= IMP.domino.RestraintScoreSubsetFilterTable(m, pst)
        dsst= nm(pst, [rssft])
        IMP.base.set_log_level(IMP.base.VERBOSE)
        print "setting"
        pss= IMP.domino.PackedAssignmentContainer()
        dsst.load_assignments(lsc, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
        print len(ss), "states"
        found_states=[]
        print repr(lsc)
        for s in ss:
            #print s, (s[1]-s[2])**2, (s[0]-s[1])**2
            found_states.append(s)
            self.assertTrue((s[1]-s[2])**2 == 1 or (s[0]-s[1])**2 == 1)
            self.assertIn(s, all_states)
        for s in all_states:
            IMP.domino.load_particle_states(lsc, s, pst)
            if m.evaluate(False) < .6:
                #print s
                self.assertIn(s, found_states)


    def test_simple(self):
        """Test filtering with simple"""
        self._test_filtering(IMP.domino.SimpleAssignmentsTable)
        self._test_total_filtering(IMP.domino.SimpleAssignmentsTable)
    def test_minimal(self):
        """Test filtering with simple and minimal"""
        self._test_minimal_filtering(IMP.domino.SimpleAssignmentsTable)

    def test_recursive(self):
        """Test filtering with recursive"""
        self._test_filtering(IMP.domino.RecursiveAssignmentsTable)
        self._test_total_filtering(IMP.domino.RecursiveAssignmentsTable)


    def _test_bandb(self):
        """Test filtering with branch and bound"""
        self._test_filtering(IMP.domino.BranchAndBoundAssignmentsTable)
        self._test_total_filtering(IMP.domino.BranchAndBoundAssignmentsTable)

if __name__ == '__main__':
    IMP.test.main()
