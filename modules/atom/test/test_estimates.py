import IMP
import IMP.test
import IMP.core
import IMP.atom



class Tests(IMP.test.TestCase):
    def test_massvol(self):
        """Testing the mass and volume estimates"""
        m= IMP.atom.get_mass_from_number_of_residues(250)
        print m
        v= IMP.atom.get_volume_from_mass(m)
        print v
        r= (v/(4.0*3.1415)*3.0)**.333
        rc= 0.726*m**.333 # from Frank's paper
        print r
        print rc
        self.assertAlmostEqual(r, rc, delta=.1*r)
    def test_d(self):
        """Testing the diffusion coeff estimate"""
        m= IMP.Model()
        p= IMP.Particle(m)
        d= IMP.core.XYZR.setup_particle(p)
        d.set_coordinates_are_optimized(True)
        d.set_radius(10)
        dd= IMP.atom.Diffusion.setup_particle(p)
        bd= IMP.atom.BrownianDynamics(m)
        dt=10000
        bd.set_maximum_time_step(dt)
        bd.set_log_level(IMP.base.SILENT)
        m.set_log_level(IMP.base.SILENT)
        diffs=[]
        for i in range(0,3000):
            d.set_coordinates(IMP.algebra.get_zero_vector_3d())
            bd.optimize(1)
            diffs.append(d.get_coordinates())
        nd= IMP.atom.get_diffusion_coefficient(diffs, dt)
        print nd, dd.get_diffusion_coefficient()
        self.assertAlmostEqual(nd, dd.get_diffusion_coefficient(), delta= .1*nd)

if __name__ == '__main__':
    IMP.test.main()
