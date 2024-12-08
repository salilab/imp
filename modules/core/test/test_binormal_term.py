import IMP.core
import IMP
import IMP.test
import io
import sys
import pickle


class Tests(IMP.test.TestCase):

    def test_binormal_term(self):
        """Test BinormalTerm class"""
        t = IMP.core.BinormalTerm()
        t.set_correlation(0.5)
        t.set_weight(0.8)
        t.set_means([10., 20.])
        t.set_standard_deviations([1., 2.])
        if sys.version_info[0] >= 3:
            out = io.StringIO()
        else:
            out = io.BytesIO()
        print(t, file=out)
        self.assertIn("means: 10, 20", out.getvalue())

    def test_pickle(self):
        """Test (un-)pickle of BinormalTerm"""
        t = IMP.core.BinormalTerm()
        t.set_correlation(0.5)
        t.set_weight(0.8)
        t.set_means([10., 20.])
        t.set_standard_deviations([1., 2.])
        dump = pickle.dumps(t)

        newt = pickle.loads(dump)
        if sys.version_info[0] >= 3:
            out = io.StringIO()
        else:
            out = io.BytesIO()
        print(newt, file=out)
        self.assertIn("means: 10, 20", out.getvalue())

if __name__ == '__main__':
    IMP.test.main()
