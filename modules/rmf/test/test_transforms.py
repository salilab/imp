import RMF
import IMP.rmf
import IMP.algebra
import IMP.test
import RMF
import unittest


class Tests(IMP.test.TestCase):

    def test_simple(self):
        """Test that nested transforms match in IMP and RMF"""
        coords = list(IMP.algebra.get_random_vector_in(
            IMP.algebra.get_unit_sphere_3d()))

        name = RMF._get_temporary_file_path("test_transform.rmfz")
        fh = RMF.create_rmf_file(name)

        rr0 = IMP.algebra.get_random_rotation_3d()
        rr1 = IMP.algebra.get_random_rotation_3d()

        t0 = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_sphere_3d())
        t1 = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_sphere_3d())

        rff = RMF.ReferenceFrameFactory(fh)
        rfcf = RMF.ReferenceFrameFactory(fh)
        pf = RMF.ParticleFactory(fh)

        ch0 = fh.get_root_node().add_child("ch0", RMF.REPRESENTATION)
        ch1 = ch0.add_child("ch1", RMF.REPRESENTATION)
        n = ch1.add_child("n", RMF.REPRESENTATION)

        rff.get(ch0).set_translation(RMF.Vector3(*t0))
        rff.get(ch1).set_translation(RMF.Vector3(*t1))
        rff.get(ch0).set_rotation(RMF.Vector4(*rr0.get_quaternion()))
        rff.get(ch1).set_rotation(RMF.Vector4(*rr1.get_quaternion()))
        pf.get(n).set_coordinates(RMF.Vector3(*coords))
        pf.get(n).set_radius(1)
        pf.get(n).set_mass(1)

        tr = IMP.algebra.Transformation3D(
            rr0,
            t0) * IMP.algebra.Transformation3D(
            rr1,
            t1)
        ctro = RMF.CoordinateTransformer()
        ctri = RMF.CoordinateTransformer(ctro, rfcf.get(ch0))
        ctr = RMF.CoordinateTransformer(ctri, rfcf.get(ch1))

        vr = ctr.get_global_coordinates(pf.get(n).get_coordinates())
        vi = tr.get_transformed(IMP.algebra.Vector3D(*coords))
        print vr, vi
        self.assertAlmostEqual(
            IMP.algebra.get_distance(vi,
                                     IMP.algebra.Vector3D(*vr)),
            0,
            delta=.1)

    def test_npc_like(self):
        """Test that nested transforms match in IMP and RMF with npc-like config"""
        coords = list(IMP.algebra.get_random_vector_in(
            IMP.algebra.get_unit_sphere_3d()))

        name = RMF._get_temporary_file_path("test_transform_npc.rmfz")
        fh = RMF.create_rmf_file(name)

        rr0 = IMP.algebra.get_random_rotation_3d()

        t0 = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_sphere_3d())

        rff = RMF.ReferenceFrameFactory(fh)
        rfcf = RMF.ReferenceFrameFactory(fh)
        pf = RMF.ParticleFactory(fh)

        ch0 = fh.get_root_node().add_child("ch0", RMF.REPRESENTATION)
        n0 = ch0.add_child("n0", RMF.REPRESENTATION)
        n1 = ch0.add_child("n1", RMF.REPRESENTATION)
        n2 = ch0.add_child("n2", RMF.REPRESENTATION)

        rff.get(ch0).set_translation(RMF.Vector3(*t0))
        rff.get(ch0).set_rotation(RMF.Vector4(*rr0.get_quaternion()))
        pf.get(ch0).set_coordinates(RMF.Vector3(0, 0, 0))
        pf.get(ch0).set_radius(1)
        pf.get(ch0).set_mass(1)

        pf.get(n0).set_coordinates(RMF.Vector3(1, 0, 0))
        pf.get(n0).set_mass(1)
        pf.get(n0).set_radius(1)
        pf.get(n1).set_coordinates(RMF.Vector3(0, 1, 0))
        pf.get(n1).set_mass(1)
        pf.get(n1).set_radius(1)
        pf.get(n2).set_coordinates(RMF.Vector3(0, 0, 1))
        pf.get(n2).set_mass(1)
        pf.get(n2).set_radius(1)

        tr = IMP.algebra.Transformation3D(rr0, t0)
        ctro = RMF.CoordinateTransformer()
        ctr = RMF.CoordinateTransformer(ctro, rfcf.get(ch0))

        vch0 = ctr.get_global_coordinates(pf.get(ch0).get_coordinates())
        vn0 = ctr.get_global_coordinates(pf.get(n0).get_coordinates())
        vn1 = ctr.get_global_coordinates(pf.get(n1).get_coordinates())
        vn2 = ctr.get_global_coordinates(pf.get(n2).get_coordinates())
        vich0 = tr.get_transformed(IMP.algebra.Vector3D(0, 0, 0))
        vin0 = tr.get_transformed(IMP.algebra.Vector3D(1, 0, 0))
        vin1 = tr.get_transformed(IMP.algebra.Vector3D(0, 1, 0))
        vin2 = tr.get_transformed(IMP.algebra.Vector3D(0, 0, 1))
        print IMP.algebra.get_distance(vin0, vich0),\
            IMP.algebra.get_distance(vin1, vich0),\
            IMP.algebra.get_distance(vin2, vich0)
        for p in [(vch0, vich0), (vn0, vin0), (vn1, vin1), (vn2, vin2)]:
            print p[0], p[1]
            self.assertAlmostEqual(
                IMP.algebra.get_distance(p[1],
                                         IMP.algebra.Vector3D(*p[0])),
                0,
                delta=.1)

    def test_matching(self):
        """Test that things match when on different levels"""


if __name__ == '__main__':
    IMP.test.main()
