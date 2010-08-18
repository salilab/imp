import unittest
import IMP
import IMP.test
import IMP.core


class ParticleTests(IMP.test.TestCase):
    """Test particles"""
    def test_many_particle(self):
        """Test cleanup of failure handlers"""
        m= IMP.Model()
        IMP.set_log_level(IMP.MEMORY)
        if True:
            ps= IMP.core.create_xyzr_particles(m, 10, 10, 10)
        else:
            ps=[]
            for i in range(0,10):
                ps.append(IMP.Particle(m))
        IMP.add_failure_handler(IMP.WriteFailureHandler(ps, self.get_tmp_file_name("failure.yaml")))


if __name__ == '__main__':
    unittest.main()
