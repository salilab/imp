import IMP
import IMP.test
import IMP.example
import IMP.core
import sys
import random
from StringIO import StringIO

class LogTests(IMP.test.TestCase):
    def _show(self, ps, pairs, name):
        w=IMP.display.PymolWriter(name)
        [w.add_geometry(IMP.core.XYZRGeometry(p)) for p in ps]
        [w.add_geometry(IMP.core.EdgePairGeometry((ps[p[0]], ps[p[1]]))) for p in pairs]
    def test_log_targets(self):
        """Test getting complex assembly"""
        # assemble a cube
        m= IMP.Model()
        bb= IMP.algebra.get_cube_3d(10)
        ps= [IMP.Particle(m) for i in range(0,8)]
        ds= [IMP.core.XYZR.setup_particle(p) for p in ps]
        [d.set_coordinates_are_optimized(True) for d in ds]
        [x.set_radius(1) for x in ds]
        pairs=[(0,1), (0,2), (0,4),
               (1,3), (1,5),
               (2,3), (2,6),
               (3,7),
               (4,5), (4,6),
               (5,7),
               (6,7)]
        diags=[(0,7), (1, 6), (2, 5), (3, 4)]
        score= IMP.core.HarmonicDistancePairScore(1.8,10)
        dscore= IMP.core.HarmonicDistancePairScore(1.8*3**.5,10)
        rs=[IMP.core.PairRestraint(score, (ps[p[0]], ps[p[1]]),
                                   ps[p[0]].get_name()+ " " +ps[p[1]].get_name())
            for p in pairs]\
            + [IMP.core.PairRestraint(dscore, (ps[p[0]], ps[p[1]]),
                                   ps[p[0]].get_name()+ " " +ps[p[1]].get_name())
            for p in diags]
        [m.add_restraint(r) for r in rs]
        IMP.example.optimize_assembly(m, ps, rs, [], bb,
                                      IMP.core.SoftSpherePairScore(1),
                                      10, [])
        self._show(ps, pairs, "out.pym")
        print ds
        for r in rs:
            print r.get_name(), r.evaluate(False)


if __name__ == '__main__':
    IMP.test.main()
