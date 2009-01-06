import unittest
import IMP
import IMP.test
import IMP.modulename
import IMP.modulename.randomize
import IMP.core


class GenericTest(IMP.test.TestCase):
    """Test the python code"""

    def test_python(self):
        """Checking module python code"""
        m = IMP.Model()
        p= IMP.Particle(m)
        d= IMP.core.XYZDecorator.create(p)
        IMP.modulename.randomize.randomize_particle(p)
        self.assert_(IMP.modulename.get_is_xyz_particle(p))

if __name__ == '__main__':
    unittest.main()
