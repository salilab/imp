import IMP
import IMP.test
import sys

class Tests(IMP.test.TestCase):
    """Test refcounting of particles"""

    def test_simple(self):
        """Check overloading of particle methods works"""
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP._overloaded_particles(m, [p.get_index()])
        IMP._overloaded_particles(p)
        IMP._overloaded_particles([p])
        IMP._overloaded_particles([IMP._TrivialDecorator.setup_particle(p)])
        IMP._overloaded_particles(IMP._TrivialDecorator(p))
        IMP._implicit_particles(IMP._ImplicitParticles(m, [p.get_index()]))
        IMP._implicit_particles(p)
        IMP._implicit_particles([p])
        IMP._implicit_particles([IMP._TrivialDecorator.setup_particle(p)])
        IMP._implicit_particles(IMP._TrivialDecorator(p))


if __name__ == '__main__':
    IMP.test.main()
