import os,math
import IMP
import IMP.test
import IMP.multifit
import IMP.algebra
class Tests(IMP.test.TestCase):
    """Class to test radius of gyration restraint """

    def test_connectivity(self):
        """Test radius of gyration restraint
           All particles should be at most radius apart"""

        IMP.base.set_log_level(IMP.base.VERBOSE)
        m = IMP.Model()
        #create particles in a box
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-5,-5,-5),
                                       IMP.algebra.Vector3D(5,5,5))
        ps=[]
        for i in range(10):
            p=IMP.algebra.get_random_vector_in(bb)
            ps.append(self.create_point_particle(m,p[0],p[1],p[2]))
        max_radius=math.sqrt(200)
        r1= IMP.multifit.RadiusOfGyrationRestraint(ps,50)
        m.add_restraint(r1)
        self.assertLessEqual(r1.evaluate(None), 0.01)

if __name__ == '__main__':
    IMP.test.main()
