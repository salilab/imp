import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit
import IMP.restrainer


class FFTFittingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_data(self):
        self.mdl=IMP.Model()
        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        [self.ps,self.rb,self.refiner,self.xyz]=self.create_points(self.mdl)
        [self.ps_copy,self.rb_copy,self.refiner_copy,self.xyz_copy]=self.create_points(self.mdl)

        scene1=IMP.em.particles2density(self.ps,6,1.5)
        header1=scene1.get_header()
        self.start_center=scene1.get_origin()
        self.end_center=scene1.get_top()
        #double the size of the grid
        scene2 = scene1.pad_margin(header1.get_nx(),
                                       header1.get_ny(),header1.get_nz())
        self.scene = IMP.em.SampledDensityMap(scene2.get_header())
        self.scene.set_particles(self.ps)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.load_data()

    def test_fft_based_rigid_fitting_translation(self):
        """test FFT based fitting for a protein of three points"""
        #Position the protein center at every possible place on the map
        rots=IMP.algebra.Rotation3Ds()
        rots.append(IMP.algebra.get_identity_rotation_3d())
        step=2*IMP.algebra.Vector3D(self.scene.get_spacing(),
                                 self.scene.get_spacing(),
                                 self.scene.get_spacing())
        pos = self.start_center
        while IMP.algebra.get_distance(self.end_center,pos)>3.:
            pos +=step
            t = IMP.algebra.Transformation3D(
                IMP.algebra.get_identity_rotation_3d(),
                pos-self.orig_center)
            IMP.core.transform(self.rb,t)
            self.scene.resample()
            i=0
            #fit protein
            fs = IMP.multifit.fft_based_rigid_fitting(
                self.rb_copy,self.refiner_copy,self.scene,0.1,rots,1,False)
            #check that the score makes sense
            sols=fs.get_solutions()
            print "number of solutions:",sols.get_number_of_solutions()
            for i in range(sols.get_number_of_solutions()):
                print sols.get_transformation(i), sols.get_score(i)
            IMP.core.transform(self.rb_copy,sols.get_transformation(0));

            rmsd=IMP.atom.get_rmsd(self.xyz,self.xyz_copy)
            w= IMP.display.BildWriter("test.%d.bild"%i)
            for p in self.ps_copy:
                g= IMP.display.XYZRGeometry(p)
                w.add_geometry(g)
            w=None
            print "rmsd:",rmsd
            IMP.core.transform(self.rb_copy,sols.get_transformation(0).get_inverse());
            IMP.core.transform(self.rb,t.get_inverse())
            self.assertLess(rmsd,5.)
            #check that the rmsd is low
        print "END TEST"
    def _test_fft_based_rigid_fitting_roations(self):
        """test rotations for a protein of three points"""
        #use 10 rotations the protein center at every possible place on the map
        rots=IMP.algebra.get_uniform_cover_rotations_3d(10)
        self.scene.resample()
        best_sol=[];best_score=9999
        for rot in rots:
            t=IMP.algebra.Transformation3D(rot,IMP.algebra.Vector3D(0,0,0))
            IMP.core.transform(self.rb_copy,t)
            #fit protein
            fs = IMP.multifit.fft_based_rigid_fitting(
                self.rb_copy,self.refiner_copy,self.scene,0.1,rots,1,False)
            #check that the score makes sense
            sols=fs.get_solutions()
            if sols.get_score(0)<best_score:
                best_score=sols.get_score(0)
                best_trans=sols.get_transformation(0)
        IMP.core.transform(self.rb_copy,best_trans)
        rmsd=IMP.atom.get_rmsd(self.xyz,self.xyz_copy)
        print "rmsd:",rmsd
        self.assertLess(rmsd,2.)

    def create_points(self,mdl):
        rd= IMP.core.XYZ.setup_particle(IMP.Particle(mdl),
                                        IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        hd= IMP.core.Hierarchy.setup_particle(rd.get_particle())
        ps = IMP.Particles()
        origin =  3.0
        ps.append(self.create_point_particle(mdl,
                                             9.+origin, 9.+origin,
                                             9.+origin))
        ps.append(self.create_point_particle(mdl,
                                             12.+origin, 3.+origin,
                                             3.+origin))
        ps.append(self.create_point_particle(mdl,
                                             3.+origin, 12.+origin,
                                             12.+origin))
        self.orig_center=IMP.algebra.Vector3D(0.,0.,0.)
        for i in range(len(ps)):
            p1 = ps[i]
            self.orig_center += IMP.core.XYZ(ps[i]).get_coordinates()
            p1.add_attribute(self.radius_key, 1.0)
            p1.add_attribute(self.weight_key, 1.0)
            chd=IMP.core.Hierarchy.setup_particle(p1)
            hd.add_child(chd)
        rb=IMP.core.RigidBody.setup_particle(rd.get_particle(),hd.get_children())
        refiner = IMP.core.ChildrenRefiner(IMP.core.Hierarchy.get_default_traits())
        return [ps,rb,refiner,IMP.core.XYZs(ps)]

if __name__ == '__main__':
    IMP.test.main()
