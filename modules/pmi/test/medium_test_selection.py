import IMP
import IMP.test
import IMP.pmi.tools as tools

class Tests(IMP.test.TestCase):

    def init_representation_complex(self, m):
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name(
                                                '1WCM.fasta.txt'))

        pdbfile = self.get_input_file_name("1WCM.pdb")

        components = ["Rpb3","Rpb4","Rpb5","Rpb6"]
        chains = "CDEF"
        colors = [0.,0.1,0.5,1.0]
        beadsize = 20

        m = IMP.Model()
        simo = IMP.pmi.topology.System(m)
        st1 = simo.create_state()

        for n in range(len(components)):
            mol = st1.create_molecule(components[n],
                                      sequence=seqs["1WCM:" + chains[n]])
            atomic = mol.add_structure(pdbfile, chain_id=chains[n])
            mol.add_representation(mol.get_atomic_residues(),
                                   resolutions=[1, 10, 100], color=colors[n])
            mol.add_representation(mol.get_non_atomic_residues(),
                                   resolutions=[10], color=colors[n])
        return simo.build()

    def test_get_terminal_residue_two_methods(self):
        m = IMP.Model()
        root_hier = self.init_representation_complex(m)

        def test(molecule, terminus, residue_index=None, fragment_range=None):
            s = IMP.atom.Selection(root_hier, molecule=molecule, resolution=1,
                                   terminus=terminus)
            r = s.get_selected_particles(with_representation=False)
            self.assertEqual(len(r), 1)
            if residue_index is not None:
                self.assertTrue(IMP.atom.Residue.get_is_setup(r[0]))
                r = IMP.atom.Residue(r[0])
                self.assertEqual(r.get_index(), residue_index)
            if fragment_range is not None:
                self.assertTrue(IMP.atom.Fragment.get_is_setup(r[0]))
                f = IMP.atom.Fragment(r[0])
                inds = f.get_residue_indexes()
                self.assertEqual((inds[0], inds[-1]), fragment_range)

        test("Rpb4", IMP.atom.Selection.C, residue_index=221)
        test("Rpb4", IMP.atom.Selection.N, fragment_range=(1,3))
        test("Rpb5", IMP.atom.Selection.C, residue_index=215)
        test("Rpb5", IMP.atom.Selection.N, residue_index=1)
        test("Rpb6", IMP.atom.Selection.C, residue_index=155)
        test("Rpb6", IMP.atom.Selection.N, fragment_range=(1,10))
        test("Rpb3", IMP.atom.Selection.C, fragment_range=(309,318))
        test("Rpb3", IMP.atom.Selection.N, fragment_range=(1,2))

    def test_selection(self):
        """Test selection"""
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name(
                                                '1WCM.fasta.txt'))

        pdbfile = self.get_input_file_name("1WCM.pdb")

        components = ["Rpb3", "Rpb4"]
        chains = "CD"
        colors = [0., 1.0]
        beadsize = 20
        fastids = ['1WCM:C', '1WCM:D']

        m = IMP.Model()
        simo = IMP.pmi.topology.System(m)
        st1 = simo.create_state()

        for n in range(len(components)):
            mol = st1.create_molecule(components[n], sequence=seqs[fastids[n]])
            atomic = mol.add_structure(pdbfile, chain_id=chains[n])
            mol.add_representation(mol.get_atomic_residues(),
                                   resolutions=[1, 10, 100], color=colors[n])
            mol.add_representation(mol.get_non_atomic_residues(),
                                   resolutions=[10], color=colors[n])
            if components[n] == 'Rpb3':
                mol.create_clone(chains[n])
        root_hier = simo.build()

        def test(k, selection):
            self.assertEqual(k, len(selection.get_selected_particle_indexes()))

        test(727, IMP.atom.Selection(root_hier))
        test(727, IMP.atom.Selection(root_hier, resolution=1))
        test(727, IMP.atom.Selection(root_hier, resolution=2))
        test(91, IMP.atom.Selection(root_hier, resolution=7))
        test(91, IMP.atom.Selection(root_hier, resolution=10))
        test(27, IMP.atom.Selection(root_hier, resolution=100))
        test(3, IMP.atom.Selection(root_hier, resolution=1, residue_index=10))
        test(1, IMP.atom.Selection(root_hier, resolution=1, residue_index=10,
                                   molecule="Rpb3", copy_index=0))
        test(2, IMP.atom.Selection(root_hier, resolution=1, residue_index=10,
                                   molecule="Rpb3"))
        test(1, IMP.atom.Selection(root_hier, resolution=1, residue_index=10,
                                   molecule="Rpb4"))
        test(11, IMP.atom.Selection(root_hier, resolution=1,
                                    residue_indexes=range(10,21),
                                    molecule="Rpb3", copy_index=0))
        test(22, IMP.atom.Selection(root_hier, resolution=1,
                                    residue_indexes=range(10,21),
                                    molecule="Rpb3"))
        test(2, IMP.atom.Selection(root_hier, resolution=10,
                                   residue_indexes=range(10,21),
                                   molecule="Rpb3", copy_index=0))
        test(4, IMP.atom.Selection(root_hier, resolution=10,
                                   residue_indexes=range(10,21),
                                   molecule="Rpb3"))
        test(1, IMP.atom.Selection(root_hier, resolution=100,
                                   residue_indexes=range(10,21),
                                   molecule="Rpb3", copy_index=0))
        test(2, IMP.atom.Selection(root_hier, resolution=100,
                                   residue_indexes=range(10,21),
                                   molecule="Rpb3"))


if __name__ == '__main__':
    IMP.test.main()
