from __future__ import print_function
import IMP.test
import IMP.mmcif.restraint

class MockGaussianEMRestraint(IMP.Restraint):

    def __init__(self, m, em_filename):
        self.em_filename = em_filename
        IMP.Restraint.__init__(self, m, "MockRestraint %1%")
    def unprotected_evaluate(self, accum):
        return 0.
    def get_version_info(self):
        return IMP.VersionInfo("IMP authors", "0.1")
    def do_show(self, fh):
        fh.write('MockRestraint')
    def do_get_inputs(self):
        return []

    def get_static_info(self):
        i = IMP.RestraintInfo()
        i.add_string("type", "IMP.isd.GaussianEMRestraint")
        i.add_filename("filename", self.em_filename)
        return i

    def get_dynamic_info(self):
        i = IMP.RestraintInfo()
        i.add_float("cross correlation", 0.4)
        return i


class Tests(IMP.test.TestCase):
    def test_parse_restraint_info_empty(self):
        """Test _parse_restraint_info() with empty RestraintInfo"""
        d = IMP.mmcif.restraint._parse_restraint_info(None)
        self.assertEqual(d, {})

    def test_parse_restraint_info(self):
        """Test _parse_restraint_info()"""
        info = IMP.RestraintInfo()
        info.add_int("test int", 42)
        info.add_float("test float", 5.6)
        info.add_string("test string", "foo")
        info.add_filename("test filename", "/foo")
        info.add_floats("test floats", [1.2, 3.4])
        info.add_filenames("test filenames", ["/foo", "/bar"])
        d = IMP.mmcif.restraint._parse_restraint_info(info)
        self.assertEqual(sorted(d.keys()),
                         ['test filename', 'test filenames', 'test float',
                          'test floats', 'test int', 'test string'])
        self.assertEqual(d['test int'], 42)
        self.assertAlmostEqual(d['test float'], 5.6, delta=1e-4)
        self.assertEqual(d['test string'], "foo")
        self.assertEqual(d['test filename'], "/foo")
        self.assertAlmostEqual(d['test floats'][0], 1.2, delta=1e-4)
        self.assertAlmostEqual(d['test floats'][1], 3.4, delta=1e-4)
        self.assertEqual(d['test filenames'], ["/foo", "/bar"])

    def test_restraint_mapper_none(self):
        """Test _RestraintMapper with non-handled restraint"""
        m = IMP.Model()
        frame = None
        r = IMP._ConstRestraint(m, [], 1)
        r.set_was_used(True)
        rm = IMP.mmcif.restraint._RestraintMapper(None)
        self.assertEqual(rm.handle(r, frame), None)

    def test_restraint_mapper_gaussian_em(self):
        """Test _RestraintMapper with GaussianEM restraint"""
        s = IMP.mmcif.System()
        m = IMP.Model()
        em_filename = self.get_input_file_name('test.gmm.txt')
        r = MockGaussianEMRestraint(m, em_filename)
        r.set_was_used(True)
        rm = IMP.mmcif.restraint._RestraintMapper(s)
        frame = None
        wr = rm.handle(r, frame)
        self.assertEqual(type(wr), IMP.mmcif.restraint._GaussianEMRestraint)
        self.assertEqual(type(wr.dataset), IMP.mmcif.dataset.EMDensityDataset)


if __name__ == '__main__':
    IMP.test.main()
