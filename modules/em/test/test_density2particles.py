import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_maps(self):
        mrw = IMP.em.MRCReaderWriter()
        erw = IMP.em.EMReaderWriter()
        print self.get_input_file_name("in.em")
        self.scene1= IMP.em.read_map(self.get_input_file_name("in.em"), erw)
        self.scene2= IMP.em.read_map(self.get_input_file_name("1z5s.mrc"), mrw)
        self.scene1.get_header_writable().set_resolution(3.)
        self.scene2.get_header_writable().set_resolution(10.)
        self.scene2.update_voxel_size(3.0)
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.imp_model = IMP.Model()
        self.load_density_maps()

    def test_density2particles_map2(self):
        """Test conversion of a density map into a set of particles
           This is done after updating the voxel size of the map"""
        m = IMP.Model()
        self.scene1.get_header().show()
        ps=IMP.em.density2particles(self.scene2,9.0,m)
        self.assertGreater(len(ps), 0)

    def test_particles2density(self):
        """Test conversion of particles to a density map"""
        m = IMP.Model()
        ps=[]
        res=3
        apix=1
        radius_key = IMP.core.XYZR.get_radius_key()
        weight_key = IMP.atom.Mass.get_mass_key()
        for i in range(10):
            ps.append(self.create_point_particle(m,i,i,i))
            ps[-1].add_attribute(radius_key, 5)
            ps[-1].add_attribute(weight_key, 100.0)
        dmap = IMP.em.particles2density(ps,res,apix)
        dmap.calcRMS()
        self.assertGreater(dmap.get_header().rms, 0)
        #check origin:
        r = IMP.em.FitRestraint(ps,dmap)
        m.add_restraint(r)
        score = self.imp_model.evaluate(False)
        self.assertLess(score, 0.01)
        print "score:",score,r.evaluate(None)
        #check origin
        ps_centroid=IMP.core.get_centroid(IMP.core.XYZs(ps))
        map_centroid=dmap.get_centroid()
        self.assertLess(IMP.algebra.get_distance(ps_centroid,map_centroid),
                        0.01)
if __name__ == '__main__':
    IMP.test.main()
