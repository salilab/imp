from __future__ import print_function
import sys
import IMP
import IMP.core
import IMP.test
import pickle


class Tests(IMP.test.TestCase):

    def test_simple(self):
        """Test simple usage of ChecksScoreState"""
        m = IMP.Model()
        ss = IMP.core.ChecksScoreState(m, 0.0)
        m.update()
        # probability is zero, so checked should not get updated
        self.assertEqual(ss.get_number_of_checked(), 0)

    def test_pickle(self):
        """Test (un-)pickle of ChecksScoreState"""
        m = IMP.Model()
        ss = IMP.core.ChecksScoreState(m, 0.0)
        dump = pickle.dumps(ss)
        newss = pickle.loads(dump)
        self.assertEqual(newss.get_number_of_checked(), 0)


if __name__ == '__main__':
    IMP.test.main()
