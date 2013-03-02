import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.example

class Tests(IMP.test.TestCase):
    """Test the symmetry restraint"""
    def test_symmetry(self):
        """Test example modifier"""
        IMP.base.set_log_level(IMP.MEMORY)
        m= IMP.Model()
        print "creating particle"
        p= IMP.Particle(m)
        print "created"
        bbb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                       IMP.algebra.Vector3D(100,100,100))
        sbb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(20,30,40),
                                       IMP.algebra.Vector3D(30,39,48))
        print "setting up"
        d= IMP.core.XYZ.setup_particle(p, IMP.algebra.get_random_vector_in(bbb))
        print "creating m"
        mod= IMP.example.ExampleSingletonModifier(sbb)
        print "applying"
        mod.apply(d.get_particle())
        for i in range(0,3):
            self.assertGreaterEqual(d.get_coordinate(i), sbb.get_corner(0)[i])
            self.assertLessEqual(d.get_coordinate(i), sbb.get_corner(1)[i])

if __name__ == '__main__':
    IMP.test.main()
