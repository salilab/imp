import IMP
import IMP.test
import IMP.atom

class DihedralTests(IMP.test.TestCase):
    """Test dihedral functions"""

    def test_dihedral(self):
        """Test dihedral functions"""
        m = IMP.Model()
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = IMP.atom.CHARMMTopology(ff)
        topology.add_sequence('PACCCAGCY')
        topology.apply_default_patches()
        h = topology.create_hierarchy(m)
        topology.setup_hierarchy(h)

        res = IMP.atom.get_by_type(h, IMP.atom.RESIDUE_TYPE)
        first_r = IMP.atom.Residue(res[0])
        last_r = IMP.atom.Residue(res[-1])
        # phi/psi are not defined for first/last residue in chain
        self.assertEqual(len(IMP.atom.get_phi_dihedral(first_r)), 0)
        self.assertEqual(len(IMP.atom.get_phi_dihedral(last_r)), 4)
        self.assertEqual(len(IMP.atom.get_psi_dihedral(first_r)), 4)
        self.assertEqual(len(IMP.atom.get_psi_dihedral(last_r)), 0)
        # both are defined for all other residues
        for h_r in res[1:-2]:
            r = IMP.atom.Residue(h_r)
            self.assertEqual(len(IMP.atom.get_phi_dihedral(r)), 4)
            self.assertEqual(len(IMP.atom.get_psi_dihedral(r)), 4)
        # phi/psi cease to be defined if at least one atom is missing
        r = IMP.atom.Residue(res[2])
        a = IMP.atom.get_atom(r, IMP.atom.AT_CA)
        IMP.atom.destroy(a)
        self.assertEqual(len(IMP.atom.get_phi_dihedral(r)), 0)
        self.assertEqual(len(IMP.atom.get_psi_dihedral(r)), 0)



if __name__ == '__main__':
    IMP.test.main()
