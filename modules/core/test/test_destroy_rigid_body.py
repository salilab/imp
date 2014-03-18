import IMP
import IMP.test
import IMP.core
import IMP.algebra


class Tests(IMP.test.TestCase):

    """Tests for RigidBody function"""

    def _check_distance(self, pa, pb, idist):
        dist = IMP.core.get_distance(IMP.core.XYZ(pa),
                                     IMP.core.XYZ(pb))
        self.assertAlmostEqual(dist, idist, delta=.1)

    def _check_separated(self, pa, pb):
        dist = IMP.core.get_distance(IMP.core.XYZ(pa),
                                     IMP.core.XYZ(pb))
        self.assertGreater(dist, 10.)

    def _perturb_rb(self, rb):
        r = IMP.algebra.get_random_rotation_3d()
        t = IMP.algebra.get_random_vector_in(
            IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                      IMP.algebra.Vector3D(10, 10, 10)))
        rb.set_reference_frame(
            IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(r, t)))

    def _perturb_xyz(self, xyz):
        t = IMP.algebra.get_random_vector_in(
            IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                      IMP.algebra.Vector3D(10, 10, 10)))
        xyz.set_coordinates(t)

    def _try_separate_particles(self, p1, p2):
        t = IMP.algebra.get_random_vector_in(
            IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(5, 5, 5),
                                      IMP.algebra.Vector3D(10, 10, 10)))
        IMP.core.XYZ(p1).set_coordinates(t)
        IMP.core.XYZ(p2).set_coordinates(-t)
        p1.get_model().update()

    def _perturb_all(self, all):
        for a in all:
            if IMP.core.RigidBody.get_is_setup(a):
                rb = IMP.core.RigidBody(a)
                self._perturb_rb(rb)
            else:
                self._perturb_xyz(IMP.core.XYZ(a))
        all[0].get_model().update()

    def test_create_one_from_pdb(self):
        """Testing destroy rigid body"""
        m = IMP.kernel.Model()
        ps = []
        ds = []
        for i in range(0, 4):
            p = IMP.kernel.Particle(m)
            ps.append(p)
            d = IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(i, 0, 0))
            ds.append(d)
        p0 = IMP.kernel.Particle(m)
        p1 = IMP.kernel.Particle(m)
        p01 = IMP.kernel.Particle(m)
        rb0 = IMP.core.RigidBody.setup_particle(p0, [ps[0], ps[1]])
        rb1 = IMP.core.RigidBody.setup_particle(p1, [ps[2], ps[3]])
        rb01 = IMP.core.RigidBody.setup_particle(p01, [rb0, rb1])
        #dg = IMP.get_dependency_graph(m)
        #dg.show()
        #ordered = IMP.get_update_order(m.get_score_states())
        #print [o.get_name() for o in ordered]
        self._perturb_all(ds + [rb0, rb1, rb01])
        self._try_separate_particles(rb0, rb1)
        self._check_distance(ps[0], ps[1], 1)
        self._check_distance(ps[2], ps[3], 1)
        self._check_distance(ps[1], ps[2], 1)

        IMP.core.RigidBody.teardown_particle(rb01)
        self._perturb_all(ds + [rb0, rb1, rb01])
        self._try_separate_particles(rb0, rb1)
        self._check_distance(ps[0], ps[1], 1)
        self._check_distance(ps[2], ps[3], 1)
        self._check_separated(ps[1], ps[2])

        IMP.core.RigidBody.teardown_particle(rb0)
        self._perturb_all(ds + [rb0, rb1, rb01])
        self._try_separate_particles(ps[0], ps[1])
        self._check_distance(ps[2], ps[3], 1)
        self._check_separated(ps[0], ps[1])

        self._try_separate_particles(ps[1], rb1)
        self._check_distance(ps[2], ps[3], 1)
        self._check_separated(ps[1], ps[2])

if __name__ == '__main__':
    IMP.test.main()
