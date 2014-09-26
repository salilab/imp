import unittest
import IMP.rmf
import IMP.test
import IMP.core
import RMF
from IMP.algebra import *


class Tests(IMP.test.TestCase):

    def test_navigation(self):
        """Test loading and saving of rigid bodies implicitly"""
        for suffix in IMP.rmf.suffixes:
            m = IMP.kernel.Model()
            m.set_log_level(IMP.base.SILENT)
            r = IMP.atom.Hierarchy.setup_particle(IMP.kernel.Particle(m))
            r.set_name("rt")
            rbd = IMP.core.RigidBody.setup_particle(
                IMP.kernel.Particle(m, "rb"),
                IMP.algebra.ReferenceFrame3D())
            ps = []
            for i in range(0, 4):
                p = IMP.kernel.Particle(m)
                v = IMP.algebra.Vector3D(0, 0, 0)
                v[0] = i % 2
                if i >= 2:
                    v[1] = 1
                else:
                    v[1] = 0
                d = IMP.core.XYZR.setup_particle(p)
                d.set_coordinates(v)
                d.set_radius(.5)
                IMP.atom.Mass.setup_particle(p, .1)
                r.add_child(IMP.atom.Hierarchy.setup_particle(p))
                rbd.add_member(p)
                ps.append(p)

            p = IMP.kernel.Particle(m)
            v = IMP.algebra.Vector3D(.5, .5, .5)
            d = IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(v)
            d.set_radius(.5)
            IMP.atom.Mass.setup_particle(p, .1)
            r.add_child(IMP.atom.Hierarchy.setup_particle(p))
            rbd.add_member(p)
            ps.append(p)

            nrbps = []
            for i in range(0, 4):
                p = IMP.kernel.Particle(m)
                v = IMP.algebra.Vector3D(0, 0, 1)
                v[0] = i % 2
                if i >= 2:
                    v[1] = 1
                else:
                    v[1] = 0
                d = IMP.core.XYZR.setup_particle(p)
                d.set_coordinates(v)
                d.set_radius(.5)
                IMP.atom.Mass.setup_particle(p, .1)
                r.add_child(IMP.atom.Hierarchy.setup_particle(p))
                rbd.add_non_rigid_member(p.get_index())
                ps.append(p)
                nrbps.append(p)
            fn = self.get_tmp_file_name("rigid_implicit" + suffix)
            f = RMF.create_rmf_file(fn)
            IMP.rmf.add_hierarchies(f, [r])
            coords = []
            coords.append([IMP.core.XYZ(p).get_coordinates() for p in ps])
            IMP.rmf.save_frame(f, str(0))
            frames = [rbd.get_reference_frame()]
            for i in range(0, 10):
                bb = IMP.algebra.get_unit_bounding_box_3d()
                tr = IMP.algebra.Transformation3D(
                    IMP.algebra.get_random_rotation_3d(),
                    IMP.algebra.get_random_vector_in(bb))
                rf = IMP.algebra.ReferenceFrame3D(tr)
                rbd.set_reference_frame(rf)
                frames.append(rf)
                for p in nrbps:
                    IMP.core.NonRigidMember(p)\
                        .set_internal_coordinates(IMP.algebra.get_random_vector_in(bb))
                m.update()
                IMP.rmf.save_frame(f, str(i + 1))
                coords.append([IMP.core.XYZ(p).get_coordinates() for p in ps])
            del f

            f = RMF.open_rmf_file_read_only(fn)
            IMP.base.set_log_level(IMP.base.VERBOSE)
            r2 = IMP.rmf.create_hierarchies(f, m)[0]
            for pi in m.get_particle_indexes():
                if IMP.core.RigidBody.get_is_setup(m, pi) and\
                        not IMP.core.RigidBodyMember.get_is_setup(m, pi):
                    IMP.core.show_rigid_body_hierarchy(
                        IMP.core.RigidBody(m, pi), sys.stdout)
            ps = IMP.atom.get_leaves(r2)
            rb = IMP.core.RigidMember(ps[0]).get_rigid_body()
            frame0 = rb.get_reference_frame()
            self.assert_(IMP.core.RigidMember.get_is_setup(r2.get_child(0)))
            for i in range(0, 11):
                print "loading", i
                # if i != 0:
                IMP.rmf.load_frame(f, RMF.FrameID(i))
                print i, frames[i], frame0.get_local_reference_frame(rb.get_reference_frame())
                for j, c in enumerate(ps):
                    oc = IMP.core.XYZ(c).get_coordinates()
                    print "before update", j, c, oc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates()
                    m.update()
                    nc = IMP.core.XYZ(c).get_coordinates()
                    print "after update", i, j, c, nc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates()
                    self.assertAlmostEqual((oc - nc).get_magnitude(), 0,
                                           delta=.1)
                    self.assertAlmostEqual(
                        (coords[i][j] - nc).get_magnitude(),
                        0,
                        delta=.1)
                    self.assertAlmostEqual(
                        (coords[i][j] - oc).get_magnitude(),
                        0,
                        delta=.1)
                print "ok"

    def test_gaussian_round_trip(self):
        """Make sure that Gaussians can be written to and read from RMFs"""
        for suffix in IMP.rmf.suffixes:
            m = IMP.kernel.Model()
            r = IMP.atom.Hierarchy.setup_particle(IMP.kernel.Particle(m))
            r.set_name("rt")
            p = IMP.Particle(m)
            v = IMP.algebra.Vector3D(1, 2, 3)
            d = IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(v)
            d.set_radius(.5)
            IMP.atom.Mass.setup_particle(p, .1)
            t = IMP.algebra.Transformation3D(
                                     IMP.algebra.get_identity_rotation_3d(),
                                     IMP.algebra.Vector3D(1,2,3))
            IMP.core.Gaussian.setup_particle(p,IMP.algebra.Gaussian3D(
                                     IMP.algebra.ReferenceFrame3D(t),[4,5,6]))
            r.add_child(IMP.atom.Hierarchy.setup_particle(p))
            fn = self.get_tmp_file_name("gaussian" + suffix)
            f = RMF.create_rmf_file(fn)
            IMP.rmf.add_hierarchies(f, [r])
            IMP.rmf.save_frame(f, 0)
            del f
            f = RMF.open_rmf_file_read_only(fn)
            prots = IMP.rmf.create_hierarchies(f, m)
            g = IMP.core.Gaussian(prots[0].get_child(0))
            self.assertEqual([v for v in g.get_variances()], [4,5,6])
            self.assertAlmostEqual(IMP.core.XYZR(g).get_radius(), .5,
                                   delta=1e-6)
            self.assertAlmostEqual(IMP.atom.Mass(g).get_mass(), .1, delta=1e-6)
            v = g.get_reference_frame().get_transformation_to()
            v = v.get_translation()
            self.assertEqual([i for i in v], [1,2,3])

    def test_nested_rigid_body(self):
        """Test loading and saving of rigid bodies that contain
        non-rigid members that are also Gaussians (and thus Rigid Bodies)"""
        for suffix in IMP.rmf.suffixes:
            m = IMP.kernel.Model()
            m.set_log_level(IMP.base.SILENT)
            r = IMP.atom.Hierarchy.setup_particle(IMP.kernel.Particle(m))
            r.set_name("rt")
            rbd = IMP.core.RigidBody.setup_particle(
                IMP.kernel.Particle(m, "rb"),
                IMP.algebra.ReferenceFrame3D())
            ps = []
            for i in range(0, 4):
                p = IMP.kernel.Particle(m)
                v = IMP.algebra.Vector3D(0, 0, 0)
                v[0] = i % 2
                if i >= 2:
                    v[1] = 1
                else:
                    v[1] = 0
                d = IMP.core.XYZR.setup_particle(p)
                d.set_coordinates(v)
                d.set_radius(.5)
                IMP.atom.Mass.setup_particle(p, .1)
                r.add_child(IMP.atom.Hierarchy.setup_particle(p))
                rbd.add_member(p)
                ps.append(p)

            p = IMP.kernel.Particle(m)
            v = IMP.algebra.Vector3D(.5, .5, .5)
            d = IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(v)
            d.set_radius(.5)
            IMP.atom.Mass.setup_particle(p, .1)
            r.add_child(IMP.atom.Hierarchy.setup_particle(p))
            rbd.add_member(p)
            ps.append(p)

            nrbps = []
            for i in range(0, 4):
                p = IMP.kernel.Particle(m)
                v = IMP.algebra.Vector3D(0, 0, 1)
                v[0] = i % 2
                if i >= 2:
                    v[1] = 1
                else:
                    v[1] = 0
                d = IMP.core.XYZR.setup_particle(p)
                d.set_coordinates(v)
                d.set_radius(.5)
                IMP.atom.Mass.setup_particle(p, .1)
                IMP.core.Gaussian.setup_particle(p,IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(),[1,1,1]))
                r.add_child(IMP.atom.Hierarchy.setup_particle(p))
                rbd.add_non_rigid_member(p.get_index())
                ps.append(p)
                nrbps.append(p)
            fn = self.get_tmp_file_name("rigid_implicit" + suffix)
            f = RMF.create_rmf_file(fn)
            IMP.rmf.add_hierarchies(f, [r])
            coords = []
            coords.append([IMP.core.XYZ(p).get_coordinates() for p in ps])
            IMP.rmf.save_frame(f, str(0))
            frames = [rbd.get_reference_frame()]
            for i in range(0, 10):
                bb = IMP.algebra.get_unit_bounding_box_3d()
                tr = IMP.algebra.Transformation3D(
                    IMP.algebra.get_random_rotation_3d(),
                    IMP.algebra.get_random_vector_in(bb))
                rf = IMP.algebra.ReferenceFrame3D(tr)
                rbd.set_reference_frame(rf)
                frames.append(rf)
                for p in nrbps:
                    IMP.core.NonRigidMember(p)\
                        .set_internal_coordinates(IMP.algebra.get_random_vector_in(bb))
                m.update()
                IMP.rmf.save_frame(f, str(i + 1))
                coords.append([IMP.core.XYZ(p).get_coordinates() for p in ps])
            del f

            f = RMF.open_rmf_file_read_only(fn)
            IMP.base.set_log_level(IMP.base.VERBOSE)
            r2 = IMP.rmf.create_hierarchies(f, m)[0]
            for pi in m.get_particle_indexes():
                if IMP.core.RigidBody.get_is_setup(m, pi) and\
                        not IMP.core.RigidBodyMember.get_is_setup(m, pi):
                    IMP.core.show_rigid_body_hierarchy(
                        IMP.core.RigidBody(m, pi), sys.stdout)
            ps = IMP.atom.get_leaves(r2)
            rb = IMP.core.RigidMember(ps[0]).get_rigid_body()
            frame0 = rb.get_reference_frame()
            self.assert_(IMP.core.RigidMember.get_is_setup(r2.get_child(0)))
            for i in range(0, 11):
                print "loading", i
                # if i != 0:
                IMP.rmf.load_frame(f, RMF.FrameID(i))
                print i, frames[i], frame0.get_local_reference_frame(rb.get_reference_frame())
                for j, c in enumerate(ps):
                    oc = IMP.core.XYZ(c).get_coordinates()
                    print "before update", j, c, oc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates()
                    m.update()
                    nc = IMP.core.XYZ(c).get_coordinates()
                    print "after update", i, j, c, nc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates()
                    self.assertAlmostEqual((oc - nc).get_magnitude(), 0,
                                           delta=.1)
                    self.assertAlmostEqual(
                        (coords[i][j] - nc).get_magnitude(),
                        0,
                        delta=.1)
                    self.assertAlmostEqual(
                        (coords[i][j] - oc).get_magnitude(),
                        0,
                        delta=.1)
                print "ok"

    def _create_rb(self, m):
        h = IMP.atom.Hierarchy.setup_particle(m, m.add_particle("h"))
        IMP.core.XYZR.setup_particle(
            h, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0, 0, 0), 0))
        rb = IMP.core.RigidBody.setup_particle(
            m,
            m.add_particle("rb"),
            IMP.algebra.ReferenceFrame3D())
        IMP.atom.Mass.setup_particle(h, 1)
        rb.add_member(h)
        return h

    def test_multiple(self):
        """Test that adding with multiple calls results in unique ids"""
        m = IMP.kernel.Model()
        h0 = self._create_rb(m)
        h1 = self._create_rb(m)
        path = self.get_tmp_file_name("multiple_rb.rmf3")
        print path
        fh = RMF.create_rmf_file(path)
        IMP.rmf.add_hierarchy(fh, h0)
        IMP.rmf.add_hierarchy(fh, h1)
        IMP.rmf.save_frame(fh, "frame")
        del fh, m, h0, h1

        m = IMP.kernel.Model()
        fh = RMF.open_rmf_file_read_only(path)
        hs = IMP.rmf.create_hierarchies(fh, m)
        rb_count = 0
        for pi in m.get_particle_indexes():
            if IMP.core.RigidBody.get_is_setup(m, pi):
                print m.get_particle_name(pi)
                rb_count += 1
        self.assertEqual(rb_count, 2)

if __name__ == '__main__':
    IMP.test.main()
