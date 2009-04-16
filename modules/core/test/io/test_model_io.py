import unittest
import IMP
import IMP.test
import IMP.core

class YamlTests(IMP.test.TestCase):
    def _create_model(self):
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        p= IMP.Particle(m)
        IMP.core.XYZDecorator.create(p)
        p= IMP.Particle(m)
        IMP.core.XYZDecorator.create(p)
        return m
    def test_yaml(self):
        """Check writing to yaml """
        m= self._create_model()
        IMP.core.write(m, IMP.ostringstring())
    def test_read(self):
        """Check reading from yaml"""
        m= self._create_model()
        st="""particle: 32941241
  name: a particle
  float-attributes:
    x: 0
    y: 1
    z: 2
  int-attributes:
  string-attributes:
  particle-attributes:
particle: 1324215
  float-attributes:
    x: 3
    y: 4
    z: 5
  int-attributes:
  string-attributes:
  particle-attributes:
"""
        iss= IMP.istringstream(st)
        IMP.core.read(iss, m)
        ps= m.get_particles()
        d0= IMP.core.XYZDecorator.cast(ps[0])
        self.assertEqual(d0.get_z(), 2)
        d1= IMP.core.XYZDecorator.cast(ps[1])
        self.assertEqual(d1.get_z(), 5)
        self.assertEqual(ps[0].get_name(), "a particle")
        oss= IMP.ostringstream()
        IMP.core.write(m, oss)




if __name__ == '__main__':
    unittest.main()
