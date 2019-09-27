import IMP
import IMP.test
import IMP.pmi.macros
import IMP.pmi.topology
import IMP.pmi.tools

class Tests(IMP.test.TestCase):

    def make_representation(self):
        pdbfile = self.get_input_file_name("nonbond.pdb")

        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        state = s.create_state()

        c = state.create_molecule("A", sequence='KF')
        struc = c.add_structure(pdbfile, chain_id="A", offset=-10)
        c.add_representation(struc, resolutions=[1, 10])
        root_hier = s.build()
        return m, root_hier

    def test_pdb_provenance(self):
        """Make sure that provenance info is added for each input PDB file"""
        def _check_provenance(prov):
            self.assertEqual(len(prov), 1)
            self.assertEqual(prov[0].get_filename(),
                             self.get_input_file_name("nonbond.pdb"))
            self.assertEqual(prov[0].get_chain_id(), 'A')
            self.assertEqual(prov[0].get_residue_offset(), -10)
        m, root_hier = self.make_representation()

        # Check both resolution=1 and resolution=10
        sel = IMP.atom.Selection(root_hier, residue_index=1, resolution=1)
        p = IMP.atom.Hierarchy(sel.get_selected_particles()[0]).get_parent()
        prov = list(IMP.core.get_all_provenance(p))
        _check_provenance(prov)

        sel = IMP.atom.Selection(root_hier, residue_index=1, resolution=10)
        p = IMP.atom.Hierarchy(sel.get_selected_particles()[0]).get_parent()
        prov = list(IMP.core.get_all_provenance(p))
        _check_provenance(prov)


if __name__ == '__main__':
    IMP.test.main()
