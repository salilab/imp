%pythoncode %{

class PythonExampleConstraint(IMP.Constraint):
    """An example Constraint written in Python.
       This should be functionally equivalent to the C++ ExampleConstraint.
    """

    k = None

    def __init__(self, p):
        IMP.Constraint.__init__(self, p.get_model(), "ExampleConstraint%1%")
        self.p = p
        if self.k is None:
            PythonExampleConstraint.k = IMP.IntKey("Constraint key")
        if not p.has_attribute(self.k):
            p.add_attribute(self.k, 0)

    def do_update_attributes(self):
        self.p.set_value(self.k, self.p.get_value(self.k) + 1)

    def do_update_derivatives(self, da):
        pass

    def do_get_inputs(self):
        return [self.p]

    def do_get_outputs(self):
        return [self.p]

%}
