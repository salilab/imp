import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random


class Tests(IMP.test.TestCase):

    def _are_close(self, m, a, b, d):
        da = IMP.core.XYZR(m, a)
        db = IMP.core.XYZR(m, b)
        cd = IMP.core.get_distance(da, db)
        return (cd <= d)

    def _compare_lists(self, m, pc, d, out):
        print("comparing")
        print(m.get_number_of_score_states())
        m.update()
        all = out.get_indexes()
        indexes = pc.get_indexes()
        for i in range(0, len(indexes)):
            for j in range(0, i):
                a = indexes[i]
                b = indexes[j]
                pp = (a, b)
                if self._are_close(m, a, b, d):
                    print("expecting " + str(pp))
                    self.assertTrue((a, b) in all or (b, a) in all)

    def test_it(self):
        """Test ClosePairContainer"""
        m = IMP.Model()
        IMP.set_log_level(IMP.SILENT)
        ps = IMP.get_indexes(self.create_particles_in_box(m, 20))
        # test rebuilding under move, set input and change radius
        pc = IMP.container.ListSingletonContainer(m, ps)
        print("creat cpss " + str(pc))
        # IMP.set_log_level(IMP.VERBOSE)
        print(1)
        threshold = 1
        cpss = IMP.container.ClosePairContainer(pc, threshold,
                                                IMP.core.QuadraticClosePairsFinder(
                                                ),
                                                1)
        cpss.set_was_used(True)
        for p in ps:
            d = IMP.core.XYZR.setup_particle(m, p)
            d.set_radius(random.uniform(0, 2))
        self._compare_lists(m, pc, threshold, cpss)

        # move things a little
        for p in ps:
            d = IMP.core.XYZ(m, p)
            d.set_coordinates(d.get_coordinates()
                              + IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0, 0, 0), .55)))
        print("first time")
        self._compare_lists(m, pc, threshold, cpss)
        # move things a lot
        for i in range(0, 10):
            print("moving " + str(i))
            for p in ps:
                d = IMP.core.XYZ(m, p)
                d.set_coordinates(d.get_coordinates()
                                  + IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0, 0, 0), .7 * (i + 1))))
            self._compare_lists(m, pc, threshold, cpss)

    def test_restraint_0(self):
        """Test ClosePairContainer over time"""
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
        cpss = IMP.container.ClosePairContainer(pc, threshold,
                                                IMP.core.QuadraticClosePairsFinder(
                                                ),
                                                1)
        for i in range(0, 100):
            for p in ps:
                r = IMP.algebra.get_random_vector_in(
                    IMP.algebra.get_unit_sphere_3d())
                d = IMP.core.XYZ(m, p)
                d.set_coordinates(d.get_coordinates() + r)
            # make sure internal checks in container pass

            m.update()

    def test_restraint(self):
        """Test ClosePairContainer with a restraint"""
        m = IMP.Model()
        IMP.set_log_level(IMP.VERBOSE)
        ps = IMP.get_indexes(self.create_particles_in_box(m, 10))
        # test rebuilding under move, set input and change radius
        pc = IMP.container.ListSingletonContainer(m, ps)
        print("creat cpss " + str(pc))
        # IMP.set_log_level(IMP.VERBOSE)
        print(1)
        threshold = 1
        cpss = IMP.container.ClosePairContainer(pc, threshold,
                                                IMP.core.QuadraticClosePairsFinder(
                                                ),
                                                1)
        r = IMP.container.PairsRestraint(
            IMP.core.DistancePairScore(IMP.core.Harmonic(3, 1)), cpss)
        for p in ps:
            d = IMP.core.XYZR.setup_particle(m, p)
            d.set_radius(random.uniform(0, 2))
        self._compare_lists(m, pc, threshold, cpss)

        # move things a little
        for p in ps:
            d = IMP.core.XYZ(m, p)
            d.set_coordinates(d.get_coordinates()
                              + IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0, 0, 0), .55)))
        print("first time")
        self._compare_lists(m, pc, threshold, cpss)
        # move things a lot
        for i in range(0, 10):
            print("moving")
            j = 0
            for p in ps:
                j = j + 1
                if ((i + j) % 2) == 0:
                    d = IMP.core.XYZ(m, p)
                    d.set_coordinates(d.get_coordinates()
                                      + IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0, 0, 0), .7 * (i + 1))))
            self._compare_lists(m, pc, threshold, cpss)


if __name__ == '__main__':
    IMP.test.main()
