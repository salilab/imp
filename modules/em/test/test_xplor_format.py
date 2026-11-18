import IMP
import IMP.test
import IMP.em


class Tests(IMP.test.TestCase):

    def _make_map(self):
        m = IMP.em.create_density_map(4, 5, 6, 2.0)
        m.set_origin(IMP.algebra.Vector3D(10, 20, 30))
        for i in range(m.get_number_of_voxels()):
            m.set_value(i, i + 0.5)
        return m

    def _write(self, m):
        fname = self.get_tmp_file_name('test.xplor')
        IMP.em.write_map(m, fname, IMP.em.XplorReaderWriter())
        with open(fname) as fh:
            return fname, fh.readlines()

    def _read(self, fname):
        return IMP.em.read_map(fname, IMP.em.XplorReaderWriter())

    def test_trailer(self):
        """XPLOR files with the standard mean/stdev trailer should be read"""
        m = self._make_map()
        fname, lines = self._write(m)
        # Most XPLOR writers add the map mean and standard deviation
        # after the -9999 end-of-data marker
        with open(fname, 'a') as fh:
            fh.write(" 0.12345E+03 0.65432E+02\n")
        r = self._read(fname)
        self.assertEqual(r.get_number_of_voxels(), m.get_number_of_voxels())
        for i in range(m.get_number_of_voxels()):
            self.assertAlmostEqual(r.get_value(i), i + 0.5, delta=1e-3)

    def test_extra_sections(self):
        """XPLOR data beyond the declared extent should be ignored"""
        m = self._make_map()
        fname, lines = self._write(m)
        data_start = lines.index("XYZ\n") + 1
        data_end = lines.index("  -9999\n")
        data = lines[data_start:data_end]
        with open(fname, 'w') as fh:
            fh.writelines(lines[:data_start])
            # Repeat the sections many times
            for i in range(20):
                fh.writelines(data)
            fh.writelines(lines[data_end:])
        r = self._read(fname)
        for i in range(m.get_number_of_voxels()):
            self.assertAlmostEqual(r.get_value(i), i + 0.5, delta=1e-3)

    def test_truncated(self):
        """Truncated XPLOR data should raise an IOException"""
        m = self._make_map()
        fname, lines = self._write(m)
        with open(fname, 'w') as fh:
            fh.writelines(lines[:len(lines) // 2])
        self.assertRaises(IOError, self._read, fname)

    def test_bad_header(self):
        """Malformed XPLOR headers should raise an IOException"""
        m = self._make_map()
        fname, lines = self._write(m)
        # Fewer than 9 fields in the grid line
        bad = list(lines)
        bad[4] = "       4       5       8\n"
        with open(fname, 'w') as fh:
            fh.writelines(bad)
        self.assertRaises(IOError, self._read, fname)
        # Fewer than 6 fields in the cell line
        bad = list(lines)
        bad[5] = " 8.00000e+00\n"
        with open(fname, 'w') as fh:
            fh.writelines(bad)
        self.assertRaises(IOError, self._read, fname)
        # Zero grid size
        bad = list(lines)
        bad[4] = "       0" + lines[4][8:]
        with open(fname, 'w') as fh:
            fh.writelines(bad)
        self.assertRaises(IOError, self._read, fname)

    def test_missing_file(self):
        """Reading a nonexistent XPLOR file should raise an IOException"""
        self.assertRaises(IOError, self._read, "/does/not/exist.xplor")


if __name__ == '__main__':
    IMP.test.main()
