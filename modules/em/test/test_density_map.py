import IMP
import IMP.test
import IMP.em
import pickle


class Tests(IMP.test.TestCase):
    def test_pickle(self):
        """Test (un-)pickle of DensityMap"""
        m = IMP.em.read_map(self.get_input_file_name("mini-4.0.mrc"))
        origin = IMP.algebra.Vector3D(89.725, 44.746, -53.842)
        m.set_name('foo')
        self.assertEqual(m.get_number_of_voxels(), 19656)
        self.assertLess(IMP.algebra.get_distance(m.get_origin(), origin), 1e-4)
        self.assertAlmostEqual(m.get_spacing(), 1.3333, delta=1e-3)

        dump = pickle.dumps(m)
        del m
        m2 = pickle.loads(dump)
        self.assertEqual(m2.get_name(), 'foo')
        self.assertEqual(m2.get_number_of_voxels(), 19656)
        self.assertLess(IMP.algebra.get_distance(m2.get_origin(), origin),
                        1e-4)
        self.assertAlmostEqual(m2.get_spacing(), 1.3333, delta=1e-3)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            data = m2.get_data()
            self.assertEqual(data.shape, (27, 28, 26))
            self.assertAlmostEqual(data[17, 14, 10], 1.006, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
