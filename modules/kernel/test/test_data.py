from __future__ import print_function
import IMP
import IMP.test

class Tests(IMP.test.TestCase):

    def test_get_set_data(self):
        """Test Model get/set data methods"""
        m = IMP.Model()
        mk = IMP.ModelKey("data_key")
        self.assertFalse(m.get_has_data(mk))
        rs = IMP.RestraintSet(m)
        m.add_data(mk, rs)
        self.assertTrue(m.get_has_data(mk))
        rs2 = IMP.RestraintSet.get_from(m.get_data(mk))
        self.assertEqual(rs, rs2)
        m.remove_data(mk)
        self.assertFalse(m.get_has_data(mk))

if __name__ == '__main__':
    IMP.test.main()
