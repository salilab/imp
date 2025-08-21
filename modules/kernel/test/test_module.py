import IMP
import IMP.test


class Tests(IMP.test.TestCase):

    def test_module(self):
        """Test standard module functions"""
        _ = IMP.get_module_version()
        self.assertEqual(IMP.get_module_name(), "IMP")


if __name__ == '__main__':
    IMP.test.main()
