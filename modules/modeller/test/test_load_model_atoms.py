import unittest
import modeller
import modeller.scripts
import IMP
import IMP.test
import IMP.modeller
import IMP.core

class ModelLoaderTests(IMP.test.TestCase):

    def get_environ(self):
        """Get a Modeller environ object"""
        if not hasattr(self, '_modeller_environ'):
            # Speed tests up a little by only creating this object once
            env = modeller.environ()
            env.libs.topology.read('${LIB}/top_heav.lib')
            env.libs.parameters.read('${LIB}/par.lib')
            ModelLoaderTests._modeller_environ = env
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
            mp.validate()
            hc= IMP.core.HierarchyCounter()
            IMP.core.depth_first_traversal(mp, hc)
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
            self.assertEqual(1377, hc.get_count(),
                             "Wrong number of particles created")
            rd= IMP.atom.Residue(IMP.atom.get_residue(mp, 29).get_particle())
            at= IMP.atom.get_atom(rd, IMP.atom.AtomType("C"))
            self.assertEqual(IMP.atom.get_residue(at).get_index(),
                             rd.get_index())
            self.assertEqual(rd.get_index(), 29)

    def test_bonds(self):
        """Check that the file loader produces bonds"""
        modmodel = modeller.scripts.complete_pdb(self.get_environ(),
                             self.get_input_file_name('single_protein.pdb'))
        m = IMP.Model()
        mp = IMP.modeller.ModelLoader(modmodel).load_atoms(m)
        all_atoms= IMP.atom.get_by_type(mp,
                             IMP.atom.ATOM_TYPE)
        self.assertEqual(1221, len(all_atoms),
                         "Wrong number of atoms found in protein")

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
        mp.validate()
        hc= IMP.core.HierarchyCounter()
        IMP.core.depth_first_traversal(mp, hc)
        mpp= mp.get_parent()
        self.assertEqual(mpp, IMP.atom.Hierarchy(),
                         "Should not have a parent")
        mpc= mp.get_child(0)
        self.assertEqual(mpc.get_parent(), mp,
                         "Should not have a parent")
        self.assertEqual(3160, hc.get_count(),
                         "Wrong number of particles created")

if __name__ == '__main__':
    unittest.main()
