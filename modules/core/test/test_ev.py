import IMP
import IMP.test
import sys
import IMP.core
import IMP.container
import os
import time

class Tests(IMP.test.TestCase):
    """Tests for sampled density maps"""
    def test_filters(self):
        """Test filters on excluded volume"""
        m = IMP.Model()
        m.set_log_level(IMP.base.SILENT)
        print "pdb"
        h0s=IMP.kernel._create_particles_from_pdb(self.get_input_file_name("1z5s_A.pdb"),
                                           m)
        l0= h0s[0:10]
        rm= h0s[10:]
        for r in rm:
            m.remove_particle(r)
        del h0s
        del rm
        print "ev"
        #set the restraint
        r=IMP.core.ExcludedVolumeRestraint(l0, 1, 0)
        print "cpc"
        cpc= IMP.container.ClosePairContainer(l0, 0, 0)
        cr= IMP.container.PairsRestraint(IMP.core.SoftSpherePairScore(1), cpc)
        dg= IMP.get_dependency_graph(m)
        #IMP.base.show_graphviz(dg)
        idx= IMP.get_vertex_index(dg)
        ss= IMP.get_required_score_states(cr, [], dg, idx)
        print "ss", ss
        self.assert_(len(ss) > 0)
        crsf= IMP.core.RestraintsScoringFunction([cr])
        print crsf.get_score_states()
        print r.evaluate(False)
        m.set_log_level(IMP.base.VERBOSE)
        print cr.evaluate(False)
        pp= cpc.get_particle_pairs()
        print  "pairs are", pp
        self.assertAlmostEqual(r.evaluate(False), cr.evaluate(False),
                               delta=.1)
if __name__ == '__main__':
    IMP.test.main()
