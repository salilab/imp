from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random


class Tests(IMP.test.TestCase):

    def test_restraint_1(self):
        """Test ClosePairContainer complete list over time"""
        m = IMP.Model()
        IMP.set_log_level(IMP.SILENT)
        ps = IMP.get_indexes(self.create_particles_in_box(m, 10))
        for p in ps:
            IMP.core.XYZR.setup_particle(m, p, 0)
        # test rebuilding under move, set input and change radius
        pc = IMP.container.ListSingletonContainer(m, ps)
        print("creat cpss " + str(pc))
        # IMP.set_log_level(IMP.VERBOSE)
        print(1)
        threshold = 1
        slack= 100
        cpss = IMP.container.ClosePairContainer(pc, threshold,
                                                IMP.core.GridClosePairsFinder(
                                                ),
                                                slack)
        self.assertEqual(cpss.get_slack(), slack)
        m.update()
        n = len(cpss.get_indexes())
        for i in range(0, 100):
            for p in ps:
                r = IMP.algebra.get_random_vector_in(
                    IMP.algebra.Sphere3D(IMP.algebra.get_zero_vector_3d(), 5))
                d = IMP.core.XYZ(m, p)
                d.set_coordinates(d.get_coordinates() + r)
            # make sure internal checks in continer pass
            m.update()
            self.assertEqual(n, len(cpss.get_indexes()))

    def test_cache(self):
        m = IMP.Model()
        IMP.set_log_level(IMP.TERSE)
        ps = IMP.get_indexes(self.create_particles_in_box(m, 10))
        for p in ps:
            IMP.core.XYZR.setup_particle(m, p, 1)
        pc = IMP.container.ListSingletonContainer(m, ps)
        threshold = 1
        slack= .01
        cpss = IMP.container.ClosePairContainer(pc, threshold,
                                                IMP.core.GridClosePairsFinder(
                                                ),
                                                slack)
        self.assertEqual(slack, cpss.get_slack())
        m.update()
        n = len(cpss.get_indexes())
        for p in ps:
            r = IMP.algebra.get_random_vector_in(
                IMP.algebra.Sphere3D(IMP.algebra.get_zero_vector_3d(), 10))
            d = IMP.core.XYZ(m, p)
            d.set_coordinates(d.get_coordinates() + r)
        print(cpss.get_number_of_full_rebuilds(), cpss.get_number_of_partial_rebuilds())
        m.update()
        print(cpss.get_number_of_full_rebuilds(), cpss.get_number_of_partial_rebuilds())
        print("updating without moving")
        m.update()
        print(cpss.get_number_of_full_rebuilds(), cpss.get_number_of_partial_rebuilds())
        self.assertEqual(cpss.get_number_of_update_calls(), 3)
        self.assertEqual(
            cpss.get_number_of_full_rebuilds(
            ) + cpss.get_number_of_partial_rebuilds(
            ),
            2)
if __name__ == '__main__':
    IMP.test.main()
