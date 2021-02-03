from __future__ import print_function
import IMP.test
import IMP.pmi.restraints.em
import IMP.pmi.mmcif
import IMP.pmi.dof
import IMP.pmi.topology
import IMP.pmi.macros
import sys
import os
import io
import ihm.format
import ihm.location
import ihm.dataset
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

class DummyState(object):
    short_name = None
    long_name = None

class DummyRepr(object):
    def __init__(self, short_name, long_name):
        self.state = DummyState()
        self.state.short_name = short_name
        self.state.long_name = long_name

class EmptyObject(object):
    state = DummyState()

def get_all_models_group(simo, po):
    state = simo._protocol_output[0][1]
    return state.add_model_group(ihm.model.ModelGroup(name="All models"))

class Tests(IMP.test.TestCase):

    def test_assign_id(self):
        """Test _assign_id utility function"""
        class DummyObj(object):
            def __init__(self, hashval):
                self.hashval = hashval
            def __eq__(self, other):
                return self.hashval == other.hashval
            def __hash__(self):
                return self.hashval
        seen_objs = {}
        obj_by_id = []
        obj1a = DummyObj(42) # obj1a and 1b are identical
        obj1b = DummyObj(42)
        obj2 = DummyObj(34)
        obj3 = DummyObj(23) # obj3 already has an id
        obj3.id = 'foo'
        for obj in (obj1a, obj1b, obj2, obj3):
            IMP.pmi.mmcif._assign_id(obj, seen_objs, obj_by_id)
        self.assertEqual(obj1a.id, 1)
        self.assertEqual(obj1b.id, 1)
        self.assertEqual(obj2.id, 2)
        self.assertEqual(obj3.id, 'foo')
        self.assertEqual(obj_by_id, [obj1a, obj2])

    def test_software_modeller(self):
        """Test AllSoftware.set_modeller_used"""
        system = ihm.System()
        d = IMP.pmi.mmcif._AllSoftware(system)
        self.assertEqual(d.modeller_used, False)
        d.set_modeller_used('9.18', '2018-01-01')
        self.assertEqual(d.modeller_used, True)
        self.assertEqual(len(system.software), 3)
        self.assertEqual(system.software[-1].version, '9.18')
        # Further calls should have no effect
        d.set_modeller_used('9.0', 'xxx')
        self.assertEqual(len(system.software), 3)
        self.assertEqual(system.software[-1].version, '9.18')

    def test_software_phyre2(self):
        """Test AllSoftware.set_phyre2_used"""
        system = ihm.System()
        d = IMP.pmi.mmcif._AllSoftware(system)
        self.assertEqual(d.phyre2_used, False)
        d.set_phyre2_used()
        self.assertEqual(d.phyre2_used, True)
        self.assertEqual(len(system.software), 3)
        self.assertEqual(system.software[-1].version, '2.0')
        # Further calls should have no effect
        d.set_phyre2_used()
        self.assertEqual(len(system.software), 3)

    def test_single_state(self):
        """Test with a single state"""
        po = IMP.pmi.mmcif.ProtocolOutput()
        po._add_state(DummyRepr(None, None))
        d = ihm.dumper._MultiStateDumper()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump(po.system, w)
        self.assertEqual(fh.getvalue(), "")

    def test_multi_state(self):
        """Test with multiple states"""
        po = IMP.pmi.mmcif.ProtocolOutput()
        r1 = DummyRepr(None, None)
        state1 = po._add_state(r1)
        state1.add_model_group(ihm.model.ModelGroup(name="Group 1"))
        state1.add_model_group(ihm.model.ModelGroup(name="Group 2"))
        r2 = DummyRepr('state2 short', 'state2 long')
        state2 = po._add_state(r2)
        state2.add_model_group(ihm.model.ModelGroup(name="Group 3"))
        d = ihm.dumper._MultiStateDumper()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        ihm.dumper._ModelDumper().finalize(po.system)  # assign model group IDs
        d.finalize(po.system)
        d.dump(po.system, w)
        self.assertEqual(fh.getvalue(), """#
loop_
_ihm_multi_state_modeling.state_id
_ihm_multi_state_modeling.state_group_id
_ihm_multi_state_modeling.population_fraction
_ihm_multi_state_modeling.state_type
_ihm_multi_state_modeling.state_name
_ihm_multi_state_modeling.experiment_type
_ihm_multi_state_modeling.details
1 1 . . . 'Fraction of bulk' .
2 1 . . 'state2 long' 'Fraction of bulk' .
#
#
loop_
_ihm_multi_state_model_group_link.state_id
_ihm_multi_state_model_group_link.model_group_id
1 1
1 2
2 3
#
""")

    def test_create_component_repeat(self):
        """Test repeated calls to create_component()"""
        po = IMP.pmi.mmcif.ProtocolOutput()
        state = po._add_state(EmptyObject())
        po.create_component(state, "foo", True)
        po.add_component_sequence(state, "foo", "CCC")

        # Repeated call is OK
        po.create_component(state, "foo", True)

        # Repeated creation of sequence is OK if it's the same
        po.add_component_sequence(state, "foo", "CCC")
        # Not OK if it differs
        self.assertRaises(ValueError, po.add_component_sequence, state,
                          "foo", "AAA")

    def test_asym_units_pmi1(self):
        """Test assigning asym units and entities, PMI1 style"""
        # In PMI1 multiple copies have different component names
        po = IMP.pmi.mmcif.ProtocolOutput()
        state1 = po._add_state(EmptyObject())
        state2 = po._add_state(EmptyObject())
        for state, c, seq in ((state1, "foo", "AAA"), (state1, "bar", "AAA"),
                              (state2, "foo", "AAA"),
                              (state1, "baz", "AA")):
            po.create_component(state, c, True)
            po.add_component_sequence(state, c, seq)
        self.assertEqual(len(po.system.entities), 2)
        self.assertEqual(po.system.entities[0].description, 'foo')
        self.assertEqual(''.join(x.code for x in
                                 po.system.entities[0].sequence), 'AAA')
        self.assertEqual(''.join(x.code for x in
                                 po.system.entities[1].sequence), 'AA')
        self.assertEqual(len(po.system.asym_units), 3)
        self.assertEqual(po.system.asym_units[0].details, 'foo')
        self.assertEqual(po.system.asym_units[1].details, 'bar')
        self.assertEqual(po.system.asym_units[2].details, 'baz')

    def test_asym_units_pmi2(self):
        """Test assigning asym units and entities, PMI2 style"""
        # In PMI2 multiple copies have the same component name, different asym
        # names
        po = IMP.pmi.mmcif.ProtocolOutput()
        state1 = po._add_state(EmptyObject())
        state2 = po._add_state(EmptyObject())
        for state, c, asym, seq in ((state1, "foo", "foo.1", "AAA"),
                                    (state1, "foo", "foo.2", "AAA"),
                                    (state2, "foo", "foo.2", "AAA"),
                                    (state1, "baz", "baz.1", "AA")):
            po.create_component(state, c, True, asym_name=asym)
            po.add_component_sequence(state, c, seq, asym_name=asym)
        self.assertEqual(len(po.system.entities), 2)
        self.assertEqual(po.system.entities[0].description, 'foo')
        self.assertEqual(po.system.entities[1].description, 'baz')
        self.assertEqual(''.join(x.code for x in
                                 po.system.entities[0].sequence), 'AAA')
        self.assertEqual(''.join(x.code for x in
                                 po.system.entities[1].sequence), 'AA')
        self.assertEqual(len(po.system.asym_units), 3)
        self.assertEqual(po.system.asym_units[0].details, 'foo.1')
        self.assertEqual(po.system.asym_units[1].details, 'foo.2')
        self.assertEqual(po.system.asym_units[2].details, 'baz.1')

    def test_entity_creation(self):
        """Test creation of Entity objects"""
        m = IMP.Model()
        simo = IMP.pmi.topology.System(m)
        st = simo.create_state()
        po = IMP.pmi.mmcif.ProtocolOutput()
        simo.add_protocol_output(po)
        po.add_component_sequence(po._last_state, 'foo.1@12', 'ACGT')
        po.add_component_sequence(po._last_state, 'bar', 'ACGT')
        po.add_component_sequence(po._last_state, 'baz', 'ACCX')

        self.assertEqual(len(po.system.entities), 2)
        self.assertEqual(po.system.entities[0].description, 'foo')
        self.assertEqual(po.system.entities[1].description, 'baz')
        self.assertEqual([s.id for s in po.system.entities[1].sequence],
                         ['ALA', 'CYS', 'CYS', 'UNK'])

    def test_asym_id_mapper(self):
        """Test AsymIDMapper class"""
        m = IMP.Model()
        po = IMP.pmi.mmcif.ProtocolOutput()
        s = IMP.pmi.topology.System(m)
        s.add_protocol_output(po)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_representation(resolutions=[1])
        nup85 = st1.create_molecule("Nup85", "SELM", "Y")
        nup85.add_representation(resolutions=[1])

        st2 = s.create_state()
        nup85_2 = st2.create_molecule("Nup85", "SELM", "Z")
        nup85_2.add_representation(resolutions=[1])

        hier = s.build()

        self.assertEqual(len(po.system.asym_units), 2)
        po.system.asym_units[0]._id = 'A'
        po.system.asym_units[1]._id = 'B'

        mapper = IMP.pmi.mmcif._AsymMapper(po, st1.hier)
        h1 = IMP.atom.get_by_type(nup84.hier, IMP.atom.RESIDUE_TYPE)
        h2 = IMP.atom.get_by_type(nup85.hier, IMP.atom.RESIDUE_TYPE)
        self.assertEqual(mapper[h1[0]]._id, 'A')
        self.assertEqual(mapper[h1[1]]._id, 'A')
        self.assertEqual(mapper[h2[0]]._id, 'B')
        self.assertEqual(mapper[h2[1]]._id, 'B')

        # Check handling of multiple states
        mapper = IMP.pmi.mmcif._AsymMapper(po, st2.hier)
        # First chain, but ID isn't "A" since it gets the same chain ID
        # as the component in the first state (nup85)
        h1 = IMP.atom.get_by_type(nup85_2.hier, IMP.atom.RESIDUE_TYPE)
        self.assertEqual(mapper[h1[0]]._id, 'B')

    def test_rna_dna(self):
        """Test handling of RNA/DNA sequences"""
        m = IMP.Model()
        po = IMP.pmi.mmcif.ProtocolOutput()
        s = IMP.pmi.topology.System(m)
        s.add_protocol_output(po)
        st1 = s.create_state()
        rna = st1.create_molecule("RNA1", "ACGU", "A",
                                  alphabet=IMP.pmi.alphabets.rna)
        rna.add_representation(resolutions=[1])
        dna = st1.create_molecule("DNA1", "ACGT", "B",
                                  alphabet=IMP.pmi.alphabets.dna)
        dna.add_representation(resolutions=[1])

        hier = s.build()
        self.assertEqual(len(po.system.entities), 2)
        self.assertEqual(len(po.system.entities[0].sequence), 4)
        for r in po.system.entities[0].sequence:
            self.assertIsInstance(r, ihm.RNAChemComp)
        self.assertEqual(len(po.system.entities[1].sequence), 4)
        for r in po.system.entities[1].sequence:
            self.assertIsInstance(r, ihm.DNAChemComp)
        self.assertEqual(len(po.system.asym_units), 2)

    def test_component_mapper(self):
        """Test ComponentMapper class"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_representation(resolutions=[1])
        nup85 = st1.create_molecule("Nup85", "SELM", "Y")
        nup85.add_representation(resolutions=[1])
        hier = s.build()

        mapper = IMP.pmi.mmcif._ComponentMapper(hier)
        h1 = IMP.atom.get_by_type(nup84.hier, IMP.atom.RESIDUE_TYPE)
        h2 = IMP.atom.get_by_type(nup85.hier, IMP.atom.RESIDUE_TYPE)
        self.assertEqual(mapper[h1[0]], 'Nup84.0')
        self.assertEqual(mapper[h1[1]], 'Nup84.0')
        self.assertEqual(mapper[h2[0]], 'Nup85.0')
        self.assertEqual(mapper[h2[1]], 'Nup85.0')

    def test_cif_entities(self):
        """Test _EntityMapper class"""
        system = ihm.System()
        c = IMP.pmi.mmcif._EntityMapper(system)
        c.add('foo', 'MELS', 0, alphabet=None)
        c.add('bar', 'SELM', 0, alphabet=IMP.pmi.alphabets.amino_acid)
        c.add('foo_2', 'MELS', 0, alphabet=None)
        self.assertRaises(TypeError, c.add, 'baz', 'MELSXX', 0,
                          alphabet='garbage')
        self.assertEqual(len(system.entities), 2)
        self.assertIs(c['foo'], c['foo_2'])
        self.assertIsNot(c['foo'], c['bar'])
        a = system.entities
        self.assertEqual(len(a), 2)
        self.assertEqual(a[0].description, 'foo')
        self.assertEqual(''.join(x.code for x in a[0].sequence), 'MELS')
        self.assertEqual(a[1].description, 'bar')
        self.assertEqual(''.join(x.code for x in a[1].sequence), 'SELM')

    def test_all_datasets_all_group(self):
        """Test AllDatasets.get_all_group()"""
        s = ihm.System()
        state1 = 'state1'
        state2 = 'state2'

        alld = IMP.pmi.mmcif._AllDatasets(s)

        l = ihm.location.InputFileLocation(repo='foo', path='baz')
        ds1 = ihm.dataset.EM2DClassDataset(l)
        l = ihm.location.InputFileLocation(repo='foo', path='bar')
        ds2 = ihm.dataset.CXMSDataset(l)
        l = ihm.location.PDBLocation('1abc', '1.0', 'test details')
        ds3 = ihm.dataset.PDBDataset(l)

        g1 = alld.get_all_group(state1)

        alld.add(state1, ds1)
        alld.add(state1, ds2)
        g2 = alld.get_all_group(state1)
        g3 = alld.get_all_group(state1)

        alld.add(state1, ds3)
        g4 = alld.get_all_group(state1)
        alld.add(state2, ds3)
        g5 = alld.get_all_group(state2)

        self.assertEqual(list(g1), [])
        self.assertEqual(list(g2), [ds1, ds2])
        self.assertEqual(list(g4), [ds1, ds2, ds3])
        self.assertEqual(list(g5), [ds3])

    def test_model_dumper_sphere(self):
        """Test ModelDumper sphere_obj output"""
        m = IMP.Model()
        po = IMP.pmi.mmcif.ProtocolOutput()
        s = IMP.pmi.topology.System(m)
        s.add_protocol_output(po)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(resolutions=[1])
        hier = s.build()

        state = po._last_state

        d = ihm.dumper._ModelDumper()

        asym1 = po.asym_units['Nup84.0']
        assembly = ihm.Assembly([asym1])
        assembly._id = 42
        s1 = ihm.representation.ResidueSegment(asym1, True, 'sphere')
        representation = ihm.representation.Representation([s1])
        representation._id = 99
        protocol = ihm.protocol.Protocol()
        protocol._id = 93
        group = ihm.model.ModelGroup(name="all models")
        state.append(group)
        model = IMP.pmi.mmcif._Model(hier, po, protocol, assembly,
                                     representation)
        group.append(model)
        self.assertEqual(model.get_rmsf('Nup84.0', (1,)), None)
        fh = StringIO()
        self.assign_entity_asym_ids(po.system)
        w = ihm.format.CifWriter(fh)
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 . 42 93 99
#
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 'all models' .
#
#
loop_
_ihm_model_group_link.group_id
_ihm_model_group_link.model_id
1 1
#
#
loop_
_ihm_sphere_obj_site.id
_ihm_sphere_obj_site.entity_id
_ihm_sphere_obj_site.seq_id_begin
_ihm_sphere_obj_site.seq_id_end
_ihm_sphere_obj_site.asym_id
_ihm_sphere_obj_site.Cartn_x
_ihm_sphere_obj_site.Cartn_y
_ihm_sphere_obj_site.Cartn_z
_ihm_sphere_obj_site.object_radius
_ihm_sphere_obj_site.rmsf
_ihm_sphere_obj_site.model_id
1 1 1 1 A -8.986 11.688 -5.817 3.068 . 1
2 1 2 2 A -8.986 11.688 -5.817 2.997 . 1
3 1 3 3 A -8.986 11.688 -5.817 3.052 . 1
4 1 4 4 A -8.986 11.688 -5.817 2.609 . 1
#
""")

    def test_model_dumper_atom(self):
        """Test ModelDumper atom_site output"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        # Make atomic representation for residues 1-2, beads for 3-4
        nup84.add_representation(nup84.get_atomic_residues(), resolutions=[0])
        nup84.add_representation(nup84.get_non_atomic_residues(),
                                 resolutions=[10])
        hier = s.build()

        d = ihm.dumper._ModelDumper()
        asym = po.asym_units['Nup84.0']
        assembly = ihm.Assembly([asym])
        assembly._id = 42
        s1 = ihm.representation.AtomicSegment(asym(1,2), True)
        s2 = ihm.representation.FeatureSegment(asym(3,4), True,
                                               primitive='sphere', count=1)
        representation = ihm.representation.Representation([s1, s2])
        representation._id = 99
        protocol = ihm.protocol.Protocol()
        protocol._id = 93
        group = ihm.model.ModelGroup(name="all models")
        po.system.state_groups[0][0].append(group)
        model = IMP.pmi.mmcif._Model(hier, po, protocol, assembly,
                                     representation)
        group.append(model)
        self.assertEqual(model.get_rmsf('Nup84', (1,)), None)
        self.assign_entity_asym_ids(po.system)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 . 42 93 99
#
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 'all models' .
#
#
loop_
_ihm_model_group_link.group_id
_ihm_model_group_link.model_id
1 1
#
#
loop_
_atom_site.group_PDB
_atom_site.id
_atom_site.type_symbol
_atom_site.label_atom_id
_atom_site.label_alt_id
_atom_site.label_comp_id
_atom_site.label_seq_id
_atom_site.label_asym_id
_atom_site.Cartn_x
_atom_site.Cartn_y
_atom_site.Cartn_z
_atom_site.occupancy
_atom_site.label_entity_id
_atom_site.auth_asym_id
_atom_site.B_iso_or_equiv
_atom_site.pdbx_PDB_model_num
_atom_site.ihm_model_id
ATOM 1 . CA . MET 1 A -8.986 11.688 -5.817 . 1 A . 1 1
ATOM 2 . CA . GLU 2 A -8.986 11.688 -5.817 . 1 A . 1 1
#
#
loop_
_ihm_sphere_obj_site.id
_ihm_sphere_obj_site.entity_id
_ihm_sphere_obj_site.seq_id_begin
_ihm_sphere_obj_site.seq_id_end
_ihm_sphere_obj_site.asym_id
_ihm_sphere_obj_site.Cartn_x
_ihm_sphere_obj_site.Cartn_y
_ihm_sphere_obj_site.Cartn_z
_ihm_sphere_obj_site.object_radius
_ihm_sphere_obj_site.rmsf
_ihm_sphere_obj_site.model_id
1 1 3 4 A -8.986 11.688 -5.817 3.504 . 1
#
""")

    def test_model_dumper_sphere_rmsf(self):
        """Test ModelDumper sphere_obj output with RMSF"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(nup84.get_atomic_residues(), resolutions=[1])
        nup84.add_representation(nup84.get_non_atomic_residues(),
                                 resolutions=[10])
        hier = s.build()

        d = ihm.dumper._ModelDumper()
        asym = po.asym_units['Nup84.0']
        assembly = ihm.Assembly([asym])
        assembly._id = 42
        s = ihm.representation.FeatureSegment(asym, True, 'sphere', count=3)
        representation = ihm.representation.Representation([s])
        representation._id = 99
        protocol = ihm.protocol.Protocol()
        protocol._id = 93
        group = ihm.model.ModelGroup(name='all models')
        po.system.state_groups[0][0].append(group)
        model = IMP.pmi.mmcif._Model(hier, po, protocol, assembly,
                                     representation)
        group.append(model)
        model.name = 'foo'
        model.parse_rmsf_file(self.get_input_file_name('test.nup84.rmsf'),
                              'Nup84.0')
        self.assertAlmostEqual(model.get_rmsf('Nup84.0', (1,)), 4.5, delta=1e-4)
        self.assertRaises(ValueError, model.get_rmsf, 'Nup84.0', (1,2))
        self.assign_entity_asym_ids(po.system)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 foo 42 93 99
#
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 'all models' .
#
#
loop_
_ihm_model_group_link.group_id
_ihm_model_group_link.model_id
1 1
#
#
loop_
_ihm_sphere_obj_site.id
_ihm_sphere_obj_site.entity_id
_ihm_sphere_obj_site.seq_id_begin
_ihm_sphere_obj_site.seq_id_end
_ihm_sphere_obj_site.asym_id
_ihm_sphere_obj_site.Cartn_x
_ihm_sphere_obj_site.Cartn_y
_ihm_sphere_obj_site.Cartn_z
_ihm_sphere_obj_site.object_radius
_ihm_sphere_obj_site.rmsf
_ihm_sphere_obj_site.model_id
1 1 1 1 A -8.986 11.688 -5.817 3.068 4.500 1
2 1 2 2 A -8.986 11.688 -5.817 2.997 3.500 1
3 1 3 4 A -8.986 11.688 -5.817 3.504 5.500 1
#
""")

    def assign_dataset_ids(self, po):
        """Assign IDs to all Datasets in the system"""
        system = po.system
        d = ihm.dumper._DatasetDumper()
        d.finalize(system)

    def assign_entity_asym_ids(self, system):
        """Assign IDs to all Entities and AsymUnits in the system"""
        d = ihm.dumper._EntityDumper()
        d.finalize(system)
        d = ihm.dumper._StructAsymDumper()
        d.finalize(system)

    def assign_range_ids(self, system):
        """Assign IDs to all Entity/AsymUnit segments in the system"""
        d = ihm.dumper._EntityPolySegmentDumper()
        d.finalize(system)

    def test_starting_model_dumper(self):
        """Test StartingModelDumper"""
        m = IMP.Model()
        po = IMP.pmi.mmcif.ProtocolOutput()
        s = IMP.pmi.topology.System(m)
        s.add_protocol_output(po)
        st1 = s.create_state()

        nup84_1 = st1.create_molecule("Nup84", "MELS", "A")
        nup84_1.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84_1.add_representation(resolutions=[1])

        # Test multiple states: components that are the same in both states
        # (Nup84) should not be duplicated in the mmCIF output
        st2 = s.create_state()

        nup84_2 = st2.create_molecule("Nup84", "MELS", "A")
        nup84_2.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84_2.add_representation(resolutions=[1])

        nup85 = st2.create_molecule("Nup85", "SELM", "B")
        nup85.add_structure(self.get_input_file_name('test.nup85.pdb'), 'A',
                            res_range=(8,9), offset=-7)
        nup85.add_representation(resolutions=[1])
        hier = s.build()

        self.assign_entity_asym_ids(po.system)

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        # Assign ID (2) to referenced alignment file (id=1 is assigned to
        # the Python script)
        ihm.dumper._ExternalReferenceDumper().finalize(po.system)
        ihm.dumper._SoftwareDumper().finalize(po.system)  # assign software IDs
        self.assign_dataset_ids(po)
        self.assign_range_ids(po.system)
        d = ihm.dumper._StartingModelDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_starting_model_details.starting_model_id
