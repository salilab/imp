import IMP
import IMP.test
import IMP.em


class Tests(IMP.test.TestCase):

    def _check_round_trip(self, suffix, rw):
        for data_type in (0, 1, 2, 5):
            m = IMP.em.create_density_map(4, 4, 4, 1.0)
            for i in range(m.get_number_of_voxels()):
                m.set_value(i, 10.0 + i)
            # A map read from an 8- or 16-bit file has a data type that
            # does not match the float data we write
            m.get_header_writable().set_data_type(data_type)
            fname = self.get_tmp_file_name('dt%d.%s' % (data_type, suffix))
            IMP.em.write_map(m, fname, rw)
            r = IMP.em.read_map(fname, rw)
            for i in range(m.get_number_of_voxels()):
                self.assertAlmostEqual(r.get_value(i), 10.0 + i, delta=1e-5)

    def test_mrc(self):
        """MRC files should be written as float regardless of data type"""
        self._check_round_trip('mrc', IMP.em.MRCReaderWriter())

    def test_em(self):
        """EM files should be written as float regardless of data type"""
        self._check_round_trip('em', IMP.em.EMReaderWriter())


if __name__ == '__main__':
    IMP.test.main()
