#test that impEM is able to sample IMP particles on a grid.

import IMP.em
import IMP.test
import os
import sys

class Tests(IMP.test.TestCase):
    """test adding two maps"""

    def setUp(self):
        """ create particles """
        IMP.test.TestCase.setUp(self)
        self.mdl=IMP.Model()
        self.particles_p = []
        self.rad_key=IMP.core.XYZR.get_radius_key()
        self.mass_key=IMP.FloatKey("mass")
        for val in [[9.,5.,5.,1.,1.],[12.,9.,4.,1.,1.],[4.,5.,5.,1.,1.]]:
            p=IMP.Particle(self.mdl)
            IMP.core.XYZR.setup_particle(p,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(val[0],val[1],val[2]),val[3]))
            p.add_attribute(self.mass_key,5.)
            self.particles_p.append(p)
        resolution = 2.0
        voxel_size = 1.0
        self.scene1 = IMP.em.SampledDensityMap(self.particles_p,
                                               resolution,voxel_size)

        self.scene2 = IMP.em.SampledDensityMap(self.particles_p,
                                               resolution,voxel_size)

        self.scene3 = IMP.em.SampledDensityMap(self.particles_p,
                                               resolution,voxel_size*2)


    def test_addition(self):
        """ test that adding a map to each other works """
        self.scene1.add(self.scene2)
        for i in [4931,5391,5396,5477,5968]:
            x = self.scene1.get_location_in_dim_by_voxel(i,0)
            y = self.scene1.get_location_in_dim_by_voxel(i,1)
            z = self.scene1.get_location_in_dim_by_voxel(i,2)
            self.assertEqual(self.scene1.get_value(x,y,z),
                             2 * self.scene2.get_value(x,y,z),
                             "unexpected value after addition")

    def test_boundary_conditions(self):
        """Check the last index is included"""
        sel=IMP.atom.CAlphaPDBSelector()
        mh1=IMP.atom.read_pdb(self.get_input_file_name("1tyq_A.pdb"),
                                          self.mdl,sel)
        mh2=IMP.atom.read_pdb(self.get_input_file_name("1tyq_D.pdb"),
                                          self.mdl,sel)
        IMP.atom.add_radii(mh1)
        IMP.atom.add_radii(mh2)
        grid1=IMP.em.particles2density(IMP.core.get_leaves(mh1),3,1)
        grid2=IMP.em.particles2density(IMP.core.get_leaves(mh2),3,1)
        b1=IMP.em.get_bounding_box(grid1)
        b2=IMP.em.get_bounding_box(grid2)
        b1+=b2
        merged_grid=IMP.em.create_density_map(b1,1)
        merged_grid.add(grid1)
        merged_grid.add(grid2)
        for i in range(merged_grid.get_number_of_voxels()):
            loc=merged_grid.get_location_by_voxel(i)
            expected_val=0.
            if grid1.is_part_of_volume(loc):
                expected_val+=grid1.get_value(loc)
            if grid2.is_part_of_volume(loc):
                expected_val+=grid2.get_value(loc)
            self.assertAlmostEqual(expected_val,merged_grid.get_value(i),
                                   delta=0.001)
if __name__ == '__main__':
    IMP.test.main()
