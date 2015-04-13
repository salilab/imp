%pythoncode %{

class PythonExampleSingletonModifier(IMP.SingletonModifier):
    """An example SingletonModifier written in Python.
       This should be functionally equivalent to the C++
       ExampleSingletonModifier.
    """

    def __init__(self, bb):
        IMP.SingletonModifier.__init__(self, "ExampleSingletonModifier%1%")
        self.bb = bb

    def apply_index(self, m, pi):
        d = IMP.core.XYZ(m, pi)
        for i in range(3):
            # shift the coordinate until it is in the box
            while d.get_coordinate(i) < self.bb.get_corner(0)[i]:
                d.set_coordinate(i,d.get_coordinate(i) + \
                       (self.bb.get_corner(1)[i] - self.bb.get_corner(0)[i]))
            while d.get_coordinate(i) > self.bb.get_corner(1)[i]:
                d.set_coordinate(i,d.get_coordinate(i) - \
                       (self.bb.get_corner(1)[i] - self.bb.get_corner(0)[i]))

    def do_get_inputs(self, m, pis):
        return IMP.get_particles(m, pis)

    def do_get_outputs(self, m, pis):
        return self.do_get_inputs(m, pis)

%}
