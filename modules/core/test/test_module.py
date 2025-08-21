import IMP.core
import IMP.test


class Tests(IMP.test.TestCase):

    def test_module(self):
        """Test standard module functions"""
        _ = IMP.core.get_module_version()
        self.assertEqual(IMP.core.get_module_name(), "IMP::core")


if __name__ == '__main__':
    IMP.test.main()
