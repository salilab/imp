import IMP
import IMP.test
import IMP.em
import struct


def _write_em_file(fname, em_type, fmt, values, nx=2, ny=2, nz=1):
    """Write a minimal little-endian EM file containing the given values"""
    header = struct.pack('<4B3i', 6, 0, 0, em_type, nx, ny, nz)
    header += b'\0' * 80                     # comment
    emdata = [0] * 40
    emdata[6] = 1000                         # Objectpixelsize (scaled by 1000)
    header += struct.pack('<40i', *emdata)
    header += b'\0' * 256                    # user data
    assert len(header) == 512
    with open(fname, 'wb') as fh:
        fh.write(header)
        fh.write(struct.pack('<%d%s' % (len(values), fmt), *values))


class Tests(IMP.test.TestCase):

    def test_float_round_trip(self):
        """Float voxel data should survive an EM write/read round trip"""
        # Most of these values contain a zero byte in their binary
        # representation
        vals = [1.0, 2.0, 0.5, 3.25, 1.1, -1.0, 0.0, 7.5]
        m = IMP.em.create_density_map(2, 2, 2, 1.0)
        for i, v in enumerate(vals):
            m.set_value(i, v)
        fname = self.get_tmp_file_name('float.em')
        IMP.em.write_map(m, fname, IMP.em.EMReaderWriter())
        r = IMP.em.read_map(fname, IMP.em.EMReaderWriter())
        for i, v in enumerate(vals):
            self.assertAlmostEqual(r.get_value(i), v, delta=1e-6)

    def test_read_integer_types(self):
        """Byte and integer EM voxel data should be converted to float"""
        for em_type, fmt, vals in ((1, 'B', [0, 1, 200, 255]),
                                   (2, 'h', [-300, 0, 1, 1000]),
                                   (4, 'i', [-70000, 0, 1, 70000]),
                                   (5, 'f', [1.0, -2.0, 0.5, 0.0])):
            fname = self.get_tmp_file_name('type%d.em' % em_type)
            _write_em_file(fname, em_type, fmt, vals)
            r = IMP.em.read_map(fname, IMP.em.EMReaderWriter())
            self.assertEqual(r.get_number_of_voxels(), 4)
            for i, v in enumerate(vals):
                self.assertAlmostEqual(r.get_value(i), v, delta=1e-6)

    def test_header_fields(self):
        """EM header fields should survive a write/read round trip"""
        m = IMP.em.create_density_map(2, 2, 2, 1.0)
        h = m.get_header_writable()
        h.MarkerX = 12.
        h.MarkerY = 34.
        h.FocusIncrement = 5.
        h.CountsPerElectron = 7.
        fname = self.get_tmp_file_name('header.em')
        IMP.em.write_map(m, fname, IMP.em.EMReaderWriter())
        r = IMP.em.read_map(fname, IMP.em.EMReaderWriter())
        rh = r.get_header()
        self.assertAlmostEqual(rh.MarkerX, 12., delta=1e-4)
        self.assertAlmostEqual(rh.MarkerY, 34., delta=1e-4)
        self.assertAlmostEqual(rh.FocusIncrement, 5., delta=1e-4)
        self.assertAlmostEqual(rh.CountsPerElectron, 7., delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
