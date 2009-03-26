import unittest
import IMP, IMP.test
import IMP.core
import IMP.atom

class MCOptimizerTest(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.TERSE)
        self.m = IMP.Model()
        #read molecules
        self.m1 = IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),
                                    self.m,IMP.atom.NonWaterSelector())
        self.m2 = IMP.atom.read_pdb(self.get_input_file_name("1z5s_C.pdb"),
                                    self.m,IMP.atom.NonWaterSelector())
        #create rigid bodies
        self.rb1 = IMP.core.create_rigid_body(self.m1.get_particle(),IMP.core.get_leaves(self.m1))
        self.rb2 = IMP.core.create_rigid_body(self.m2.get_particle(),IMP.core.get_leaves(self.m2))
        self.m.add_score_state(self.rb1)
        self.m.add_score_state(self.rb2)
        #add restraints
        self.rsrs=IMP.core.RestraintSet()
        self.rsrs.set_model(self.m)
        self.h = IMP.core.HarmonicUpperBound(IMP.algebra.distance(
            IMP.core.XYZDecorator.cast(self.m1.get_particle()).get_coordinates(),
            IMP.core.XYZDecorator.cast(self.m2.get_particle()).get_coordinates()),
            3.)

        self.dr = IMP.core.DistanceRestraint(self.h,self.m1.get_particle(),
                                          self.m2.get_particle())
        self.rsrs.add_restraint(self.dr)

    def test_c1(self):
        """test monte carlo with rigid bodies"""
        #rigid transformation of the two molecules
        mhs = IMP.atom.MolecularHierarchyDecorators()
        mhs.append(self.m1)
        mhs.append(self.m2)
        rot1 = IMP.algebra.random_vector_on_sphere(IMP.algebra.Vector3D(0.,0.,0.),1.)
        point1 = IMP.algebra.random_vector_in_box(IMP.algebra.Vector3D(0.,0.,0.),
                                                  IMP.algebra.Vector3D(5.,5.,5.))
        trans1 = IMP.algebra.Transformation3D(
          IMP.algebra.Rotation3D(rot1[0],rot1[1],rot1[2],0.),
          point1)
        rot2 = IMP.algebra.random_vector_on_sphere(IMP.algebra.Vector3D(0.,0.,0.),1.)
        point2 = IMP.algebra.random_vector_in_box(IMP.algebra.Vector3D(0.,0.,0.),
                                                  IMP.algebra.Vector3D(5.,5.,5.))
        trans2 = IMP.algebra.Transformation3D(
          IMP.algebra.Rotation3D(rot2[0],rot2[1],rot2[2],0.),
          point2)
        IMP.atom.write_pdb(mhs,"orig.pdb")
        IMP.core.RigidBodyDecorator.cast(self.m1.get_particle()).set_transformation(trans1)
        IMP.core.RigidBodyDecorator.cast(self.m2.get_particle()).set_transformation(trans2)
        self.m.evaluate(False) #to transform the children
        IMP.atom.write_pdb(mhs,"before.pdb")
        #optimize
        opt = IMP.core.MonteCarlo()
        opt.set_model(self.m)
        mover1 = IMP.core.RigidBodyMover(self.m1.get_particle(),5.,15.)
        opt.add_mover(mover1)
        mover2 = IMP.core.RigidBodyMover(self.m2.get_particle(),5.,15.)
        opt.add_mover(mover2)
        lopt= IMP.core.ConjugateGradients()
        lopt.set_model(self.m)
        opt.set_local_optimizer(lopt)
        for i in range(0,5):
            opt.optimize(20)
            e = self.m.evaluate(False)
        IMP.atom.write_pdb(mhs,"after.pdb")
        self.assertAlmostEqual(e, 0.0, places=2)

if __name__ == '__main__':
    unittest.main()
