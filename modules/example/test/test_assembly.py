import IMP
import IMP.test
import IMP.example
import IMP.core
import sys
import random
from StringIO import StringIO

class Tests(IMP.test.TestCase):
    def _show(self, ps, pairs, name):
        w=IMP.display.PymolWriter(name)
        [w.add_geometry(IMP.core.XYZRGeometry(p)) for p in ps]
        [w.add_geometry(IMP.core.EdgePairGeometry((ps[p[0]], ps[p[1]]))) for p in pairs]
    def test_log_targets(self):
        """Test getting complex assembly"""
        # assemble an interaction between 2 particles
        m= IMP.Model()
        bb= IMP.algebra.get_cube_3d(10)
        ps= [IMP.Particle(m) for i in range(2)]
        ds= [IMP.core.XYZR.setup_particle(p) for p in ps]
        [d.set_coordinates_are_optimized(True) for d in ds]
        [x.set_radius(1) for x in ds]
        pairs=[(0,1)]
        score= IMP.core.HarmonicDistancePairScore(1.8,10)
        rs=[IMP.core.PairRestraint(score, (ps[p[0]], ps[p[1]]),
                               ps[p[0]].get_name()+ " " +ps[p[1]].get_name()) \
            for p in pairs]
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
