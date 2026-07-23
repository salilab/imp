import IMP
import IMP.test
import IMP.core
import IMP.algebra


class FloatValues:
    """Store values in Float attributes, one per particle"""
    def __init__(self, model):
        self.m = model
        self.xkey = IMP.FloatKey("x")

    def make_particles(self, values):
        self.particles = []
        for value in values:
            p = self.m.add_particle("p")
            self.particles.append(p)
            self.m.add_attribute(self.xkey, p, value)
            self.m.set_is_optimized(self.xkey, p, True)
        return self.particles

    def get_values(self):
        for p in self.particles:
            yield self.m.get_attribute(self.xkey, p)

    def add_to_derivative(self, dx, accum):
        for (p, d) in zip(self.particles, dx):
            self.m.add_to_derivative(self.xkey, p, d, accum)


class VectorValues:
    """Store values in Vector3D/4D attributes, dim per particle"""
    def __init__(self, model, dim):
        self.m, self.dim = model, dim
        if dim == 4:
            self.xkey = IMP.Vector4DDerivKey("x")
            self.vectyp = IMP.algebra.Vector4D
        else:
            self.xkey = IMP.Vector3DDerivKey("x")
            self.vectyp = IMP.algebra.Vector3D

    def make_particles(self, values):
        self.particles = []
        self.len_values = len(values)
        padded = list(values) + [0.0] * self.dim
        for i in range(0, len(values), self.dim):
            p = self.m.add_particle("p")
            self.particles.append(p)
            v = self.vectyp(*padded[i:i+self.dim])
            self.m.add_attribute(self.xkey, p, v)
            self.m.set_is_optimized(self.xkey, p, True)
        return self.particles

    def get_values(self):
        valind = 0
        for p in self.particles:
            val = self.m.get_attribute(self.xkey, p)
            for i in range(self.dim):
                yield val[i]
                valind += 1
                if valind >= self.len_values:
                    return

    def add_to_derivative(self, dx, accum):
        padded = dx + [0.0] * self.dim
        for (p, i) in zip(self.particles, range(0, len(dx), self.dim)):
            d = self.vectyp(*padded[i:i+self.dim])
            self.m.add_to_derivative(self.xkey, p, d, accum)


class WoodsFunc(IMP.Restraint):

    """Woods function for four input values, defined as an IMP restraint"""

    def __init__(self, model, values):
        super().__init__(model, "WoodsFunc %1%")
        self.values = values

    def do_show(self, fh):
        fh.write("Woods function")

    def get_version_info(self):
        return IMP.VersionInfo("Daniel Russel", "0.5")

    def unprotected_evaluate(self, accum):
        (x1, x2, x3, x4) = list(self.values.get_values())
        a = x2 - x1 * x1
        b = x4 - x3 * x3
        e = 100.0 * a * a + (1.0 - x1) ** 2 + 90.0 * b * b + (1.0 - x3) ** 2 \
            + 10.1 * ((x2 - 1.0) ** 2 + (x4 - 1.0) ** 2) \
            + 19.8 * (x2 - 1.0) * (x4 - 1.0)
        if accum:
            dx = [-2.0 * (200.0 * x1 * a + 1.0 - x1),
                  2.0 * (100.0 * a + 10.1 * (x2 - 1.0) + 9.9 * (x4 - 1.0)),
                  -2.0 * (180.0 * x3 * b + 1.0 - x3),
                  2.0 * (90.0 * b + 10.1 * (x4 - 1.0) + 9.9 * (x2 - 1.0))]
            self.values.add_to_derivative(dx, accum)
            # for (i, d) in zip(self.indices, dx):
            #    accum.add_to_deriv(i, d)
        return e

    def do_get_inputs(self):
        m = self.get_model()
        return IMP.get_particles(m, self.values.particles)


class Tests(IMP.test.TestCase):

    def test_no_scoring_function(self):
        """Optimizer should fail without a scoring function set"""
        model = IMP.Model()
        opt = IMP.core.ConjugateGradients(model)
        self.assertRaises(ValueError, opt.get_scoring_function)

    def test_cg_woods_func_float(self):
        """Check that we can optimize the Woods function with CG on FloatKey"""
        self._test_starting_conditions((-3.0, -1.0, -3.0, -1.0))
        self._test_starting_conditions((2.0, 3.0, 8.0, -5.0))

    def test_cg_woods_func_vector3d(self):
        """Check CG optimization of Vector3DDerivKey attributes"""
        self._test_starting_conditions((-3.0, -1.0, -3.0, -1.0),
                                       use_vectord=3)
        self._test_starting_conditions((2.0, 3.0, 8.0, -5.0),
                                       use_vectord=3)

    def test_cg_woods_func_vector4d(self):
        """Check CG optimization of Vector4DDerivKey attributes"""
        self._test_starting_conditions((-3.0, -1.0, -3.0, -1.0),
                                       use_vectord=4)
        self._test_starting_conditions((2.0, 3.0, 8.0, -5.0),
                                       use_vectord=4)

    def _test_starting_conditions(self, starting_values, use_vectord=None):
        """Test the optimizer with given starting conditions"""
        model = IMP.Model()

        if use_vectord:
            ph = VectorValues(model, use_vectord)
        else:
            ph = FloatValues(model)
        particles = ph.make_particles(starting_values)
        rsr = WoodsFunc(model, ph)
        _ = rsr.do_get_inputs()
        opt = IMP.core.ConjugateGradients(model)
        opt.set_scoring_function([rsr])
        opt.set_gradient_threshold(1e-5)
        e = opt.optimize(100)
        for val in ph.get_values():
            self.assertAlmostEqual(val, 1.0, places=1)
        self.assertAlmostEqual(e, 0.0, places=2)


if __name__ == '__main__':
    IMP.test.main()
