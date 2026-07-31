import IMP
import IMP.test
import IMP.atom
import pickle
try:
    import jax
except ImportError:
    jax = None


def make_dihedrals():
    m = IMP.Model()
    dihs = []
    for stiffness in (5.0, -3.0, 0.0):
        p1 = IMP.Particle(m)
        xyz1 = IMP.core.XYZ.setup_particle(p1, IMP.algebra.Vector3D(0,0,0))
        p2 = IMP.Particle(m)
        xyz2 = IMP.core.XYZ.setup_particle(p2, IMP.algebra.Vector3D(1,0,0))
        p3 = IMP.Particle(m)
        xyz3 = IMP.core.XYZ.setup_particle(p3, IMP.algebra.Vector3D(0,1,0))
        p4 = IMP.Particle(m)
        xyz4 = IMP.core.XYZ.setup_particle(p4, IMP.algebra.Vector3D(0,0,1))
        d = IMP.Particle(m)
        dih = IMP.atom.Dihedral.setup_particle(d, xyz1, xyz2, xyz3, xyz4)
        dih.set_ideal(0.)
        dih.set_multiplicity(1)
        dih.set_stiffness(stiffness)
        dihs.append(dih)
    return m, dihs


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of DihedralSingletonScore"""
        m, dih = make_dihedrals()
        ss = IMP.atom.DihedralSingletonScore()
        ss.set_name('foo')
        self.assertAlmostEqual(ss.evaluate_index(m, dih[0], None), 19.717,
                               delta=0.01)

        dump = pickle.dumps(ss)
        newss = pickle.loads(dump)
        self.assertEqual(newss.get_name(), 'foo')
        self.assertAlmostEqual(newss.evaluate_index(m, dih[0], None), 19.717,
                               delta=0.01)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of DihedralSingletonScore via polymorphic pointer"""
        m, dih = make_dihedrals()
        ss = IMP.atom.DihedralSingletonScore()
        ss.set_name('foo')
        lsc = IMP.container.ListSingletonContainer(m, dih)
        r = IMP.container.SingletonsRestraint(ss, lsc)
        self.assertAlmostEqual(r.evaluate(False), 21.619, delta=0.01)

        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(r.evaluate(False), 21.619, delta=0.01)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of DihedralSingletonScore"""
        m, dih = make_dihedrals()
        ss = IMP.atom.DihedralSingletonScore()
        lsc = IMP.container.ListSingletonContainer(m, dih)
        r = IMP.container.SingletonsRestraint(ss, lsc)
        imp_score = r.evaluate(False)
        ji = r._get_jax()
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        jax_score = j(jm)
        self.assertAlmostEqual(imp_score, 21.619, delta=0.01)
        self.assertAlmostEqual(imp_score, jax_score, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
