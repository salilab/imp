from __future__ import print_function
import IMP
import IMP.test
import IMP.em
import os, sys
import numpy as np
import random, math

def get_particles(m, input_pdb):
    ### Read input fragment
    sel0 = IMP.atom.BackbonePDBSelector()
    sel1 = IMP.atom.CBetaPDBSelector()
    mh = IMP.atom.read_pdb(input_pdb, m, IMP.atom.OrPDBSelector(sel0, sel1))
    IMP.atom.add_radii(mh)
    return mh, IMP.core.get_leaves(mh)

class Tests(IMP.test.TestCase):
    """Derivative score stability Test"""

    def setUp(self):

        IMP.test.TestCase.setUp(self)


        IMP.set_log_level(IMP.SILENT)
        IMP.set_check_level(IMP.NONE)


        self.m = IMP.Model()
        self.mh, self.ps = get_particles(self.m, "input/input_atom.pdb")
        IMP.atom.create_rigid_body(self.mh)
        self.prot_rb = IMP.core.RigidMember(IMP.core.get_leaves(self.mh)[0]).get_rigid_body()

        ##Read and setup EM
        self.res = 5.
        voxel_size = 1.5

        dmap = IMP.em.read_map("input/sampled_all.mrc", IMP.em.MRCReaderWriter())
        dmap.get_header_writable().set_resolution(self.res)
        dmap.update_voxel_size(voxel_size)
        self.m.update()

        #Set up the restraints
        ## Normalize EM map
        mass = IMP.atom.get_mass_from_number_of_residues(157)
        t = IMP.em.get_threshold_for_approximate_mass(dmap, mass)
        self.fmap = IMP.em.get_threshold_map(dmap, t)
        IMP.em.bayesem3d_get_normalized_intensities(self.fmap, self.ps, self.res);

        self.ps_map = IMP.em.bayesem3d_get_density_from_particle(self.fmap, self.ps, self.res)
        rand_angle = random.uniform(-0.01 / 180 * math.pi, 0.01 / 180 * math.pi)
        axis = IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
        q = IMP.algebra.get_rotation_about_axis(axis, rand_angle)

        rot = IMP.algebra.Rotation3D(q)

        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-14.0,-14.0,-14.0),
                                       IMP.algebra.Vector3D(-13.0,-13.0,-13.0))
        tr =  IMP.algebra.get_random_vector_in(bb)

        trans = IMP.algebra.Transformation3D(rot, tr)
        IMP.core.transform(self.prot_rb, trans)
        self.m.update()
        self.ps_map2 = IMP.em.bayesem3d_get_density_from_particle(self.fmap, self.ps, self.res)

    def test_ccc_mrc_to_mrc(self):
        cc1 = IMP.em.bayesem3d_get_cross_correlation_coefficient(self.fmap, self.fmap)
        self.assertAlmostEqual(cc1, 1.0, delta=1e-8)

    def test_ccc_ps_to_ps(self):
        cc2 = IMP.em.bayesem3d_get_cross_correlation_coefficient(self.ps_map, self.ps_map)
        self.assertAlmostEqual(cc2, 1.0, delta=1e-8)

    def test_ccc_non_overlapping(self):
        cc3 = IMP.em.bayesem3d_get_cross_correlation_coefficient(self.ps_map, self.ps_map2)
        self.assertAlmostEqual(cc3, 0.0, delta=1e-8)



if __name__ == '__main__':
    IMP.test.main()
