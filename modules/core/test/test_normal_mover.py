import IMP
import IMP.test
import IMP.core
import IMP.container
import pickle
try:
    import jax
except ImportError:
    jax = None


class NormalMoverTest(IMP.test.TestCase):

    mv = IMP.core.NormalMover

    def _make_stuff(self):
        m = IMP.Model()
        pa = m.add_particle("p")
        att = IMP.FloatKey("test")
        m.add_attribute(att, pa, 5.0)
        m.set_is_optimized(att, pa, True)
        mv = self.mv(m, pa, [att], 1.0)
        return m, mv, pa, att

    def _make_xyz_stuff(self):
        m = IMP.Model()
        pa = m.add_particle("p")
        xyz = IMP.core.XYZ.setup_particle(
            m, pa, IMP.algebra.Vector3D(5.0, 6.0, 7.0))
        xyz.set_coordinates_are_optimized(True)
        mv = self.mv(m, pa, 1.0)
        return m, mv, pa

    def _make_intcoord_stuff(self):
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        d1 = IMP.core.XYZR.setup_particle(p1)
        d1.set_coordinates(IMP.algebra.Vector3D(1,2,3))
        d1.set_radius(4)

        rb2 = IMP.core.RigidBody.setup_particle(p2, [p1])
        mv = self.mv(m, p1,
                     IMP.core.RigidBodyMember.get_internal_coordinate_keys(),
                     1.0)
        return m, mv, p2

    def test_propose(self):
        m, mv, pa, att = self._make_stuff()
        old = m.get_attribute(att, pa)
        result = mv.propose()
        new = m.get_attribute(att, pa)
        parts = result.get_moved_particles()
        self.assertEqual(len(parts), 1)
        self.assertEqual(parts[0], pa)
        self.assertTrue(abs(old - new) > 1e-7)

    def test_reject(self):
        m, mv, pa, att = self._make_stuff()
        old = m.get_attribute(att, pa)
        mv.propose()
        mv.reject()
        new = m.get_attribute(att, pa)
        self.assertAlmostEqual(new, old)

    def test_pickle(self):
        m, mv, pa, att = self._make_stuff()
        mv.set_name("foo")
        dump = pickle.dumps(mv)

        newmv = pickle.loads(dump)
        self.assertEqual(newmv.get_name(), "foo")

    def test_pickle_polymorphic(self):
        m, mv, pa, att = self._make_stuff()
        mv.set_name("foo")
        sm = IMP.core.SerialMover([mv])
        dump = pickle.dumps(sm)

        newsm = pickle.loads(dump)
        newmv, = newsm.get_movers()
        self.assertEqual(newmv.get_name(), "foo")

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_one_float_key(self):
        """Test (Log)NormalMover JAX implementation with one FloatKey"""
        import IMP._jax_util
        m, mv, pa, att = self._make_stuff()
        ji = mv._get_jax()
        jm = IMP._jax_util._get_jax_model(m, ji._keys)
        init_func = jax.jit(ji.init_func)
        propose_func = jax.jit(ji.propose_func)

        mvs = init_func(jax.random.key(42))
        new_jm, mvs, ratio = propose_func(jm, mvs)
        old = jm['test'][0]
        new = new_jm['test'][0]
        self.assertGreater(abs(old - new), 1e-7)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_xyz_keys(self):
        """Test (Log)NormalMover JAX implementation with XYZ keys"""
        import IMP._jax_util
        m, mv, pa = self._make_xyz_stuff()
        ji = mv._get_jax()
        jm = IMP._jax_util._get_jax_model(m, ji._keys)
        init_func = jax.jit(ji.init_func)
        propose_func = jax.jit(ji.propose_func)

        mvs = init_func(jax.random.key(42))
        new_jm, mvs, ratio = propose_func(jm, mvs)
        for i in range(3):
            old = jm['xyz'][0][i]
            new = new_jm['xyz'][0][i]
            self.assertGreater(abs(old - new), 1e-7)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_intcoord_keys(self):
        """Test (Log)NormalMover JAX with internal coordinate keys"""
        import IMP._jax_util
        m, mv, pa = self._make_intcoord_stuff()
        ji = mv._get_jax()
        jm = IMP._jax_util._get_jax_model(m, ji._keys)
        init_func = jax.jit(ji.init_func)
        propose_func = jax.jit(ji.propose_func)

        mvs = init_func(jax.random.key(42))
        new_jm, mvs, ratio = propose_func(jm, mvs)
        for i in range(3):
            old = jm['rigid_bodies'].intcoord[0][i]
            new = new_jm['rigid_bodies'].intcoord[0][i]
            self.assertGreater(abs(old - new), 1e-7)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_bad_key(self):
        """Test (Log)NormalMover JAX implementation with bad FloatKey"""
        m = IMP.Model()
        pa = m.add_particle("p")
        xyz = IMP.core.XYZ.setup_particle(
            m, pa, IMP.algebra.Vector3D(5.0, 6.0, 7.0))
        xyz.set_coordinates_are_optimized(True)
        mv = self.mv(m, pa, [xyz.get_coordinate_key(0)], 1.0)
        # Currently not possible to move just "x"
        self.assertRaises(NotImplementedError, mv._get_jax)


class LogNormalMoverTest(NormalMoverTest):

    mv = IMP.core.LogNormalMover


if __name__ == '__main__':
    IMP.test.main()
