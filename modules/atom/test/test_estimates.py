from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.atom
import math

class Tests(IMP.test.TestCase):

    def test_massvol(self):
        """Testing the mass and volume estimates"""
        m = IMP.atom.get_mass_from_number_of_residues(250)
        print(m)
        v = IMP.atom.get_volume_from_mass(m)
        print(v)
        r = (v / (4.0 * 3.1415) * 3.0) ** .333
        rc = 0.726 * m ** .333  # from Frank's paper
        print(r)
        print(rc)
        self.assertAlmostEqual(r, rc, delta=.1 * r)

    def _test_d_for_dt(self, dt, nonrandom_translation=[0.0,0.0,0.0]):
        """Testing the diffusion coeff estimate for a specific delta-T
        nonrandom_translation - non-random translation added to the coordinates at each time step and should be filtered out correctly
        """
        # see expensive_test_rot_bd.py for test of rotational diffusion coeff
        print("dt=%.1f [fs]" % dt)
        m = IMP.Model()
        p = IMP.Particle(m)
        d = IMP.core.XYZR.setup_particle(p)
        d.set_coordinates_are_optimized(True)
        d.set_radius(10)
        dd = IMP.atom.Diffusion.setup_particle(p)
        bd = IMP.atom.BrownianDynamics(m)
        rs = IMP.RestraintSet(m)
        bd.set_scoring_function(rs)
        dt = 10000
        bd.set_maximum_time_step(dt)
        bd.set_log_level(IMP.SILENT)
        m.set_log_level(IMP.SILENT)
        diffs = []
        for i in range(0, 2000):
            d.set_coordinates(IMP.algebra.get_zero_vector_3d())
            bd.optimize(1)
            d.set_coordinates(d.get_coordinates()+nonrandom_translation)
            diffs.append(d.get_coordinates())
        nd = IMP.atom.get_diffusion_coefficient(diffs, dt)
        real_D= dd.get_diffusion_coefficient()
        print("Estimated/real D: {:.4e} {:.4e} [A^2/fs]".format(nd, real_D))
        self.assertAlmostEqual(
            nd, real_D, delta=.12 * (nd + real_D))
        # Test with vector of dts:
        dts= [float(dt)]*len(diffs)
        nd2 = IMP.atom.get_diffusion_coefficient(diffs, dts)
        print("nd2 {:.4e}".format(nd2))
        self.assertAlmostEqual(
            nd, nd2, delta=.000001*(nd+nd2))
        # Test with a change of timestep on-the-fly:
        scale_dt= .01
        new_dt= dt * scale_dt
        new_nonrandom_translation= [x*scale_dt for x in nonrandom_translation]
        bd.set_maximum_time_step(new_dt)
        for i in range(0, 2000):
            d.set_coordinates(IMP.algebra.get_zero_vector_3d())
            bd.optimize(1)
            d.set_coordinates(d.get_coordinates() + new_nonrandom_translation)
            diffs.append(d.get_coordinates())
            dts.append(new_dt)
        nd3 = IMP.atom.get_diffusion_coefficient(diffs, dts)
        print("After change of time step: {:.4e} [A^2/fs]".format(nd3))
        self.assertAlmostEqual(nd, nd3, delta=.1*(nd+nd3))

        return nd

    def test_d(self):
        """Testing the diffusion coeff estimate"""
        nd10000= self._test_d_for_dt(10000)
        nd10= self._test_d_for_dt(10)
        self.assertAlmostEqual(nd10000, nd10, delta=.1*(nd10+nd10000))
        nd10b= self._test_d_for_dt(10,[.5,.1,.2])
        self.assertAlmostEqual(nd10, nd10b, delta=.1*(nd10+nd10000))

if __name__ == '__main__':
    IMP.test.main()
