import sys
import unittest
import IMP.utils
import IMP.test, IMP
import my_optimizer


class DOMINOTests(IMP.test.TestCase):
    def setUp(self):
        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)
        self.sampler=my_optimizer.my_optimizer("simple_jt2.txt", "simple_jt2_restraints.txt",8)
        self.infered_score = self.sampler.optimize()
    def test_global_min(self):
        min_score2 = self.sampler.exhaustive_search()
        self.assert_( self.infered_score == min_score2 , "the minimum score as calculated by the inference differs from the one calculated by the exhaustive search " + str(self.infered_score) + " != " + str(min_score2))
        self.assert_( self.infered_score == min_score2 , "the score of the minimum configuration as calculated by the inference differs from the one calculated by the model " + str(self.infered_score) + " != " + str(min_score2))

if __name__ == '__main__':
    unittest.main()
