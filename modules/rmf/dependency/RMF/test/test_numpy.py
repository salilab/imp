from __future__ import print_function
import RMF
import unittest
if RMF.RMF_HAS_NUMPY:
    import numpy


def _make_rmf(with_coordinates=True):
    b = RMF.BufferHandle()
    rmf = RMF.create_rmf_buffer(b)
    pf = RMF.ParticleFactory(rmf)
    rf = RMF.ReferenceFrameFactory(rmf)
    root = rmf.get_root_node()
    p = root.add_child("particle", RMF.REPRESENTATION)
    pf.get(p).set_radius(1.0)
    pf.get(p).set_mass(1.0)
    pf.get(p).set_coordinates(RMF.Vector3(1, 2, 3))
    refframe = root.add_child("refframe", RMF.REPRESENTATION)
    rf.get(refframe).set_translation(RMF.Vector3(4, 1, 1))
    rf.get(refframe).set_rotation(RMF.Vector4(1, 0, 0, 0))
    p = refframe.add_child("particle", RMF.REPRESENTATION)
    pf.get(p).set_mass(1.0)
    if with_coordinates:
        pf.get(p).set_radius(1.0)
        pf.get(p).set_coordinates(RMF.Vector3(4, 5, 6))
    return rmf


class Tests(unittest.TestCase):

    @unittest.skipUnless(RMF.RMF_HAS_NUMPY, "No numpy support")
    def test_get_global_coordinates(self):
        """Test get global coordinates as numpy array"""
        rmf = _make_rmf()
        root = rmf.get_root_node()
        # coord array of wrong type
        self.assertRaises(TypeError, RMF.get_all_global_coordinates, rmf,
                          root, "foo")
        # coord array of wrong dimension
        self.assertRaises(ValueError, RMF.get_all_global_coordinates, rmf,
                          root, numpy.empty((1, 3)))
        self.assertRaises(ValueError, RMF.get_all_global_coordinates, rmf,
                          root, numpy.empty((8, 3)))
        coord = numpy.empty((2, 3))
        RMF.get_all_global_coordinates(rmf, root, coord)
        # Second coordinate should be transformed by the reference frame
        expected_coord = numpy.array([[1., 2., 3.], [8., 6., 7.]])
        self.assertLess(numpy.linalg.norm(coord - expected_coord), 1e-4)

    @unittest.skipUnless(RMF.RMF_HAS_NUMPY, "No numpy support")
    def test_get_global_coordinates_particle_no_coords(self):
        """Test get global coordinates with a Particle with no coordinates"""
        rmf = _make_rmf(with_coordinates=False)
        root = rmf.get_root_node()
        # The second Particle should be ignored since it has no coordinates
        coord = numpy.empty((1, 3))
        RMF.get_all_global_coordinates(rmf, root, coord)
        expected_coord = numpy.array([[1., 2., 3.]])
        self.assertLess(numpy.linalg.norm(coord - expected_coord), 1e-4)


if __name__ == '__main__':
    unittest.main()
