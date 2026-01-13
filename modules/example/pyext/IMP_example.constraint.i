%pythoncode %{

class PythonExampleConstraint(IMP.Constraint):
    """An example Constraint written in Python.
       This should be functionally equivalent to the C++ ExampleConstraint.
    """

    k = None

    def __init__(self, p):
        super().__init__(p.get_model(), "ExampleConstraint%1%")
        self.p = p
        if self.k is None:
            PythonExampleConstraint.k = IMP.IntKey("Constraint key")
        if not p.has_attribute(self.k):
            p.add_attribute(self.k, 0)

    def do_update_attributes(self):
        self.p.set_value(self.k, self.p.get_value(self.k) + 1)

    def _get_jax(self):
        """Implementation of the constraint using JAX"""
        import functools
        def apply_func(X, key, index):
            X[key] = X[key].at[index].add(1)
            return X
        f = functools.partial(apply_func, key=self.k.get_string(),
                              index=self.p.get_index())
        return self._wrap_jax(f, keys=(self.k,))

    def do_update_derivatives(self, da):
        pass

    def do_get_inputs(self):
        return [self.p]

    def do_get_outputs(self):
        return [self.p]

%}
