import IMP
import IMP.test
import os.path

class Tests(IMP.test.TestCase):
    def test_data_directory(self):
        """Test key aliases"""
        k0= IMP.IntKey("0")
        k1= IMP.IntKey("1")
        ak1= IMP.IntKey.add_alias(k0, "0p")
        k2= IMP.IntKey("2")
        self.assertEqual(IMP.IntKey("0p"), k0)
        self.assertEqual(IMP.IntKey("2"), k2)
        self.assertNotEqual(k2, ak1)

if __name__ == '__main__':
    IMP.test.main()
