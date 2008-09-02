import sys
import unittest
import IMP.utils
import IMP.test, IMP
try:
    import annotation_enumeration
except ImportError:
    annotation_enumeration = None
import my_optimizer

class DOMINOTests(IMP.test.TestCase):
    def setUp(self):
        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)
        self.sampler=my_optimizer.my_optimizer("simple_jt2.txt", "simple_jt2_restraints.txt",8)
        self.infered_score = self.sampler.optimize()
    def test_global_min(self):
        if annotation_enumeration:
            min_score2 = self.sampler.exhaustive_search()
            self.assert_(abs(self.infered_score - min_score2) < 0.1 , "the score of the minimum configuration as calculated by the inference differs from the one calculated by the model " + str(self.infered_score) + " != " + str(min_score2))
        else:
            print >> sys.stderr, "test skipped: probstat module unavailable"

    def test_inference(self):
        score = -13.8
        self.assert_( abs(self.infered_score -score) < 0.1 , "the score of the minimum configuration as calculated by the inference is wrong " + str(self.infered_score) + " != " + str(score))

if __name__ == '__main__':
    unittest.main()
