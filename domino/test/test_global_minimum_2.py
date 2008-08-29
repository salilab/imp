import sys
import unittest
import IMP.utils
import IMP.test, IMP
TRY_EX=True
try:
    import AnnotationEnumeration
except:
    TRY_EX=False
    print "test_global_minimum_2: can not load AnnotationEnumeration, inference results will not be compared to exhaustive_search"
import my_optimizer

class DOMINOTests(IMP.test.TestCase):
    def setUp(self):
        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)
        self.sampler=my_optimizer.my_optimizer("simple_jt2.txt", "simple_jt2_restraints.txt",8)
        self.infered_score = self.sampler.optimize()
    def test_global_min(self):
        if TRY_EX==True:
            min_score2 = self.sampler.exhaustive_search()
            self.assert_(abs(self.infered_score - min_score2) < 0.1 , "the score of the minimum configuration as calculated by the inference differs from the one calculated by the model " + str(self.infered_score) + " != " + str(min_score2))
    def test_inference(self):
        score = -13.8
        self.assert_( abs(self.infered_score -score) < 0.1 , "the score of the minimum configuration as calculated by the inference is wrong " + str(self.infered_score) + " != " + str(score))

if __name__ == '__main__':
    unittest.main()
