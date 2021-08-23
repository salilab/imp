from __future__ import print_function
import IMP
import IMP.test
import IMP.em
import weakref


class Tests(IMP.test.TestCase):
    def test_xyz_loc(self):
        """Test get_x_loc() and friends"""
        m = IMP.em.read_map(self.get_input_file_name("mini-4.0.mrc"))
        p = weakref.ref(m)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            x_loc, y_loc, z_loc = m.get_x_loc(), m.get_y_loc(), m.get_z_loc()
            self.assertEqual(len(x_loc), 19656)
            self.assertEqual(len(y_loc), 19656)
            self.assertEqual(len(z_loc), 19656)
            self.assertAlmostEqual(x_loc[0], 89.73, delta=0.01)
            self.assertAlmostEqual(y_loc[0], 44.75, delta=0.01)
            self.assertAlmostEqual(z_loc[0], -53.84, delta=0.01)
            # Locations cannot be altered in Python
            self.assertRaises(ValueError, x_loc.__setitem__, 0, 42)
            self.assertRaises(ValueError, y_loc.__setitem__, 0, 42)
            self.assertRaises(ValueError, z_loc.__setitem__, 0, 42)
            # DensityMap should not be freed until all views are
            del m
            self.assertIsNotNone(p())
            del x_loc
            self.assertIsNotNone(p())
            del y_loc
            self.assertIsNotNone(p())
            del z_loc
            self.assertIsNone(p())
        else:
            self.assertRaises(NotImplementedError, m.get_x_loc)
            self.assertRaises(NotImplementedError, m.get_y_loc)
            self.assertRaises(NotImplementedError, m.get_z_loc)

    def test_data(self):
        """Test get_data()"""
        m = IMP.em.read_map(self.get_input_file_name("mini-4.0.mrc"))
        p = weakref.ref(m)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            data = m.get_data()
            self.assertEqual(data.shape, (27, 28, 26))
            self.assertAlmostEqual(data[17,14,10], 1.006, delta=1e-3)
            data[17,14,10] = 2.0
            self.assertAlmostEqual(data[17,14,10], 2.000, delta=1e-3)
            # DensityMap should not be freed until all views are
            del m
            self.assertIsNotNone(p())
            del data
            self.assertIsNone(p())
        else:
            self.assertRaises(NotImplementedError, m.get_data)


if __name__ == '__main__':
    IMP.test.main()
