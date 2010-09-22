import sys
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.helper
import time


class DOMINOTests(IMP.test.TestCase):

    def test_global_min1(self):
        """Test six particles optimization"""
        spo= IMP.domino2.get_example_path("six_particles_optimization.py")
        vars={'cs':None}
        execfile(spo, vars)
        self.assertEqual(vars['cs'].get_number_of_configurations(), 1)
    def test_global_min2(self):
        """Test six particles loopy optimization"""
        spo= IMP.domino2.get_example_path("six_particles_loopy_optimization.py")
        vars={'cs':None}
        execfile(spo, vars)
        self.assertEqual(vars['cs'].get_number_of_configurations(), 1)
    def test_global_min3(self):
        """Test six particles custom filter optimization"""
        spo= IMP.domino2.get_example_path("custom_filter.py")
        vars={'cs':None}
        execfile(spo, vars)
        self.assertEqual(vars['cs'].get_number_of_configurations(), 1)
if __name__ == '__main__':
    IMP.test.main()
