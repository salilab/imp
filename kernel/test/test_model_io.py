import unittest
import IMP
import IMP.test
import StringIO

class YamlTests(IMP.test.TestCase):
    def _create_model(self):
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        ps=IMP.ParticlesTemp()
        p= IMP.Particle(m)
        p.add_attribute(IMP.FloatKey("x"), 0, False)
        p.add_attribute(IMP.FloatKey("y"), 1, True)
        p.add_attribute(IMP.FloatKey("z"), 2, False)
        ps.append(p)
        p= IMP.Particle(m)
        p.add_attribute(IMP.FloatKey("y"), 4, True)
        p.add_attribute(IMP.FloatKey("z"), 7, False)
        p.add_attribute(IMP.FloatKey("q"), 9, False)
        p.add_attribute(IMP.StringKey("hi"), "there")
        ps.append(p)
        p= IMP.Particle(m)
        p.add_attribute(IMP.FloatKey("q"), 9, False)
        p.add_attribute(IMP.IntKey("int"), 1)
        p.add_attribute(IMP.ParticleKey("k"), p)
        ps.append(p)
        IMP.set_log_level(IMP.VERBOSE)
        return (m, ps)
    def test_yaml(self):
        """Check writing to yaml """
        (m, ps)= self._create_model()
        s= StringIO.StringIO()
        IMP.write_model(m, ps, s)
        first=s.getvalue()
        print first
        sto= StringIO.StringIO(first)
        ps[0].set_value(IMP.FloatKey("x"), 11)
        ps[0].add_attribute(IMP.ParticleKey("y"), ps[1])
        IMP.read_model(sto, ps, m)
        s= StringIO.StringIO()
        IMP.write_model(m, ps, s)
        print s.getvalue()
        self.assertEqual(first, s.getvalue())




if __name__ == '__main__':
    unittest.main()
