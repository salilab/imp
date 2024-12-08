import IMP
import IMP.test
import IMP.em2d
import pickle


class Tests(IMP.test.TestCase):
    def test_pickle(self):
        """Test (un-)pickle of MasksManager"""
        p = IMP.em2d.MasksManager(2.0, 4.0)
        p.create_mask(42.0)
        dump = pickle.dumps(p)
        newp = pickle.loads(dump)
        self.assertEqual(newp.get_number_of_masks(), 1)


if __name__ == '__main__':
    IMP.test.main()
