import IMP
import IMP.core
import IMP.atom
import IMP.algebra
import IMP.isd
import IMP.test
import numpy as np
try:
    import jax
    import IMP.jax
except ImportError:
    jax = None


def create_random_gaussians(m, num, rad_scale=1.0):
    ret = []
    for n in range(num):
        std = np.random.random_sample(3) * 4 + 0.5
        center = np.random.random_sample(3) * 10 - 5
        rot = IMP.algebra.get_random_rotation_3d()
        trans = IMP.algebra.Transformation3D(rot, center)
        shape = IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(trans),
                                       std ** 2)
        p = IMP.Particle(m)
        IMP.core.Gaussian.setup_particle(p, shape)
        IMP.atom.Mass.setup_particle(p, 1.0 / num)
        IMP.core.XYZR.setup_particle(p).set_radius(max(std) * rad_scale)
        ret.append(p)
    return ret


def get_covariance(p):
    return np.asarray(IMP.core.Gaussian(p).get_global_covariance())


def get_overlap(p1, p2):
    """Reference implementation of IMP::isd::score_gaussian_overlap"""
    c = get_covariance(p1) + get_covariance(p2)
    v = (np.asarray(IMP.core.XYZ(p2).get_coordinates())
         - np.asarray(IMP.core.XYZ(p1).get_coordinates()))
    mass12 = IMP.atom.Mass(p1).get_mass() * IMP.atom.Mass(p2).get_mass()
    return (mass12 / np.sqrt((2. * np.pi) ** 3 * np.linalg.det(c))
            * np.exp(-0.5 * v.dot(np.linalg.solve(c, v))))


def get_sphere_distance(p1, p2):
    return IMP.core.get_distance(IMP.core.XYZR(p1), IMP.core.XYZR(p2))


def reference_score(model_ps, density_ps, model_cutoff, density_cutoff):
    """Reference implementation using exact cutoffs (the C++ restraint
       uses close pair containers which may include some pairs beyond
       the cutoff)"""
    dd = sum(get_overlap(p1, p2) for p1 in density_ps for p2 in density_ps)
    mm = sum(get_overlap(p, p) for p in model_ps)
    for i, p1 in enumerate(model_ps):
        for p2 in model_ps[i + 1:]:
            if get_sphere_distance(p1, p2) < model_cutoff:
                mm += 2. * get_overlap(p1, p2)
    md = sum(get_overlap(p1, p2) for p1 in model_ps for p2 in density_ps
             if get_sphere_distance(p1, p2) < density_cutoff)
    return -np.log(2. * md / (mm + dd))


def shuffle_particles(ps):
    for p in ps:
        tr = IMP.algebra.get_random_local_transformation(
            IMP.algebra.Vector3D(0, 0, 0), 2.0, 1.0)
        IMP.core.transform(IMP.core.RigidBody(p), tr)


@IMP.test.skipIf(jax is None, "No JAX support")
class Tests(IMP.test.TestCase):
    def make_system(self, model_cutoff=1e8, density_cutoff=1e8, slope=0.,
                    local=False):
        m = IMP.Model()
        density_ps = create_random_gaussians(m, 4)
        model_ps = create_random_gaussians(m, 10)
        psigma = IMP.Particle(m)
        IMP.isd.Scale.setup_particle(psigma, 1.0)
        r = IMP.isd.GaussianEMRestraint(
            m, model_ps, density_ps, psigma, model_cutoff, density_cutoff,
            slope, True, False, local)
        return m, model_ps, density_ps, r

    def get_jax_score_and_derivs(self, r, model_ps):
        ji = r._get_jax(space=IMP.jax.FreeSpace)
        jm = ji.get_jax_model()

        def score_xyz(xyz):
            jm2 = jm.copy()
            jm2['xyz'] = xyz
            return ji.score_func(jm2)
        score, grad = jax.jit(jax.value_and_grad(score_xyz))(jm['xyz'])
        pis = [p.get_index() for p in model_ps]
        return float(score), np.asarray(grad)[np.array(pis, dtype=int)]

    def assert_matches_cpp(self, m, r, model_ps):
        # Make sure that the C++ global covariances are up to date
        sf = IMP.core.RestraintsScoringFunction([r])
        cpp_score = sf.evaluate(True)
        cpp_derivs = np.array([IMP.core.XYZ(p).get_derivatives()
                               for p in model_ps])
        jax_score, jax_derivs = self.get_jax_score_and_derivs(r, model_ps)
        # JAX uses 32-bit floats by default, so use relative tolerances
        self.assertAlmostEqual(cpp_score, jax_score,
                               delta=1e-5 * abs(cpp_score))
        scale = np.max(np.abs(cpp_derivs))
        self.assertLess(np.max(np.abs(cpp_derivs - jax_derivs)),
                        1e-4 * scale)

    def test_overlap(self):
        """Test JAX Gaussian overlap function"""
        from IMP.isd._jax_util import get_gaussian_overlap
        m = IMP.Model()
        p1, p2 = create_random_gaussians(m, 2)
        v = (np.asarray(IMP.core.XYZ(p2).get_coordinates())
             - np.asarray(IMP.core.XYZ(p1).get_coordinates()))
        mass12 = IMP.atom.Mass(p1).get_mass() * IMP.atom.Mass(p2).get_mass()
        for xp in (np, jax.numpy):
            ov = get_gaussian_overlap(v, get_covariance(p1)
                                      + get_covariance(p2), mass12, xp=xp)
            self.assertAlmostEqual(float(ov), get_overlap(p1, p2),
                                   delta=1e-6 * get_overlap(p1, p2))

    def test_jax(self):
        """Test JAX implementation of GaussianEMRestraint"""
        m, model_ps, density_ps, r = self.make_system()
        for i in range(3):
            # Moving the model also rotates the Gaussians; JAX should
            # calculate covariances from the new orientations
            shuffle_particles(model_ps)
            self.assert_matches_cpp(m, r, model_ps)

    def test_jax_slope(self):
        """Test JAX implementation of GaussianEMRestraint with slope"""
        m, model_ps, density_ps, r = self.make_system(slope=0.1)
        self.assert_matches_cpp(m, r, model_ps)

    def test_jax_local(self):
        """Test JAX implementation of local GaussianEMRestraint"""
        m, model_ps, density_ps, r = self.make_system(local=True)
        self.assert_matches_cpp(m, r, model_ps)

    def test_jax_cutoff(self):
        """Test JAX implementation of GaussianEMRestraint with cutoffs"""
        m, model_ps, density_ps, r = self.make_system(
            model_cutoff=0., density_cutoff=0.)
        exp_score = reference_score(model_ps, density_ps, 0., 0.)
        jax_score, jax_derivs = self.get_jax_score_and_derivs(r, model_ps)
        self.assertAlmostEqual(exp_score, jax_score,
                               delta=1e-5 * abs(exp_score))

    def test_jax_weight(self):
        """Test that JAX GaussianEMRestraint respects restraint weight"""
        m, model_ps, density_ps, r = self.make_system()
        r.set_weight(10.)
        self.assert_matches_cpp(m, r, model_ps)

    def test_jax_periodic(self):
        """Test that JAX GaussianEMRestraint rejects periodic boundaries"""
        m, model_ps, density_ps, r = self.make_system()
        space = IMP.jax.PeriodicSpace([100., 100., 100.])
        self.assertRaises(NotImplementedError, r._get_jax, space=space)


if __name__ == '__main__':
    IMP.test.main()