_ihm_starting_model_details.entity_id
_ihm_starting_model_details.entity_description
_ihm_starting_model_details.asym_id
_ihm_starting_model_details.entity_poly_segment_id
_ihm_starting_model_details.starting_model_source
_ihm_starting_model_details.starting_model_auth_asym_id
_ihm_starting_model_details.starting_model_sequence_offset
_ihm_starting_model_details.dataset_list_id
_ihm_starting_model_details.description
1 1 Nup84 A 1 'comparative model' A 0 3 .
2 2 Nup85 B 2 'comparative model' A -7 4 .
#
#
loop_
_ihm_starting_computational_models.starting_model_id
_ihm_starting_computational_models.software_id
_ihm_starting_computational_models.script_file_id
1 3 5
2 3 .
#
#
loop_
_ihm_starting_comparative_models.id
_ihm_starting_comparative_models.starting_model_id
_ihm_starting_comparative_models.starting_model_auth_asym_id
_ihm_starting_comparative_models.starting_model_seq_id_begin
_ihm_starting_comparative_models.starting_model_seq_id_end
_ihm_starting_comparative_models.template_auth_asym_id
_ihm_starting_comparative_models.template_seq_id_begin
_ihm_starting_comparative_models.template_seq_id_end
_ihm_starting_comparative_models.template_sequence_identity
_ihm_starting_comparative_models.template_sequence_identity_denominator
_ihm_starting_comparative_models.template_dataset_list_id
_ihm_starting_comparative_models.alignment_file_id
1 1 A 33 424 C 33 424 100.000 1 1 2
2 1 A 429 488 G 482 551 10.000 1 2 2
3 2 A 26 417 C 33 424 100.000 1 1 2
4 2 A 422 481 G 482 551 10.000 1 2 2
#
#
loop_
_ihm_starting_model_coord.starting_model_id
_ihm_starting_model_coord.group_PDB
_ihm_starting_model_coord.id
_ihm_starting_model_coord.type_symbol
_ihm_starting_model_coord.atom_id
_ihm_starting_model_coord.comp_id
_ihm_starting_model_coord.entity_id
_ihm_starting_model_coord.asym_id
_ihm_starting_model_coord.seq_id
_ihm_starting_model_coord.Cartn_x
_ihm_starting_model_coord.Cartn_y
_ihm_starting_model_coord.Cartn_z
_ihm_starting_model_coord.B_iso_or_equiv
_ihm_starting_model_coord.ordinal_id
1 ATOM 1 C CA MET 1 A 1 -8.986 11.688 -5.817 91.820 1
1 ATOM 2 C CA GLU 1 A 2 -8.986 11.688 -5.817 91.820 2
2 ATOM 1 C CA SER 2 B 1 -8.986 11.688 -5.817 91.820 3
2 ATOM 2 C CA GLU 2 B 2 -7.986 18.688 -5.817 91.820 4
#
""")

    def test_protocol_dumper(self):
        """Test ModelProtocolDumper output"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(resolutions=[1])
        hier = s.build()
        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        dof.create_rigid_body(nup84)

        mc1 = IMP.pmi.macros.ReplicaExchange0(m, root_hier=hier,
                                 monte_carlo_sample_objects=dof.get_movers(),
                                 output_objects=[],
                                 test_mode=True)
        mc1.execute_macro()
        mc2 = IMP.pmi.macros.ReplicaExchange0(m, root_hier=hier,
                                 monte_carlo_sample_objects=dof.get_movers(),
                                 output_objects=[],
                                 test_mode=True)
        mc2.execute_macro()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        ihm.dumper._AssemblyDumper().finalize(po.system)  # assign assembly IDs
        ihm.dumper._SoftwareDumper().finalize(po.system)  # assign software IDs
        self.assign_dataset_ids(po)
        d = ihm.dumper._ProtocolDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_modeling_protocol.id
