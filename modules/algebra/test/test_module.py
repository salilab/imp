import IMP.algebra
import IMP.test


class Tests(IMP.test.TestCase):

    def test_module(self):
        """Test standard module functions"""
        _ = IMP.algebra.get_module_version()
        self.assertEqual(IMP.algebra.get_module_name(), "IMP::algebra")


if __name__ == '__main__':
    IMP.test.main()
