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
            m = IMP.Model()
            m.set_log_level(IMP.SILENT)
            r = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            r.set_name("rt")
            rbd = IMP.core.RigidBody.setup_particle(
                IMP.Particle(m, "rb"),
                IMP.algebra.ReferenceFrame3D())
            ps = []
            for i in range(0, 4):
                p = IMP.Particle(m)
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

            p = IMP.Particle(m)
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
                p = IMP.Particle(m)
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
                rbd.add_non_rigid_member(p)
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
            IMP.set_log_level(IMP.VERBOSE)
            r2 = IMP.rmf.create_hierarchies(f, m)[0]
            for pi in m.get_particle_indexes():
                if IMP.core.RigidBody.get_is_setup(m, pi) and\
                        not IMP.core.RigidBodyMember.get_is_setup(m, pi):
                    IMP.core.show_rigid_body_hierarchy(
                        IMP.core.RigidBody(m, pi), sys.stdout)
            ps = IMP.atom.get_leaves(r2)
            rb = IMP.core.RigidMember(ps[0]).get_rigid_body()
            frame0 = rb.get_reference_frame()
            self.assertTrue(IMP.core.RigidMember.get_is_setup(r2.get_child(0)))
            for i in range(0, 11):
                print("loading", i)
                # if i != 0:
                IMP.rmf.load_frame(f, RMF.FrameID(i))
                print(i, frames[i], frame0.get_local_reference_frame(rb.get_reference_frame()))
                for j, c in enumerate(ps):
                    oc = IMP.core.XYZ(c).get_coordinates()
                    print("before update", j, c, oc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates())
                    m.update()
                    nc = IMP.core.XYZ(c).get_coordinates()
                    print("after update", i, j, c, nc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates())
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
                print("ok")

    def test_gaussian_round_trip(self):
        """Make sure that Gaussians can be written to and read from RMFs"""
        for suffix in IMP.rmf.suffixes:
            m = IMP.Model()
            r = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
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
            IMP.rmf.save_frame(f)
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

    def test_resolution_round_trip(self):
        """Make sure that rigid bodies encompassing different representations
        can be written to and read from RMFs"""
        for suffix in IMP.rmf.suffixes:
            m = IMP.Model()
            proot1=IMP.Particle(m)
            hroot1 = IMP.atom.Hierarchy.setup_particle(proot1)
            hroot1.set_name("root1")
            rep = IMP.atom.Representation.setup_particle(proot1,1)
            ps=[]
            for coor in [(1,2,3),(4,5,6)]:
                p = IMP.Particle(m)
                v = IMP.algebra.Vector3D(coor)
                d = IMP.core.XYZR.setup_particle(p)
                d.set_coordinates(v)
                d.set_radius(.5)
                IMP.atom.Mass.setup_particle(p, .1)
                hroot1.add_child(p)
                ps.append(p)
            proot10=IMP.Particle(m)
            hroot10=IMP.atom.Hierarchy.setup_particle(proot10)
            hroot10.set_name("root10")
            rep.add_representation(proot10,IMP.atom.BALLS,10)
            for coor in [(7,8,9),(10,11,12)]:
                p = IMP.Particle(m)
                v = IMP.algebra.Vector3D(coor)
                d = IMP.core.XYZR.setup_particle(p)
                d.set_coordinates(v)
                d.set_radius(.5)
                IMP.atom.Mass.setup_particle(p, .1)
                hroot10.add_child(p)
                ps.append(p)

            com=IMP.atom.CenterOfMass.setup_particle(IMP.Particle(m),ps)
            comcoor=IMP.core.XYZ(com).get_coordinates()
            tr=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),comcoor)
            rf = IMP.algebra.ReferenceFrame3D(tr)
            rbp=IMP.Particle(m)
            rb=IMP.core.RigidBody.setup_particle(rbp,rf)
            for p in ps:
                rb.add_member(p)

            fn = self.get_tmp_file_name("representation" + suffix)
            f = RMF.create_rmf_file(fn)
            IMP.rmf.add_hierarchies(f, [hroot1])
            IMP.rmf.save_frame(f)
            del f
            f = RMF.open_rmf_file_read_only(fn)
            rmfh = IMP.rmf.create_hierarchies(f, m)[0]


            lvs=IMP.atom.get_leaves(hroot1)
            rmflvs=IMP.atom.get_leaves(rmfh)
            self.assertEqual(len(rmflvs),len(lvs))

            rmflvs10=IMP.atom.get_leaves(IMP.atom.Representation(rmfh).get_representation(10))

            rmfrbs=set()
            for p in rmflvs:
                if IMP.core.RigidBodyMember.get_is_setup(p):
                    rbt = IMP.core.RigidMember(p).get_rigid_body()
                    rmfrbs.add(rbt)

            for p in rmflvs10:
                if IMP.core.RigidBodyMember.get_is_setup(p):
                    rbt = IMP.core.RigidMember(p).get_rigid_body()
                    rmfrbs.add(rbt)

            # we expected a single rigid body, containing four particles,
            # however the following fails
            self.assertEqual(len(rmfrbs),1)
            self.assertEqual(len(list(rmfrbs)[0].get_rigid_members()),len(rb.get_rigid_members()))



    def test_nested_rigid_body(self):
        """Test loading and saving of rigid bodies that contain
        non-rigid members that are also Gaussians (and thus Rigid Bodies)"""
        for suffix in IMP.rmf.suffixes:
            m = IMP.Model()
            m.set_log_level(IMP.SILENT)
            r = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            r.set_name("rt")
            rbd = IMP.core.RigidBody.setup_particle(
                IMP.Particle(m, "rb"),
                IMP.algebra.ReferenceFrame3D())
            ps = []
            for i in range(0, 4):
                p = IMP.Particle(m)
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

            p = IMP.Particle(m)
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
                p = IMP.Particle(m)
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
                rbd.add_non_rigid_member(p)
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
            IMP.set_log_level(IMP.VERBOSE)
            r2 = IMP.rmf.create_hierarchies(f, m)[0]
            for pi in m.get_particle_indexes():
                if IMP.core.RigidBody.get_is_setup(m, pi) and\
                        not IMP.core.RigidBodyMember.get_is_setup(m, pi):
                    IMP.core.show_rigid_body_hierarchy(
                        IMP.core.RigidBody(m, pi), sys.stdout)
            ps = IMP.atom.get_leaves(r2)
            rb = IMP.core.RigidMember(ps[0]).get_rigid_body()
            frame0 = rb.get_reference_frame()
            self.assertTrue(IMP.core.RigidMember.get_is_setup(r2.get_child(0)))
            for i in range(0, 11):
                print("loading", i)
                # if i != 0:
                IMP.rmf.load_frame(f, RMF.FrameID(i))
                print(i, frames[i], frame0.get_local_reference_frame(rb.get_reference_frame()))
                for j, c in enumerate(ps):
                    oc = IMP.core.XYZ(c).get_coordinates()
                    print("before update", j, c, oc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates())
                    m.update()
                    nc = IMP.core.XYZ(c).get_coordinates()
                    print("after update", i, j, c, nc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates())
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
                print("ok")
    def test_nested_rigid_body_linking(self):
        """Test create, save, load, link, and save with nested rigid bodies"""
        for suffix in IMP.rmf.suffixes:
            m = IMP.Model()
            m.set_log_level(IMP.SILENT)
            r = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            r.set_name("rt")
            rbd = IMP.core.RigidBody.setup_particle(
                IMP.Particle(m, "rb"),
                IMP.algebra.ReferenceFrame3D())
            ps = []
            for i in range(0, 4):
                p = IMP.Particle(m)
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

            p = IMP.Particle(m)
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
                p = IMP.Particle(m)
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
                rbd.add_non_rigid_member(p)
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


            ######## this is the new code ########
            # copy the just-written RMF file to a new RMF file
            #  the copy is performed by creating a new RMF, linking the hier,
            #  and loading/saving frames one by one

            # read the just-written RMF file once to get the hierarchy info
            f0 = RMF.open_rmf_file_read_only(fn)
            h0 = IMP.rmf.create_hierarchies(f0, m)
            del f0

            # prepare an output RMF file and add the hierarchy
            out_fn = self.get_tmp_file_name("rigid_implicit2" + suffix)
            outf = RMF.create_rmf_file(out_fn)
            IMP.rmf.add_hierarchies(outf,h0)

            # re-read the original RMF, link it, and save the frames
            f1 = RMF.open_rmf_file_read_only(fn)
            IMP.rmf.link_hierarchies(f1,h0)
            for i in range(11):
                IMP.rmf.load_frame(f1,i)
                IMP.rmf.save_frame(outf)
            del f1
            del outf

            # finally read the RMF file copy nd check if coordinates match...
            f = RMF.open_rmf_file_read_only(out_fn)
            r2 = IMP.rmf.create_hierarchies(f,m)[0]
            #####################################


            IMP.set_log_level(IMP.VERBOSE)
            for pi in m.get_particle_indexes():
                if IMP.core.RigidBody.get_is_setup(m, pi) and\
                        not IMP.core.RigidBodyMember.get_is_setup(m, pi):
                    IMP.core.show_rigid_body_hierarchy(
                        IMP.core.RigidBody(m, pi), sys.stdout)
            ps = IMP.atom.get_leaves(r2)
            rb = IMP.core.RigidMember(ps[0]).get_rigid_body()
            frame0 = rb.get_reference_frame()
            self.assertTrue(IMP.core.RigidMember.get_is_setup(r2.get_child(0)))
            for i in range(0, 11):
                print("loading", i)
                # if i != 0:
                IMP.rmf.load_frame(f, RMF.FrameID(i))
                print(i, frames[i], frame0.get_local_reference_frame(rb.get_reference_frame()))
                for j, c in enumerate(ps):
                    oc = IMP.core.XYZ(c).get_coordinates()
                    print("before update", j, c, oc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates())
                    m.update()
                    nc = IMP.core.XYZ(c).get_coordinates()
                    print("after update", i, j, c, nc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates())
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
                print("ok")

    def test_nested_rigid_body_all_nonrigid(self):
        """Create a rigid body that ONLY consists of nonrigid members
        that themselves are rigid bodies"""
        for suffix in IMP.rmf.suffixes:
            m = IMP.Model()
            m.set_log_level(IMP.SILENT)

            # create rigid body
            r = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            r.set_name("rt")
            rbd = IMP.core.RigidBody.setup_particle(
                IMP.Particle(m, "rb"),
                IMP.algebra.ReferenceFrame3D())

            # add non-rigid members
            ps = []
            nrbps = []
            for i in range(0, 4):
                p = IMP.Particle(m)
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
                rbd.add_non_rigid_member(p)
                ps.append(p)
                nrbps.append(p)


            # setup output
            fn = self.get_tmp_file_name("rigid_implicit" + suffix)
            f = RMF.create_rmf_file(fn)
            IMP.rmf.add_hierarchies(f, [r])

            # randomly move the RB and then the non-rigid members
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

            # re-read RMF file
            f = RMF.open_rmf_file_read_only(fn)
            IMP.set_log_level(IMP.VERBOSE)
            r2 = IMP.rmf.create_hierarchies(f, m)[0]
            for pi in m.get_particle_indexes():
                if IMP.core.RigidBody.get_is_setup(m, pi) and\
                        not IMP.core.RigidBodyMember.get_is_setup(m, pi):
                    IMP.core.show_rigid_body_hierarchy(
                        IMP.core.RigidBody(m, pi), sys.stdout)

            # check that coordinates are read correctly
            ps = IMP.atom.get_leaves(r2)
            rb = IMP.core.RigidBodyMember(ps[0]).get_rigid_body()
            frame0 = rb.get_reference_frame()
            self.assertTrue(
                IMP.core.RigidBodyMember.get_is_setup(r2.get_child(0)))
            for i in range(0, 11):
                print("loading", i)
                # if i != 0:
                IMP.rmf.load_frame(f, RMF.FrameID(i))
                print(i, frames[i], frame0.get_local_reference_frame(rb.get_reference_frame()))
                for j, c in enumerate(ps):
                    oc = IMP.core.XYZ(c).get_coordinates()
                    print("before update", j, c, oc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates())
                    m.update()
                    nc = IMP.core.XYZ(c).get_coordinates()
                    print("after update", i, j, c, nc, coords[i][j], IMP.core.RigidBodyMember(c).get_internal_coordinates())
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
                print("ok")

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
        m = IMP.Model()
        h0 = self._create_rb(m)
        h1 = self._create_rb(m)
        path = self.get_tmp_file_name("multiple_rb.rmf3")
        print(path)
        fh = RMF.create_rmf_file(path)
        IMP.rmf.add_hierarchy(fh, h0)
        IMP.rmf.add_hierarchy(fh, h1)
        IMP.rmf.save_frame(fh, "frame")
        del fh, m, h0, h1

        m = IMP.Model()
        fh = RMF.open_rmf_file_read_only(path)
        hs = IMP.rmf.create_hierarchies(fh, m)
        rb_count = 0
        for pi in m.get_particle_indexes():
            if IMP.core.RigidBody.get_is_setup(m, pi):
                print(m.get_particle_name(pi))
                rb_count += 1
        self.assertEqual(rb_count, 2)

if __name__ == '__main__':
    IMP.test.main()
