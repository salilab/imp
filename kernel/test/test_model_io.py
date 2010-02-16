import unittest
import IMP
import IMP.test
import StringIO

class YamlTests(IMP.test.TestCase):
    def _create_model(self):
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        ps=[]
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
        to= IMP.ToParticles()
        fr= IMP.FromParticles()
        to[0]=ps[0]
        to[1]=ps[1]
        to[2]=ps[2]
        fr[ps[0]]=0
        fr[ps[1]]=1
        fr[ps[2]]=2
        IMP.set_log_level(IMP.VERBOSE)
        return (m, to, fr)
    def test_yaml(self):
        """Check writing to yaml """
        (m, to, fr)= self._create_model()
        s= StringIO.StringIO()
        IMP.write_model(m, fr, s)
        first=s.getvalue()
        print first
        sto= StringIO.StringIO(first)
        to[0].set_value(IMP.FloatKey("x"), 11)
        to[0].add_attribute(IMP.ParticleKey("y"), to[1])
        IMP.read_model(sto, to, m)
        s= StringIO.StringIO()
        IMP.write_model(m, fr, s)
        print s.getvalue()
        self.assertEqual(first, s.getvalue())




if __name__ == '__main__':
    unittest.main()
