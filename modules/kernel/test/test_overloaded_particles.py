import IMP
import IMP.test
import sys

class Tests(IMP.test.TestCase):
    """Test refcounting of particles"""

    def test_simple(self):
        """Check overloading of particle methods works"""
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP.kernel._overloaded_particles(m, [p.get_index()])
        IMP.kernel._overloaded_particles(p)
        IMP.kernel._overloaded_particles([p])
        IMP.kernel._overloaded_particles([IMP.kernel._TrivialDecorator.setup_particle(p)])
        IMP.kernel._overloaded_particles(IMP.kernel._TrivialDecorator(p))
        IMP.kernel._implicit_particles(IMP.kernel._ImplicitParticles(m, [p.get_index()]))
        IMP.kernel._implicit_particles(p)
        IMP.kernel._implicit_particles([p])
        IMP.kernel._implicit_particles([IMP.kernel._TrivialDecorator.setup_particle(p)])
        IMP.kernel._implicit_particles(IMP.kernel._TrivialDecorator(p))


if __name__ == '__main__':
    IMP.test.main()
