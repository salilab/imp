import unittest
import IMP
import IMP.test
import IMP.core

class YamlTests(IMP.test.TestCase):
    def _create_model(self):
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        p= IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p)
        p= IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p)
        return m
    def test_yaml(self):
        """Check writing to yaml """
        m= self._create_model()
        IMP.core.write(m, open(self.get_tmp_file_name("test.yaml"), "w"))
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
        f= open(self.get_tmp_file_name("tmp.yaml"), "w")
        f.write(st)
        f.close()
        print "m has " + str(m.get_number_of_particles())
        IMP.core.read(open(self.get_tmp_file_name("tmp.yaml")), m)
        ps= m.get_particles()
        d0= IMP.core.XYZ.decorate_particle(ps[0])
        self.assertEqual(d0.get_z(), 2)
        d1= IMP.core.XYZ.decorate_particle(ps[1])
        self.assertEqual(d1.get_z(), 5)
        self.assertEqual(ps[0].get_name(), "a particle")
        #IMP.core.write(m)




if __name__ == '__main__':
    unittest.main()
