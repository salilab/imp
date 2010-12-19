import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit

class UnwrappingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_data(self):
        self.mdl=IMP.Model()
        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        [self.ps,self.rb,self.refiner,self.xyz]=self.create_points(self.mdl)
        [self.ps_copy,self.rb_copy,self.refiner_copy,self.xyz_copy]=self.create_points(self.mdl)

        self.scene=IMP.em.particles2density(self.ps,6,1.5)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.load_data()

    def test_wrapping(self):
        """test wrapping and unwrapping"""
        self.scene.resample()
        fft=IMP.multifit.FFTFitting(self.scene,self.rb_copy,self.refiner_copy)
        fft.prepare(0.1)
        h=self.scene.get_header()
        for x in range(h.get_nx()):
            for y in range(h.get_ny()):
                for z in range(h.get_nz()):
                    wx=fft.get_wrapped_index(x,y,z,0)
                    wy=fft.get_wrapped_index(x,y,z,1)
                    wz=fft.get_wrapped_index(x,y,z,2)
                    ux=fft.get_unwrapped_index(wx,wy,wz,0)
                    uy=fft.get_unwrapped_index(wx,wy,wz,1)
                    uz=fft.get_unwrapped_index(wx,wy,wz,2)
                    self.assertEqual(x,ux)
                    self.assertEqual(y,uy)
                    self.assertEqual(z,uz)

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
