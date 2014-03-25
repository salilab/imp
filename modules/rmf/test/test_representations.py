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

    def test_0(self):
        """Test writing mult resolutions"""
        m = IMP.kernel.Model()
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
        print back_resolutions
        for p in zip(resolutions, back_resolutions):
            self.assertAlmostEqual(p[0], p[1], delta=.1)

if __name__ == '__main__':
    IMP.test.main()
