import IMP.test
import IMP.core
try:
    import jax
    import jax.numpy as jnp
    import jax.random
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):
    def test_ball_mover_accessors(self):
        """Test BallMover accessor methods"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        mv = IMP.core.BallMover(m, p1, 1.0)
        self.assertAlmostEqual(mv.get_radius(), 1.0, delta=1e-5)
        self.assertEqual(frozenset(mv.get_keys()),
                         frozenset(IMP.core.XYZ.get_xyz_keys()))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_xyz(self):
        """Test JAX implementation of BallMover on XYZ particles"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        d1 = IMP.core.XYZ.setup_particle(m, p1)
        p2 = IMP.Particle(m)
        d = IMP.core.XYZ.setup_particle(m, p2)
        mv = IMP.core.BallMover(m, (p1, p2), 1.0)
        X = {'xyz': jnp.array(m.get_spheres_numpy()[0])}
        ji = mv._get_jax()
        self.assertEqual(ji._keys, frozenset())

        init_func = jax.jit(ji.init_func)
        mvs = init_func(jax.random.key(42))

        j = jax.jit(ji.propose_func)
        newX, mvs, ratio = j(X, mvs)
        self.assertEqual(newX['xyz'].shape, (2, 3))
        self.assertAlmostEqual(ratio, 1.0, delta=1e-5)
        # Both particles should be moved in the same fashion
        self.assertTrue(jnp.allclose(newX['xyz'][0], newX['xyz'][1]))
        # Particles should not be further than radius from origin
        self.assertLessEqual(jnp.linalg.norm(newX['xyz'][0]), 1.0)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_float_attr(self):
        """Test JAX implementation of BallMover on arbitrary attributes"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        k = IMP.FloatKey("myf")
        p1.add_attribute(k, 1.0)
        mv = IMP.core.BallMover(m, p1, [k], 1.0)
        self.assertRaises(NotImplementedError, mv._get_jax)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_int_coord(self):
        """Test JAX implementation of BallMover on internal coordinates"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        d1 = IMP.core.XYZR.setup_particle(p1)
        d1.set_coordinates(IMP.algebra.Vector3D(1,2,3))
        d1.set_radius(4)

        rb2 = IMP.core.RigidBody.setup_particle(p2, [p1])
        mv = IMP.core.BallMover(
            m, p1, IMP.core.RigidBodyMember.get_internal_coordinate_keys(),
            1.0)
        ji = mv._get_jax()
        self.assertEqual(ji._keys, frozenset(('rigid_bodies',)))
        jm = IMP._jax_util._get_jax_model(m, ['rigid_bodies'])

        init_func = jax.jit(ji.init_func)
        mvs = init_func(jax.random.key(42))

        j = jax.jit(ji.propose_func)
        new_jm, mvs, ratio = j(jm, mvs)
        self.assertAlmostEqual(ratio, 1.0, delta=1e-5)
        # Particle should not be further than radius from starting
        # position (internal coordinate of [0,0,0])
        self.assertLessEqual(
            jnp.linalg.norm(new_jm['rigid_bodies'].intcoord[0]), 1.0)


if __name__ == '__main__':
    IMP.test.main()
