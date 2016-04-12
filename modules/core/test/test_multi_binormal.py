from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.algebra

class Tests(IMP.test.TestCase):

    def test_multi_binormal(self):
        """Test MultipleBinormalRestraint"""
        m = IMP.Model()
        ps = []
        for n in range(4):
            p = m.add_particle("P%d" % n)
            v = IMP.algebra.Vector3D(0,0,0)
            if n < 3:
                v[n] = 1.
            IMP.core.XYZ.setup_particle(m, p, v)
            ps.append(p)
        t = IMP.core.BinormalTerm()
        t.set_correlation(0.5)
        t.set_weight(1.0)
        t.set_means([0., 0.])
        t.set_standard_deviations([1., 1.])
        r = IMP.core.MultipleBinormalRestraint(m, ps, ps)
        r.add_term(t)
        for deriv in (False, True):
            self.assertAlmostEqual(r.evaluate(deriv), 1.40246, delta=1e-4)

if __name__ == '__main__':
    IMP.test.main()
