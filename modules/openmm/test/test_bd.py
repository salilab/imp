import IMP
import IMP.test
import IMP.openmm
import IMP.core
import IMP.algebra

class AngleRestraintTests(IMP.test.TestCase):
    """Tests for angle restraints"""
    def test_bd(self):
        m= IMP.Model()
        for i in range(0,10):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(i*30,0,0), .5*i*10+10))
            d.set_coordinates_are_optimized(True)
            dd= IMP.atom.Diffusion.setup_particle(p)
            dd.set_D_from_radius()
            md= IMP.atom.Mass.setup_particle(p, 1)
        dp=IMP.container.ListSingletonContainer(m.get_particles())
        evr= IMP.core.ExcludedVolumeRestraint(dp)
        m.add_restraint(evr)
        sp = IMP.atom.SimulationParameters.setup_particle(IMP.Particle(m))

        sp.set_maximum_time_step(1000)
        o= IMP.openmm.BrownianDynamics(sp)
        o.optimize(100)
        for i in range(0,dp.get_number_of_particles()):
            print IMP.core.XYZ(dp.get_particle(i)).get_coordinates()

if __name__ == '__main__':
    IMP.test.main()
