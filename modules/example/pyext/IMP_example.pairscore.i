%pythoncode %{

class PythonExamplePairScore(IMP.PairScore):
    """An example PairScore written in Python.
       This should be functionally equivalent to the C++ ExamplePairScore.
    """
    def __init__(self, x0, k):
        IMP.PairScore.__init__(self)
        self.x0, self.k = x0, k

    def evaluate_index(self, m, pip, da):
        d0 = IMP.core.XYZ(m, pip[0])
        d1 = IMP.core.XYZ(m, pip[1])
        diff = (d0.get_coordinates()
                - d1.get_coordinates()).get_magnitude() - self.x0
        score = .5 * self.k * diff * diff
        if da:
            # derivatives are requested
            delta = d0.get_coordinates() - d1.get_coordinates()
            udelta = delta.get_unit_vector()
            dv = self.k * diff
            d0.add_to_derivatives(udelta * dv, da)
            d1.add_to_derivatives(-udelta * dv, da)
        return score

    def do_get_inputs(self, m, pis):
        return IMP.get_particles(m, pis)

%}
