import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random


class Tests(IMP.test.TestCase):

    def _are_close(self, m, a, b, rk, d):
        da = IMP.core.XYZ(m, a)
        db = IMP.core.XYZ(m, b)
        r = 0
        if rk != IMP.FloatKey():
            r = m.get_attribute(rk, a) + m.get_attribute(rk, b)
        cd = IMP.core.get_distance(da, db)
        return (cd - r <= d)

    def _compare_lists(self, m, pc0, pc1, out, d):
        print("comparing")
        IMP.set_log_level(IMP.VERBOSE)
        m.update()
        pairs = out.get_indexes()
        print("list is " + str(len(pairs)))
        for a in pc0:
            for b in pc1:
                if self._are_close(m, a, b, IMP.core.XYZR.get_radius_key(), d):
                    self.assertTrue((a, b) in pairs)

    def _test_one(self, rb0, rb1):
        m = IMP.Model()
        IMP.set_log_level(IMP.VERBOSE)
        ps0 = IMP.get_indexes(self.create_particles_in_box(m, 10))
        ps1 = IMP.get_indexes(self.create_particles_in_box(m, 10))
        print("adding a radius")
        for p in ps0:
            d = IMP.core.XYZR.setup_particle(m, p)
            d.set_radius(random.uniform(0, 2))
        for p in ps1:
            d = IMP.core.XYZR.setup_particle(m, p)
            d.set_radius(random.uniform(0, 2))

        if rb0:
            IMP.core.RigidBody.setup_particle(IMP.Particle(m), ps0)
        if rb1:
            IMP.core.RigidBody.setup_particle(IMP.Particle(m), ps1)
        # test rebuilding under move, set input and change radius
        pc0 = IMP.container.ListSingletonContainer(m, ps0)
        pc1 = IMP.container.ListSingletonContainer(m, ps1)
        print("creat cpss " + str(pc0))
        # IMP.set_log_level(IMP.VERBOSE)
        print(1)
        threshold = .3

        cpss = IMP.container.CloseBipartitePairContainer(
            pc0, pc1, threshold, 3)

        self._compare_lists(m, ps0, ps1, cpss, threshold)

        for p in ps0:
            d = IMP.core.XYZR(m, p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(
                IMP.algebra.get_unit_bounding_box_3d()))
        self._compare_lists(m, ps0, ps1, cpss, threshold)

        for p in ps1:
            d = IMP.core.XYZR(m, p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(
                IMP.algebra.get_unit_bounding_box_3d()))
        self._compare_lists(m, ps0, ps1, cpss, threshold)
        print("done")
        del m
        print("deleted model")

    def test_it_0(self):
        """Test CloseBipartitePairContainer"""
        self._test_one(False, False)

    def test_it_1(self):
        """Test CloseBipartitePairContainer with rigid body 0"""
        self._test_one(True, False)

    def test_it_2(self):
        """Test CloseBipartitePairContainer with rigid body 1"""
        self._test_one(False, True)

    def test_it_3(self):
        """Test CloseBipartitePairContainer with rigid body 2"""
        self._test_one(True, True)

    def test_filtering(self):
        """Testing that CloseBipartitePairContainer responds to changes in filters"""
        m = IMP.Model()
        ps0 = list(IMP.get_indexes(self.create_particles_in_box(m, 30)))
        ps1 = list(IMP.get_indexes(self.create_particles_in_box(m, 30)))
        for p in ps0 + ps1:
            IMP.core.XYZR.setup_particle(m, p, 1)
        # test rebuilding under move, set input and change radius
        pc0 = IMP.container.ListSingletonContainer(m, ps0)
        pc1 = IMP.container.ListSingletonContainer(m, ps1)
        cpss = IMP.container.CloseBipartitePairContainer(pc0, pc1, 100000, 3)
        m.update()
        self.assertEqual(len(cpss.get_indexes()), len(ps0) * len(ps1))
        abpc = IMP.container.AllBipartitePairContainer(pc0, pc1)
        cpss.add_pair_filter(IMP.container.InContainerPairFilter(abpc))
        print("re-evaluate")
        m.update()
        self.assertEqual(len(cpss.get_indexes()), 0)


if __name__ == '__main__':
    IMP.test.main()