_ihm_modeling_protocol.protocol_name
_ihm_modeling_protocol.num_steps
1 . 2
#
#
loop_
_ihm_modeling_protocol_details.id
_ihm_modeling_protocol_details.protocol_id
_ihm_modeling_protocol_details.step_id
_ihm_modeling_protocol_details.struct_assembly_id
_ihm_modeling_protocol_details.dataset_group_id
_ihm_modeling_protocol_details.struct_assembly_description
_ihm_modeling_protocol_details.step_name
_ihm_modeling_protocol_details.step_method
_ihm_modeling_protocol_details.num_models_begin
_ihm_modeling_protocol_details.num_models_end
_ihm_modeling_protocol_details.multi_scale_flag
_ihm_modeling_protocol_details.multi_state_flag
_ihm_modeling_protocol_details.ordered_flag
_ihm_modeling_protocol_details.software_id
_ihm_modeling_protocol_details.script_file_id
_ihm_modeling_protocol_details.description
1 1 1 2 1 'All components modeled by IMP in state State_0' Sampling
'Replica exchange monte carlo' 0 1000 YES NO NO 1 . .
2 1 2 2 1 'All components modeled by IMP in state State_0' Sampling
'Replica exchange monte carlo' 1000 1000 YES NO NO 1 . .
#
""")

    def test_rex_protocol_dumper(self):
        """Test IMP-specific replica exchange dumper output"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(resolutions=[1])
        hier = s.build()
        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        dof.create_rigid_body(nup84)

        mc1 = IMP.pmi.macros.ReplicaExchange0(m, root_hier=hier,
                                 monte_carlo_sample_objects=dof.get_movers(),
                                 output_objects=[],
                                 test_mode=True,
                                 monte_carlo_temperature=42.0,
                                 replica_exchange_minimum_temperature=100.,
                                 replica_exchange_maximum_temperature=200.)
        mc1.execute_macro()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        ihm.dumper._ProtocolDumper().finalize(po.system) # assign step IDs
        d = IMP.pmi.mmcif._ReplicaExchangeProtocolDumper()
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_imp_replica_exchange_protocol.protocol_id
_imp_replica_exchange_protocol.step_id
_imp_replica_exchange_protocol.monte_carlo_temperature
_imp_replica_exchange_protocol.replica_exchange_minimum_temperature
_imp_replica_exchange_protocol.replica_exchange_maximum_temperature
1 1 42.000 100.000 200.000
#
""")

    def test_add_simple_dynamics(self):
        """Test add_simple_dynamics()"""
        po = IMP.pmi.mmcif.ProtocolOutput()
        po._add_state(EmptyObject())
        po._add_simple_dynamics(100, "Brownian dynamics")

    def test_simple_postprocessing(self):
        """Test add_simple_postprocessing"""
        class DummyProtocolStep(object):
            pass
        po = IMP.pmi.mmcif.ProtocolOutput()
        po._add_state(DummyRepr(None, None))
        p = DummyProtocolStep()
        p.num_models_end = 10
        po.all_protocols.add_step(p, po._last_state)
        pp = po._add_simple_postprocessing(10, 90)
        self.assertEqual(pp.type, 'cluster')
        self.assertEqual(pp.feature, 'RMSD')
        self.assertEqual(pp.num_models_begin, 10)
        self.assertEqual(pp.num_models_end, 90)
        po._add_simple_postprocessing(12, 90)

        # Start a new protocol
        po._add_protocol()
        po._add_simple_postprocessing(34, 56)

        # Add protocol and postprocessing for a second state
        po._add_state(DummyRepr(None, None))
        p = DummyProtocolStep()
        p.num_models_end = 10
        po.all_protocols.add_step(p, po._last_state)
        po._add_simple_postprocessing(20, 80)

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        # Assign protocol IDs
        ihm.dumper._ProtocolDumper().finalize(po.system)
        d = ihm.dumper._PostProcessDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_modeling_post_process.id
_ihm_modeling_post_process.protocol_id
_ihm_modeling_post_process.analysis_id
_ihm_modeling_post_process.step_id
_ihm_modeling_post_process.type
_ihm_modeling_post_process.feature
_ihm_modeling_post_process.num_models_begin
_ihm_modeling_post_process.num_models_end
_ihm_modeling_post_process.struct_assembly_id
_ihm_modeling_post_process.dataset_group_id
_ihm_modeling_post_process.software_id
_ihm_modeling_post_process.script_file_id
_ihm_modeling_post_process.details
1 1 1 1 cluster RMSD 10 90 . . . . .
2 1 1 2 cluster RMSD 12 90 . . . . .
3 2 1 1 cluster RMSD 34 56 . . . . .
4 3 1 1 cluster RMSD 20 80 . . . . .
#
""")

    def test_no_postprocessing(self):
        """Test add_no_postprocessing"""
        class DummyProtocolStep(object):
            pass
        po = IMP.pmi.mmcif.ProtocolOutput()
        po._add_state(DummyRepr(None, None))
        p = DummyProtocolStep()
        p.num_models_end = 10
        po.all_protocols.add_step(p, po._last_state)
        pp = po._add_no_postprocessing(10)
        self.assertEqual(pp.type, 'none')
        self.assertEqual(pp.feature, 'none')
        self.assertEqual(pp.num_models_begin, 10)
        self.assertEqual(pp.num_models_end, 10)

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        # Assign protocol IDs
        ihm.dumper._ProtocolDumper().finalize(po.system)
        d = ihm.dumper._PostProcessDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_modeling_post_process.id
