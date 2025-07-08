import utils
import os
import sys
import unittest
import subprocess

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.reader
import ihm.util.make_mmcif  # Script should also be importable


MAKE_MMCIF = os.path.join(TOPDIR, 'ihm', 'util', 'make_mmcif.py')


class Tests(unittest.TestCase):
    def test_simple(self):
        """Simple test of make_mmcif utility script"""
        incif = utils.get_input_file_name(TOPDIR, 'struct_only.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(s.title,
                         'Architecture of Pol II(G) and molecular mechanism '
                         'of transcription regulation by Gdown1')
        os.unlink('output.cif')

    def test_non_default_output(self):
        """Simple test of make_mmcif with non-default output name"""
        incif = utils.get_input_file_name(TOPDIR, 'struct_only.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif,
                               'non-default-output.cif'])
        with open('non-default-output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(s.title,
                         'Architecture of Pol II(G) and molecular mechanism '
                         'of transcription regulation by Gdown1')
        os.unlink('non-default-output.cif')

    def test_no_title(self):
        """Check that make_mmcif adds missing title"""
        incif = utils.get_input_file_name(TOPDIR, 'no_title.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(s.title, 'Auto-generated system')
        os.unlink('output.cif')

    def test_bad_usage(self):
        """Bad usage of make_mmcif utility script"""
        ret = subprocess.call([sys.executable, MAKE_MMCIF])
        self.assertEqual(ret, 2)

    def test_same_file(self):
        """Check that make_mmcif fails if input and output are the same"""
        incif = utils.get_input_file_name(TOPDIR, 'struct_only.cif')
        ret = subprocess.call([sys.executable, MAKE_MMCIF, incif, incif])
        self.assertEqual(ret, 1)

    def test_mini(self):
        """Check that make_mmcif works given only basic atom info"""
        incif = utils.get_input_file_name(TOPDIR, 'mini.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(len(s.state_groups), 1)
        self.assertEqual(len(s.state_groups[0]), 1)
        self.assertEqual(len(s.state_groups[0][0]), 1)
        self.assertEqual(len(s.state_groups[0][0][0]), 1)
        m = s.state_groups[0][0][0][0]
        self.assertEqual(m.protocol.name, 'modeling')
        self.assertEqual(m.assembly.name, 'Complete assembly')
        chain_a, chain_b, = m.representation
        for chain in chain_a, chain_b:
            self.assertIsInstance(chain, ihm.representation.AtomicSegment)
            self.assertFalse(chain.rigid)
        self.assertEqual(s.title, 'Auto-generated system')
        os.unlink('output.cif')

    def test_pass_through(self):
        """Check that make_mmcif passes through already-compliant files"""
        incif = utils.get_input_file_name(TOPDIR, 'docking.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(len(s.state_groups), 1)
        self.assertEqual(len(s.state_groups[0]), 1)
        self.assertEqual(len(s.state_groups[0][0]), 1)
        self.assertEqual(len(s.state_groups[0][0][0]), 1)
        m = s.state_groups[0][0][0][0]
        self.assertEqual(m.protocol.name, 'Modeling')
        self.assertEqual(m.assembly.name, 'Our complete assembly')
        self.assertEqual(m.assembly.description, 'All our known components')
        chain_a, chain_b, = m.representation
        self.assertIsInstance(chain_a, ihm.representation.AtomicSegment)
        self.assertTrue(chain_a.rigid)
        self.assertIsInstance(chain_b, ihm.representation.FeatureSegment)
        self.assertFalse(chain_b.rigid)
        self.assertEqual(s.title, 'Output from simple-docking example')
        os.unlink('output.cif')

    def test_complete_assembly_order(self):
        """Check that assembly order does not matter"""
        incif = utils.get_input_file_name(TOPDIR, 'docking_order.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        m = s.state_groups[0][0][0][0]
        self.assertEqual(m.assembly.name, 'Our complete assembly')
        self.assertEqual(m.assembly.description, 'All our known components')

    def test_add_polymers(self):
        """Check that make_mmcif combines polymer information"""
        # mini.cif contains two chains A, B
        incif = utils.get_input_file_name(TOPDIR, 'mini.cif')
        # mini_add.cif also contains A, B; A is the same sequence as mini.cif
        # but B is different (so should be renamed C when we add)
        addcif = utils.get_input_file_name(TOPDIR, 'mini_add.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif,
                               '--add', addcif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(len(s.entities), 3)
        self.assertEqual(len(s.asym_units), 3)
        self.assertEqual(len(s.state_groups), 2)
        # Model from mini.cif
        self.assertEqual(len(s.state_groups[0]), 1)
        self.assertEqual(len(s.state_groups[0][0]), 1)
        self.assertEqual(len(s.state_groups[0][0][0]), 1)
        m = s.state_groups[0][0][0][0]
        self.assertEqual(m.protocol.name, 'modeling')
        self.assertEqual(m.assembly.name, 'Modeled assembly')
        chain_a, chain_b, = m.representation
        self.assertIs(chain_a.asym_unit.asym, s.asym_units[0])
        self.assertIs(chain_b.asym_unit.asym, s.asym_units[1])
        for chain in chain_a, chain_b:
            self.assertIsInstance(chain, ihm.representation.AtomicSegment)
            self.assertFalse(chain.rigid)
        # Model from mini_add.cif
        self.assertEqual(len(s.state_groups[1]), 1)
        self.assertEqual(len(s.state_groups[1][0]), 1)
        self.assertEqual(len(s.state_groups[1][0][0]), 1)
        m = s.state_groups[1][0][0][0]
        self.assertEqual(m.protocol.name, 'modeling')
        self.assertEqual(m.assembly.name, 'Modeled assembly')
        chain_a, chain_c, = m.representation
        self.assertIs(chain_a.asym_unit.asym, s.asym_units[0])
        self.assertIs(chain_c.asym_unit.asym, s.asym_units[2])
        for chain in chain_a, chain_c:
            self.assertIsInstance(chain, ihm.representation.AtomicSegment)
            self.assertFalse(chain.rigid)
        self.assertEqual(s.title, 'Auto-generated system')
        os.unlink('output.cif')

    def test_add_non_polymers(self):
        """Check that make_mmcif combines non-polymer information"""
        # mini_nonpoly.cif contains two hemes A, B
        incif = utils.get_input_file_name(TOPDIR, 'mini_nonpoly.cif')
        # mini_nonpoly_add.cif also contains A, B; A has the same author
        # provided residue number as mini_nonpoly.cif but B is different
        # (so should be renamed C when we add)
        addcif = utils.get_input_file_name(TOPDIR, 'mini_nonpoly_add.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif,
                               '--add', addcif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(len(s.entities), 1)
        self.assertEqual(len(s.asym_units), 3)
        self.assertEqual(len(s.state_groups), 2)
        # Model from mini_nonpoly.cif
        self.assertEqual(len(s.state_groups[0]), 1)
        self.assertEqual(len(s.state_groups[0][0]), 1)
        self.assertEqual(len(s.state_groups[0][0][0]), 1)
        m = s.state_groups[0][0][0][0]
        self.assertEqual(m.protocol.name, 'modeling')
        self.assertEqual(m.assembly.name, 'Modeled assembly')
        chain_a, chain_b, = m.representation
        self.assertIs(chain_a.asym_unit, s.asym_units[0])
        self.assertIs(chain_b.asym_unit, s.asym_units[1])
        for chain in chain_a, chain_b:
            self.assertIsInstance(chain, ihm.representation.AtomicSegment)
            self.assertFalse(chain.rigid)
        # Model from mini_nonpoly_add.cif
        self.assertEqual(len(s.state_groups[1]), 1)
        self.assertEqual(len(s.state_groups[1][0]), 1)
        self.assertEqual(len(s.state_groups[1][0][0]), 1)
        m = s.state_groups[1][0][0][0]
        self.assertEqual(m.protocol.name, 'modeling')
        self.assertEqual(m.assembly.name, 'Modeled assembly')
        chain_a, chain_c, = m.representation
        self.assertIs(chain_a.asym_unit, s.asym_units[0])
        self.assertIs(chain_c.asym_unit, s.asym_units[2])
        for chain in chain_a, chain_c:
            self.assertIsInstance(chain, ihm.representation.AtomicSegment)
            self.assertFalse(chain.rigid)
        self.assertEqual(s.title, 'Auto-generated system')
        os.unlink('output.cif')

    def test_add_multi_data(self):
        """make_mmcif should fail to add system with multiple data blocks"""
        incif = utils.get_input_file_name(TOPDIR, 'mini.cif')
        addcif = utils.get_input_file_name(TOPDIR, 'mini_add.cif')
        with open(addcif) as fh:
            addcif_contents = fh.read()
        addcif_multi = 'addcif_multi.cif'
        with open(addcif_multi, 'w') as fh:
            fh.write(addcif_contents)
            fh.write(addcif_contents.replace('data_model', 'data_model2'))
        ret = subprocess.call([sys.executable, MAKE_MMCIF, incif,
                               '--add', addcif_multi])
        self.assertEqual(ret, 1)
        os.unlink(addcif_multi)

    def test_not_modeled(self):
        """Check addition of not-modeled residue information"""
        incif = utils.get_input_file_name(TOPDIR, 'not_modeled.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        # Residues 5 and 6 in chain A, and 2 in chain B, are missing from
        # atom_site. But the file already has an _ihm_residues_not_modeled
        # table listing residue 5:A, so we expect to see just 6:A and 2:B
        # added
        m = s.state_groups[0][0][0][0]
        r1, r2, r3 = m.not_modeled_residue_ranges
        self.assertEqual((r1.seq_id_begin, r1.seq_id_end), (5, 5))
        self.assertEqual(r1.asym_unit._id, 'A')
        self.assertEqual((r2.seq_id_begin, r2.seq_id_end), (6, 6))
        self.assertEqual(r2.asym_unit._id, 'A')
        self.assertEqual((r3.seq_id_begin, r3.seq_id_end), (2, 2))
        self.assertEqual(r3.asym_unit._id, 'B')
        os.unlink('output.cif')

    def test_histidine(self):
        """Test handling multiple histidine protonation states"""
        incif = utils.get_input_file_name(TOPDIR, 'histidine.cif')
        with open(incif) as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual([c.id for c in s.entities[0].sequence],
                         ['ALA', 'HIS', 'HIE', 'HIP', 'ALA'])

        subprocess.check_call([sys.executable, MAKE_MMCIF,
                               '--histidine', incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        # All histidines should now be HIS
        self.assertEqual([c.id for c in s.entities[0].sequence],
                         ['ALA', 'HIS', 'HIS', 'HIS', 'ALA'])
        # All modified histidine atoms should now be ATOM not HETATM;
        # the last atom in the last ALA residue (which was marked HETATM in
        # the input) should still be HETATM.
        for state_group in s.state_groups:
            for state in state_group:
                for model_group in state:
                    for model in model_group:
                        self.assertEqual([x.het for x in model._atoms],
                                         [False, False, False, False, True])
        os.unlink('output.cif')

    def test_check_non_canon_atom_standard(self):
        """Test check for non-canonical atom names, standard restypes"""
        minicif = utils.get_input_file_name(TOPDIR, 'mini.cif')
        incif = utils.get_input_file_name(TOPDIR, 'non_canon_atom.cif')

        # Should work fine without check
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        os.unlink('output.cif')

        # Should also work fine for file with canonical atom names
        subprocess.check_call([sys.executable, MAKE_MMCIF,
                               "--check_atom_names=standard", minicif])
        os.unlink('output.cif')

        # Should fail with check enabled
        r = subprocess.Popen([sys.executable, MAKE_MMCIF,
                             "--check_atom_names=standard", incif],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             universal_newlines=True)
        out, err = r.communicate()
        self.assertEqual(r.returncode, 1)
        # Non-canonical atoms in standard residues should be reported
        # Non-standard residues (ZN, ...) are not checked
        self.assertIn("Non-canonical atom names found in the following "
                      "residues: GLN: ['bad3']; VAL: ['bad1', 'bad2']",
                      err)
        os.unlink('output.cif')

    def test_check_non_canon_atom_all(self):
        """Test check for non-canonical atom names, all restypes"""
        incif = utils.get_input_file_name(TOPDIR, 'non_canon_atom.cif')

        # Use mock urllib so we don't hit the network during this test
        env = os.environ.copy()
        mockdir = os.path.join(TOPDIR, 'test', 'mock', 'non_canon_atom')
        env['PYTHONPATH'] = mockdir + os.pathsep + env['PYTHONPATH']

        r = subprocess.Popen([sys.executable, MAKE_MMCIF,
                             "--check_atom_names=all", incif],
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True, env=env)
        out, err = r.communicate()
        self.assertEqual(r.returncode, 1)
        # Non-canonical atoms in standard residues should be reported
        # Non-standard residue (ZN) should also be checked
        self.assertIn("Non-canonical atom names found in the following "
                      "residues: GLN: ['bad3']; VAL: ['bad1', 'bad2']; "
                      "ZN: ['bad4']", err)
        # Residues not in CCD should give a warning
        self.assertIn("Component invalid-comp-name could not be found in CCD",
                      err)
        os.unlink('output.cif')


if __name__ == '__main__':
    unittest.main()
