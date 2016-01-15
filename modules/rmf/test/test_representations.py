from __future__ import print_function
import unittest
import IMP.rmf
import IMP.test
import IMP.container
import RMF
from IMP.algebra import *


class Tests(IMP.test.TestCase):

    def _create(self, m):
        pdbname = self.get_input_file_name("simple.pdb")
        full = IMP.atom.read_pdb(pdbname, m)
        chain = IMP.atom.get_by_type(full, IMP.atom.CHAIN_TYPE)[0]
        rep = IMP.atom.Representation.setup_particle(full)
        for i in range(1, 3):
            simp = IMP.atom.create_simplified_along_backbone(chain, i)
            simp.set_name("A " + str(i))
            rep.add_representation(simp, IMP.atom.BALLS)
        return full

    def _create_custom(self, m):
        pdbname = self.get_input_file_name("simple.pdb")
        full = IMP.atom.read_pdb(pdbname, m)
        chain = IMP.atom.get_by_type(full, IMP.atom.CHAIN_TYPE)[0]
        rep = IMP.atom.Representation.setup_particle(full, 0)
        for i in range(1, 3):
            simp = IMP.atom.create_simplified_along_backbone(chain, i)
            simp.set_name("A " + str(i))
            rep.add_representation(simp, IMP.atom.BALLS, i + 1)
        return full

    def test_0(self):
        """Test writing mult resolutions"""
        m = IMP.Model()
        h = self._create(m)
        IMP.atom.show_molecular_hierarchy(h)
        resolutions = IMP.atom.Representation(h).get_resolutions()
        rmfname = self.get_tmp_file_name("multires.rmfz")
        fh = RMF.create_rmf_file(rmfname)
        IMP.rmf.add_hierarchy(fh, h)
        IMP.rmf.save_frame(fh, "frame")
        del fh

        fh = RMF.open_rmf_file_read_only(rmfname)
        RMF.show_hierarchy(fh.get_root_node())
        IMP.rmf.link_hierarchies(fh, [h])
        del fh
        fh = RMF.open_rmf_file_read_only(rmfname)
        h2 = IMP.rmf.create_hierarchies(fh, m)
        rd = IMP.atom.Representation(h2[0])
        back_resolutions = rd.get_resolutions()
        print(back_resolutions)
        for p in zip(resolutions, back_resolutions):
            self.assertAlmostEqual(p[0], p[1], delta=.1)

    def test_custom_resolutions(self):
        """Test writing RMF file w/ explicit resolutions is read correctly"""
        m = IMP.Model()
        h = self._create_custom(m)
        IMP.atom.show_molecular_hierarchy(h)
        resolutions = IMP.atom.Representation(h).get_resolutions()
        rmfname = self.get_tmp_file_name("multires.rmfz")
        fh = RMF.create_rmf_file(rmfname)
        IMP.rmf.add_hierarchy(fh, h)
        IMP.rmf.save_frame(fh, "frame")
        del fh

        fh = RMF.open_rmf_file_read_only(rmfname)
        RMF.show_hierarchy(fh.get_root_node())
        IMP.rmf.link_hierarchies(fh, [h])
        del fh
        fh = RMF.open_rmf_file_read_only(rmfname)
        h2 = IMP.rmf.create_hierarchies(fh, m)
        rd = IMP.atom.Representation(h2[0])
        back_resolutions = rd.get_resolutions()
        print(back_resolutions)
        for p in zip(resolutions, back_resolutions):
            self.assertAlmostEqual(p[0], p[1], delta=.1)

    def test_multi_type(self):
        """Test using same particle for two representations"""
        m = IMP.Model()
        p = IMP.Particle(m)
        center = IMP.algebra.Vector3D(0,0,0)
        rad = 1.0
        IMP.core.XYZR.setup_particle(p,IMP.algebra.Sphere3D(center,rad))
        IMP.atom.Mass.setup_particle(p,1.0)
        trans = IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),center)
        shape = IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(trans),[rad]*3)
        g = IMP.core.Gaussian.setup_particle(p,shape)

        root = IMP.atom.Hierarchy(IMP.Particle(m))
        rep = IMP.atom.Representation.setup_particle(root, 0)

        # particle is BALLS resolution 0 and DENSITIES resolution 0
        root.add_child(IMP.atom.Hierarchy(p))
        rep.add_representation(g,IMP.atom.DENSITIES,0)

        rmfname = self.get_tmp_file_name("multitype.rmfz")
        fh = RMF.create_rmf_file(rmfname)
        IMP.rmf.add_hierarchy(fh, root)
        IMP.rmf.save_frame(fh, "frame")
        del fh

        # check upon reading you get the same particle as both BALLS and DENSITIES
        fh = RMF.open_rmf_file_read_only(rmfname)
        h2 = IMP.rmf.create_hierarchies(fh, m)
        selA = IMP.atom.Selection(h2)
        selD = IMP.atom.Selection(h2,representation_type=IMP.atom.DENSITIES)
        self.assertEqual(selA.get_selected_particles()[0],
                         selD.get_selected_particles()[0])


if __name__ == '__main__':
    IMP.test.main()
