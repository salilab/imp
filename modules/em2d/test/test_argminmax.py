import IMP
import IMP.em2d as em2d
import IMP.em2d.argminmax as argminmax
import IMP.test
import sys
import os
import random


class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.numbers = random.sample(range(0,1000), 100)
        self.keys = random.sample(range(0,1000), 100)
        self.dictionary = dict()
        for key,val  in zip(self.keys, self.numbers):
            self.dictionary[key] = val

    def test_functions(self):
        """ Test the helper functions in argminmax """
        min_numbers = min(self.numbers)
        max_numbers = max(self.numbers)
        (min_value,min_index) =  argminmax.argmin(self.numbers)
        self.assertEqual(min_value, min_numbers)
        self.assertEqual(self.numbers[min_index], min_numbers)

        (max_value, max_index) =  argminmax.argmax(self.numbers)
        self.assertEqual(max_value, max_numbers)
        self.assertEqual(self.numbers[max_index], max_numbers)


        (min_value, min_key) = argminmax.keymin(self.dictionary)
        self.assertEqual(min_value, min_numbers)
        self.assertEqual(self.dictionary[min_key], min_numbers)

        (max_value, max_key) = argminmax.keymax(self.dictionary)
        self.assertEqual(max_value, max_numbers)
        self.assertEqual(self.dictionary[max_key], max_numbers)


if __name__ == '__main__':
    IMP.test.main()
