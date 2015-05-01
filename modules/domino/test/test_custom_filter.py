import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.atom


class Tests(IMP.test.TestCase):

    def test_global_min3(self):
        """Test example custom_filter.py"""
        vars = self.run_example(
            IMP.domino.get_example_path("custom_filter.py"))
        self.assertEqual(vars['cs'].get_number_of_configurations(), 1)

if __name__ == '__main__':
    IMP.test.main()
