import IMP
import IMP.test
import IMP.core
import IMP.algebra


class Tests(IMP.test.TestCase):
    """Tests for RigidBody function"""
    def test_nested(self):
        """Test nested rigid bodies and close pairs"""
        m= IMP.Model()
        ps=[IMP.Particle(m) for i in range(0,10)]
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10,10,10))
        ds=[IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb),
                                                                  1)) for p in ps]

        rb0= IMP.core.RigidBody.setup_particle(IMP.Particle(m), ps[0:3])
        rb1= IMP.core.RigidBody.setup_particle(IMP.Particle(m), ps[3:6])
        rb2= IMP.core.RigidBody.setup_particle(IMP.Particle(m), ps[6:10])
        rb0.add_member(rb1)
        cpf= IMP.core.RigidClosePairsFinder()
        cpf.set_distance(.5)
        # check that no internal checks fail
        cp0= cpf.get_close_pairs(ps)
        print [(p[0].get_name(), p[1].get_name()) for p in cp0]
        cp1= cpf.get_close_pairs(ps[3:])
        print [(p[0].get_name(), p[1].get_name()) for p in cp1]

if __name__ == '__main__':
    IMP.test.main()
