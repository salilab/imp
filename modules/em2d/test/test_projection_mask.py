from __future__ import print_function
import IMP
import IMP.test
import IMP.em2d
import pickle


class Tests(IMP.test.TestCase):
    def test_pickle(self):
        """Test (un-)pickle of ProjectionMask"""
        # Limited access from Python, so use default constructor
        p = IMP.em2d.ProjectionMask()
        dump = pickle.dumps(p)
        newp = pickle.loads(dump)


if __name__ == '__main__':
    IMP.test.main()
