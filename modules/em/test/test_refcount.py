import IMP.em
import IMP.test

class Tests(IMP.test.TestCase):
    """Test refcounting of IMP.em classes"""

    def test_read_density_map(self):
        """Check refcounting of IMP.em.DensityMap objects read from files"""
        refcnt = IMP.test.RefCountChecker(self)
        erw = IMP.em.EMReaderWriter()
        # ReaderWriters are refcounted
        refcnt.assert_number(1)
        map = IMP.em.read_map(self.get_input_file_name("in.em"), erw)
        refcnt.assert_number(2)
        # read_map does not keep a C++ reference, so Python's should
        # be the only one
        self.assertEqual(map.get_ref_count(), 1)
        del map
        refcnt.assert_number(1)
        del erw
        refcnt.assert_number(0)

if __name__ == '__main__':
    IMP.test.main()
