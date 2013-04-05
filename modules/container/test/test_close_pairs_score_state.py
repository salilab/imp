import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random

class Tests(IMP.test.TestCase):
    def _are_close(self, a, b, d):
        da= IMP.core.XYZR(a)
        db= IMP.core.XYZR(b)
        cd= IMP.core.get_distance(da, db)
        return (cd <= d)

    def _compare_lists(self, m, pc, d, out):
        print "comparing"
        print m.get_number_of_score_states()
        m.update()
        print "list is " + str(out.get_number_of_particle_pairs())
        print [(p[0].get_name(), p[1].get_name()) for p in out.get_particle_pairs()]
        print [(p[0].get_index(), p[1].get_index()) for p in out.get_particle_pairs()]
        all= out.get_particle_pairs()
        for i in range(0, pc.get_number_of_particles()):
            for j in range(0, i):
                a= pc.get_particle(i)
                b= pc.get_particle(j)
                pp= (a,b)
                if self._are_close(a,b, d):
                    print "expecting "+str(pp)
                    print IMP.core.XYZR(a)
                    print IMP.core.XYZR(b)
                    print IMP.core.get_distance(IMP.core.XYZR(a), IMP.core.XYZR(b))
                    self.assertTrue((a,b) in all or (b,a) in all)
    def test_it(self):
        """Test ClosePairContainer"""
        m=IMP.Model()
        IMP.base.set_log_level(IMP.base.SILENT)
        ps= self.create_particles_in_box(m, 20)
        # test rebuilding under move, set input and change radius
        pc= IMP.container.ListSingletonContainer(ps)
        print "creat cpss "+str(pc)
        #IMP.base.set_log_level(IMP.base.VERBOSE)
        print 1
        threshold=1
        cpss= IMP.container.ClosePairContainer(pc, threshold,
                                          IMP.core.QuadraticClosePairsFinder(),
                                          1)
        cpss.set_was_used(True)
        for p in ps:
            d= IMP.core.XYZR.setup_particle(p)
            d.set_radius(random.uniform(0,2))
        self._compare_lists(m, pc, threshold, cpss)

        # move things a little
        for p in ps:
            d= IMP.core.XYZ(p)
            d.set_coordinates(d.get_coordinates()
                              + IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), .55)))
        print "first time"
        self._compare_lists(m, pc, threshold, cpss)
        # move things a lot
        for i in range(0,10):
            print "moving "+str(i)
            for p in ps:
                d= IMP.core.XYZ(p)
                d.set_coordinates(d.get_coordinates()
                                  + IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), .7*(i+1))))
            self._compare_lists(m, pc, threshold, cpss)
    def test_restraint_0(self):
        """Test ClosePairContainer over time"""
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
                                          IMP.core.QuadraticClosePairsFinder(),
                                          1)
        for i in range(0,100):
            for p in ps:
                r= IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_sphere_3d())
                d= IMP.core.XYZ(p)
                d.set_coordinates(d.get_coordinates()+r)
            # make sure internal checks in continer pass

            m.evaluate(False)
    def test_restraint(self):
        """Test ClosePairContainer with a restraint"""
        m=IMP.Model()
        IMP.base.set_log_level(IMP.base.VERBOSE)
        ps= self.create_particles_in_box(m, 10)
        # test rebuilding under move, set input and change radius
        pc= IMP.container.ListSingletonContainer(ps)
        print "creat cpss "+str(pc)
        #IMP.base.set_log_level(IMP.base.VERBOSE)
        print 1
        threshold=1
        cpss= IMP.container.ClosePairContainer(pc, threshold,
                                          IMP.core.QuadraticClosePairsFinder(),
                                          1)
        r= IMP.container.PairsRestraint(IMP.core.DistancePairScore(IMP.core.Harmonic(3, 1)), cpss)
        m.add_restraint(r)
        for p in ps:
            d= IMP.core.XYZR.setup_particle(p)
            d.set_radius(random.uniform(0,2))
        self._compare_lists(m, pc, threshold, cpss)

        # move things a little
        for p in ps:
            d= IMP.core.XYZ(p)
            d.set_coordinates(d.get_coordinates()
                              + IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), .55)))
        print "first time"
        self._compare_lists(m, pc, threshold, cpss)
        # move things a lot
        for i in range(0,10):
            print "moving"
            j=0
            for p in ps:
                j=j+1
                if ((i+j)%2) == 0:
                    print "Moving particle " +str(p.get_name())
                    d= IMP.core.XYZ(p)
                    d.set_coordinates(d.get_coordinates()
                                      + IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), .7*(i+1))))
            self._compare_lists(m, pc, threshold, cpss)


if __name__ == '__main__':
    IMP.test.main()