_ihm_modeling_post_process.protocol_id
_ihm_modeling_post_process.analysis_id
_ihm_modeling_post_process.step_id
_ihm_modeling_post_process.type
_ihm_modeling_post_process.feature
_ihm_modeling_post_process.num_models_begin
_ihm_modeling_post_process.num_models_end
_ihm_modeling_post_process.struct_assembly_id
_ihm_modeling_post_process.dataset_group_id
_ihm_modeling_post_process.software_id
_ihm_modeling_post_process.script_file_id
_ihm_modeling_post_process.details
1 1 1 1 none none 10 10 . . . . .
#
""")

    def test_simple_ensemble(self):
        """Test add_simple_ensemble"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_representation(resolutions=[1])
        nup85 = st1.create_molecule("Nup85", "SELM", "Y")
        nup85.add_representation(resolutions=[1])
        hier = s.build()

        densities = {'Nup84.0': "foo.mrc"}
        pp = None
        e = po._add_simple_ensemble(pp, 'Ensemble 1', 5, 0.1, 1, densities,
                                    None)
        self.assertEqual(e.num_models, 5)
        self.assertEqual(e.num_models_deposited, 1)

    def test_rex_postproces(self):
        """Test ReplicaExchangeAnalysisPostProcess"""
        class DummyRex(object):
            _number_of_clusters = 2
        d = DummyRex()
        with IMP.test.temporary_directory() as tmpdir:
            d._outputdir = tmpdir
            for i in range(d._number_of_clusters):
                subdir = os.path.join(tmpdir, 'cluster.%d' % i)
                os.mkdir(subdir)
                with open(os.path.join(subdir, 'stat.out'), 'w') as fh:
                    # 1 model for first cluster, 2 for second cluster
                    for line in range(i + 1):
                        fh.write('#\n')
            pp = IMP.pmi.mmcif._ReplicaExchangeAnalysisPostProcess(d, 45)
        self.assertEqual(pp.rex, d)
        self.assertEqual(pp.num_models_begin, 45)
        self.assertEqual(pp.num_models_end, 3)

    def test_rex_ensemble(self):
        """Test ReplicaExchangeAnalysisEnsemble"""
        class DummyModel(object):
            def parse_rmsf_file(self, fname, comp):
                self.comp = comp
        class DummyRepresentation(object):
            def set_coordinates_from_rmf(self, comp, fname, frame,
                                         force_rigid_update):
                pass
        class DummySimo(object):
            all_modeled_components = ['Nup84', 'Nup85']
        class DummyState(object):
            all_modeled_components = ['Nup84', 'Nup85']
        class DummyRex(object):
            _number_of_clusters = 1
        class DummyGroup(object):
            name = 'dgroup'
        comp_to_asym = {'Nup84':None}
        with IMP.test.temporary_directory() as tmpdir:
            d = DummyRex()
            d._outputdir = tmpdir
            subdir = os.path.join(tmpdir, 'cluster.0')
            os.mkdir(subdir)
            # Two models
            with open(os.path.join(subdir, 'stat.out'), 'w') as fh:
                fh.write("{'modelnum': 0}\n")
                fh.write("{'modelnum': 1}\n")
            # Mock localization density file
            with open(os.path.join(subdir, 'Nup84.mrc'), 'w') as fh:
                pass
            # Mock RMSF file
            with open(os.path.join(subdir, 'rmsf.Nup84.dat'), 'w') as fh:
                pass
            pp = IMP.pmi.mmcif._ReplicaExchangeAnalysisPostProcess(d, 45)
            mg = DummyGroup()
            e = IMP.pmi.mmcif._ReplicaExchangeAnalysisEnsemble(pp, 0, mg, 1)
            self.assertEqual(e.cluster_num, 0)
            self.assertEqual(e.post_process, pp)
            self.assertEqual(e.num_models_deposited, 1)
            self.assertEqual(e.densities, [])
            self.assertEqual(e.num_models, 2)
            self.assertEqual(e.clustering_feature, 'RMSD')
            self.assertEqual(e.name, 'dgroup')
            self.assertEqual(e.get_rmsf_file('Nup84'),
                             os.path.join(tmpdir, 'cluster.0',
                                          'rmsf.Nup84.dat'))
            # RMSF that doesn't exist
            e.load_rmsf(None, 'normsf')
            # RMSF that does exist
            dm = DummyModel()
            e.load_rmsf(dm, 'Nup84')
            self.assertEqual(dm.comp, 'Nup84')
            self.assertEqual(e.get_localization_density_file('Nup84'),
                             os.path.join(tmpdir, 'cluster.0', 'Nup84.mrc'))
            self.assertEqual(len(e.densities), 0)
            # Density that doesn't exist
            e.load_localization_density(None, 'noden', [], comp_to_asym)
            self.assertEqual(len(e.densities), 0)
            # Density that does exist
            po = IMP.pmi.mmcif.ProtocolOutput()
            r = DummyRepr('dummy', 'none')
            state = po._add_state(r)
            e.load_localization_density(state, 'Nup84', ['Nup84'], comp_to_asym)
            self.assertEqual(e.densities[0].file.path,
                             os.path.join(tmpdir, 'cluster.0', 'Nup84.mrc'))
            self.assertEqual(e.densities[0].file.details,
                         'Localization density for Nup84 dgroup')
            # No precision available
            self.assertEqual(e._get_precision(), ihm.unknown)
            self.assertEqual(e.precision, ihm.unknown)
            # Make precision available
            with open(os.path.join(tmpdir, 'precision.0.0.out'), 'w') as fh:
                fh.write("""
All kmeans_weight_500_2/cluster.0/ average centroid distance 24.3744728893
All kmeans_weight_500_2/cluster.0/ centroid index 49
""")
            self.assertAlmostEqual(e._get_precision(), 24.374, delta=1e-3)
            ds = DummySimo()
            state = DummyState()
            state._pmi_object = DummyRepresentation()
            stats = list(e.load_all_models(ds, state))
            self.assertEqual(stats, [{'modelnum': 0}])

    def test_add_rex(self):
        """Test add_replica_exchange_analysis"""
        class DummyProtocolStep(object):
            pass
        class DummyRex(object):
            _number_of_clusters = 1
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st = s.create_state()
        nup84 = st.create_molecule("Nup84", "MELS", "X")
        nup84.add_representation(resolutions=[1])
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        po_state = st._protocol_output[0][1]
        hier = s.build()

        with IMP.test.temporary_directory() as tmpdir:
            rex = DummyRex()
            rex._outputdir = tmpdir
            subdir = os.path.join(tmpdir, 'cluster.0')
            os.mkdir(subdir)
            # Two models
            with open(os.path.join(subdir, 'stat.out'), 'w') as fh:
                fh.write("{'modelnum': 0}\n")
                fh.write("{'modelnum': 1}\n")
            prot = DummyProtocolStep()
            prot.num_models_end = 10
            po.all_protocols.add_step(prot, po_state)
            po.add_replica_exchange_analysis(po_state, rex, {})

    def test_ensemble_dumper(self):
        """Test dumping of simple ensembles"""
        class DummyPostProcess(object):
            pass
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        st1 = s.create_state()

        pp = DummyPostProcess()
        pp._id = 99
        e1 = po._add_simple_ensemble(pp, 'Ensemble 1', 5, 0.1, 1,
                                     {}, None)
        e2 = po._add_simple_ensemble(pp, 'Ensemble 2', 5, 0.1, 1,
                                     {}, None)
        loc = ihm.location.InputFileLocation(repo='foo', path='bar')
        po.set_ensemble_file(1, loc)
        loc._id = 42
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        ihm.dumper._ModelDumper().finalize(po.system)  # assign model group IDs
        d = ihm.dumper._EnsembleDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_ensemble_info.ensemble_id
