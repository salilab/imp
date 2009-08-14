import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom



class MCOptimizerTest(IMP.test.TestCase):
    def _compute_moments(self, ps):
        m= IMP.algebra.Vector3D(0,0,0)
        for p in ps:
            d= IMP.core.XYZ(p.get_particle())
            m+= d.get_coordinates()
        m=m/len(ps)
        s= IMP.algebra.Vector3D(0,0,0)
        for p in ps:
            d= IMP.core.XYZ(p.get_particle())
            diff= d.get_coordinates()-m
            for i in range(0,3):
                s[i]= s[i]+diff[i]*diff[i]
        s=s/len(ps)
        for i in range(0,3):
            s[i]= s[i]**.5
        return (m[0], m[1], m[2], s[0], s[1], s[2])
    def _test_step_size(self):
        """Testing the step size invariance of free diffusion"""
        m= IMP.Model()
        si=IMP.atom.SimulationParameters.setup_particle(IMP.Particle(m), 1e3)
        si.show()
        bd= IMP.atom.BrownianDynamics(si)
        bd.set_model(m)
        radius=2
        ps= IMP.core.create_xyzr_particles(m, 1000, radius)
        for p in ps:
            d= IMP.atom.Diffusion.setup_particle(p.get_particle())
            d.set_D_from_radius_in_angstroms(radius)
            d.set_coordinates_are_optimized(True)
            d.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        si.set_maximum_time_step_in_femtoseconds(1e4)
        IMP.set_log_level(IMP.SILENT)
        bd.set_log_level(IMP.TERSE)
        bd.optimize(1000)
        print "time is " + str(si.get_current_time_in_femtoseconds())
        moments= self._compute_moments(ps)
        print moments
        for p in ps:
            d= IMP.atom.Diffusion(p.get_particle())
            d.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        si.set_maximum_time_step_in_femtoseconds(1e7)
        bd.optimize(1)
        print si.get_current_time_in_femtoseconds()
        big_moments= self._compute_moments(ps)
        print big_moments
        for i in range(0,3):
            print i
            self.assertInTolerance(big_moments[i], moments[i],
                                   len(ps)**.5)
        for i in range(3,6):
            print i
            self.assertInTolerance(big_moments[i], moments[i],
                                   .1*(big_moments[i]+ moments[i]))

if __name__ == '__main__':
    unittest.main()
