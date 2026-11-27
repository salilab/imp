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

    def _make_beads(self, m, xs, radius):
        return [IMP.core.XYZR.setup_particle(
                    IMP.Particle(m), IMP.algebra.Sphere3D(
                        IMP.algebra.Vector3D(x, 0, 0), radius)).get_particle_index()
                for x in xs]

    def _move_beads(self, m, ps, xs):
        for p, x in zip(ps, xs):
            IMP.core.XYZ(m, p).set_coordinates(IMP.algebra.Vector3D(x, 0, 0))
        m.update()

    def _get_listed(self, cpss):
        return set(tuple(sorted(p)) for p in cpss.get_indexes())

    def _get_finders(self):
        return [IMP.core.GridClosePairsFinder(),
                IMP.core.RigidClosePairsFinder()]

    def test_incremental_drop(self):
        """Incremental update must not drop pairs that can still close"""
        for cpf in self._get_finders():
            m = IMP.Model()
            # A and B (surface gap 15) are listed; C is far away
            a, b, c = self._make_beads(m, [0., 21., 100.], 3.)
            cpss = IMP.container.ClosePairContainer(
                IMP.container.ListSingletonContainer(m, [a, b, c]),
                0., cpf, 10.)
            m.update()
            self.assertIn(tuple(sorted((a, b))), self._get_listed(cpss))
            # A and B move apart by less than the slack each; C moves by more
            # than the slack, so the list is updated incrementally
            self._move_beads(m, [a, b, c], [-9., 30., 111.])
            # A and B move back past their starting points, still less than
            # the slack from where they started, and now overlap
            self._move_beads(m, [a, b, c], [9., 12., 111.])
            self.assertIn(tuple(sorted((a, b))), self._get_listed(cpss))

    def test_incremental_add(self):
        """Incremental update must find pairs with particles that didn't move"""
        for cpf in self._get_finders():
            m = IMP.Model()
            # A and B (surface gap 25) are not listed
            a, b = self._make_beads(m, [0., 31.], 3.)
            cpss = IMP.container.ClosePairContainer(
                IMP.container.ListSingletonContainer(m, [a, b]),
                0., cpf, 10.)
            m.update()
            # A moves by less than the slack
            self._move_beads(m, [a, b], [-9., 31.])
            # B moves by more than the slack (incremental update)
            self._move_beads(m, [a, b], [-9., 19.5])
            # Both move by less than the slack from their last update
            # positions, and now overlap
            self._move_beads(m, [a, b], [8., 10.])
            self.assertIn(tuple(sorted((a, b))), self._get_listed(cpss))

    def test_incremental_random_walk(self):
        """Close pairs list stays complete under small random moves"""
        # With this seed, the list was first incomplete at step 1514 before
        # the incremental update was fixed
        rng = random.Random(4)
        nbead, box, radius, step = 40, 45., 3., 3.
        m = IMP.Model()
        ps = self._make_beads(m, [0.] * nbead, radius)
        xyz = [[rng.uniform(0., box) for _ in range(3)] for p in ps]
        for p, x in zip(ps, xyz):
            IMP.core.XYZ(m, p).set_coordinates(IMP.algebra.Vector3D(*x))
        # Same container as PMI's ExcludedVolumeSphere
        cpss = IMP.container.ClosePairContainer(
            IMP.container.ListSingletonContainer(m, ps), 0.,
            IMP.core.RigidClosePairsFinder(), 10.)
        m.update()
        for istep in range(3000):
            # Move one bead by up to step in each direction, reflecting at
            # the box walls
            i = rng.randrange(nbead)
            x = [c + rng.uniform(-step, step) for c in xyz[i]]
            xyz[i] = [-c if c < 0. else 2. * box - c if c > box else c
                      for c in x]
            IMP.core.XYZ(m, ps[i]).set_coordinates(
                IMP.algebra.Vector3D(*xyz[i]))
            m.update()
            listed = self._get_listed(cpss)
            for i in range(nbead):
                xi = xyz[i]
                for j in range(i):
                    xj = xyz[j]
                    d2 = ((xi[0] - xj[0]) ** 2 + (xi[1] - xj[1]) ** 2
                          + (xi[2] - xj[2]) ** 2)
                    if d2 < 4. * radius * radius:
                        pair = tuple(sorted((ps[i], ps[j])))
                        self.assertTrue(pair in listed, "overlapping pair %s "
                                        "not listed at step %d"
                                        % (str(pair), istep))


if __name__ == '__main__':
    IMP.test.main()
