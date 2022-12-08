from __future__ import print_function
import IMP
import IMP.test
import pickle


class Tests(IMP.test.TestCase):
    """Test VersionInfo"""

    def test_pickle(self):
        """Test (un-)pickle of VersionInfo"""
        v1 = IMP.VersionInfo("mod1", "ver1")
        v2 = IMP.VersionInfo("mod2", "ver2")
        v2.foo = 'bar'
        dump = pickle.dumps((v1, v2))

        newv1, newv2 = pickle.loads(dump)
        self.assertEqual(v1, newv1)
        self.assertEqual(v2, newv2)
        self.assertEqual(newv2.foo, 'bar')

        self.assertRaises(TypeError, v1._set_from_binary, 42)


if __name__ == '__main__':
    IMP.test.main()
