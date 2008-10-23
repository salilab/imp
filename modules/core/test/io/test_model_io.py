import unittest
import IMP
import IMP.test
import IMP.core

class YamlTests(IMP.test.TestCase):
    def _create_model(self):
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        p= IMP.Particle()
        m.add_particle(p)
        IMP.core.XYZDecorator.create(p)
        p= IMP.Particle()
        m.add_particle(p)
        IMP.core.XYZDecorator.create(p)
        return m
    def test_yaml(self):
        """Check writing to yaml """
        m= self._create_model()
        IMP.core.write(m)
    def test_read(self):
        """Check reading from yaml"""
        m= self._create_model()
        st="""particle: 0
  float-attributes:
    x: 0
    y: 1
    z: 2
  int-attributes:
  string-attributes:
  particle-attributes:
particle: 1
  float-attributes:
    x: 3
    y: 4
    z: 5
  int-attributes:
  string-attributes:
  particle-attributes:
"""
        IMP.core.read_from_string(st, m)
        IMP.core.write(m)




if __name__ == '__main__':
    unittest.main()
