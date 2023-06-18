import IMP
import IMP.test
import IMP.core
import IMP.container
import pickle


def _make_system():
    m = IMP.Model()
    p1 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        m, p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0., 0., 0.), 1.))
    p2 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        m, p2, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5., 0., 0.), 1.))
    p3 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        m, p3, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(20., 0., 0.), 1.))
    return m, p1, p2, p3


class Tests(IMP.test.TestCase):

    def test_grid_pickle(self):
        """Test (un-)pickle of GridClosePairsFinder"""
        m, p1, p2, p3 = _make_system()
        lpc = IMP.container.ListPairContainer(m, [])
        gpf = IMP.core.GridClosePairsFinder()
        gpf.set_name("foo")
        gpf.set_distance(6.0)
        # todo: add a PairFilter too

        dump = pickle.dumps(gpf)
        newgpf = pickle.loads(dump)
        self.assertEqual(newgpf.get_name(), "foo")
        self.assertAlmostEqual(newgpf.get_distance(), 6.0, delta=1e-5)
        self.assertEqual(len(newgpf.pair_filters), 0)
        # Coerce from numpy.array to plain Python array
        cps = newgpf.get_close_pairs(m, ([p1, p2, p3]))
        if IMP.IMP_KERNEL_HAS_NUMPY:
            cps = [tuple(p) for p in cps]
        self.assertEqual(cps, [(p2.get_index(), p1.get_index())])

    def test_rigid_pickle(self):
        """Test (un-)pickle of RigidClosePairsFinder"""
        m, p1, p2, p3 = _make_system()
        lpc = IMP.container.ListPairContainer(m, [])
        gpf = IMP.core.GridClosePairsFinder()
        gpf.set_name("foo")
        gpf.set_distance(6.0)

        rpf = IMP.core.RigidClosePairsFinder(gpf)
        rpf.set_name("bar")
        rpf.set_distance(4.0)
        # todo: add a PairFilter too

        dump = pickle.dumps(rpf)
        newrpf = pickle.loads(dump)
        self.assertEqual(newrpf.get_name(), "bar")
        self.assertAlmostEqual(newrpf.get_distance(), 4.0, delta=1e-5)
        self.assertEqual(len(newrpf.pair_filters), 0)
        # Coerce from numpy.array to plain Python array
        cps = newrpf.get_close_pairs(m, ([p1, p2, p3]))
        if IMP.IMP_KERNEL_HAS_NUMPY:
            cps = [tuple(p) for p in cps]
        self.assertEqual(cps, [(p2.get_index(), p1.get_index())])


if __name__ == '__main__':
    IMP.test.main()
