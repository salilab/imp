import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random

class Tests(IMP.test.TestCase):

    def test_restraint_1(self):
        """Test ClosePairContainer complete list over time"""
        m=IMP.Model()
        IMP.base.set_log_level(IMP.base.SILENT)
        ps= self.create_particles_in_box(m, 10)
        for p in ps:
            IMP.core.XYZR.setup_particle(p, 0)
        # test rebuilding under move, set input and change radius
        pc= IMP.container.ListSingletonContainer(ps)
        print "creat cpss "+str(pc)
        #IMP.base.set_log_level(IMP.base.VERBOSE)
        print 1
        threshold=1
        cpss= IMP.container.ClosePairContainer(pc, threshold,
                                          IMP.core.GridClosePairsFinder(),
                                          100)
        m.evaluate(False)
        n= cpss.get_number_of_particle_pairs()
        for i in range(0,100):
            for p in ps:
                r= IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.get_zero_vector_3d(), 5))
                d= IMP.core.XYZ(p)
                d.set_coordinates(d.get_coordinates()+r)
            # make sure internal checks in continer pass
            m.evaluate(False)
            self.assertEqual(n, cpss.get_number_of_particle_pairs())



if __name__ == '__main__':
    IMP.test.main()
