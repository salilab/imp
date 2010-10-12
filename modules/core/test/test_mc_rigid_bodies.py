import IMP
import IMP.test
import IMP.core
import IMP.atom

class MCOptimizerTest(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.TERSE)
        self.m = IMP.Model()
        #read molecules
        self.m1 = IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),
                                    self.m,IMP.atom.NonWaterPDBSelector())
        self.m2 = IMP.atom.read_pdb(self.get_input_file_name("1z5s_C.pdb"),
                                    self.m,IMP.atom.NonWaterPDBSelector())
        #create rigid bodies
        IMP.core.RigidBody.setup_particle(self.m1.get_particle(),
                                          IMP.core.XYZs(IMP.core.get_leaves(self.m1))).set_coordinates_are_optimized(True)
        IMP.core.RigidBody.setup_particle(self.m2.get_particle(),
                                          IMP.core.XYZs(IMP.core.get_leaves(self.m2))).set_coordinates_are_optimized(True)
        #add restraints
        self.h = IMP.core.HarmonicUpperBound(0,3.)

        self.dr = IMP.core.DistanceRestraint(self.h,self.m1,
                                             self.m2)
        self.m.add_restraint(self.dr)

    def randomize(self, mh):
        point1 = IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0.,0.,0.),
                                                  IMP.algebra.Vector3D(5.,5.,5.)))
        trans1 = IMP.algebra.Transformation3D(
          IMP.algebra.get_random_rotation_3d(),
          point1)
        IMP.core.RigidBody(mh).set_transformation(trans1)
    def test_c1(self):
        """test monte carlo with rigid bodies"""
        #rigid transformation of the two molecules
        mhs = IMP.atom.Hierarchies()
        mhs.append(self.m1)
        mhs.append(self.m2)
        for i in range(0,1000):
            print "randomizing"
            print IMP.core.XYZ(self.m1).get_coordinates()
            print IMP.core.XYZ(self.m2).get_coordinates()
            self.randomize(self.m1)
            self.randomize(self.m2)
            print IMP.core.XYZ(self.m1).get_coordinates()
            print IMP.core.XYZ(self.m2).get_coordinates()
            score= self.m.evaluate(False) #to transform the children
            print score
            if score >0:
                break
        #optimize
        opt = IMP.core.MonteCarlo()
        opt.set_model(self.m)
        mover1 = IMP.core.RigidBodyMover(IMP.core.RigidBody(self.m1),5.,15.)
        opt.add_mover(mover1)
        mover2 = IMP.core.RigidBodyMover(IMP.core.RigidBody(self.m2),5.,15.)
        opt.add_mover(mover2)
        lopt= IMP.core.ConjugateGradients()
        lopt.set_model(self.m)
        opt.set_local_optimizer(lopt)
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