_ihm_ensemble_info.ensemble_name
_ihm_ensemble_info.post_process_id
_ihm_ensemble_info.model_group_id
_ihm_ensemble_info.ensemble_clustering_method
_ihm_ensemble_info.ensemble_clustering_feature
_ihm_ensemble_info.num_ensemble_models
_ihm_ensemble_info.num_ensemble_models_deposited
_ihm_ensemble_info.ensemble_precision_value
_ihm_ensemble_info.ensemble_file_id
_ihm_ensemble_info.details
_ihm_ensemble_info.sub_sample_flag
_ihm_ensemble_info.sub_sampling_type
1 'Ensemble 1 in state State_0' 99 1 . dRMSD 5 1 0.100 . . NO .
2 'Ensemble 2 in state State_0' 99 2 . dRMSD 5 1 0.100 42 . NO .
#
""")

    def test_density_dumper(self):
        """Test DensityDumper"""
        class DummyEnsemble(object):
            pass

        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_representation(resolutions=[1])
        hier = s.build()

        ensemble = DummyEnsemble()
        ensemble._id = 42
        loc = ihm.location.OutputFileLocation(repo='foo', path='bar')
        loc._id = 97
        den = ihm.model.LocalizationDensity(file=loc,
                                asym_unit=po.asym_units['Nup84.0'])
        ensemble.densities = [den]
        po.system.ensembles.append(ensemble)

        fh = StringIO()
        self.assign_entity_asym_ids(po.system)
        self.assign_range_ids(po.system)
        w = ihm.format.CifWriter(fh)
        d = ihm.dumper._DensityDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_localization_density_files.id
_ihm_localization_density_files.file_id
_ihm_localization_density_files.ensemble_id
_ihm_localization_density_files.entity_id
_ihm_localization_density_files.asym_id
_ihm_localization_density_files.entity_poly_segment_id
1 97 42 1 A 1
#
""")

    def test_cross_link_dumper(self):
        """Test the CrossLinkDumper"""
        class DummyDataset(object):
            pass
        class DummyRestraint(object):
            label = 'foo'
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        po_state = state._protocol_output[0][1]
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(nup84.get_atomic_residues(), resolutions=[1])
        nup84.add_representation(nup84.get_non_atomic_residues(),
                                 resolutions=[10])
        hier = s.build()

        r = DummyRestraint()
        r.dataset = DummyDataset()
        r.dataset._id = 42
        xl_group = po.get_cross_link_group(r)
        ex_xl = po.add_experimental_cross_link(1, 'Nup84',
                                               2, 'Nup84', xl_group)
        ex_xl2 = po.add_experimental_cross_link(1, 'Nup84',
                                                3, 'Nup84', xl_group)
        # Duplicates should be ignored
        po.add_experimental_cross_link(1, 'Nup84', 3, 'Nup84', xl_group)
        # Non-modeled component should be ignored
        nm_ex_xl = po.add_experimental_cross_link(1, 'Nup85',
                                                  2, 'Nup84', xl_group)
        self.assertEqual(nm_ex_xl, None)
        sel = IMP.atom.Selection(hier, molecule='Nup84',
                                 resolution=1)
        rs = sel.get_selected_particles()
        sigma1 = IMP.isd.Scale.setup_particle(IMP.Particle(m), 1.0)
        sigma2 = IMP.isd.Scale.setup_particle(IMP.Particle(m), 0.5)
        psi = IMP.isd.Scale.setup_particle(IMP.Particle(m), 0.8)
        po.add_cross_link(po_state, ex_xl, rs[0], rs[1], 42.0, sigma1, sigma2,
                          psi, xl_group)
        # Duplicates should be ignored
        po.add_cross_link(po_state, ex_xl, rs[0], rs[1], 42.0, sigma1, sigma2,
                          psi, xl_group)

        fh = StringIO()
        self.assign_entity_asym_ids(po.system)
        d = ihm.dumper._ChemDescriptorDumper()
        d.finalize(po.system) # Assign chemical descriptor IDs
        w = ihm.format.CifWriter(fh)
        d = ihm.dumper._CrossLinkDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_cross_link_list.id
_ihm_cross_link_list.group_id
_ihm_cross_link_list.entity_description_1
_ihm_cross_link_list.entity_id_1
_ihm_cross_link_list.seq_id_1
_ihm_cross_link_list.comp_id_1
_ihm_cross_link_list.entity_description_2
_ihm_cross_link_list.entity_id_2
_ihm_cross_link_list.seq_id_2
_ihm_cross_link_list.comp_id_2
_ihm_cross_link_list.linker_chem_comp_descriptor_id
_ihm_cross_link_list.linker_type
_ihm_cross_link_list.dataset_list_id
_ihm_cross_link_list.details
1 1 Nup84 1 1 MET Nup84 1 2 GLU 1 foo 42 .
2 2 Nup84 1 1 MET Nup84 1 3 LEU 1 foo 42 .
#
#
loop_
_ihm_cross_link_restraint.id
_ihm_cross_link_restraint.group_id
_ihm_cross_link_restraint.entity_id_1
_ihm_cross_link_restraint.asym_id_1
_ihm_cross_link_restraint.seq_id_1
_ihm_cross_link_restraint.comp_id_1
_ihm_cross_link_restraint.entity_id_2
_ihm_cross_link_restraint.asym_id_2
_ihm_cross_link_restraint.seq_id_2
_ihm_cross_link_restraint.comp_id_2
_ihm_cross_link_restraint.atom_id_1
_ihm_cross_link_restraint.atom_id_2
_ihm_cross_link_restraint.restraint_type
_ihm_cross_link_restraint.conditional_crosslink_flag
_ihm_cross_link_restraint.model_granularity
_ihm_cross_link_restraint.distance_threshold
_ihm_cross_link_restraint.psi
_ihm_cross_link_restraint.sigma_1
_ihm_cross_link_restraint.sigma_2
_ihm_cross_link_restraint.pseudo_site_flag
1 1 1 A 1 MET 1 A 2 GLU . . 'upper bound' ALL by-residue 42.000 0.800 1.000
0.500 NO
#
""")

    def test_add_em2d_restraint(self):
        """Test add_em2d_restraint method"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        st1 = s.create_state()
        state = st1._protocol_output[0][1]
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_representation(resolutions=[1])
        hier = s.build()

        class DummyRestraint(object):
            label = 'foo'
        class DummyProtocolStep(object):
            pass
        pr = DummyRestraint()
        pr.datasets = [None]
        po.add_em2d_restraint(state, pr, 0,
                              resolution=10.0, pixel_size=4.2,
                              image_resolution=1.0,
                              projection_number=200,
                              micrographs_number=50)
        lp = ihm.location.InputFileLocation(repo='foo', path='baz')
        dp = ihm.dataset.EMMicrographsDataset(lp)
        l = ihm.location.InputFileLocation(repo='foo', path='bar')
        d = ihm.dataset.EM2DClassDataset(l)
        d._id = 4
        d.parents.append(dp)
        pr.datasets[0] = d
        p = DummyProtocolStep()
        p.assembly = None
        po.all_protocols.add_step(p, po._last_state)
        group = get_all_models_group(st1, po)
        m = po.add_model(group)
        m._id = 9
        prefix = 'ElectronMicroscopy2D_foo_Image1_'
        m.stats = {prefix + 'CCC': '0.872880665234',
                   prefix + 'Translation0': '304.187464117',
                   prefix + 'Translation1': '219.585852373',
                   prefix + 'Translation2': '0.0',
                   prefix + 'Rotation0': '0.443696289233',
                   prefix + 'Rotation1': '0.316041672423',
                   prefix + 'Rotation2': '-0.419293315413',
                   prefix + 'Rotation3': '-0.726253660826'}
        po._add_restraint_model_fits()

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        ihm.dumper._AssemblyDumper().finalize(po.system)  # assign assembly IDs
        d = ihm.dumper._EM2DDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_2dem_class_average_restraint.id
