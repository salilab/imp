import IMP
import IMP.test
import IMP.domino
import IMP.core


class Tests(IMP.test.TestCase):
    def _print_order(self, order, s):
        for i in order:
            print s[order[i]].get_name(),
    def _test_global_min2(self):
        """Testing ordering"""
        m= IMP.Model()
        ps=[]
        pst= IMP.domino.ParticleStatesTable()
        state= IMP.domino.IndexStates(10)
        for i in range(0,10):
            ps.append(IMP.Particle(m))
            pst.set_particle_states(ps[-1], state)
        eqft= IMP.domino.EquivalenceSubsetFilterTable(pst)
        ps.sort()
        s= IMP.domino.Subset(ps)
        order= IMP.domino.get_order(IMP.domino.Subset(ps),
                                    [eqft])
        print "order is", order
        print s
        self._print_order(order, s)
        print
        for i, oi in enumerate(order):
            self.assertEqual(i, oi)


    def test_global_min4(self):
        """Testing enumeration with ordering"""
        m= IMP.Model()
        ps=[]
        IMP.base.set_log_level(IMP.base.SILENT)
        pst= IMP.domino.ParticleStatesTable()
        state= IMP.domino.IndexStates(8)
        for i in range(0,5):
            ps.append(IMP.Particle(m))
            pst.set_particle_states(ps[-1], state)
        eqft= IMP.domino.EquivalenceSubsetFilterTable(pst)
        exft= IMP.domino.ExclusionSubsetFilterTable(pst)
        ps.sort()
        s= IMP.domino.Subset(ps)
        sst= IMP.domino.BranchAndBoundAssignmentsTable(pst, [eqft, exft])
        sst.set_log_level(IMP.base.VERBOSE)
        pss= IMP.domino.PackedAssignmentContainer()
        sst.load_assignments(s, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
        print ss
        self.assertEqual(len(ss), 56)

    def test_global_min5(self):
        """Testing enumeration with equiv excl ordering"""
        m= IMP.Model()
        ps=[]
        IMP.base.set_log_level(IMP.base.SILENT)
        pst= IMP.domino.ParticleStatesTable()
        state= IMP.domino.IndexStates(11)
        for i in range(0,10):
            ps.append(IMP.Particle(m))
            pst.set_particle_states(ps[-1], state)
        eqft= IMP.domino.EquivalenceAndExclusionSubsetFilterTable(pst)
        ps.sort()
        s= IMP.domino.Subset(ps)
        sst= IMP.domino.BranchAndBoundAssignmentsTable(pst, [eqft])
        sst.set_log_level(IMP.base.VERBOSE)
        pss= IMP.domino.PackedAssignmentContainer()
        sst.load_assignments(s, pss)
        ss= pss.get_assignments((0, pss.get_number_of_assignments()))
        print ss
        self.assertEqual(len(ss), 11)

    def _test_global_min3(self):
        """Testing ordering with other node"""
        m= IMP.Model()
        ps=[]
        pst= IMP.domino.ParticleStatesTable()
        state= IMP.domino.IndexStates(10)
        for i in range(0,4):
            ps.append(IMP.Particle(m))
            pst.set_particle_states(ps[-1], state)
        statep= IMP.domino.IndexStates(8)
        pp=IMP.Particle(m)
        pst.set_particle_states(pp, statep)
        eqft= IMP.domino.EquivalenceSubsetFilterTable(pst)
        ps.sort()
        s=IMP.domino.Subset(ps+[pp])
        order= IMP.domino.get_order(s,
                                    [eqft])
        print "order is", order
        self._print_order(order, s)
        for i,e in enumerate(order):
            if i < 4:
                self.assertEqual(s[order[i]], ps[i])
            else:
                pass


if __name__ == '__main__':
    IMP.test.main()
