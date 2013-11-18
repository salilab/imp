import unittest
import IMP.rmf
import IMP.test
import RMF
from IMP.algebra import *


class Tests(IMP.test.TestCase):

    def test_navigation(self):
        """Test loading and saving of rigid bodies implicitly"""
        for suffix in RMF.suffixes:
            m = IMP.kernel.Model()
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
            fn = self.get_tmp_file_name("rigid_implicit." + suffix)
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
                    print j, c, oc, coords[i][j]
                    m.update()
                    nc = IMP.core.XYZ(c).get_coordinates()
                    print i, j, c, nc, coords[i][j]
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

if __name__ == '__main__':
    unittest.main()
