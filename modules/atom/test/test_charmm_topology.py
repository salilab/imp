import unittest
import IMP
import IMP.test
import IMP.atom

def _make_test_atom():
    at = IMP.atom.CHARMMAtomTopology('CA')
    at.set_charmm_type('CT1')
    at.set_charge(0.8)
    return at

class CHARMMTopologyTests(IMP.test.TestCase):
    """Test CHARMM topology classes"""

    def assertAtomsBonded(self, a1, a2, typ1, typ2, bondlen, forcecon):
        bonded1 = IMP.atom.Bonded(a1.get_particle())
        bonded2 = IMP.atom.Bonded(a2.get_particle())
        exp_typ1 = IMP.atom.CHARMMAtom(a1.get_particle()).get_charmm_type()
        exp_typ2 = IMP.atom.CHARMMAtom(a2.get_particle()).get_charmm_type()
        self.assertEqual(typ1, exp_typ1)
        self.assertEqual(typ2, exp_typ2)
        for i in range(bonded1.get_number_of_bonds()):
            other = bonded1.get_bonded(i)
            if other == bonded2:
                bond = bonded1.get_bond(i)
                self.assertInTolerance(bond.get_length(), bondlen, 1e-4)
                self.assertInTolerance(bond.get_stiffness(), forcecon, 1e-4)
                return
        self.fail("No bond defined between %s and %s" % (a1, a2))

    def test_atom(self):
        """Check creation and methods of CHARMMAtomTopology"""
        at = _make_test_atom()
        self.assertEqual(at.get_name(), 'CA')
        self.assertEqual(at.get_charmm_type(), 'CT1')
        self.assertInTolerance(at.get_charge(), 0.8, 1e-6)

    def test_add_get_atom(self):
        """Check add/get atom from CHARMM residues"""
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        self.assertEqual(res.get_type(), 'FOO')
        self.assertRaises(IMP.ValueException, res.get_atom, 'CA')
        at = _make_test_atom()
        res.add_atom(at)
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT1')
        # Modifying our copy after it was inserted should have no effect
        at.set_charmm_type('CT2')
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT1')
        # Modifying the residue copy should work, however
        at = res.get_atom('CA')
        at.set_charmm_type('CT2')
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT2')

    def test_delete_atom(self):
        """Check delete atom from CHARMM residues"""
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        at = _make_test_atom()
        res.add_atom(at)
        self.assertRaises(IMP.ValueException, res.delete_atom, 'CB')
        res.delete_atom('CA')
        self.assertRaises(IMP.ValueException, res.get_atom, 'CA')

    def test_default_patches(self):
        """Check default patches of CHARMM residues"""
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        self.assertEqual(res.get_default_first_patch(), '')
        self.assertEqual(res.get_default_last_patch(), '')
        res.set_default_first_patch('NTER')
        res.set_default_last_patch('CTER')
        self.assertEqual(res.get_default_first_patch(), 'NTER')
        self.assertEqual(res.get_default_last_patch(), 'CTER')

    def test_bond_endpoint(self):
        """Check CHARMM bond endpoint class"""
        e = IMP.atom.CHARMMBondEndpoint('CA')
        self.assertEqual(e.get_atom_name(), 'CA')

    def test_bond(self):
        """Check CHARMM bond class"""
        bond = IMP.atom.CHARMMBond2(IMP.Strings(['CA', 'CB']))
        self.assertEqual(bond.contains_atom('CA'), True)
        self.assertEqual(bond.contains_atom('N'), False)

    def test_add_bond(self):
        """Check addition of bonds/angles/dihedrals/impropers"""
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        atoms = IMP.Strings(['CA', 'CB'])
        # Test construction from strings or from CHARMMBond object
        res.add_bond(atoms)
        res.add_bond(IMP.atom.CHARMMBond2(atoms))
        self.assertEqual(res.get_number_of_bonds(), 2)
        atoms = IMP.Strings(['CA', 'CB', 'N'])
        res.add_angle(IMP.atom.CHARMMBond3(atoms))
        self.assertEqual(res.get_number_of_angles(), 1)
        atoms = IMP.Strings(['CA', 'CB', 'N', 'O'])
        res.add_dihedral(IMP.atom.CHARMMBond4(atoms))
        self.assertEqual(res.get_number_of_dihedrals(), 1)
        res.add_improper(atoms)
        self.assertEqual(res.get_number_of_impropers(), 1)
        self.assertEqual(res.get_bond(0).contains_atom('CA'), True)
        self.assertEqual(res.get_angle(0).contains_atom('N'), True)
        self.assertEqual(res.get_dihedral(0).contains_atom('O'), True)
        self.assertEqual(res.get_improper(0).contains_atom('O'), True)

    def test_patch(self):
        """Check the CHARMM patch class"""
        patch = IMP.atom.CHARMMPatch('PFOO')
        self.assertEqual(patch.get_type(), 'PFOO')
        patch.add_deleted_atom('CA')

    def test_forcefield_add_get(self):
        """Test adding/getting patches and residues to/from forcefields"""
        ff = IMP.atom.CharmmParameters(IMP.atom.get_data_path("top.lib"))
        self.assertRaises(IMP.ValueException, ff.get_residue_topology, 'FOO')
        self.assertRaises(IMP.ValueException, ff.get_patch, 'PFOO')
        patch = IMP.atom.CHARMMPatch('PFOO')
        res = IMP.atom.CHARMMIdealResidueTopology('FOO')
        ff.add_patch(patch)
        ff.add_residue_topology(res)
        ff.get_residue_topology('FOO')
        ff.get_patch('PFOO')

    def test_forcefield_read(self):
        """Test read of topology from files"""
        ff = IMP.atom.CharmmParameters(IMP.atom.get_data_path("top.lib"))
        self.assertRaises(ValueError, ff.get_residue_topology, 'CTER')
        res = ff.get_residue_topology('CYS')
        self.assertEqual(res.get_number_of_bonds(), 11)
        self.assertEqual(res.get_number_of_angles(), 0)
        self.assertEqual(res.get_number_of_dihedrals(), 0)
        self.assertEqual(res.get_number_of_impropers(), 3)
        for (name, typ, charge) in [('N', 'NH1', -0.47), ('H', 'H', 0.31),
                                    ('CA', 'CT1', 0.07), ('HA', 'HB', 0.09),
                                    ('CB', 'CT2', -0.11), ('HB3', 'HA', 0.09),
                                    ('HB2', 'HA', 0.09), ('SG', 'S', -0.23),
                                    ('HG', 'HS', 0.16), ('C', 'C',  0.51),
                                    ('O', 'O',  -0.51)]:
            at = res.get_atom(name)
            self.assertEqual(at.get_charmm_type(), typ)
            self.assertInTolerance(at.get_charge(), charge, 1e-3)

        self.assertRaises(ValueError, ff.get_patch, 'CYS')
        patch = ff.get_patch('CTER')
        for (name, typ, charge) in [('C', 'CC', 0.34), ('O', 'OC', -0.67),
                                    ('OXT', 'OC', -0.67)]:
            at = patch.get_atom(name)
            self.assertEqual(at.get_charmm_type(), typ)
            self.assertInTolerance(at.get_charge(), charge, 1e-3)

        for (name, first, last) in [('ALA', 'NTER', 'CTER'),
                                    ('GLY', 'GLYP', 'CTER'),
                                    ('UNK', 'GLYP', 'CTER'),
                                    ('PRO', 'PROP', 'CTER'),
                                    ('TIP3', '', ''),
                                    ('SOD', '', '')]:
            res = ff.get_residue_topology(name)
            self.assertEqual(res.get_default_first_patch(), first)
            self.assertEqual(res.get_default_last_patch(), last)

    def test_residue_topology(self):
        """Test CHARMM residue topology objects"""
        ideal = IMP.atom.CHARMMIdealResidueTopology('ALA')
        at = _make_test_atom()
        ideal.add_atom(at)
        res = IMP.atom.CHARMMResidueTopology(ideal)
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT1')
        self.assertEqual(res.get_patched(), False)
        res.set_patched(True)
        self.assertEqual(res.get_patched(), True)

    def test_patching(self):
        """Test application of patches"""
        ff = IMP.atom.CharmmParameters(IMP.atom.get_data_path("top.lib"))
        patch = ff.get_patch('CTER')
        res = IMP.atom.CHARMMResidueTopology(ff.get_residue_topology('ALA'))
        self.assertEqual(res.get_atom('C').get_charmm_type(), 'C')
        self.assertEqual(res.get_number_of_bonds(), 10)
        self.assertEqual(res.get_number_of_impropers(), 3)
        patch.apply(res)
        # Patch should change atom type of existing atoms
        self.assertEqual(res.get_atom('C').get_charmm_type(), 'CC')
        # Should also add new atoms
        self.assertEqual(res.get_atom('OXT').get_charmm_type(), 'OC')
        self.assertEqual(res.get_patched(), True)
        # Should add bonds/angles/dihedrals/impropers
        self.assertEqual(res.get_number_of_bonds(), 11)
        self.assertEqual(res.get_number_of_impropers(), 4)
        # Repeated patching should not be possible
        self.assertRaises(IMP.ValueException, patch.apply, res)

        # Patches should delete atoms
        patch = ff.get_patch('TP1A')
        res = IMP.atom.CHARMMResidueTopology(ff.get_residue_topology('TYR'))
        self.assertEqual(res.get_atom('CB').get_charmm_type(), 'CT2')
        patch.apply(res)
        self.assertRaises(IMP.ValueException, res.get_atom, 'CB')

    def test_manual_make_topology(self):
        """Test manual construction of topology"""
        model = IMP.atom.CHARMMTopology()
        self.assertEqual(model.get_number_of_segments(), 0)
        segment = IMP.atom.CHARMMSegmentTopology()
        model.add_segment(segment)

        ideal = IMP.atom.CHARMMIdealResidueTopology('ALA')
        at = _make_test_atom()
        ideal.add_atom(at)
        res = IMP.atom.CHARMMResidueTopology(ideal)
        segment.add_residue(res)

        self.assertEqual(model.get_number_of_segments(), 1)
        self.assertEqual(segment.get_number_of_residues(), 1)

    def test_make_topology(self):
        """Test construction of topology"""
        m = IMP.Model()
        pdb = IMP.atom.read_pdb(self.get_input_file_name('1z5s_C.pdb'), m)
        ff = IMP.atom.CharmmParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = ff.make_topology(pdb)
        topology.apply_default_patches(ff)
        self.assertEqual(topology.get_number_of_segments(), 1)
        segment = topology.get_segment(0)
        self.assertEqual(segment.get_number_of_residues(), 156)
        self.assertRaises(IMP.ValueException, segment.apply_default_patches, ff)
        atoms = IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE)
        residues = IMP.atom.get_by_type(pdb, IMP.atom.RESIDUE_TYPE)
        last_atom = atoms[-1].get_particle()
        for typ in (IMP.atom.CHARMMAtom, IMP.core.XYZR, IMP.atom.Charged,
                    IMP.atom.LennardJones):
            self.assertEqual(typ.particle_is_instance(last_atom), False)
        topology.add_atom_types(pdb)
        self.assertEqual(IMP.atom.CHARMMAtom(last_atom).get_charmm_type(), 'OC')
        topology.add_charges(pdb)
        self.assertInTolerance(IMP.atom.Charged(last_atom).get_charge(),
                               -0.67, 1e-3)
        topology.add_bonds(pdb, ff)
        for (bondr1, bondr2, bonda1, bonda2, atyp1, atyp2, bondlen, fcon) in [
           # intraresidue bond
           (residues[0], residues[0], IMP.atom.AT_CA, IMP.atom.AT_CB,
            'CT1', 'CT3', 1.5380, 222.500),
           # backbone bond
           (residues[0], residues[1], IMP.atom.AT_C, IMP.atom.AT_N,
            'C', 'NH1', 1.3450, 370.000),
           # CTER bond
           (residues[-1], residues[-1], IMP.atom.AT_OXT, IMP.atom.AT_C,
            'OC', 'CC', 1.2600, 525.000)]:
            r1 = bondr1.get_as_residue()
            r2 = bondr2.get_as_residue()
            a1 = IMP.atom.get_atom(r1, bonda1)
            a2 = IMP.atom.get_atom(r2, bonda2)
            self.assertAtomsBonded(a1, a2, atyp1, atyp2, bondlen, fcon)

        ff.add_radii(pdb)
        ff.add_well_depths(pdb)
        self.assertInTolerance(IMP.core.XYZR(last_atom).get_radius(),
                               1.70, 1e-3)
        self.assertInTolerance(
              IMP.atom.LennardJones(last_atom).get_well_depth(), 0.12, 1e-3)
        last_cg1 = atoms[-3].get_particle()
        self.assertInTolerance(IMP.core.XYZR(last_cg1).get_radius(),
                               2.06, 1e-3)

    def test_make_hierarchy(self):
        """Test construction of hierarchy from topology"""
        ff = IMP.atom.CharmmParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = IMP.atom.CHARMMTopology()
        segment = IMP.atom.CHARMMSegmentTopology()
        topology.add_segment(segment)
        for res in ('ALA', 'CYS', 'TYR'):
            restop = IMP.atom.CHARMMResidueTopology(
                                     ff.get_residue_topology(res))
            segment.add_residue(restop)
        m = IMP.Model()
        hierarchy = topology.make_hierarchy(m)
        chains = IMP.atom.get_by_type(hierarchy, IMP.atom.CHAIN_TYPE)
        residues = IMP.atom.get_by_type(hierarchy, IMP.atom.RESIDUE_TYPE)
        atoms = IMP.atom.get_by_type(hierarchy, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(chains), 1)
        self.assertEqual(len(residues), 3)
        self.assertEqual(len(atoms), 42)

if __name__ == '__main__':
    unittest.main()
