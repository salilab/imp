import modeller
import modeller.scripts
import IMP
import IMP.test
import IMP.modeller
import IMP.core

class Tests(IMP.test.TestCase):

    def get_environ(self):
        """Get a Modeller environ object"""
        if not hasattr(self, '_modeller_environ'):
            # Speed tests up a little by only creating this object once
            env = modeller.environ()
            env.libs.topology.read('${LIB}/top_heav.lib')
            env.libs.parameters.read('${LIB}/par.lib')
            Tests._modeller_environ = env
        return self._modeller_environ

    def test_hierarchy(self):
        """Check reading a Modeller model with one protein"""
        i_num_res_type= IMP.atom.ResidueType.get_number_unique()
        i_num_atom_type= IMP.atom.AtomType.get_number_unique()
        m = IMP.Model()
        # Test both ModelLoader class and deprecated read_pdb function
        modmodel = modeller.scripts.complete_pdb(self.get_environ(),
                             self.get_input_file_name('single_protein.pdb'))
        loader = IMP.modeller.ModelLoader(modmodel)

        for mp in (loader.load_atoms(m),
                   IMP.modeller.read_pdb(
                       self.get_input_file_name('single_protein.pdb'), m)):
            desc = IMP.core.get_all_descendants(mp)
            f_num_res_type= IMP.atom.ResidueType.get_number_unique()
            f_num_atom_type= IMP.atom.AtomType.get_number_unique()
            mpp= mp.get_parent()
            self.assertEqual(mpp, IMP.atom.Hierarchy(),
                             "Should not have a parent")
            mpc= mp.get_child(0)
            self.assertEqual(mpc.get_parent(), mp,
                             "Should not have a parent")
            self.assertEqual(i_num_res_type, f_num_res_type,
                             "too many residue types")
            self.assertEqual(i_num_atom_type, f_num_atom_type,
                             "too many atom types")
            self.assertEqual(1377, len(desc),
                             "Wrong number of particles created")
            rd= IMP.atom.Residue(IMP.atom.get_residue(mp, 29).get_particle())
            at= IMP.atom.get_atom(rd, IMP.atom.AtomType("C"))
            self.assertEqual(IMP.atom.get_residue(at).get_index(),
                             rd.get_index())
            self.assertEqual(rd.get_index(), 29)

    def test_bonds(self):
        """Check that Modeller bonds and angles are loaded"""
        modmodel = modeller.scripts.complete_pdb(self.get_environ(),
                             self.get_input_file_name('single_protein.pdb'))
        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        mp = loader.load_atoms(m)
        all_atoms= IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(1221, len(all_atoms),
                         "Wrong number of atoms found in protein")
        bonds = list(loader.load_bonds())
        self.assertEqual(len(bonds), 1248)
        angles = list(loader.load_angles())
        self.assertEqual(len(angles), 1677)
        dihedrals = list(loader.load_dihedrals())
        self.assertEqual(len(dihedrals), 1973)
        impropers = list(loader.load_impropers())
        self.assertEqual(len(impropers), 520)

    def test_dna(self):
        """Check reading a dna with one chain"""
        def na_patches(mdl):
            """Nucleic acid terminal patches."""
            mdl.patch('5TER', mdl.residues[0])
            mdl.patch('3TER', mdl.residues[-1])
        m = IMP.Model()
        modmodel = modeller.scripts.complete_pdb(self.get_environ(),
                             self.get_input_file_name('single_dna.pdb'),
                             special_patches=na_patches)
        mp = IMP.modeller.ModelLoader(modmodel).load_atoms(m)
        desc = IMP.core.get_all_descendants(mp)
        mpp= mp.get_parent()
        self.assertEqual(mpp, IMP.atom.Hierarchy(),
                         "Should not have a parent")
        mpc= mp.get_child(0)
        self.assertEqual(mpc.get_parent(), mp,
                         "Should not have a parent")
        self.assertEqual(3160, len(desc),
                         "Wrong number of particles created")

if __name__ == '__main__':
    IMP.test.main()
