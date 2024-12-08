import IMP.test
import io


class NullConstraint(IMP.Constraint):

    def __init__(self, m, inputs=[], outputs=[]):
        IMP.Constraint.__init__(self, m, "NullConstraint%1%")
        self.inputs = inputs
        self.outputs = outputs

    def do_update_attributes(self):
        pass

    def do_update_derivatives(self):
        pass

    def do_get_inputs(self):
        return self.inputs

    def do_get_outputs(self):
        return self.outputs


class Tests(IMP.test.TestCase):

    def test_state_show(self):
        """Test dependency graph"""
        m = IMP.Model("dependency graph")
        dg = IMP.get_dependency_graph(m)
        print(dir(dg))
        dg.show()

    def test_loop_detection(self):
        """Test that reasonable errors are returned for dependency graph loops"""
        m = IMP.Model()
        p0 = m.get_particle(m.add_particle("p0"))
        p1 = m.get_particle(m.add_particle("p1"))
        s0 = NullConstraint(m, inputs=[p0], outputs=[p1])
        s1 = NullConstraint(m, inputs=[p1], outputs=[p0])
        self.assertRaises(IMP.ModelException, m.update)

    def test_get_dependent_particles(self):
        """Test get_dependent_particles() function"""
        log = []
        m = IMP.Model()
        p0 = IMP.Particle(m)
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        s0 = NullConstraint(m, inputs=[p0], outputs=[p1])
        self.assertEqual(m.get_dependent_particles_uncached(p0), [p0, p1])
        self.assertEqual(m.get_dependent_particles_uncached(p1), [p1])
        self.assertEqual(m.get_dependent_particles_uncached(p2), [p2])

    def test_dependency_age(self):
        """Test dependency age counter"""
        m = IMP.Model()
        # No dependencies yet, so age==0
        self.assertEqual(m.get_age(), 1)
        self.assertEqual(m.get_dependencies_updated(), 0)
        # Increasing model age should not affect dependency age
        m.update()
        self.assertEqual(m.get_age(), 2)
        self.assertEqual(m.get_dependencies_updated(), 0)
        # Adding particles should update dep age
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        m.update()
        self.assertEqual(m.get_age(), 3)
        self.assertEqual(m.get_dependencies_updated(), 2)
        # Adding ScoreStates should update dep age
        s0 = NullConstraint(m, inputs=[p1], outputs=[p2])
        m.update()
        self.assertEqual(m.get_age(), 4)
        self.assertEqual(m.get_dependencies_updated(), 3)
        # Adding Restraints should update dep age
        r = IMP._ConstRestraint(m, [p2], 1)
        m.update()
        self.assertEqual(m.get_age(), 5)
        self.assertEqual(m.get_dependencies_updated(), 4)
        # Increasing model age should not affect dependency age
        m.update()
        self.assertEqual(m.get_age(), 6)
        self.assertEqual(m.get_dependencies_updated(), 4)


if __name__ == '__main__':
    IMP.test.main()
