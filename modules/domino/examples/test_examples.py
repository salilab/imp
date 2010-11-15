import sys
import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.atom
import IMP.restrainer
import time


class DOMINOTests(IMP.test.TestCase):

    def test_global_min1(self):
        """Test example six_particles_optimization.py"""
        vars = self.run_example(
                IMP.domino.get_example_path("six_particles_optimization.py"))
        self.assertEqual(vars['cs'].get_number_of_configurations(), 1)

    def test_global_min2(self):
        """Test example six_particles_loopy_optimization.py"""
        vars = self.run_example(
           IMP.domino.get_example_path("six_particles_loopy_optimization.py"))
        self.assertEqual(vars['cs'].get_number_of_configurations(), 1)

    def test_global_min3(self):
        """Test example custom_filter.py"""
        vars = self.run_example(
           IMP.domino.get_example_path("custom_filter.py"))
        self.assertEqual(vars['cs'].get_number_of_configurations(), 1)

    def test_domino_approach(self):
        """Test example domino_approach.py"""
        vars = self.run_example(
           IMP.domino.get_example_path("domino_approach.py"))
    # This test is not yet fully implemented, so we expect it to fail
    test_domino_approach = IMP.test.expectedFailure(test_domino_approach)

if __name__ == '__main__':
    IMP.test.main()
