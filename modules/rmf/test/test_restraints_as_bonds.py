import unittest
import IMP.rmf
import IMP.test
import RMF
import IMP.container


class Tests(IMP.test.TestCase):

    def test_perturbed(self):
        """Test writing a simple hierarchy"""
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(
            self.get_input_file_name("small_protein.pdb"),
            m)
        hs = IMP.atom.create_simplified_along_backbone(
            IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)[0], 1)
        IMP.base.set_log_level(IMP.base.SILENT)
        fname = self.get_tmp_file_name("restraint_bonds.rmf3")
        fh = RMF.create_rmf_file(fname)
        IMP.rmf.add_hierarchy(fh, hs)
        leaves = IMP.atom.get_leaves(hs)
        cpc = IMP.container.ConsecutivePairContainer(leaves)
        br = IMP.container.PairsRestraint(
            IMP.core.HarmonicSphereDistancePairScore(0, 1), cpc)
        IMP.rmf.add_restraints_as_bonds(fh, [br])
        IMP.rmf.save_frame(fh, "frame")

        bf = RMF.BondFactory(fh)
        count = 0
        for nid in fh.get_node_ids():
            n = fh.get_node(nid)
            if bf.get_is(n):
                count += 1
        self.assertEqual(count, len(leaves) - 1)


if __name__ == '__main__':
    IMP.test.main()
