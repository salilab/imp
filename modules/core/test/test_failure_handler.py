import IMP
import IMP.test
import IMP.core


class ParticleTests(IMP.test.TestCase):
    """Test particles"""
    def test_many_particle(self):
        """Test cleanup of failure handlers"""
        m= IMP.Model()
        IMP.set_log_level(IMP.WARNING)
        if True:
            ps= IMP.core.create_xyzr_particles(m, 10, 10, 10)
        else:
            ps=[]
            for i in range(0,10):
                ps.append(IMP.Particle(m))
        s=IMP.ScopedFailureHandler(IMP.WriteFailureHandler(ps, self.get_tmp_file_name("failure.yaml")))
        del m
        if IMP.get_check_level()>= IMP.USAGE:
            self.assertRaises(IMP.UsageException, ps[0].get_value, IMP.FloatKey("x"))


if __name__ == '__main__':
    IMP.test.main()