_ihm_2dem_class_average_restraint.dataset_list_id
_ihm_2dem_class_average_restraint.number_raw_micrographs
_ihm_2dem_class_average_restraint.pixel_size_width
_ihm_2dem_class_average_restraint.pixel_size_height
_ihm_2dem_class_average_restraint.image_resolution
_ihm_2dem_class_average_restraint.image_segment_flag
_ihm_2dem_class_average_restraint.number_of_projections
_ihm_2dem_class_average_restraint.struct_assembly_id
_ihm_2dem_class_average_restraint.details
1 4 50 4.200 4.200 1.000 NO 200 2 .
#
#
loop_
_ihm_2dem_class_average_fitting.id
_ihm_2dem_class_average_fitting.restraint_id
_ihm_2dem_class_average_fitting.model_id
_ihm_2dem_class_average_fitting.cross_correlation_coefficient
_ihm_2dem_class_average_fitting.rot_matrix[1][1]
_ihm_2dem_class_average_fitting.rot_matrix[2][1]
_ihm_2dem_class_average_fitting.rot_matrix[3][1]
_ihm_2dem_class_average_fitting.rot_matrix[1][2]
_ihm_2dem_class_average_fitting.rot_matrix[2][2]
_ihm_2dem_class_average_fitting.rot_matrix[3][2]
_ihm_2dem_class_average_fitting.rot_matrix[1][3]
_ihm_2dem_class_average_fitting.rot_matrix[2][3]
_ihm_2dem_class_average_fitting.rot_matrix[3][3]
_ihm_2dem_class_average_fitting.tr_vector[1]
_ihm_2dem_class_average_fitting.tr_vector[2]
_ihm_2dem_class_average_fitting.tr_vector[3]
1 1 9 0.873 -0.406503 -0.909500 -0.086975 0.379444 -0.254653 0.889480 -0.831131
0.328574 0.448622 304.187 219.586 0.000
#
""")

    def test_add_zaxial_restraint(self):
        """Test add_zaxial_restraint method"""
        self._check_geom_restraint('add_zaxial_restraint', """#
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
1 plane . .
#
#
loop_
_ihm_geometric_object_plane.object_id
_ihm_geometric_object_plane.plane_type
_ihm_geometric_object_plane.transformation_id
1 xy-plane .
#
""")

    def test_add_yaxial_restraint(self):
        """Test add_yaxial_restraint method"""
        self._check_geom_restraint('add_yaxial_restraint', """#
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
1 plane . .
#
#
loop_
_ihm_geometric_object_plane.object_id
_ihm_geometric_object_plane.plane_type
_ihm_geometric_object_plane.transformation_id
1 xz-plane .
#
""")

    def test_add_xyradial_restraint(self):
        """Test add_xyradial_restraint method"""
        self._check_geom_restraint('add_xyradial_restraint', """#
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
1 axis . .
#
#
loop_
_ihm_geometric_object_axis.object_id
_ihm_geometric_object_axis.axis_type
_ihm_geometric_object_axis.transformation_id
1 z-axis .
#
""")

    def _check_geom_restraint(self, method_name, expected_output):
        class MockObject(object):
            pass
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(nup84, resolutions=[1])
        hier = s.build()
        po_state = po._last_state

        residues = IMP.atom.Selection(hier, molecule='Nup84',
                                      resolution=1).get_selected_particles()
        pmi_r = MockObject()
        pmi_r.dataset = None
        method = getattr(po, method_name)
        method(po_state, residues, lower_bound=4.0,
               upper_bound=8.0, sigma=2.0, pmi_restraint=pmi_r)
        # duplicate restraint should use the same feature
        method(po_state, residues, lower_bound=4.0,
               upper_bound=8.0, sigma=2.0, pmi_restraint=pmi_r)
        self.assertRaises(ValueError, method,
                          po_state, [hier], lower_bound=4.0,
                          upper_bound=8.0, sigma=2.0, pmi_restraint=pmi_r)
        self.assign_entity_asym_ids(po.system)
        d = ihm.dumper._GeometricObjectDumper()
        d.finalize(po.system)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, expected_output)
        d = ihm.dumper._FeatureDumper()
        d.finalize(po.system)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_feature_list.feature_id
_ihm_feature_list.feature_type
_ihm_feature_list.entity_type
_ihm_feature_list.details
1 'residue range' polymer .
#
#
loop_
_ihm_poly_residue_feature.ordinal_id
_ihm_poly_residue_feature.feature_id
_ihm_poly_residue_feature.entity_id
_ihm_poly_residue_feature.asym_id
_ihm_poly_residue_feature.seq_id_begin
_ihm_poly_residue_feature.comp_id_begin
_ihm_poly_residue_feature.seq_id_end
_ihm_poly_residue_feature.comp_id_end
1 1 1 A 1 MET 4 SER
#
""")
        d = ihm.dumper._GeometricRestraintDumper()
        d.finalize(po.system)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_geometric_object_distance_restraint.id
_ihm_geometric_object_distance_restraint.object_id
_ihm_geometric_object_distance_restraint.feature_id
_ihm_geometric_object_distance_restraint.object_characteristic
_ihm_geometric_object_distance_restraint.restraint_type
_ihm_geometric_object_distance_restraint.harmonic_force_constant
_ihm_geometric_object_distance_restraint.distance_lower_limit
_ihm_geometric_object_distance_restraint.distance_upper_limit
_ihm_geometric_object_distance_restraint.group_conditionality
_ihm_geometric_object_distance_restraint.dataset_list_id
1 1 1 other 'lower and upper bound' 0.500 4.000 8.000 ALL .
2 1 1 other 'lower and upper bound' 0.500 4.000 8.000 ALL .
#
""")

    def test_get_membrane(self):
        """Test _get_membrane() method"""
        po = IMP.pmi.mmcif.ProtocolOutput()
        m1 = po._get_membrane(1.0, 2.0, 3.0)
        m2 = po._get_membrane(1.000001, 2.0, 3.0)
        m3 = po._get_membrane(2.0, 2.0, 3.0)
        self.assertEqual(id(m1), id(m2))
        self.assertNotEqual(id(m1), id(m3))
        self.assertAlmostEqual(m1.major_radius, 1.0, places=1)
        self.assertAlmostEqual(m1.minor_radius, 2.0, places=1)

    def test_add_membrane_surface_restraint(self):
        """Test add_membrane_surface_restraint method"""
        self._check_membrane_restraint(
                   'add_membrane_surface_location_restraint',
                   "'upper bound' 0.500 . 0.000")

    def test_add_membrane_exclusion_restraint(self):
        """Test add_membrane_exclusion_restraint method"""
        self._check_membrane_restraint(
                   'add_membrane_exclusion_restraint',
                   "'lower bound' 0.500 0.000 .")

    def _check_membrane_restraint(self, method_name, expected_dist_rsr):
        class MockObject(object):
            pass
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(nup84, resolutions=[1])
        hier = s.build()
        po_state = po._last_state

        residues = IMP.atom.Selection(hier, molecule='Nup84',
                                      resolution=1).get_selected_particles()
        nterm = residues[:1]
        pmi_r = MockObject()
        pmi_r.dataset = None
        method = getattr(po, method_name)
        method(po_state, nterm, tor_R=4.0, tor_r=1.0, tor_th=0.1, sigma=2.0,
               pmi_restraint=pmi_r)
        self.assign_entity_asym_ids(po.system)
        d = ihm.dumper._GeometricObjectDumper()
        d.finalize(po.system)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_geometric_object_center.id
_ihm_geometric_object_center.xcoord
_ihm_geometric_object_center.ycoord
_ihm_geometric_object_center.zcoord
1 0 0 0
#
#
loop_
_ihm_geometric_object_transformation.id
_ihm_geometric_object_transformation.rot_matrix[1][1]
_ihm_geometric_object_transformation.rot_matrix[2][1]
_ihm_geometric_object_transformation.rot_matrix[3][1]
_ihm_geometric_object_transformation.rot_matrix[1][2]
_ihm_geometric_object_transformation.rot_matrix[2][2]
_ihm_geometric_object_transformation.rot_matrix[3][2]
_ihm_geometric_object_transformation.rot_matrix[1][3]
_ihm_geometric_object_transformation.rot_matrix[2][3]
_ihm_geometric_object_transformation.rot_matrix[3][3]
_ihm_geometric_object_transformation.tr_vector[1]
_ihm_geometric_object_transformation.tr_vector[2]
_ihm_geometric_object_transformation.tr_vector[3]
1 1.000000 0.000000 0.000000 0.000000 1.000000 0.000000 0.000000 0.000000
1.000000 0.000 0.000 0.000
#
#
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
1 half-torus Membrane .
#
#
loop_
_ihm_geometric_object_torus.object_id
_ihm_geometric_object_torus.center_id
_ihm_geometric_object_torus.transformation_id
_ihm_geometric_object_torus.major_radius_R
_ihm_geometric_object_torus.minor_radius_r
1 1 1 4.000 1.000
#
#
loop_
_ihm_geometric_object_half_torus.object_id
_ihm_geometric_object_half_torus.thickness_th
_ihm_geometric_object_half_torus.section
1 0.100 'inner half'
#
""")

        d = ihm.dumper._FeatureDumper()
        d.finalize(po.system)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_feature_list.feature_id
