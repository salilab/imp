import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.display

class Tests(IMP.test.TestCase):
    def test_global_min2(self):
        """Test proper ints list"""
        iin= [[0,1,2,3], [4,5,6,7,8]]
        out= IMP.domino._get_ints_list(iin)
        self.assertEqual(iin, out);

if __name__ == '__main__':
    IMP.test.main()
