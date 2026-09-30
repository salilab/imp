import IMP
import IMP.test
import IMP.core
import pickle


def _make_model():
    m = IMP.Model()
    p1 = IMP.Particle(m)
    p2 = IMP.Particle(m)
    p3 = IMP.Particle(m)
    p4 = IMP.Particle(m)
    d1 = IMP.core.XYZ.setup_particle(p1, IMP.algebra.Vector3D(0, 0, 0))
    d2 = IMP.core.XYZ.setup_particle(p2, IMP.algebra.Vector3D(0, 1, 0))
    d3 = IMP.core.XYZ.setup_particle(p3, IMP.algebra.Vector3D(1, 0, 0))
    d4 = IMP.core.XYZ.setup_particle(p4, IMP.algebra.Vector3D(0.5, 0.5, 1))
    return m, p1, p2, p3, p4, d1, d2, d3, d4


class Tests(IMP.test.TestCase):

    def test_pickle_linear(self):
        """Test (un-)pickle of DihedralQuadScore with Linear"""
        m, p1, p2, p3, p4, d1, d2, d3, d4 = _make_model()

        uf = IMP.core.Linear(-1.0, 2.0)
        dqs = IMP.core.DihedralQuadScore(uf)
        dqs.set_name('foo')
        self.assertAlmostEqual(dqs.evaluate_index(m, [p1, p2, p3, p4], None),
                               -1.14159, delta=1e-2)

        dump = pickle.dumps(dqs)
        newdqs = pickle.loads(dump)
        self.assertEqual(dqs.get_name(), 'foo')
        self.assertAlmostEqual(
            newdqs.evaluate_index(m, [p1, p2, p3, p4], None),
            -1.14159, delta=1e-2)


if __name__ == '__main__':
    IMP.test.main()
