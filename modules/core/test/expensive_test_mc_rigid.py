import IMP
import IMP.test
import IMP.core

class MCOptimizerTest(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.base.TERSE)
        self.m = IMP.Model()
        #read molecules
        self.m1 = IMP.kernel._create_particles_from_pdb(self.get_input_file_name("1z5s_A.pdb"),
                                                self.m)
        self.m2 = IMP.kernel._create_particles_from_pdb(self.get_input_file_name("1z5s_C.pdb"),
                                                self.m)
        #create rigid bodies
        self.rb0=IMP.core.RigidBody.setup_particle(IMP.Particle(self.m), self.m1)
        self.rb0.set_coordinates_are_optimized(True)
        self.rb1=IMP.core.RigidBody.setup_particle(IMP.Particle(self.m), self.m2)
        self.rb1.set_coordinates_are_optimized(True)
        #add restraints
        self.h = IMP.core.HarmonicUpperBound(0,3.)

        self.dr = IMP.core.DistanceRestraint(self.h,self.rb0, self.rb1)
        self.m.add_restraint(self.dr)

    def randomize(self, mh):
        point1 = IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0.,0.,0.),
                                                  IMP.algebra.Vector3D(5.,5.,5.)))
        trans1 = IMP.algebra.Transformation3D(
          IMP.algebra.get_random_rotation_3d(),
          point1)
        IMP.core.RigidBody(mh).set_reference_frame(IMP.algebra.ReferenceFrame3D(trans1))
    def test_c1(self):
        """test monte carlo with rigid bodies"""
        #rigid transformation of the two molecules
        mhs = []
        mhs.append(self.m1)
        mhs.append(self.m2)
        for i in range(0,1000):
            print "randomizing"
            print IMP.core.XYZ(self.rb0).get_coordinates()
            print IMP.core.XYZ(self.rb1).get_coordinates()
            self.randomize(self.rb0)
            self.randomize(self.rb1)
            print IMP.core.XYZ(self.rb0).get_coordinates()
            print IMP.core.XYZ(self.rb1).get_coordinates()
            score= self.m.evaluate(False)
            print score
            if score >0:
                break
        #optimize
        lopt= IMP.core.ConjugateGradients(self.m)
        opt = IMP.core.MonteCarloWithLocalOptimization(lopt, 100)
        mover1 = IMP.core.RigidBodyMover(IMP.core.RigidBody(self.rb0),5.,15.)
        opt.add_mover(mover1)
        mover2 = IMP.core.RigidBodyMover(IMP.core.RigidBody(self.rb1),5.,15.)
        opt.add_mover(mover2)
        opt.set_score_threshold(.001)
        for i in range(0,5):
            print "run", i
            opt.optimize(20)
            e = self.m.evaluate(False)
            if e<.001:
                break
        self.assertAlmostEqual(e, 0.0, places=2)
        print "all done"

if __name__ == '__main__':
    IMP.test.main()
