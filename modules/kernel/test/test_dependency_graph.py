import IMP.kernel
import IMP.test
import StringIO


class NullConstraint(IMP.kernel.Constraint):

    def __init__(self, m, inputs=[], outputs=[]):
        IMP.kernel.Constraint.__init__(self, m, "NullConstraint%1%")
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
        m = IMP.kernel.Model("dependency graph")
        dg = IMP.kernel.get_dependency_graph(m)
        print dir(dg)
        dg.show()

    def test_loop_detection(self):
        """Test that reasonable errors are returned for dependency graph loops"""
        m = IMP.kernel.Model()
        p0 = m.get_particle(m.add_particle("p0"))
        p1 = m.get_particle(m.add_particle("p1"))
        s0 = NullConstraint(m, inputs=[p0], outputs=[p1])
        s1 = NullConstraint(m, inputs=[p1], outputs=[p0])
        self.assertRaises(IMP.base.ModelException, m.update)

if __name__ == '__main__':
    IMP.test.main()
