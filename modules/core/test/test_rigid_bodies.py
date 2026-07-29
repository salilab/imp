import IMP
import IMP.test
import IMP.core
import IMP.algebra
try:
    import jax
except ImportError:
    jax = None


_RB_QUAT_KEY = IMP.core.RigidBody.get_rotation_key()


class Tests(IMP.test.TestCase):

    """Tests for RigidBody function"""

    def _add_rb_restraints(self, rbd):
        # intentionally kept trivial to ensure convergence
        r = IMP.algebra.get_identity_rotation_3d()
        t = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_sphere_3d())
        tr = IMP.algebra.Transformation3D(r, t)
        mbs = rbd.get_rigid_members()
        m = rbd.get_particle().get_model()
        rs = []
        for b in mbs:
            mb = IMP.core.RigidMember(b.get_particle())
            lc = mb.get_internal_coordinates()
            lct = tr.get_transformed(lc)
            dt = IMP.core.DistanceToSingletonScore(
                IMP.core.Harmonic(0, 1), lct)
            r = IMP.core.SingletonRestraint(m, dt, mb.get_particle())
            rs.append(r)
        return rs

    def _create_hierarchy(self, m, n=10):
        rd = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                         IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        rd.set_name("rigid body")
        hd = IMP.core.Hierarchy.setup_particle(rd.get_particle())
        for i in range(0, n):
            crd = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                              IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
            chd = IMP.core.Hierarchy.setup_particle(crd.get_particle())
            hd.add_child(chd)
            chd.set_name("child%d" % i)
        return rd.get_particle()

    def test_dependencies(self):
        """Test dependencies"""
        m = IMP.Model()
        p = self._create_hierarchy(m)
        h = IMP.core.Hierarchy(p)
        children = h.get_children()
        cs = IMP.core.XYZs(children)
        rbd = IMP.core.RigidBody.setup_particle(p, cs)
        p.set_has_required_score_states(True)
        print(m.get_score_states(), m.get_model_objects())
        dg = IMP.get_dependency_graph(m)
        # IMP.show_graphviz(dg)
        ss = p.get_required_score_states()
        self.assertEqual(len(ss), 1)
        rs = self._add_rb_restraints(rbd)
        rs[0].set_has_required_score_states(True)
        ss = rs[0].get_required_score_states()
        self.assertEqual(len(ss), 2)

    def test_create_one(self):
        """Testing create_rigid_body"""
        count = 3
        success = 0
        for i in range(0, count):
            m = IMP.Model()
            IMP.set_log_level(IMP.SILENT)
            print("creating")
            p = self._create_hierarchy(m)
            print("created", p)
            print("wrapping")
            h = IMP.core.Hierarchy(p)
            print("getting children")
            children = h.get_children()
            print("wrapping children")
            cs = IMP.core.XYZs(children)
            print("setting up")
            rbd = IMP.core.RigidBody.setup_particle(p, cs)
            print("set up")
            p.show()
            rbd.set_coordinates_are_optimized(True)
            rs = self._add_rb_restraints(rbd)
            sf = IMP.core.RestraintsScoringFunction(rs)
            cg = IMP.core.ConjugateGradients(m)
            cg.set_scoring_function(sf)
            print("Initial score is " + str(sf.evaluate(False)))
            cg.optimize(1000)
            if sf.evaluate(False) < .1:
                success = success + 1
        self.assertGreater(success, count / 2.0)

    def test_remove_member(self):
        """Test RigidBody.remove_member()"""
        m = IMP.Model()
        member = IMP.core.RigidMember.setup_particle(IMP.Particle(m))
        body = IMP.core.RigidBody.setup_particle(
                     IMP.core.RigidMember.setup_particle(
                           IMP.Particle(m)), IMP.algebra.ReferenceFrame3D())

        rb = IMP.core.RigidBody.setup_particle(IMP.Particle(m), [member, body])
        self.assertTrue(IMP.core.RigidMember.get_is_setup(member))
        self.assertTrue(IMP.core.RigidMember.get_is_setup(body))
        self.assertEqual(len(rb.get_body_member_particle_indexes()), 1)
        self.assertEqual(len(rb.get_member_particle_indexes()), 1)
        rb.remove_member(member)
        self.assertEqual(len(rb.get_body_member_particle_indexes()), 1)
        self.assertEqual(len(rb.get_member_particle_indexes()), 0)
        rb.remove_member(body)
        self.assertEqual(len(rb.get_body_member_particle_indexes()), 0)
        self.assertEqual(len(rb.get_member_particle_indexes()), 0)
        self.assertFalse(IMP.core.RigidMember.get_is_setup(member))
        self.assertFalse(IMP.core.RigidMember.get_is_setup(body))
        self.assertRaisesUsageException(rb.remove_member, member)
        self.assertRaisesUsageException(rb.remove_member, body)

    def test_get_members(self):
        """Test rigid body get_member* functions"""
        m = IMP.Model()
        rb = IMP.core.RigidBody.setup_particle(IMP.Particle(m),
                                               IMP.algebra.ReferenceFrame3D())
        rigid_member = IMP.core.RigidMember.setup_particle(IMP.Particle(m))
        non_rigid_member = IMP.core.NonRigidMember.setup_particle(
                                                           IMP.Particle(m))
        rigid_body = IMP.core.RigidBody.setup_particle(
                     IMP.core.RigidMember.setup_particle(
                           IMP.Particle(m)), IMP.algebra.ReferenceFrame3D())
        non_rigid_body = IMP.core.RigidBody.setup_particle(
            IMP.core.NonRigidMember.setup_particle(
                IMP.Particle(m)), IMP.algebra.ReferenceFrame3D())
        rb.add_member(rigid_member)
        rb.add_non_rigid_member(non_rigid_member)
        rb.add_member(rigid_body)
        rb.add_non_rigid_member(non_rigid_body)
        # Rigid members/bodies
        self.assertEqual(rb.get_rigid_members(), [rigid_member, rigid_body])
        # All members, rigid/nonrigid, member/body
        self.assertNumPyArrayEqual(
            rb.get_member_indexes(),
            IMP.get_indexes([rigid_member, non_rigid_member,
                             rigid_body, non_rigid_body]))
        # Rigid/nonrigid members
        self.assertNumPyArrayEqual(
            rb.get_member_particle_indexes(),
            IMP.get_indexes([rigid_member, non_rigid_member]))
        # Rigid/nonrigid bodies
        self.assertNumPyArrayEqual(
            rb.get_body_member_particle_indexes(),
            IMP.get_indexes([rigid_body, non_rigid_body]))

    def test_create_one_from_pdb(self):
        """Testing create_rigid_bodies"""
        m = IMP.Model()
        hs = IMP._create_particles_from_pdb(
            self.get_input_file_name("input.pdb"), m)
        print("done reading")
        rb = IMP.core.RigidBody.setup_particle(IMP.Particle(m), hs)
        rb.set_coordinates_are_optimized(True)
        print("done setting up")
        ls = hs
        keypts = [ls[0], ls[-1], ls[len(ls) // 3], ls[len(ls) // 3 * 2]]
        tr = IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                          IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0), IMP.algebra.Vector3D(500, 500, 500))))
        restraints = []
        for p in keypts:
            mp = IMP.core.RigidMember(m, p)
            ic = mp.get_internal_coordinates()
            nic = tr.get_transformed(ic)
            dt = IMP.core.DistanceToSingletonScore(
                IMP.core.Harmonic(0, 1), nic)
            restraints.append(IMP.core.SingletonRestraint(m, dt, rb))
        cg = IMP.core.ConjugateGradients(m)
        sf = IMP.core.RestraintsScoringFunction(restraints)
        cg.set_scoring_function(sf)
        cg.optimize(600)
        ntr = rb.get_reference_frame().get_transformation_to()
        print(ntr)
        print(tr)
        self.assertLess(
            (ntr.get_translation() - tr.get_translation()).get_magnitude(),
            2.2)

    def test_teardown(self):
        """Testing tearing down rigid bodies"""
        m = IMP.Model()
        ps = [IMP.core.XYZ.setup_particle(IMP.Particle(m))
              for i in range(3)]
        rbp0 = IMP.Particle(m)
        rbp0.set_name("rb0")
        rbp1 = IMP.Particle(m)
        rbp1.set_name("rb1")
        if IMP.get_check_level() >= IMP.USAGE_AND_INTERNAL:
            before = IMP.Object.get_number_of_live_objects()
            names_before = IMP.get_live_object_names()
        rb0 = IMP.core.RigidBody.setup_particle(rbp0, ps)
        rb1 = IMP.core.RigidBody.setup_particle(rbp1, [rb0])
        IMP.core.RigidBody.teardown_particle(rb1)
        IMP.core.RigidBody.teardown_particle(rb0)
        print("setting up again")
        rb0 = IMP.core.RigidBody.setup_particle(rbp0, ps)
        rb1 = IMP.core.RigidBody.setup_particle(rbp1, [rb0])
        print("tearing down")
        # Cannot teardown body that is a member of another body;
        # must remove it first
        if IMP.get_check_level() >= IMP.USAGE_AND_INTERNAL:
            self.assertRaises(IMP.UsageException,
                              IMP.core.RigidBody.teardown_particle, rb0)
        rb1.remove_member(rb0)
        IMP.core.RigidBody.teardown_particle(rb0)
        print("again")
        IMP.core.RigidBody.teardown_particle(rb1)
        failure = False
        # check cleanup
        if IMP.get_check_level() >= IMP.USAGE_AND_INTERNAL:
            after = IMP.Object.get_number_of_live_objects()
            names_after = IMP.get_live_object_names()
            for n in names_after:
                if n not in names_before:
                    print("found new object", n)
            print(before, after, names_before, names_after)
            if before != after or len(names_before) != len(names_after):
                failure = True
        self.assertTrue(not failure)

    def test_update_rigid_body_members(self):
        """Test _UpdateRigidBodyMembers modifier"""
        m = IMP.Model()
        p = self._create_hierarchy(m)
        h = IMP.core.Hierarchy(p)
        children = h.get_children()
        cs = IMP.core.XYZs(children)
        rbd = IMP.core.RigidBody.setup_particle(p, cs)
        # Make sure that modifier recreates the original member coordinates
        mod = IMP.core._UpdateRigidBodyMembers()
        oldxyz = cs[0].get_coordinates()
        cs[0].set_coordinates(IMP.algebra.Vector3D(0, 0, 0))
        mod.apply_index(m, rbd)
        self.assertLess(
            IMP.algebra.get_distance(oldxyz, cs[0].get_coordinates()), 1e-3)

    def test_accumulate_rigid_body_derivatives(self):
        """Test _AccumulateRigidBodyDerivatives modifier"""
        m = IMP.Model()
        p = self._create_hierarchy(m, n=10)
        h = IMP.core.Hierarchy(p)
        children = h.get_children()
        cs = IMP.core.XYZs(children)
        rbd = IMP.core.RigidBody.setup_particle(p, cs)
        rbd.set_coordinates_are_optimized(True)
        rs = self._add_rb_restraints(rbd)
        sf = IMP.core.RestraintsScoringFunction(rs)
        x = sf.evaluate(True)
        d = IMP.DerivativeAccumulator(1.0)
        for x in cs:
            x.add_to_derivatives(IMP.algebra.Vector3D(1000, 2000, 3000), d)
        # Derivatives on the rigid body should be (roughly) 10x those on
        # individual particles
        mod = IMP.core._AccumulateRigidBodyDerivatives()
        mod.apply_index(m, rbd)
        derivs = rbd.get_derivatives()
        self.assertLess(IMP.algebra.get_distance(
            derivs, IMP.algebra.Vector3D(10000, 20000, 30000)), 40.)

    def test_normalize_rotation(self):
        """Test _NormalizeRotation modifier"""
        m = IMP.Model()
        p = self._create_hierarchy(m)
        h = IMP.core.Hierarchy(p)
        children = h.get_children()
        cs = IMP.core.XYZs(children)
        rbd = IMP.core.RigidBody.setup_particle(p, cs)
        mod = IMP.core._NormalizeRotation()

        # Zero quaternion should be reset to identity
        m.set_attribute(_RB_QUAT_KEY, rbd,
                        IMP.algebra.Vector4D(0.0, 0.0, 0.0, 0.0))
        mod.apply_index(m, rbd)
        rot = rbd.get_reference_frame().get_transformation_to().get_rotation()
        self.assertEqual([int(x * 10.) for x in rot.get_quaternion()],
                         [10, 0, 0, 0])

        # Non-normalized quaternion should be normalized
        m.set_attribute(_RB_QUAT_KEY, rbd,
                        IMP.algebra.Vector4D(0.0, 2.0, 0.0, 0.0))
        mod.apply_index(m, rbd)
        rot = rbd.get_reference_frame().get_transformation_to().get_rotation()
        self.assertEqual([int(x * 10.) for x in rot.get_quaternion()],
                         [0, 10, 0, 0])

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_rigid_body_normalize_constraint(self):
        """Test JAX _RigidBodyNormalizeConstraint"""
        import IMP._jax_util
        import jax.numpy as jnp

        m = IMP.Model()
        p = self._create_hierarchy(m)
        h = IMP.core.Hierarchy(p)
        children = h.get_children()
        cs = IMP.core.XYZs(children)
        rbd = IMP.core.RigidBody.setup_particle(p, cs)

        # _RigidBodyNormalizeConstraint should have been created automatically;
        # find it by name
        ss, = [s.get_derived_object() for s in m.get_ordered_score_states()
               if s.get_name().startswith('normalize')]
        ji = ss._get_jax()
        jm = ji.get_jax_model()
        apply_func = jax.jit(ji.apply_func)

        # Zero quaternion should be reset to identity
        jm['rigid_bodies'].quaternion = jnp.array([[0., 0., 0., 0.]])
        jm = apply_func(jm)
        self.assertEqual(
            [int(x * 10.) for x in jm['rigid_bodies'].quaternion[0]],
            [10, 0, 0, 0])

        # Non-normalized quaternion should be normalized
        jm['rigid_bodies'].quaternion = jnp.array([[0., 2., 0., 0.]])
        jm = apply_func(jm)
        self.assertEqual(
            [int(x * 10.) for x in jm['rigid_bodies'].quaternion[0]],
            [0, 10, 0, 0])

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_rigid_body_position_constraint(self):
        """Test JAX _RigidBodyPositionConstraint"""
        import IMP._jax_util
        import numpy as np

        m = IMP.Model()
        p = self._create_hierarchy(m)
        h = IMP.core.Hierarchy(p)
        children = h.get_children()
        cs0_index = int(children[0].get_particle_index())
        cs = IMP.core.XYZs(children)
        rbd = IMP.core.RigidBody.setup_particle(p, cs)

        # _RigidBodyPositionConstraint should have been created automatically;
        # find it by name
        ss, = [s.get_derived_object() for s in m.get_ordered_score_states()
               if s.get_name().endswith('rigid body positions')]
        ji = ss._get_jax()
        jm = ji.get_jax_model()
        apply_func = jax.jit(ji.apply_func)

        oldxyz = jm['xyz'][cs0_index].copy()

        # Make sure that constraint recreates the original member coordinates
        jm['xyz'][cs0_index] = [0,0,0]
        jm = apply_func(jm)
        newxyz = jm['xyz'][cs0_index]

        np.testing.assert_allclose(oldxyz, newxyz, rtol=1e-5)


if __name__ == '__main__':
    IMP.test.main()
