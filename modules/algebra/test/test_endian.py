import IMP
import IMP.test
import IMP.algebra

class Tests(IMP.test.TestCase):

    def test_endian(self):
        """Check endian functions"""
        big = IMP.algebra.get_is_big_endian()
        little = IMP.algebra.get_is_little_endian()
        # Every system we test on is either big endian or little endian
        # (we don't have any machines that are PDP-endian)
        self.assertNotEqual(big, little)

if __name__ == '__main__':
    IMP.test.main()
