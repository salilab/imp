%pythoncode %{

class PythonExampleRestraint(IMP.Restraint):
    """An example restraint written in Python.
       This should be functionally equivalent to the C++ ExampleRestraint.
    """
    def __init__(self, m, p, k):
        IMP.Restraint.__init__(self, m, "PythonExampleRestraint%1%")
        self.p, self.k = p, k

    def do_add_score_and_derivatives(self, sa):
        d = IMP.core.XYZ(self.get_model(), self.p)
        score = .5 * self.k * d.get_z() * d.get_z()
        if sa.get_derivative_accumulator():
            deriv = self.k * d.get_z()
            d.add_to_derivative(2, deriv, sa.get_derivative_accumulator())
        sa.add_score(score)

    def do_get_inputs(self):
        return [self.get_model().get_particle(self.p)]

%}
