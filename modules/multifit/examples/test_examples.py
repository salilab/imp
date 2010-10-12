import IMP.test
import IMP.multifit

class MultifitTests(IMP.test.TestCase):

    def test_optimize_specific_mapping(self):
        """Test example optimize_specific_mapping.py"""
        self.run_example(
                IMP.multifit.get_example_path("optimize_specific_mapping.py"))
    # This example is not yet fully implemented, so skip it for now
    test_optimize_specific_mapping = \
       IMP.test.skip("not fully implemented")(test_optimize_specific_mapping)

if __name__ == '__main__':
    IMP.test.main()
