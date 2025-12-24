%pythoncode %{

class PythonExampleUnaryFunction(IMP.UnaryFunction):
    """An example UnaryFunction written in Python.
       This should be functionally equivalent to the C++ ExampleUnaryFunction.
    """
    def __init__(self, center, k):
        super().__init__()
        self.center, self.k = center, k

    def evaluate_with_derivative(self, feature):
        return (self.evaluate(feature), self.k * (feature - self.center))

    def evaluate(self, feature):
        return .5 * self.k * (feature - self.center) * (feature - self.center)

    def _get_jax(self):
        """Implementation of the unary function using JAX"""
        import functools
        def score(feature, center, k):
            return 0.5 * k * (feature - center) ** 2
        return functools.partial(score, center=self.center, k=self.k)
%}
