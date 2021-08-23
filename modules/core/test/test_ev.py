from __future__ import print_function
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
        m.set_log_level(IMP.SILENT)
        print("pdb")
        h0s = IMP.get_particles(m, IMP._create_particles_from_pdb(
                           self.get_input_file_name("1z5s_A.pdb"), m))
        l0 = h0s[0:10]
        rm = h0s[10:]
        for r in rm:
            m.remove_particle(r)
        del h0s
        del rm
        print("ev")
        # set the restraint
        r = IMP.core.ExcludedVolumeRestraint(l0, 1, 0)
        print("cpc")
        cpc = IMP.container.ClosePairContainer(l0, 0, 0)
        cr = IMP.container.PairsRestraint(IMP.core.SoftSpherePairScore(1), cpc)
        dg = IMP.get_dependency_graph(m)
        # IMP.show_graphviz(dg)
        idx = IMP.get_vertex_index(dg)
        ss = IMP.get_required_score_states(cr, [], dg, idx)
        print("ss", ss)
        self.assert_(len(ss) > 0)
        crsf = IMP.core.RestraintsScoringFunction([cr])
        crsf.set_has_required_score_states(True)
        print(crsf.get_required_score_states())
        print(r.evaluate(False))
        m.set_log_level(IMP.VERBOSE)
        print(cr.evaluate(False))
        ppi = cpc.get_contents()
        print("pairs are", ppi)
        self.assertAlmostEqual(r.evaluate(False), cr.evaluate(False),
                               delta=.1)
if __name__ == '__main__':
    IMP.test.main()
