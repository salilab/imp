import sys
import unittest
import IMP
import IMP.test
import my_optimizer
IMP.set_log_level(IMP.SILENT)#VERBOSE)
class DOMINOTests(IMP.test.TestCase):
    def setUp(self):
        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)
        self.sampler = my_optimizer.my_optimizer(
                          self.get_input_file_name("simple_jt2.txt"),
                          self.get_input_file_name("simple_jt2_restraints.txt"),
                          8)
        self.infered_score = self.sampler.optimize()
    def test_inference_2(self):
        self.sampler.d_opt.set_sampling_space(self.sampler.discrete_sampler)
        combs=IMP.domino.CombStates()
        self.sampler.d_opt.exhaustive_enumeration(combs,True)
        #select the minimum one
        min_comb=None
        min_score=10000
        for comb in combs:
            if min_score>comb.get_total_score():
                min_score =  comb.get_total_score()
                min_comb=comb
        self.assert_( abs(self.infered_score -min_score) < 0.1 , "the score of the minimum configuration as calculated by the inference is wrong " + str(self.infered_score) + " != " + str(min_score))

    def test_inference_2(self):
        self.sampler.d_opt.set_sampling_space(self.sampler.discrete_sampler)
        combs=IMP.domino.CombStates()
        self.sampler.d_opt.exhaustive_enumeration(combs,True,True)
        #select the minimum one
        min_comb=None
        min_score=10000
        for comb in combs:
            if min_score>comb.get_total_score():
                min_score =  comb.get_total_score()
                min_comb=comb
        self.assert_( abs(self.infered_score -min_score) < 0.1 , "the score of the minimum configuration as calculated by the inference is wrong " + str(self.infered_score) + " != " + str(min_score))

if __name__ == '__main__':
    unittest.main()
