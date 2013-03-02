import IMP
import IMP.test
import IMP.atom

def _make_test_atom():
    at = IMP.atom.CHARMMAtomTopology('CA')
    at.set_charmm_type('CT1')
    at.set_charge(0.8)
    return at

class Tests(IMP.test.TestCase):
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
                self.assertAlmostEqual(bond.get_length(), bondlen, delta=1e-4)
                self.assertAlmostEqual(bond.get_stiffness(), forcecon,
                                       delta=1e-3)
                return
        self.fail("No bond defined between %s and %s" % (a1, a2))

    def test_atom(self):
        """Check creation and methods of CHARMMAtomTopology"""
        at = _make_test_atom()
        self.assertEqual(at.get_name(), 'CA')
        self.assertEqual(at.get_charmm_type(), 'CT1')
        self.assertAlmostEqual(at.get_charge(), 0.8, delta=1e-6)

    def test_add_get_atom(self):
        """Check add/get atom from CHARMM residues"""
        res = IMP.atom.CHARMMIdealResidueTopology(IMP.atom.ResidueType('FOO'))
        self.assertEqual(res.get_type(), 'FOO')
        self.assertRaises(IMP.ValueException, res.get_atom, 'CA')
        at = _make_test_atom()
        res.add_atom(at)
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT1')
        # Modifying our copy after it was inserted should have no effect
        at.set_charmm_type('CT2')
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT1')
        # Should be possible to modify atom type after construction though
        at2 = IMP.atom.CHARMMAtomTopology('CB')
        at2.set_charmm_type('CT2')
        res.add_atom(at2)
        self.assertEqual(res.get_atom('CB').get_charmm_type(), 'CT2')

    def test_delete_atom(self):
        """Check delete atom from CHARMM residues"""
        res = IMP.atom.CHARMMIdealResidueTopology(IMP.atom.ResidueType('FOO'))
        at = _make_test_atom()
        res.add_atom(at)
        self.assertRaises(IMP.ValueException, res.remove_atom, 'CB')
        res.remove_atom('CA')
        self.assertRaises(IMP.ValueException, res.get_atom, 'CA')

    def test_default_patches(self):
        """Check default patches of CHARMM residues"""
        res = IMP.atom.CHARMMIdealResidueTopology(IMP.atom.ResidueType('FOO'))
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
        bond = IMP.atom.CHARMMBond(['CA', 'CB'])
        self.assertEqual(bond.get_contains_atom('CA'), True)
        self.assertEqual(bond.get_contains_atom('N'), False)
        self.assertEqual(bond.get_endpoint(0).get_atom_name(), 'CA')

    def test_add_bond(self):
        """Check addition of bonds/angles/dihedrals/impropers"""
        res = IMP.atom.CHARMMIdealResidueTopology(IMP.atom.ResidueType('FOO'))
        atoms = ['CA', 'CB']
        res.add_bond(IMP.atom.CHARMMBond(atoms))
        self.assertEqual(res.get_number_of_bonds(), 1)
        atoms = ['CA', 'CB', 'N']
        res.add_angle(IMP.atom.CHARMMAngle(atoms))
        self.assertEqual(res.get_number_of_angles(), 1)
        atoms = ['CA', 'CB', 'N', 'O']
        res.add_dihedral(IMP.atom.CHARMMDihedral(atoms))
        self.assertEqual(res.get_number_of_dihedrals(), 1)
        res.add_improper(IMP.atom.CHARMMDihedral(atoms))
        self.assertEqual(res.get_number_of_impropers(), 1)
        self.assertEqual(res.get_bond(0).get_contains_atom('CA'), True)
        self.assertEqual(res.get_angle(0).get_contains_atom('N'), True)
        self.assertEqual(res.get_dihedral(0).get_contains_atom('O'), True)
        self.assertEqual(res.get_improper(0).get_contains_atom('O'), True)

    def test_add_ic(self):
        """Check addition of internal coordinates"""
        res = IMP.atom.CHARMMIdealResidueTopology(IMP.atom.ResidueType('FOO'))
        ic = IMP.atom.CHARMMInternalCoordinate(['N', 'C', 'CA', 'CB'],
                                               1,2,3,4,5,True)
        self.assertEqual(ic.get_first_distance(), 1)
        self.assertEqual(ic.get_first_angle(), 2)
        self.assertEqual(ic.get_dihedral(), 3)
        self.assertEqual(ic.get_second_angle(), 4)
        self.assertEqual(ic.get_second_distance(), 5)
        self.assertEqual(ic.get_contains_atom('N'), True)
        self.assertEqual(ic.get_contains_atom('CG'), False)
        self.assertEqual(res.get_number_of_internal_coordinates(), 0)
        res.add_internal_coordinate(ic)
        self.assertEqual(res.get_number_of_internal_coordinates(), 1)

    def test_patch(self):
        """Check the CHARMM patch class"""
        patch = IMP.atom.CHARMMPatch('PFOO')
        self.assertEqual(patch.get_type(), 'PFOO')
        self.assertEqual(patch.get_number_of_removed_atoms(), 0)
        patch.add_removed_atom('CA')
        self.assertEqual(patch.get_number_of_removed_atoms(), 1)
        self.assertEqual(patch.get_removed_atom(0), 'CA')

    def test_forcefield_add_get(self):
        """Test adding/getting patches and residues to/from forcefields"""
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"))
        self.assertRaises(IMP.ValueException, ff.get_residue_topology, IMP.atom.ResidueType('FOO'))
        self.assertRaises(IMP.ValueException, ff.get_patch, 'PFOO')
        patch = IMP.atom.CHARMMPatch('PFOO')
        res = IMP.atom.CHARMMIdealResidueTopology(IMP.atom.ResidueType('FOO'))
        ff.add_patch(patch)
        ff.add_residue_topology(res)
        ff.get_residue_topology(IMP.atom.ResidueType('FOO'))
        ff.get_patch('PFOO')

    def test_forcefield_read(self):
        """Test read of topology from files"""
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"))
        self.assertRaises(ValueError, ff.get_residue_topology, IMP.atom.ResidueType('CTER'))
        res = ff.get_residue_topology(IMP.atom.CYS)
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
            self.assertAlmostEqual(at.get_charge(), charge, delta=1e-3)

        self.assertRaises(ValueError, ff.get_patch, 'CYS')
        patch = ff.get_patch('CTER')
        for (name, typ, charge) in [('C', 'CC', 0.34), ('O', 'OC', -0.67),
                                    ('OXT', 'OC', -0.67)]:
            at = patch.get_atom(name)
            self.assertEqual(at.get_charmm_type(), typ)
            self.assertAlmostEqual(at.get_charge(), charge, delta=1e-3)

        for (name, first, last) in [('ALA', 'NTER', 'CTER'),
                                    ('GLY', 'GLYP', 'CTER'),
                                    ('UNK', 'GLYP', 'CTER'),
                                    ('PRO', 'PROP', 'CTER'),
                                    ('TIP3', '', ''),
                                    ('SOD', '', '')]:
            res = ff.get_residue_topology(IMP.atom.ResidueType(name))
            self.assertEqual(res.get_default_first_patch(), first)
            self.assertEqual(res.get_default_last_patch(), last)

    def test_residue_topology(self):
        """Test CHARMM residue topology objects"""
        ideal = IMP.atom.CHARMMIdealResidueTopology(IMP.atom.ResidueType('ALA'))
        at = _make_test_atom()
        ideal.add_atom(at)
        res = IMP.atom.CHARMMResidueTopology(ideal)
        self.assertEqual(res.get_atom('CA').get_charmm_type(), 'CT1')
        self.assertEqual(res.get_patched(), False)
        res.set_patched(True)
        self.assertEqual(res.get_patched(), True)

    def test_single_patching(self):
        """Test application of single-residue patches"""
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"))
        patch = ff.get_patch('CTER')
        res = IMP.atom.CHARMMResidueTopology(
                               ff.get_residue_topology(IMP.atom.ALA))
        self.assertEqual(res.get_atom('C').get_charmm_type(), 'C')
        self.assertEqual(res.get_number_of_bonds(), 10)
        self.assertEqual(res.get_number_of_impropers(), 3)
        self.assertEqual(res.get_number_of_internal_coordinates(), 12)
        patch.apply(res)
        # Patch should change atom type of existing atoms
        self.assertEqual(res.get_atom('C').get_charmm_type(), 'CC')
        # Should also add new atoms
        self.assertEqual(res.get_atom('OXT').get_charmm_type(), 'OC')
        self.assertEqual(res.get_patched(), True)
        # Should add bonds/angles/dihedrals/impropers/internal coordinates
        self.assertEqual(res.get_number_of_bonds(), 11)
        self.assertEqual(res.get_number_of_impropers(), 4)
        self.assertEqual(res.get_number_of_internal_coordinates(), 14)
        ic = res.get_internal_coordinate(13)
        self.assertEqual(ic.get_first_distance(), 0.)
        self.assertEqual(ic.get_first_angle(), 0.)
        self.assertEqual(ic.get_dihedral(), 180.)
        # Repeated patching should not be possible
        self.assertRaises(IMP.ValueException, patch.apply, res)

        # Patches should delete atoms
        patch = ff.get_patch('TP1A')
        res = IMP.atom.CHARMMResidueTopology(
                            ff.get_residue_topology(IMP.atom.TYR))
        self.assertEqual(res.get_atom('CB').get_charmm_type(), 'CT2')
        patch.apply(res)
        self.assertRaises(IMP.ValueException, res.get_atom, 'CB')

    def test_double_patching(self):
        """Test application of two-residue patches"""
        ff = IMP.atom.CHARMMParameters(
                            self.get_input_file_name("top_2patch.lib"))
        # Check patching using both Modeller-style and CHARMM-style atom naming
        for patch_name in ('DIS1', 'DIS2'):
            patch = ff.get_patch(patch_name)
            res1 = IMP.atom.CHARMMResidueTopology(ff.get_residue_topology(
                                                IMP.atom.ResidueType('CYS')))
            res2 = IMP.atom.CHARMMResidueTopology(ff.get_residue_topology(
                                                IMP.atom.ResidueType('CYS')))
            self.assertEqual(res1.get_atom('HG').get_charmm_type(), 'HS')
            self.assertEqual(res1.get_number_of_bonds(), 11)
            self.assertEqual(res1.get_number_of_impropers(), 3)
            self.assertEqual(res2.get_number_of_bonds(), 11)
            self.assertEqual(res2.get_number_of_impropers(), 3)
            # Single-residue patches cannot be applied to two residues
            badpatch = ff.get_patch('CTER')
            self.assertRaises(IMP.ValueException, badpatch.apply, res1, res2)
            patch.apply(res1, res2)
            # Patch should change atom type of existing atoms
            self.assertEqual(res1.get_atom('SG').get_charmm_type(), 'SM')
            self.assertEqual(res2.get_atom('SG').get_charmm_type(), 'SM')
            # Should prevent further patching
            self.assertEqual(res1.get_patched(), True)
            self.assertEqual(res2.get_patched(), True)
            # Patches should delete atoms
            self.assertRaises(IMP.ValueException, res1.get_atom, 'HG')
            self.assertRaises(IMP.ValueException, res2.get_atom, 'HG')
            # Should add/delete bonds/dihedrals
            self.assertEqual(res1.get_number_of_bonds(), 11)
            self.assertEqual(res1.get_number_of_impropers(), 3)
            self.assertEqual(res2.get_number_of_bonds(), 10)
            self.assertEqual(res2.get_number_of_impropers(), 3)
            # Should have added an SG-SG bond
            bond = res1.get_bond(10)
            self.assertEqual(bond.get_endpoint(0).get_atom_name(), 'SG')
            self.assertEqual(bond.get_endpoint(1).get_atom_name(), 'SG')

    def test_manual_make_topology(self):
        """Test manual construction of topology"""
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        model = IMP.atom.CHARMMTopology(ff)
        self.assertEqual(model.get_number_of_segments(), 0)
        segment = IMP.atom.CHARMMSegmentTopology()
        model.add_segment(segment)

        ideal = IMP.atom.CHARMMIdealResidueTopology(IMP.atom.ResidueType('ALA'))
        at = _make_test_atom()
        ideal.add_atom(at)
        res = IMP.atom.CHARMMResidueTopology(ideal)
        segment.add_residue(res)

        self.assertEqual(model.get_number_of_segments(), 1)
        self.assertEqual(segment.get_number_of_residues(), 1)

    def test_add_coordinates_backbone(self):
        """Test adding coordinates to a backbone- or CA-only structure"""
        m = IMP.Model()
        pdb = IMP.atom.read_pdb(self.get_input_file_name('backbone-ca.pdb'), m)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = ff.create_topology(pdb)
        topology.apply_default_patches()
        topology.add_atom_types(pdb)
        # Add sidechain and hydrogen atoms for the backbone chain,
        # and assign all non-CA atoms for the CA-only chain
        topology.add_missing_atoms(pdb)
        topology.add_coordinates(pdb)
        # Every atom should now have XYZ coordinates
        for a in IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE):
            self.assertTrue(IMP.core.XYZ.particle_is_instance(a))

    def test_setup_hierarchy(self):
        """Test CHARMMTopology::setup_hierarchy() method"""
        m = IMP.Model()
        pdb = IMP.atom.read_pdb(
                        self.get_input_file_name('charmm_type_test.pdb'), m)
        ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
        topology = ff.create_topology(pdb)
        topology.apply_default_patches()
        topology.setup_hierarchy(pdb)
        # Every atom should now have XYZ coordinates and CHARMM type
        atoms = IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(atoms), 11)
        for a in atoms:
            self.assertTrue(IMP.core.XYZ.particle_is_instance(a))
            self.assertTrue(IMP.atom.CHARMMAtom.particle_is_instance(a))

    def test_heme(self):
        """Test CHARMM topology handling of HEME residues"""
        m = IMP.Model()
        pdb = IMP.atom.read_pdb(
                        self.get_input_file_name('HEM_model.pdb'), m)
        residue = IMP.atom.get_by_type(pdb, IMP.atom.RESIDUE_TYPE)[0]
        # Add some dummy atoms, to make sure they get removed
        a = IMP.atom.Atom.setup_particle(IMP.Particle(m), IMP.atom.AT_CA)
        residue.add_child(a)
        a = IMP.atom.Atom.setup_particle(IMP.Particle(m), IMP.atom.AT_CB)
        residue.add_child(a)
        # Remove some needed atoms, to make sure they get added
        residue.remove_child(0)
        ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
        topology = ff.create_topology(pdb)
        topology.apply_default_patches()
        topology.setup_hierarchy(pdb)
        atoms = IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(atoms), 43)
        a = atoms[-2]
        # Make sure we kept the original PDB coordinates
        coord = a.get_as_xyz().get_coordinates()
        self.assertAlmostEqual(coord[0], 20.149, delta=1e-3)
        self.assertAlmostEqual(coord[1], 33.749, delta=1e-3)
        self.assertAlmostEqual(coord[2], 3.830, delta=1e-3)
        # Make sure all atoms are HETATM
        for a in atoms:
            self.assertEqual(a.get_as_atom().get_atom_type().get_string()[:4],
                             "HET:")

    def test_add_coordinates_empty_structure(self):
        """Test adding coordinates to a completely empty structure"""
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = IMP.atom.CHARMMTopology(ff)
        topology.add_sequence('ACY')
        m = IMP.Model()
        topology.apply_default_patches()
        hierarchy = topology.create_hierarchy(m)
        topology.add_coordinates(hierarchy)
        # Every atom should now have XYZ coordinates
        for a in IMP.atom.get_by_type(hierarchy, IMP.atom.ATOM_TYPE):
            self.assertTrue(IMP.core.XYZ.particle_is_instance(a))

    def test_add_sequence(self):
        """Test CHARMMTopology::add_sequence()"""
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        t = IMP.atom.CHARMMTopology(ff)
        self.assertEqual(t.get_number_of_segments(), 0)
        # Should add an empty segment
        t.add_sequence('')
        self.assertEqual(t.get_number_of_segments(), 1)
        self.assertEqual(t.get_segment(0).get_number_of_residues(), 0)

        # Should add two non-empty segments
        t = IMP.atom.CHARMMTopology(ff)
        t.add_sequence('ACY/MF')
        self.assertEqual(t.get_number_of_segments(), 2)
        self.assertEqual(t.get_segment(0).get_number_of_residues(), 3)
        self.assertEqual(t.get_segment(1).get_number_of_residues(), 2)

        # Invalid one-letter codes should cause an exception and add no segments
        t = IMP.atom.CHARMMTopology(ff)
        self.assertRaises(ValueError, t.add_sequence, 'a')
        self.assertEqual(t.get_number_of_segments(), 0)

    def test_make_patched_topology(self):
        """Test construction of topology with manual patching"""
        m = IMP.Model()
        pdb = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = ff.create_topology(pdb)
        segment = topology.get_segment(0)
        patch = ff.get_patch('LINK')
        patch.apply(segment.get_residue(segment.get_number_of_residues() - 1),
                    segment.get_residue(0))
        topology.add_atom_types(pdb)
        bonds = topology.add_bonds(pdb)
        residues = IMP.atom.get_by_type(pdb, IMP.atom.RESIDUE_TYPE)
        # LINK residue should have constructed a backbone bond between the
        # first and last residues
        r1 = residues[-1].get_as_residue()
        r2 = residues[0].get_as_residue()
        a1 = IMP.atom.get_atom(r1, IMP.atom.AT_C)
        a2 = IMP.atom.get_atom(r2, IMP.atom.AT_N)
        self.assertAtomsBonded(a1, a2, 'C', 'NH1', 1.3450, 27.203)

    def test_add_dihedrals(self):
        """Test add_dihedrals() method"""
        # Dihedrals with missing atoms should not be created; dihedrals
        # with missing parameters should be (but with no parameters)
        for atoms,num_dihed,stiff in [(['C', 'CA', 'CB', 'HB1'], 1, 0.6325),
                                      (['C', 'CA', 'N', 'O'], 1, 0.0),
                                      (['N', 'C', 'CA', 'SG'], 0, None),
                                      (['N', 'CA', 'C', '+N'], 0, None)]:
            m = IMP.Model()
            ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                           IMP.atom.get_data_path("par.lib"))
            res = ff.get_residue_topology(IMP.atom.ResidueType('ALA'))
            res.add_dihedral(IMP.atom.CHARMMDihedral(atoms))
            topology = IMP.atom.CHARMMTopology(ff)
            topology.add_sequence('PA')
            h = topology.create_hierarchy(m)
            for a in IMP.atom.get_by_type(h, IMP.atom.ATOM_TYPE):
                IMP.core.XYZ.setup_particle(a.get_particle(),
                                            IMP.algebra.Vector3D(0,0,0))
            topology.add_atom_types(h)
            d = topology.add_dihedrals(h)
            self.assertEqual(len(d), num_dihed)
            if stiff is not None:
                self.assertAlmostEqual(stiff,
                                       IMP.atom.Dihedral(d[0]).get_stiffness(),
                                       delta=1e-3)

    def test_dihedral_stiffness(self):
        """Make sure dihedrals can have negative stiffness"""
        m = IMP.Model()
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = IMP.atom.CHARMMTopology(ff)
        topology.add_sequence('PA')
        h = topology.create_hierarchy(m)
        for a in IMP.atom.get_by_type(h, IMP.atom.ATOM_TYPE):
            IMP.core.XYZ.setup_particle(a.get_particle(),
                                        IMP.algebra.Vector3D(0,0,0))
        topology.add_atom_types(h)
        bonds = topology.add_bonds(h)
        dihedrals = ff.create_dihedrals(bonds)
        self.assertEqual(len(dihedrals), 67)
        d = IMP.atom.Dihedral(dihedrals[5])
        self.assertEqual([IMP.atom.CHARMMAtom(d.get_particle(x)).
                                  get_charmm_type() for x in range(4)],
                         ['NH1', 'C', 'CP1', 'N'])
        self.assertAlmostEqual(d.get_stiffness(), 0.7746, delta=1e-4)
        d = IMP.atom.Dihedral(dihedrals[6])
        self.assertEqual([IMP.atom.CHARMMAtom(d.get_particle(x)).
                                  get_charmm_type() for x in range(4)],
                         ['NH1', 'C', 'CP1', 'N'])
        self.assertAlmostEqual(d.get_stiffness(), -0.7746, delta=1e-4)

    def test_make_topology(self):
        """Test construction of topology"""
        m = IMP.Model()
        pdb = IMP.atom.read_pdb(self.get_input_file_name('1z5s_C.pdb'), m)
        atoms = IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE)
        residues = IMP.atom.get_by_type(pdb, IMP.atom.RESIDUE_TYPE)
        last_atom = atoms[-1].get_particle()
        # PDB reader will have already set radii but not CHARMM type:
        self.assertEqual(IMP.core.XYZR.particle_is_instance(last_atom), True)
        self.assertEqual(IMP.atom.CHARMMAtom.particle_is_instance(last_atom),
                         False)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = ff.create_topology(pdb)
        topology.apply_default_patches()
        self.assertEqual(topology.get_number_of_segments(), 1)
        segment = topology.get_segment(0)
        self.assertEqual(segment.get_number_of_residues(), 156)
        self.assertRaises(IMP.ValueException, segment.apply_default_patches, ff)
        for typ in (IMP.atom.Charged, IMP.atom.LennardJones):
            self.assertEqual(typ.particle_is_instance(last_atom), False)
        topology.add_atom_types(pdb)
        self.assertEqual(IMP.atom.CHARMMAtom.particle_is_instance(last_atom),
                         True)
        self.assertEqual(IMP.atom.CHARMMAtom(last_atom).get_charmm_type(), 'OC')
        topology.add_charges(pdb)
        self.assertAlmostEqual(IMP.atom.Charged(last_atom).get_charge(),
                               -0.67, delta=1e-3)
        bonds = topology.add_bonds(pdb)
        self.assertEqual(len(bonds), 1215)
        angles = ff.create_angles(bonds)
        self.assertEqual(len(angles), 1651)
        dihedrals = ff.create_dihedrals(bonds)
        self.assertEqual(len(dihedrals), 2161)
        for (bondr1, bondr2, bonda1, bonda2, atyp1, atyp2, bondlen, fcon) in [
           # intraresidue bond
           (residues[0], residues[0], IMP.atom.AT_CA, IMP.atom.AT_CB,
            'CT1', 'CT3', 1.5380, 21.095),
           # backbone bond
           (residues[0], residues[1], IMP.atom.AT_C, IMP.atom.AT_N,
            'C', 'NH1', 1.3450, 27.203),
           # CTER bond
           (residues[-1], residues[-1], IMP.atom.AT_OXT, IMP.atom.AT_C,
            'OC', 'CC', 1.2600, 32.404)]:
            r1 = bondr1.get_as_residue()
            r2 = bondr2.get_as_residue()
            a1 = IMP.atom.get_atom(r1, bonda1)
            a2 = IMP.atom.get_atom(r2, bonda2)
            self.assertAtomsBonded(a1, a2, atyp1, atyp2, bondlen, fcon)
        impropers = topology.add_impropers(pdb)
        self.assertEqual(len(impropers), 509)

        ff.add_radii(pdb, 1.2)
        ff.add_well_depths(pdb)
        self.assertAlmostEqual(IMP.core.XYZR(last_atom).get_radius(),
                               2.04, delta=1e-3)
        self.assertAlmostEqual(
              IMP.atom.LennardJones(last_atom).get_well_depth(), 0.12,
              delta=1e-3)
        last_cg1 = atoms[-3].get_particle()
        self.assertAlmostEqual(IMP.core.XYZR(last_cg1).get_radius(),
                               2.472, delta=1e-3)

    def test_make_hierarchy(self):
        """Test construction of hierarchy from topology"""
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = IMP.atom.CHARMMTopology(ff)
        topology.add_sequence('ACY')
        m = IMP.Model()
        hierarchy = topology.create_hierarchy(m)
        chains = IMP.atom.get_by_type(hierarchy, IMP.atom.CHAIN_TYPE)
        residues = IMP.atom.get_by_type(hierarchy, IMP.atom.RESIDUE_TYPE)
        atoms = IMP.atom.get_by_type(hierarchy, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(chains), 1)
        self.assertEqual(len(residues), 3)
        self.assertEqual(len(atoms), 42)

    def test_get_parameters(self):
        """Test CHARMMTopology::get_parameters()"""
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = IMP.atom.CHARMMTopology(ff)
        self.assertEqual(topology.get_parameters(), ff)

    def test_empty_residue_make_hierarchy(self):
        """Test construction of hierarchy from empty topology"""
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = IMP.atom.CHARMMTopology(ff)
        segment = IMP.atom.CHARMMSegmentTopology()
        topology.add_segment(segment)
        for res in ('ALA', 'CYS', 'TYR'):
            # Note that here we construct each CHARMMResidueTopology using
            # just a residue type, not an ideal residue topology from top.lib;
            # thus it should contain no atoms
            restop = IMP.atom.CHARMMResidueTopology(IMP.atom.ResidueType(res))
            segment.add_residue(restop)
        m = IMP.Model()
        hierarchy = topology.create_hierarchy(m)
        chains = IMP.atom.get_by_type(hierarchy, IMP.atom.CHAIN_TYPE)
        residues = IMP.atom.get_by_type(hierarchy, IMP.atom.RESIDUE_TYPE)
        atoms = IMP.atom.get_by_type(hierarchy, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(chains), 1)
        self.assertEqual(len(residues), 3)
        self.assertEqual(len(atoms), 0)

    def test_get_remove_untyped_atoms(self):
        """Test get or removal of untyped atoms"""
        m = IMP.Model()
        pdb = IMP.atom.read_pdb(
                 self.get_input_file_name('charmm_type_test.pdb'), m)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = ff.create_topology(pdb)
        topology.apply_default_patches()
        topology.add_atom_types(pdb)
        untyped = IMP.atom.get_charmm_untyped_atoms(pdb)
        self.assertEqual([x.get_atom_type().get_string() for x in untyped],
                         ["DUM"])
        atoms = IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(atoms), 8)
        IMP.atom.remove_charmm_untyped_atoms(pdb)
        atoms = IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(atoms), 7)

    def test_add_missing_atoms(self):
        """Test adding missing atoms"""
        m = IMP.Model()
        pdb = IMP.atom.read_pdb(
                 self.get_input_file_name('charmm_type_test.pdb'), m)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        topology = ff.create_topology(pdb)
        topology.apply_default_patches()

        atoms = IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(atoms), 8)
        topology.add_missing_atoms(pdb)

        atoms = IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE)
        atom_types = [IMP.atom.Atom(x).get_atom_type().get_string() \
                      for x in atoms]
        # Input system contains only backbone; output should also contain
        # hydrogens, etc. Newly-added atoms should be added after the
        # original atoms
        self.assertEqual(atom_types,
                         ['N', 'CA', 'C', 'O', 'H', 'HA', 'CB', 'HB3', 'HB2',
                          'OG', 'HG', 'H2', 'H3', 'N', 'CA', 'C', 'DUM', 'H',
                          'HA3', 'HA2', 'O', 'OXT'])

if __name__ == '__main__':
    IMP.test.main()
