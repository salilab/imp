import IMP
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):

    """Class to test Typed"""

    def test_typed(self):
        """Test the Typed decorator"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        pt = IMP.core.ParticleType("foo")
        self.assertFalse(IMP.core.Typed.get_is_setup(p1))
        IMP.core.Typed.setup_particle(p1, pt)
        self.assertTrue(IMP.core.Typed.get_is_setup(p1))
        t = IMP.core.Typed(p1)
        self.assertEqual(t.get_type(), pt)

if __name__ == '__main__':
    IMP.test.main()
