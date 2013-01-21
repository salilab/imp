import IMP
import IMP.test
import sys

class Tests(IMP.test.TestCase):
    """Test refcounting of particles"""

    def test_simple(self):
        """Check overloading of particle methods works"""
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP.overloaded_particles(m, [p.get_index()])
        IMP.overloaded_particles(p)
        IMP.overloaded_particles([p])


if __name__ == '__main__':
    IMP.test.main()
