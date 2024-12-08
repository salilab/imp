import IMP
import IMP.atom
import IMP.test
import pickle

class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test that WritePDBOptimizerState can be (un-)pickled"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        s = IMP.atom.WritePDBOptimizerState([p1], "test.pdb")
        s.set_period(11)
        s.set_name("foo")
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        self.assertEqual(news.get_name(), "foo")
        self.assertEqual(news.get_period(), 11)

    def test_pickle_polymorphic(self):
        """Test that WritePDBOptimizerState can be (un-)pickled
           via polymorphic pointer"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        s = IMP.atom.WritePDBOptimizerState([p1], "test.pdb")
        s.set_period(11)
        s.set_name("foo")
        opt = IMP._ConstOptimizer(m)
        opt.optimizer_states.append(s)

        dump = pickle.dumps(opt)
        newopt = pickle.loads(dump)
        news, = newopt.optimizer_states
        self.assertEqual(news.get_name(), "foo")
        self.assertEqual(news.get_period(), 11)


if __name__ == '__main__':
    IMP.test.main()
