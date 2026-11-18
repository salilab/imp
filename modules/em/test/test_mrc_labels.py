import IMP
import IMP.test
import IMP.em
import struct

NLABL_OFFSET = 220


class Tests(IMP.test.TestCase):

    def _write_mrc(self, fname):
        m = IMP.em.create_density_map(4, 4, 4, 1.0)
        for i in range(m.get_number_of_voxels()):
            m.set_value(i, float(i))
        IMP.em.write_map(m, fname, IMP.em.MRCReaderWriter())

    def test_read_bad_nlabl(self):
        """Out-of-range MRC label count should not corrupt memory"""
        fname = self.get_tmp_file_name('bad_nlabl.mrc')
        self._write_mrc(fname)
        for nlabl in (2000, -5, 11):
            with open(fname, 'r+b') as fh:
                fh.seek(NLABL_OFFSET)
                fh.write(struct.pack('<i', nlabl))
            m = IMP.em.read_map(fname, IMP.em.MRCReaderWriter())
            nl = m.get_header().nlabl
            self.assertGreaterEqual(nl, 0)
            self.assertLessEqual(nl, 10)
            self.assertAlmostEqual(m.get_value(63), 63.0, delta=1e-6)

    def test_write_bad_nlabl(self):
        """Out-of-range label count in a header should not overrun on write"""
        m = IMP.em.create_density_map(4, 4, 4, 1.0)
        m.get_header_writable().nlabl = 2000
        fname = self.get_tmp_file_name('write_nlabl.mrc')
        IMP.em.write_map(m, fname, IMP.em.MRCReaderWriter())
        with open(fname, 'rb') as fh:
            fh.seek(NLABL_OFFSET)
            nlabl, = struct.unpack('<i', fh.read(4))
        self.assertLessEqual(nlabl, 10)


if __name__ == '__main__':
    IMP.test.main()
