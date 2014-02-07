import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random


class Tests(IMP.test.TestCase):

    def test_restraint_1(self):
        """Test ClosePairContainer complete list over time"""
        m = IMP.kernel.Model()
        IMP.base.set_log_level(IMP.base.SILENT)
        ps = self.create_particles_in_box(m, 10)
        for p in ps:
            IMP.core.XYZR.setup_particle(p, 0)
        # test rebuilding under move, set input and change radius
        pc = IMP.container.ListSingletonContainer(ps)
        print "creat cpss " + str(pc)
        # IMP.base.set_log_level(IMP.base.VERBOSE)
        print 1
        threshold = 1
        cpss = IMP.container.ClosePairContainer(pc, threshold,
                                                IMP.core.GridClosePairsFinder(
                                                ),
                                                100)
        m.update()
        n = cpss.get_number_of_particle_pairs()
        for i in range(0, 100):
            for p in ps:
                r = IMP.algebra.get_random_vector_in(
                    IMP.algebra.Sphere3D(IMP.algebra.get_zero_vector_3d(), 5))
                d = IMP.core.XYZ(p)
                d.set_coordinates(d.get_coordinates() + r)
            # make sure internal checks in continer pass
            m.update()
            self.assertEqual(n, cpss.get_number_of_particle_pairs())

    def test_cache(self):
        m = IMP.kernel.Model()
        IMP.base.set_log_level(IMP.base.TERSE)
        ps = self.create_particles_in_box(m, 10)
        for p in ps:
            IMP.core.XYZR.setup_particle(p, 1)
        pc = IMP.container.ListSingletonContainer(ps)
        threshold = 1
        cpss = IMP.container.ClosePairContainer(pc, threshold,
                                                IMP.core.GridClosePairsFinder(
                                                ),
                                                .01)
        m.update()
        n = cpss.get_number_of_particle_pairs()
        for p in ps:
            r = IMP.algebra.get_random_vector_in(
                IMP.algebra.Sphere3D(IMP.algebra.get_zero_vector_3d(), 10))
            d = IMP.core.XYZ(p)
            d.set_coordinates(d.get_coordinates() + r)
        print cpss.get_number_of_full_rebuilds(), cpss.get_number_of_partial_rebuilds()
        m.update()
        print cpss.get_number_of_full_rebuilds(), cpss.get_number_of_partial_rebuilds()
        print "updating without moving"
        m.update()
        print cpss.get_number_of_full_rebuilds(), cpss.get_number_of_partial_rebuilds()
        self.assertEqual(cpss.get_number_of_update_calls(), 3)
        self.assertEqual(
            cpss.get_number_of_full_rebuilds(
            ) + cpss.get_number_of_partial_rebuilds(
            ),
            2)
if __name__ == '__main__':
    IMP.test.main()