_ihm_feature_list.feature_type
_ihm_feature_list.entity_type
_ihm_feature_list.details
1 residue polymer .
#
#
loop_
_ihm_poly_residue_feature.ordinal_id
_ihm_poly_residue_feature.feature_id
_ihm_poly_residue_feature.entity_id
_ihm_poly_residue_feature.asym_id
_ihm_poly_residue_feature.seq_id_begin
_ihm_poly_residue_feature.comp_id_begin
_ihm_poly_residue_feature.seq_id_end
_ihm_poly_residue_feature.comp_id_end
1 1 1 A 1 MET 1 MET
#
""")

        d = ihm.dumper._GeometricRestraintDumper()
        d.finalize(po.system)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_geometric_object_distance_restraint.id
_ihm_geometric_object_distance_restraint.object_id
_ihm_geometric_object_distance_restraint.feature_id
_ihm_geometric_object_distance_restraint.object_characteristic
_ihm_geometric_object_distance_restraint.restraint_type
_ihm_geometric_object_distance_restraint.harmonic_force_constant
_ihm_geometric_object_distance_restraint.distance_lower_limit
_ihm_geometric_object_distance_restraint.distance_upper_limit
_ihm_geometric_object_distance_restraint.group_conditionality
_ihm_geometric_object_distance_restraint.dataset_list_id
1 1 1 other %s ALL .
#
""" % expected_dist_rsr)

    def test_sas_dumper(self):
        """Test SASDumper class"""
        class DummyModel(object):
            pass
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        st1 = s.create_state()
        state = st1._protocol_output[0][1]
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_representation(resolutions=[1])
        hier = s.build()

        lp = ihm.location.InputFileLocation(repo='foo', path='baz')
        d = ihm.dataset.SASDataset(lp)
        d._id = 4
        model = DummyModel()
        model._id = 42
        po._add_foxs_restraint(model, 'Nup84.0', (2,3), d, 3.4, 1.2, 'test')

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        ihm.dumper._AssemblyDumper().finalize(po.system)  # assign assembly IDs
        dumper = ihm.dumper._SASDumper()
        dumper.finalize(po.system)
        dumper.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_sas_restraint.id
_ihm_sas_restraint.dataset_list_id
_ihm_sas_restraint.model_id
_ihm_sas_restraint.struct_assembly_id
_ihm_sas_restraint.profile_segment_flag
_ihm_sas_restraint.fitting_atom_type
_ihm_sas_restraint.fitting_method
_ihm_sas_restraint.fitting_state
_ihm_sas_restraint.radius_of_gyration
_ihm_sas_restraint.chi_value
_ihm_sas_restraint.details
1 4 42 3 NO 'Heavy atoms' FoXS Single 3.400 1.200 test
#
""")

    def test_add_em3d_restraint(self):
        """Test add_em3d_restraint method"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        st1 = s.create_state()
        state = st1._protocol_output[0][1]
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_representation(resolutions=[1])
        hier = s.build()

        p = IMP.atom.get_by_type(hier, IMP.atom.FRAGMENT_TYPE)[0]
        class DummyRestraint(object):
            label = 'foo'
        class DummyProtocolStep(object):
            assembly = None
        pr = DummyRestraint()
        pr.dataset = None
        po.add_em3d_restraint(state, pmi_restraint=pr,
                              target_ps=[None, None], densities=[p])

        l = ihm.location.InputFileLocation(repo='foo', path='bar')
        d = ihm.dataset.EMDensityDataset(l)
        d._id = 4
        pr.dataset = d

        p = DummyProtocolStep()
        po.all_protocols.add_step(p, po._last_state)
        group = get_all_models_group(st1, po)
        m = po.add_model(group)
        m._id = 5
        m.stats = {'GaussianEMRestraint_foo_CCC': 0.1}
        m = po.add_model(group)
        m._id = 9
        m.stats = {'GaussianEMRestraint_foo_CCC': 0.2}
        po._add_restraint_model_fits()

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        ihm.dumper._AssemblyDumper().finalize(po.system)  # assign assembly IDs
        d = ihm.dumper._EM3DDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_3dem_restraint.id
