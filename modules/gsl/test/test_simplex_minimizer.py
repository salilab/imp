import IMP, IMP.test
import IMP.core
import IMP.gsl

corners=[[0,0,0],[0,1,0], [0,0,1], [0,1,1],
         [1,0,0],[1,1,0], [1,0,1], [1,1,1]]

class EasyCubeFunc(IMP.Restraint):
    def __init__(self, model, particles):
        IMP.Restraint.__init__(self)
        self.particles= particles
        self.index= IMP.FloatKey("x")
    def do_show(self, junk):
        print "Woods function"
    def get_version_info(self):
        return IMP.VersionInfo("Daniel Russel", "0.5")
    def unprotected_evaluate(self, accum):
        print "eval"
        e=0
        for i in range(0,len(self.particles)):
            d=IMP.core.XYZ(self.particles[i])
            d.show()
            v=IMP.algebra.Vector3D(corners[i][0], corners[i][1], corners[i][2])
            dist2= (d.get_coordinates()-v).get_squared_magnitude()
            e =e+dist2
        return e
    def get_input_particles(self):
        return IMP.ParticlesTemp(self.particles)
    def get_input_objects(self):
        return IMP.ObjectsTemp()



class Tests(IMP.test.TestCase):
    def _test_it(self):
        model = IMP.Model()
        particles = []

        for i in range(0,2):
            p = IMP.Particle(model)
            particles.append(p)
            IMP.core.XYZ.setup_particle(p, IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())).set_coordinates_are_optimized(True)
        rsr = EasyCubeFunc(model, particles)
        model.add_restraint(rsr)
        print "creating"
        opt = IMP.gsl.Simplex()
        opt.set_minimum_size(.000001)
        opt.set_initial_length(1)
        opt.set_model(model)
        #opt.set_threshold(1e-5)
        print "optimize"
        e = opt.optimize(1000000)
        self.assertAlmostEqual(e, 0.0, places=1)
    def test_starting_conditions(self):
        """Test the simplex optimizer with given starting conditions"""
        self.probabilistic_test("self._test_it", .5)

if __name__ == '__main__':
    IMP.test.main()
