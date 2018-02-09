import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.npc

class Tests(IMP.test.TestCase):

    def evaluate_config(self, m, particles):
        particles_by_type = {}
        for particle_data in particles:
            name, x, y = particle_data
            p = IMP.Particle(m, name)
            IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(x,y,0.))
            particle_type = name[0]
            if particle_type in particles_by_type:
                particles_by_type[particle_type].append(p)
            else:
                particles_by_type[particle_type] = [p]
        ps = IMP.core.DistancePairScore(IMP.core.Linear(0.0, 1.0))
        r = IMP.npc.CompositeRestraint(m, ps)
        r.set_maximum_score(10)
        for val in particles_by_type.values():
            r.add_type(val)
        # Sort pairs so that outputs are consistent for assertions
        pairs = r.get_connected_pairs()
        pairs = sorted([sorted(p) for p in pairs])
        pairs = [(m.get_particle_name(p[0]), m.get_particle_name(p[1]))
                 for p in pairs]
        return r.evaluate(False), pairs

    def test_composite_restraint_subset(self):
        """Make sure CompositeRestraint finds the right subset"""
        m = IMP.Model()
        # A2, B2, C2 should be pruned since they are far away, B1-A1-C1 returned
        score, pairs = self.evaluate_config(m,
                             [('A1', 0,0),
                              ('B1', 0,-1),
                              ('C1', 1,0),
                              ('A2', -10,-10),
                              ('B2', 10,10),
                              ('C2', 10,-10)])
        self.assertAlmostEqual(score, 2.0, delta=1e-6)
        self.assertEqual(pairs, [('A1', 'B1'), ('A1', 'C1')])

    def test_composite_restraint_simple(self):
        """Check CompositeRestraint with simple systems, no copies"""
        m = IMP.Model()
        score, pairs = self.evaluate_config(m,
                             [('A1', 0,0),
                              ('B1', 1,0),
                              ('C1', 2,0)])
        self.assertAlmostEqual(score, 2.0, delta=1e-6)
        self.assertEqual(pairs, [('A1', 'B1'), ('B1', 'C1')])

    def test_composite_restraint_multiple(self):
        """Multiple copies of a particle should be OK"""
        m = IMP.Model()
        score, pairs = self.evaluate_config(m,
                             [('A1', 0,0),
                              ('B1', 1,0),
                              ('B2', 2,0),
                              ('C1', 3,0)])
        self.assertAlmostEqual(score, 3.0, delta=1e-6)
        self.assertEqual(pairs, [('A1', 'B1'), ('B1', 'B2'), ('B2', 'C1')])

    def test_composite_restraint_minimum(self):
        """Make sure minimum subtree is returned"""
        m = IMP.Model()
        # Two possible subtrees: A1-B1-B2-C1 and C1-B3-A2; make sure we find
        # the lower scoring one
        score, pairs = self.evaluate_config(m,
                             [('A1', 0,0),
                              ('B1', 1,0),
                              ('B2', 2,0),
                              ('C1', 3,0),
                              ('B3', 4,0),
                              ('A2', 5,0)])
        self.assertAlmostEqual(score, 2.0, delta=1e-6)
        self.assertEqual(pairs, [('C1', 'B3'), ('B3', 'A2')])

    def test_composite_restraint_exceed_max_score_multiple(self):
        """Check handling of multiple copies with an edge above max_score"""
        m = IMP.Model()
        # We have OK A-B and B-C interactions, but the B-B distance is too large
        score, pairs = self.evaluate_config(m,
                             [('A1', 0,0),
                              ('B1', 1,0),
                              ('B2', 30,0),
                              ('C1', 31,0)])
        self.assertAlmostEqual(score, 10.0, delta=1e-6)
        self.assertEqual(pairs, [])

    def test_composite_restraint_exceed_max_score(self):
        """Check failure with an edge above max_score"""
        m = IMP.Model()
        # Composite cannot be satisfied
        score, pairs = self.evaluate_config(m,
                             [('A1', 0,0),
                              ('B1', 1,0),
                              ('C1', 31,0)])
        self.assertAlmostEqual(score, 10.0, delta=1e-6)
        self.assertEqual(pairs, [])

if __name__ == '__main__':
    IMP.test.main()