_ihm_3dem_restraint.dataset_list_id
_ihm_3dem_restraint.fitting_method
_ihm_3dem_restraint.fitting_method_citation_id
_ihm_3dem_restraint.struct_assembly_id
_ihm_3dem_restraint.number_of_gaussians
_ihm_3dem_restraint.model_id
_ihm_3dem_restraint.cross_correlation_coefficient
1 4 'Gaussian mixture models' . 2 2 5 0.100
2 4 'Gaussian mixture models' . 2 2 9 0.200
#
""")

    def test_dump_atoms_restype_mismatch(self):
        """Test StartingModelDumper.dump_atoms with residue type mismatch"""
        m = IMP.Model()
        po = IMP.pmi.mmcif.ProtocolOutput()
        s = IMP.pmi.topology.System(m)
        s.add_protocol_output(po)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(resolutions=[1])
        hier = s.build()

        # Create sequence mismatch
        po.system.entities[0].sequence = po.system.entities[0].sequence[1:]
        self.assign_entity_asym_ids(po.system)
        d = ihm.dumper._StartingModelDumper()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.finalize(po.system)
        d.dump_coords(po.system, w) # Needed to populate _seq_dif
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump_seq_dif(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_starting_model_seq_dif.id
_ihm_starting_model_seq_dif.entity_id
_ihm_starting_model_seq_dif.asym_id
_ihm_starting_model_seq_dif.seq_id
_ihm_starting_model_seq_dif.comp_id
_ihm_starting_model_seq_dif.starting_model_id
_ihm_starting_model_seq_dif.db_asym_id
_ihm_starting_model_seq_dif.db_seq_id
_ihm_starting_model_seq_dif.db_comp_id
_ihm_starting_model_seq_dif.details
1 1 A 1 GLU 1 A 1 MET 'Mutation of MET to GLU'
2 1 A 2 LEU 1 A 2 GLU 'Mutation of GLU to LEU'
#
""")

    def test_seq_dif(self):
        """Test StartingModelDumper.dump_seq_dif"""

        class MockStartingModel(ihm.startmodel.StartingModel):
            def get_seq_dif(self):
                return [ihm.startmodel.MSESeqDif(db_seq_id=40, seq_id=42)]
        s = ihm.System()
        e = ihm.Entity('A' * 41 + 'M')
        e._id = 4
        asym = ihm.AsymUnit(e)
        asym._id = 'H'
        s.entities.append(e)
        s.asym_units.append(asym)
        sm = MockStartingModel(asym_unit=asym, dataset=None, asym_id='X',
                               offset=2)
        s.orphan_starting_models.append(sm)
        sm._id = 'dummy-m1'

        d = ihm.dumper._StartingModelDumper()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump_seq_dif(s, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_starting_model_seq_dif.id
_ihm_starting_model_seq_dif.entity_id
_ihm_starting_model_seq_dif.asym_id
_ihm_starting_model_seq_dif.seq_id
_ihm_starting_model_seq_dif.comp_id
_ihm_starting_model_seq_dif.starting_model_id
_ihm_starting_model_seq_dif.db_asym_id
_ihm_starting_model_seq_dif.db_seq_id
_ihm_starting_model_seq_dif.db_comp_id
_ihm_starting_model_seq_dif.details
1 4 H 42 MET dummy-m1 X 40 MSE 'Conversion of modified residue MSE to MET'
#
""")

    def test_beads_fragment(self):
        """Test _BeadsFragment class"""
        system = ihm.System()
        e = ihm.Entity('A' * 40)
        asym = ihm.AsymUnit(e)
        m = None
        bf1 = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=0,
                                           end=10, count=2, hier=None,
                                           asym_unit=asym)
        bf2 = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=11,
                                           end=30, count=3, hier=None,
                                           asym_unit=asym)
        bf3 = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=31,
                                           end=50, count=4, hier=None,
                                           asym_unit=asym)
        self.assertFalse(bf1.combine(None))
        self.assertFalse(bf1.combine(bf3))
        self.assertTrue(bf1.combine(bf2))
        self.assertEqual(bf1.asym_unit.seq_id_range[0], 0)
        self.assertEqual(bf1.asym_unit.seq_id_range[1], 30)
        self.assertEqual(bf1.count, 5)
        self.assertTrue(bf1.combine(bf3))
        self.assertEqual(bf1.asym_unit.seq_id_range[0], 0)
        self.assertEqual(bf1.asym_unit.seq_id_range[1], 50)
        self.assertEqual(bf1.count, 9)

    def test_model_repr_dump_add_frag(self):
        """Test ModelRepresentationDumper.add_fragment()"""
        m = None
        system = ihm.System()
        e = ihm.Entity('A' * 40)
        asym = ihm.AsymUnit(e)
        state1 = 'state1'
        state2 = 'state2'
        with IMP.allow_deprecated():
            rep1 = ihm.representation.Representation()
        d = IMP.pmi.mmcif._AllModelRepresentations(EmptyObject())
        b = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=0,
                                         end=10, count=2, hier=None,
                                         asym_unit=asym)
        d.add_fragment(state1, rep1, b)
        self.assertEqual(len(d.fragments[id(rep1)]['comp1']), 1)
        self.assertEqual(len(d.fragments[id(rep1)]['comp1'][state1]), 1)
        frag = d.fragments[id(rep1)]['comp1'][state1][0]
        self.assertEqual(frag.asym_unit.seq_id_range[0], 0)
        self.assertEqual(frag.asym_unit.seq_id_range[1], 10)

        b = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=11,
                                         end=30, count=3, hier=None,
                                         asym_unit=asym)
        d.add_fragment(state1, rep1, b)
        self.assertEqual(len(d.fragments[id(rep1)]['comp1']), 1)
        self.assertEqual(len(d.fragments[id(rep1)]['comp1'][state1]), 1)
        frag = d.fragments[id(rep1)]['comp1'][state1][0]
        self.assertEqual(frag.asym_unit.seq_id_range[0], 0)
        self.assertEqual(frag.asym_unit.seq_id_range[1], 30)

        d.add_fragment(state2, rep1, b)
        self.assertEqual(len(d.fragments[id(rep1)]['comp1']), 2)
        self.assertEqual(len(d.fragments[id(rep1)]['comp1'][state2]), 1)
        frag = d.fragments[id(rep1)]['comp1'][state2][0]
        self.assertEqual(frag.asym_unit.seq_id_range[0], 11)
        self.assertEqual(frag.asym_unit.seq_id_range[1], 30)

    def test_model_repr_dump(self):
        """Test ModelRepresentationDumper"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "ME", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(nup84, resolutions=[1])
        hier = s.build()

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        self.assign_range_ids(po.system)
        # Need this to assign starting model details
        ihm.dumper._StartingModelDumper().finalize(po.system)
        d = ihm.dumper._ModelRepresentationDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        r, = po.system.orphan_representations
        self.assertEqual([f.asym_unit.seq_id_range for f in r], [(1,2)])
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_representation.id
_ihm_model_representation.name
_ihm_model_representation.details
1 'Default representation' .
#
#
loop_
_ihm_model_representation_details.id
_ihm_model_representation_details.representation_id
_ihm_model_representation_details.entity_id
_ihm_model_representation_details.entity_description
_ihm_model_representation_details.entity_asym_id
_ihm_model_representation_details.entity_poly_segment_id
_ihm_model_representation_details.model_object_primitive
_ihm_model_representation_details.starting_model_id
_ihm_model_representation_details.model_mode
_ihm_model_representation_details.model_granularity
_ihm_model_representation_details.model_object_count
_ihm_model_representation_details.description
1 1 1 Nup84 A 1 sphere 1 flexible by-residue . .
#
""")

    def test_model_repr_dump_rigid(self):
        """Test ModelRepresentationDumper with rigid bodies"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(nup84.get_atomic_residues(), resolutions=[1])
        nup84.add_representation(nup84.get_non_atomic_residues(),
                                 resolutions=[10])
        hier = s.build()
        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        dof.create_rigid_body(nup84.get_atomic_residues())
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        self.assign_range_ids(po.system)
        # Need this to assign starting model details
        ihm.dumper._StartingModelDumper().finalize(po.system)
        d = ihm.dumper._ModelRepresentationDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        r, = po.system.orphan_representations
        self.assertEqual([f.asym_unit.seq_id_range for f in r], [(1,2), (3,4)])
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_representation.id
_ihm_model_representation.name
_ihm_model_representation.details
1 'Default representation' .
#
#
loop_
_ihm_model_representation_details.id
_ihm_model_representation_details.representation_id
_ihm_model_representation_details.entity_id
_ihm_model_representation_details.entity_description
_ihm_model_representation_details.entity_asym_id
_ihm_model_representation_details.entity_poly_segment_id
_ihm_model_representation_details.model_object_primitive
_ihm_model_representation_details.starting_model_id
_ihm_model_representation_details.model_mode
_ihm_model_representation_details.model_granularity
_ihm_model_representation_details.model_object_count
_ihm_model_representation_details.description
1 1 1 Nup84 A 1 sphere 1 rigid by-residue . .
2 1 1 Nup84 A 2 sphere . flexible by-feature 1 .
#
""")

    def test_flush(self):
        """Test get_dumpers() and  ProtocolOutput.flush()"""
        class MockSystem(ihm.System):
            def __init__(self):
                super(MockSystem, self).__init__()
                self.actions = []

        fh = StringIO()
        po = IMP.pmi.mmcif.ProtocolOutput()
        po.system = MockSystem()
        ihm.dumper.write(fh, [po.system], dumpers=IMP.pmi.mmcif.get_dumpers())
        self.assertEqual(po.system.actions, [])

    def test_state_prefix(self):
        """Test _State.get_prefixed_name()"""
        po = IMP.pmi.mmcif.ProtocolOutput()
        r = DummyRepr('short', 'long')
        state = po._add_state(r)
        self.assertEqual(state.get_prefixed_name('foo'), 'short foo')
        r = DummyRepr(None, None)
        state = po._add_state(r)
        self.assertEqual(state.get_prefixed_name('foo'), 'Foo')
        self.assertEqual(state.get_prefixed_name('FOO'), 'FOO')
        self.assertEqual(state.get_prefixed_name(''), '')

    def test_state_postfix(self):
        """Test _State.get_postfixed_name()"""
        po = IMP.pmi.mmcif.ProtocolOutput()
        r = DummyRepr('short', 'long')
        state = po._add_state(r)
        self.assertEqual(state.get_postfixed_name('foo'), 'foo in state short')
        r = DummyRepr(None, None)
        state = po._add_state(r)
        self.assertEqual(state.get_postfixed_name('foo'), 'foo')

    def test_read(self):
        """Test get_handlers() and read function"""
        mmcif = """
loop_
_ihm_modeling_protocol.id
_ihm_modeling_protocol.protocol_name
_ihm_modeling_protocol.num_steps
1 Sampling 1
#
loop_
_ihm_modeling_protocol_details.id
_ihm_modeling_protocol_details.protocol_id
_ihm_modeling_protocol_details.step_id
_ihm_modeling_protocol_details.struct_assembly_id
_ihm_modeling_protocol_details.dataset_group_id
_ihm_modeling_protocol_details.struct_assembly_description
_ihm_modeling_protocol_details.step_name
_ihm_modeling_protocol_details.step_method
_ihm_modeling_protocol_details.num_models_begin
_ihm_modeling_protocol_details.num_models_end
_ihm_modeling_protocol_details.multi_scale_flag
_ihm_modeling_protocol_details.multi_state_flag
_ihm_modeling_protocol_details.ordered_flag
_ihm_modeling_protocol_details.software_id
_ihm_modeling_protocol_details.script_file_id
1 1 1 1 1 . Prot1 'Monte Carlo' 0 500 YES NO NO . .
#
loop_
_imp_replica_exchange_protocol.protocol_id
_imp_replica_exchange_protocol.step_id
_imp_replica_exchange_protocol.monte_carlo_temperature
_imp_replica_exchange_protocol.replica_exchange_minimum_temperature
_imp_replica_exchange_protocol.replica_exchange_maximum_temperature
1 1 1.000 2.000 3.000
"""

        fh = StringIO(mmcif)
        s, = ihm.reader.read(fh, handlers=IMP.pmi.mmcif.get_handlers())

        p1, = s.orphan_protocols
        step = p1.steps[0]
        self.assertIsInstance(step,
                              IMP.pmi.mmcif._ReplicaExchangeProtocolStep)
        self.assertAlmostEqual(step.monte_carlo_temperature, 1., delta=1e-4)
        self.assertAlmostEqual(step.replica_exchange_minimum_temperature,
                               2., delta=1e-4)
        self.assertAlmostEqual(step.replica_exchange_maximum_temperature,
                               3., delta=1e-4)

    def test_gmm_parser_local_mrc(self):
        """Test GMMParser pointing to a locally-available MRC file"""
        p = IMP.pmi.mmcif.GMMParser()
        fname = self.get_input_file_name('prot_gmm.mrc.1.txt')
        m = p.parse_file(fname)
        self.assertEqual(m['number_of_gaussians'], 1)
        self.assertEqual(m['dataset'].data_type, '3DEM volume')
        self.assertEqual(m['dataset'].location.path, fname)
        self.assertIs(m['dataset'].location.repo, None)
        parent, = m['dataset'].parents
        self.assertEqual(parent.data_type, '3DEM volume')
        self.assertEqual(parent.location.path,
                         self.get_input_file_name('prot_gmm.mrc'))
        self.assertEqual(parent.location.details,
                         'Electron microscopy density map')
        self.assertIs(parent.location.repo, None)

    def test_gaussian_em_restraint(self):
        """Test adding GaussianEMRestraint"""
        m = IMP.Model()
        po = IMP.pmi.mmcif.ProtocolOutput()
        s = IMP.pmi.topology.System(m)
        s.add_protocol_output(po)
        st1 = s.create_state()
        m1 = st1.create_molecule("Prot1",sequence="QEALVVKDLL")
        atomic_res = m1.add_structure(self.get_input_file_name('prot.pdb'),
                                      chain_id='A', res_range=(55,63),
                                      offset=-54)
        fname = self.get_input_file_name("test_gmm.txt")
        m1.add_representation(atomic_res,resolutions=[1,10],
                              density_residues_per_component=2,
                              density_voxel_size=3.0,
                              density_prefix=fname[:-4])
        hier = s.build()
        densities = IMP.atom.Selection(hier,
               representation_type=IMP.atom.DENSITIES).get_selected_particles()
        self.assertEqual(len(densities), 4)
        gem = IMP.pmi.restraints.em.GaussianEMRestraint(densities,
                     target_fn=self.get_input_file_name('prot_gmm.mrc.1.txt'))
        gem.add_to_model()

        em3d, = po.system.restraints
        self.assertEqual(em3d.number_of_gaussians, 1)
        d = em3d.dataset
        self.assertEqual(d.location.path,
                         self.get_input_file_name('prot_gmm.mrc.1.txt'))
        self.assertEqual(len(d.parents), 1)
        d = d.parents[0]
        self.assertEqual(d.location.path,
                         self.get_input_file_name('prot_gmm.mrc'))


if __name__ == '__main__':
    IMP.test.main()
