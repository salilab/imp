%pythoncode %{

class PythonExampleSingletonModifier(IMP.SingletonModifier):
    """An example SingletonModifier written in Python.
       This should be functionally equivalent to the C++
       ExampleSingletonModifier.
    """

    def __init__(self, bb):
        super().__init__("ExampleSingletonModifier%1%")
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

    def _get_jax(self, m, index):
        """Implementation of the modifier using JAX"""
        import functools
        import jax.numpy as jnp
        def apply_func(X, index, box):
            X['xyz'] = X['xyz'].at[index].set(
                jnp.mod(X['xyz'][index], box[1] - box[0]) + box[0])
            return X
        box = jnp.array([self.bb.get_corner(x) for x in range(2)])
        return self._wrap_jax(
            functools.partial(apply_func, index=index, box=box))

    def do_get_inputs(self, m, pis):
        return IMP.get_particles(m, pis)

    def do_get_outputs(self, m, pis):
        return self.do_get_inputs(m, pis)

%}
