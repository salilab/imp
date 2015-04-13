%pythoncode %{

class PythonExampleUnaryFunction(IMP.UnaryFunction):
    """An example UnaryFunction written in Python.
       This should be functionally equivalent to the C++ ExampleUnaryFunction.
    """
    def __init__(self, center, k):
        IMP.UnaryFunction.__init__(self)
        self.center, self.k = center, k

    def evaluate_with_derivative(self, feature):
        return IMP.DerivativePair(self.evaluate(feature),
                                  self.k * (feature - self.center))

    def evaluate(self, feature):
        return .5 * self.k * (feature - self.center) * (feature - self.center)

%}
