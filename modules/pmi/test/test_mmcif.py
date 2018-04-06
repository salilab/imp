from __future__ import print_function
import IMP.test
import IMP.pmi.representation
import IMP.pmi.mmcif
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

class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
    def flush(self):
        pass

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

    def test_dumper(self):
        """Test _Dumper base class"""
        d = IMP.pmi.mmcif._Dumper(EmptyObject())
        d.finalize()
        d.finalize_metadata()

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
        po = DummyPO(None)
        po._add_state(DummyRepr(None, None))
        d = ihm.dumper._MultiStateDumper()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump(po.system, w)
        self.assertEqual(fh.getvalue(), "")

    def test_multi_state(self):
        """Test with multiple states"""
        po = DummyPO(None)
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
_ihm_multi_state_modeling.ordinal_id
_ihm_multi_state_modeling.state_id
_ihm_multi_state_modeling.state_group_id
_ihm_multi_state_modeling.population_fraction
_ihm_multi_state_modeling.state_type
_ihm_multi_state_modeling.state_name
_ihm_multi_state_modeling.model_group_id
_ihm_multi_state_modeling.experiment_type
_ihm_multi_state_modeling.details
1 1 1 . . . 1 'Fraction of bulk' .
2 1 1 . . . 2 'Fraction of bulk' .
3 2 1 . . 'state2 long' 3 'Fraction of bulk' .
#
""")

    def test_file_dataset(self):
        """Test get/set_file_dataset methods"""
        # Note that a ProtocolOutput can combine file datasets from multiple
        # Representation objects
        po = DummyPO(EmptyObject())

        m1 = IMP.Model()
        r1 = IMP.pmi.representation.Representation(m1)
        l1 = ihm.location.InputFileLocation(repo='foo', path='baz')
        d1 = ihm.dataset.EM2DClassDataset(l1)
        r1.add_protocol_output(po)
        r1.set_file_dataset('foo', d1)

        m2 = IMP.Model()
        r2 = IMP.pmi.representation.Representation(m2)
        l2 = ihm.location.InputFileLocation(repo='bar', path='baz')
        d2 = ihm.dataset.EM2DClassDataset(l2)
        r2.add_protocol_output(po)
        r2.set_file_dataset('bar', d2)

        self.assertEqual(r1.get_file_dataset('foo'), d1)
        self.assertEqual(r2.get_file_dataset('bar'), d2)
        self.assertEqual(po.get_file_dataset('foo'), d1)
        self.assertEqual(po.get_file_dataset('bar'), d2)
        self.assertEqual(r1._file_dataset, {os.path.abspath('foo'): d1})
        self.assertEqual(r2._file_dataset, {os.path.abspath('bar'): d2})
        self.assertEqual(r1.get_file_dataset('foobar'), None)
        self.assertEqual(r2.get_file_dataset('foobar'), None)
        self.assertEqual(po.get_file_dataset('foobar'), None)

    def test_create_component_repeat(self):
        """Test repeated calls to create_component()"""
        po = DummyPO(EmptyObject())
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

    def test_asym_units(self):
        """Test assigning asym units and entities"""
        po = DummyPO(EmptyObject())
        state1 = po._add_state(EmptyObject())
        state2 = po._add_state(EmptyObject())
        for state, c, seq in ((state1, "foo", "AAA"), (state2, "bar", "AAA"),
                              (state2, "foo", "AAA"),
                              (state1, "baz", "AA")):
            po.create_component(state, c, True)
            po.add_component_sequence(state, c, seq)
        self.assertEqual(len(po.system.entities), 2)
        self.assertEqual(po.system.asym_units[0].details, 'foo')
        self.assertEqual(''.join(x.code for x in
                                 po.system.entities[0].sequence), 'AAA')
        self.assertEqual(''.join(x.code for x in
                                 po.system.entities[1].sequence), 'AA')
        self.assertEqual(len(po.system.asym_units), 3)
        self.assertEqual(po.system.asym_units[0].details, 'foo')
        self.assertEqual(po.system.asym_units[1].details, 'bar')
        self.assertEqual(po.system.asym_units[2].details, 'baz')

    def test_entity_creation(self):
        """Test creation of Entity objects"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        simo.create_component('foo.1@12')
        simo.create_component('bar')
        simo.create_component('baz')
        po.add_component_sequence(po._last_state, 'foo.1@12', 'ACGT')
        po.add_component_sequence(po._last_state, 'bar', 'ACGT')
        po.add_component_sequence(po._last_state, 'baz', 'ACC')

        self.assertEqual(len(po.system.entities), 2)
        self.assertEqual(po.system.entities[0].description, 'foo')
        self.assertEqual(po.system.entities[1].description, 'baz')

    def test_asym_id_mapper(self):
        """Test AsymIDMapper class"""
        m = IMP.Model()
        po = DummyPO(None)
        simo = IMP.pmi.representation.Representation(m)
        simo.add_protocol_output(po)
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        simo.create_component("Nup85", True)
        simo.add_component_sequence("Nup85",
                                    self.get_input_file_name("test.fasta"))
        h1 = simo.add_component_beads("Nup84", [(1,2), (3,4)])
        h2 = simo.add_component_beads("Nup85", [(1,2), (3,4)])

        self.assertEqual(len(po.system.asym_units), 2)
        po.system.asym_units[0]._id = 'A'
        po.system.asym_units[1]._id = 'B'

        mapper = IMP.pmi.mmcif._AsymIDMapper(po, simo.prot)
        self.assertEqual(mapper[h1[0]], 'A')
        self.assertEqual(mapper[h1[1]], 'A')
        self.assertEqual(mapper[h2[0]], 'B')
        self.assertEqual(mapper[h2[1]], 'B')
        # Check handling of multiple states
        simo2 = IMP.pmi.representation.Representation(m)
        simo2.add_protocol_output(po)
        simo2.create_component("Nup85", True)
        simo2.add_component_sequence("Nup85",
                                     self.get_input_file_name("test.fasta"))
        h1 = simo2.add_component_beads("Nup85", [(1,2), (3,4)])
        mapper = IMP.pmi.mmcif._AsymIDMapper(po, simo2.prot)
        # First chain, but ID isn't "A" since it gets the same chain ID
        # as the component in the first state (simo)
        self.assertEqual(mapper[h1[0]], 'B')

    def test_component_mapper(self):
        """Test ComponentMapper class"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        simo.create_component("Nup85", True)
        simo.add_component_sequence("Nup85",
                                    self.get_input_file_name("test.fasta"))
        h1 = simo.add_component_beads("Nup84", [(1,2), (3,4)])
        h2 = simo.add_component_beads("Nup85", [(1,2), (3,4)])
        mapper = IMP.pmi.mmcif._ComponentMapper(simo.prot)
        self.assertEqual(mapper[h1[0]], 'Nup84')
        self.assertEqual(mapper[h1[1]], 'Nup84')
        self.assertEqual(mapper[h2[0]], 'Nup85')
        self.assertEqual(mapper[h2[1]], 'Nup85')

    def test_cif_entities(self):
        """Test _EntityMapper class"""
        system = ihm.System()
        c = IMP.pmi.mmcif._EntityMapper(system)
        c.add('foo', 'MELS')
        c.add('bar', 'SELM')
        c.add('foo_2', 'MELS')
        self.assertEqual(len(system.entities), 2)
        self.assertIs(c['foo'], c['foo_2'])
        self.assertIsNot(c['foo'], c['bar'])
        a = system.entities
        self.assertEqual(len(a), 2)
        self.assertEqual(a[0].description, 'foo')
        self.assertEqual(''.join(x.code for x in a[0].sequence), 'MELS')
        self.assertEqual(a[1].description, 'bar')
        self.assertEqual(''.join(x.code for x in a[1].sequence), 'SELM')

    def test_dataset_group_finalize(self):
        """Test DatasetGroup.finalize()"""
        class DummyRestraint(object):
            pass
        l = ihm.location.InputFileLocation(repo='foo', path='baz')
        ds1 = ihm.dataset.EM2DClassDataset(l)
        # Duplicate dataset
        ds2 = ihm.dataset.EM2DClassDataset(l)
        # Restraint dataset
        r = DummyRestraint()
        l = ihm.location.InputFileLocation(repo='foo', path='bar')
        ds3 = ihm.dataset.EM2DClassDataset(l)
        r.dataset = ds3
        rds = IMP.pmi.mmcif._RestraintDataset(r, None, False)
        dg = IMP.pmi.mmcif._DatasetGroup([ds1, ds2, rds])
        dg.finalize()
        # ds2 should be ignored (duplicate of ds1) and rds should have been
        # replaced by the real underlying dataset, ds3
        self.assertEqual(list(dg._datasets), [ds1, ds3])

    def test_all_datasets_all_group(self):
        """Test AllDatasets.get_all_group()"""
        state1 = 'state1'
        state2 = 'state2'

        alld = IMP.pmi.mmcif._AllDatasets()

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

        for g in (g2, g2, g3, g4, g5):
            g.finalize()

        self.assertEqual(list(g1._datasets), [])
        self.assertEqual(list(g2._datasets), [ds1, ds2])
        self.assertEqual(list(g4._datasets), [ds1, ds2, ds3])
        self.assertEqual(list(g5._datasets), [ds3])

    def test_model_dumper_sphere(self):
        """Test ModelDumper sphere_obj output"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        state = simo._protocol_output[0][1]
        po.exclude_coordinates('Nup84', (3,4))
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")
        simo.create_transformed_component("Nup84.2", "Nup84",
                IMP.algebra.Transformation3D(IMP.algebra.Vector3D(1,2,3)))

        d = IMP.pmi.mmcif._ModelDumper(po)
        assembly = ihm.Assembly()
        assembly._id = 42
        representation = IMP.pmi.mmcif._Representation(name="test rep")
        representation.id = 99
        protocol = IMP.pmi.mmcif._Protocol()
        protocol.id = 93
        group = ihm.model.ModelGroup(name="all models")
        group._id = 7
        model = d.add(simo.prot, protocol, assembly, representation, group)
        self.assertEqual(model.id, 1)
        self.assertEqual(model.get_rmsf('Nup84', (1,)), None)
        fh = StringIO()
        self.assign_entity_asym_ids(po.system)
        w = ihm.format.CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_name
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 1 7 . 'all models' 42 93 99
#
#
loop_
_ihm_sphere_obj_site.ordinal_id
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
3 1 1 1 B -7.986 13.688 -2.817 3.068 . 1
4 1 2 2 B -7.986 13.688 -2.817 2.997 . 1
#
""")

    def test_model_dumper_atom(self):
        """Test ModelDumper atom_site output"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        state = simo._protocol_output[0][1]
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A", resolutions=[0])

        d = IMP.pmi.mmcif._ModelDumper(po)
        assembly = ihm.Assembly()
        assembly._id = 42
        representation = IMP.pmi.mmcif._Representation(name="test rep")
        representation.id = 99
        protocol = IMP.pmi.mmcif._Protocol()
        protocol.id = 93
        group = ihm.model.ModelGroup(name="all models")
        group._id = 7
        model = d.add(simo.prot, protocol, assembly, representation, group)
        self.assertEqual(model.id, 1)
        self.assertEqual(model.get_rmsf('Nup84', (1,)), None)
        self.assign_entity_asym_ids(po.system)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_name
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 1 7 . 'all models' 42 93 99
#
#
loop_
_atom_site.id
_atom_site.label_atom_id
_atom_site.label_comp_id
_atom_site.label_seq_id
_atom_site.label_asym_id
_atom_site.Cartn_x
_atom_site.Cartn_y
_atom_site.Cartn_z
_atom_site.label_entity_id
_atom_site.model_id
1 CA MET 1 A -8.986 11.688 -5.817 1 1
2 CA GLU 2 A -8.986 11.688 -5.817 1 1
#
#
loop_
_ihm_sphere_obj_site.ordinal_id
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
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        state = simo._protocol_output[0][1]
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")

        d = IMP.pmi.mmcif._ModelDumper(po)
        assembly = ihm.Assembly()
        assembly._id = 42
        representation = IMP.pmi.mmcif._Representation(name="test rep")
        representation.id = 99
        protocol = IMP.pmi.mmcif._Protocol()
        protocol.id = 93
        group = ihm.model.ModelGroup(name='all models')
        state.append(group)
        group._id = 7
        model = d.add(simo.prot, protocol, assembly, representation, group)
        self.assertEqual(model.id, 1)
        model.name = 'foo'
        model.parse_rmsf_file(self.get_input_file_name('test.nup84.rmsf'),
                              'Nup84')
        self.assertAlmostEqual(model.get_rmsf('Nup84', (1,)), 4.5, delta=1e-4)
        self.assertRaises(ValueError, model.get_rmsf, 'Nup84', (1,2))
        self.assign_entity_asym_ids(po.system)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_name
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 1 7 foo 'all models' 42 93 99
#
#
loop_
_ihm_sphere_obj_site.ordinal_id
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
        # Handle RestraintDataset
        system.orphan_datasets.extend(
                           po.all_datasets._get_final_datasets())
        system.orphan_dataset_groups.extend(
                           po.all_datasets._get_final_groups())
        d = ihm.dumper._DatasetDumper()
        d.finalize(system)

    def assign_entity_asym_ids(self, system):
        """Assign IDs to all Entities and AsymUnits in the system"""
        d = ihm.dumper._EntityDumper()
        d.finalize(system)
        d = ihm.dumper._StructAsymDumper()
        d.finalize(system)

    def test_starting_model_dumper(self):
        """Test StartingModelDumper"""
        m = IMP.Model()
        po = DummyPO(None)
        simo = IMP.pmi.representation.Representation(m)
        simo.add_protocol_output(po)
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")

        # Test multiple states: components that are the same in both states
        # (Nup84) should not be duplicated in the mmCIF output
        simo2 = IMP.pmi.representation.Representation(m)
        simo2.add_protocol_output(po)
        simo2.create_component("Nup84", True)
        simo2.add_component_sequence("Nup84",
                                     self.get_input_file_name("test.fasta"))
        simo2.autobuild_model("Nup84",
                              self.get_input_file_name("test.nup84.pdb"), "A")
        simo2.create_component("Nup85", True)
        simo2.add_component_sequence("Nup85",
                                     self.get_input_file_name("test.fasta"))
        simo2.autobuild_model("Nup85",
                              self.get_input_file_name("test.nup85.pdb"), "A",
                              resrange=(8,9),offset=-7)

        self.assign_entity_asym_ids(po.system)

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        # Assign ID (2) to referenced alignment file (id=1 is assigned to
        # the Python script)
        ihm.dumper._ExternalReferenceDumper().finalize(po.system)
        self.assign_dataset_ids(po)
        po.starting_model_dump.finalize()
        po.starting_model_dump.dump(w)
        out = fh.getvalue()
        self.assertEqual(out,
"""# IMP will attempt to identify which input models are crystal structures and
# which are comparative models, but does not always have sufficient information
# to deduce all of the templates used for comparative modeling. These may need
# to be added manually below.
#
loop_
_ihm_starting_model_details.starting_model_id
_ihm_starting_model_details.entity_id
_ihm_starting_model_details.entity_description
_ihm_starting_model_details.asym_id
_ihm_starting_model_details.seq_id_begin
_ihm_starting_model_details.seq_id_end
_ihm_starting_model_details.starting_model_source
_ihm_starting_model_details.starting_model_auth_asym_id
_ihm_starting_model_details.starting_model_sequence_offset
_ihm_starting_model_details.dataset_list_id
Nup84-m1 1 Nup84 A 33 2 'comparative model' A 0 3
Nup85-m1 2 Nup85 B 33 -5 'comparative model' A -7 4
#
#
loop_
_ihm_starting_comparative_models.ordinal_id
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
1 Nup84-m1 A 33 424 C 33 424 100.000 1 1 2
2 Nup84-m1 A 429 488 G 482 551 10.000 1 2 2
3 Nup85-m1 A 33 424 C 33 424 100.000 1 1 2
4 Nup85-m1 A 429 488 G 482 551 10.000 1 2 2
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
Nup84-m1 ATOM 1 C CA MET 1 A 1 -8.986 11.688 -5.817 91.820 1
Nup84-m1 ATOM 2 C CA GLU 1 A 2 -8.986 11.688 -5.817 91.820 2
Nup85-m1 ATOM 1 C CA MET 2 B 1 -8.986 11.688 -5.817 91.820 3
Nup85-m1 ATOM 2 C CA GLU 2 B 2 -8.986 11.688 -5.817 91.820 4
#
""")

    def get_dumper_sources(self, pdbname):
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        state = simo._protocol_output[0][1]
        chain = 'A'
        fragment = IMP.pmi.mmcif._PDBFragment(state, "mypdb", 1, 10, 0,
                                              pdbname, chain, None)
        model = IMP.pmi.mmcif._StartingModel(fragment)
        sources = po.starting_model_dump.get_sources(model, pdbname, chain)
        return m, model, sources

    def test_protocol_dumper(self):
        """Test ModelProtocolDumper output"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        # Need Repository in order to handle PDB file datasets
        simo.add_metadata(ihm.location.Repository(doi='foo', root='.'))
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")
        mc1 = IMP.pmi.macros.ReplicaExchange0(m, simo,
                                 monte_carlo_sample_objects=[simo],
                                 output_objects=[simo],
                                 test_mode=True)
        mc1.execute_macro()
        mc2 = IMP.pmi.macros.ReplicaExchange0(m, simo,
                                 monte_carlo_sample_objects=[simo],
                                 output_objects=[simo],
                                 test_mode=True)
        mc2.execute_macro()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        ihm.dumper._AssemblyDumper().finalize(po.system)  # assign assembly IDs
        self.assign_dataset_ids(po)
        po.model_prot_dump.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_modeling_protocol.ordinal_id
_ihm_modeling_protocol.protocol_id
_ihm_modeling_protocol.step_id
_ihm_modeling_protocol.struct_assembly_id
_ihm_modeling_protocol.dataset_group_id
_ihm_modeling_protocol.struct_assembly_description
_ihm_modeling_protocol.protocol_name
_ihm_modeling_protocol.step_name
_ihm_modeling_protocol.step_method
_ihm_modeling_protocol.num_models_begin
_ihm_modeling_protocol.num_models_end
_ihm_modeling_protocol.multi_scale_flag
_ihm_modeling_protocol.multi_state_flag
_ihm_modeling_protocol.ordered_flag
1 1 1 1 1 . . Sampling 'Replica exchange monte carlo' 0 1000 YES NO NO
2 1 2 1 1 . . Sampling 'Replica exchange monte carlo' 1000 1000 YES NO NO
#
""")

    def test_add_simple_dynamics(self):
        """Test add_simple_dynamics()"""
        po = DummyPO(None)
        po._add_state(EmptyObject())
        po._add_simple_dynamics(100, "Brownian dynamics")

    def test_simple_postprocessing(self):
        """Test add_simple_postprocessing"""
        class DummyProtocolStep(object):
            pass
        po = DummyPO(None)
        po._add_state(DummyRepr(None, None))
        p = DummyProtocolStep()
        p.state = po._last_state
        p.num_models_end = 10
        po.model_prot_dump.add_step(p)
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
        p.state = po._last_state
        p.num_models_end = 10
        po.model_prot_dump.add_step(p)
        po._add_simple_postprocessing(20, 80)

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        po.post_process_dump.finalize()
        po.post_process_dump.dump(w)
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
1 1 1 1 cluster RMSD 10 90
2 1 1 2 cluster RMSD 12 90
3 2 1 1 cluster RMSD 34 56
4 3 1 1 cluster RMSD 20 80
#
""")

    def test_no_postprocessing(self):
        """Test add_no_postprocessing"""
        class DummyProtocolStep(object):
            pass
        po = DummyPO(None)
        po._add_state(DummyRepr(None, None))
        p = DummyProtocolStep()
        p.state = po._last_state
        p.num_models_end = 10
        po.model_prot_dump.add_step(p)
        pp = po._add_no_postprocessing(10)
        self.assertEqual(pp.type, 'none')
        self.assertEqual(pp.feature, 'none')
        self.assertEqual(pp.num_models_begin, 10)
        self.assertEqual(pp.num_models_end, 10)

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        po.post_process_dump.finalize()
        po.post_process_dump.dump(w)
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
1 1 1 1 none none 10 10
#
""")

    def test_simple_ensemble(self):
        """Test add_simple_ensemble"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        simo.create_component("Nup85", True)
        simo.add_component_sequence("Nup85",
                                    self.get_input_file_name("test.fasta"))

        densities = {'Nup84': "foo.mrc"}
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
            pp = IMP.pmi.mmcif._ReplicaExchangeAnalysisPostProcess(None, d, 45)
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
        locations = []
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
            pp = IMP.pmi.mmcif._ReplicaExchangeAnalysisPostProcess(None, d, 45)
            mg = DummyGroup()
            e = IMP.pmi.mmcif._ReplicaExchangeAnalysisEnsemble(pp, 0, mg, 1)
            self.assertEqual(e.cluster_num, 0)
            self.assertEqual(e.post_process, pp)
            self.assertEqual(e.num_models_deposited, 1)
            self.assertEqual(e.densities, [])
            self.assertEqual(e.num_models, 2)
            self.assertEqual(e.clustering_feature, 'RMSD')
            self.assertEqual(e.name, 'cluster 1')
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
            e.load_localization_density(None, 'noden', locations)
            self.assertEqual(len(e.densities), 0)
            # Density that does exist
            po = DummyPO(None)
            r = DummyRepr('dummy', 'none')
            state = po._add_state(r)
            e.load_localization_density(state, 'Nup84', locations)
            self.assertEqual(e.densities[0].file.path,
                             os.path.join(tmpdir, 'cluster.0', 'Nup84.mrc'))
            self.assertEqual(e.densities[0].file.details,
                         'Localization density for Nup84 dgroup in state dummy')
            # No precision available
            self.assertEqual(e._get_precision(), '?')
            self.assertEqual(e.precision, '?')
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
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
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
            prot.state = po._last_state
            prot.num_models_end = 10
            po.model_prot_dump.add_step(prot)
            po.add_replica_exchange_analysis(simo._protocol_output[0][1], rex)

    def test_ensemble_dumper(self):
        """Test dumping of simple ensembles"""
        class DummyPostProcess(object):
            pass
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)

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
1 'Ensemble 1' 99 1 . dRMSD 5 1 0.100 .
2 'Ensemble 2' 99 2 . dRMSD 5 1 0.100 42
#
""")

    def test_density_dumper(self):
        """Test DensityDumper"""
        class DummyEnsemble(object):
            pass

        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))

        ensemble = DummyEnsemble()
        ensemble._id = 42
        loc = ihm.location.OutputFileLocation(repo='foo', path='bar')
        loc._id = 97
        den = ihm.model.LocalizationDensity(file=loc,
                                asym_unit=po.asym_units['Nup84'])
        ensemble.densities = [den]
        po.system.ensembles.append(ensemble)

        fh = StringIO()
        self.assign_entity_asym_ids(po.system)
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
_ihm_localization_density_files.seq_id_begin
_ihm_localization_density_files.seq_id_end
1 97 42 1 A 1 4
#
""")

    def test_cross_link_dumper(self):
        """Test the CrossLinkDumper"""
        class DummyDataset(object):
            pass
        class DummyRestraint(object):
            label = 'foo'
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        state = simo._protocol_output[0][1]
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")
        r = DummyRestraint()
        r.dataset = DummyDataset()
        r.dataset._id = 42
        xl_group = po.get_cross_link_group(r)
        ex_xl = po.add_experimental_cross_link(1, 'Nup84',
                                               2, 'Nup84', 42.0, xl_group)
        ex_xl2 = po.add_experimental_cross_link(1, 'Nup84',
                                                3, 'Nup84', 42.0, xl_group)
        # Duplicates should be ignored
        po.add_experimental_cross_link(1, 'Nup84',
                                       3, 'Nup84', 42.0, xl_group)
        # Non-modeled component should be ignored
        nm_ex_xl = po.add_experimental_cross_link(1, 'Nup85',
                                                  2, 'Nup84', 42.0, xl_group)
        self.assertEqual(nm_ex_xl, None)
        rs = nup84[0].get_children()
        sigma1 = IMP.isd.Scale.setup_particle(IMP.Particle(m), 1.0)
        sigma2 = IMP.isd.Scale.setup_particle(IMP.Particle(m), 0.5)
        psi = IMP.isd.Scale.setup_particle(IMP.Particle(m), 0.8)
        po.add_cross_link(state, ex_xl, rs[0], rs[1], sigma1, sigma2, psi)
        # Duplicates should be ignored
        po.add_cross_link(state, ex_xl, rs[0], rs[1], sigma1, sigma2, psi)

        fh = StringIO()
        self.assign_entity_asym_ids(po.system)
        w = ihm.format.CifWriter(fh)
        po.cross_link_dump.dump(w)
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
_ihm_cross_link_list.linker_type
_ihm_cross_link_list.dataset_list_id
1 1 Nup84 1 1 MET Nup84 1 2 GLU foo 42
2 2 Nup84 1 1 MET Nup84 1 3 LEU foo 42
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
_ihm_cross_link_restraint.restraint_type
_ihm_cross_link_restraint.conditional_crosslink_flag
_ihm_cross_link_restraint.model_granularity
_ihm_cross_link_restraint.distance_threshold
_ihm_cross_link_restraint.psi
_ihm_cross_link_restraint.sigma_1
_ihm_cross_link_restraint.sigma_2
1 1 1 A 1 MET 1 A 2 GLU 'upper bound' ALL by-residue 42.000 0.800 1.000 0.500
#
""")

    def test_restraint_dataset(self):
        """Test RestraintDataset class"""
        class DummyRestraint(object):
            pass
        r = DummyRestraint()
        rd = IMP.pmi.mmcif._RestraintDataset(r, num=None,
                                             allow_duplicates=False)
        l = ihm.location.InputFileLocation(repo='foo', path='bar')
        d = ihm.dataset.CXMSDataset(l)
        r.dataset = d
        # Get current dataset from restraint
        d2 = rd.dataset
        self.assertEqual(d2.data_type, 'CX-MS data')
        self.assertEqual(d2.location.repo, 'foo')
        # Should be a copy, so we can change it without affecting the original
        self.assertEqual(d, d2)
        self.assertNotEqual(id(d), id(d2))
        d2.location.repo = 'bar'
        self.assertEqual(d2.location.repo, 'bar')
        self.assertEqual(d.location.repo, 'foo')
        # Subsequent accesses should be cached, not copying again
        d3 = rd.dataset
        self.assertEqual(id(d2), id(d3))

    def test_restraint_dataset_num(self):
        """Test RestraintDataset with num!=None"""
        class DummyRestraint(object):
            pass
        r = DummyRestraint()
        rd = IMP.pmi.mmcif._RestraintDataset(r, num=1, allow_duplicates=False)
        l = ihm.location.InputFileLocation(repo='foo', path='bar')
        d1 = ihm.dataset.CXMSDataset(l)
        l = ihm.location.InputFileLocation(repo='bar', path='baz')
        d2 = ihm.dataset.CXMSDataset(l)
        r.datasets = [d1, d2]
        # Get current dataset from restraint
        d2 = rd.dataset
        self.assertEqual(d2.data_type, 'CX-MS data')
        self.assertEqual(d2.location.repo, 'bar')

    def test_restraint_dataset_duplicate(self):
        """Test RestraintDataset with allow_duplicates=True"""
        class DummyRestraint(object):
            pass
        r = DummyRestraint()
        rd = IMP.pmi.mmcif._RestraintDataset(r, num=None, allow_duplicates=True)
        l = ihm.location.InputFileLocation(repo='foo', path='bar')
        d = ihm.dataset.CXMSDataset(l)
        r.dataset = d
        # Get current dataset from restraint
        d2 = rd.dataset
        self.assertEqual(d2.data_type, 'CX-MS data')
        self.assertEqual(d2.location.repo, 'foo')
        # Should be a copy, but should not compare equal
        # since allow_duplicates=True
        self.assertNotEqual(d, d2)

    def test_add_em2d_restraint(self):
        """Test add_em2d_restraint method"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        state = simo._protocol_output[0][1]
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")

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
        p.state = po._last_state
        po.model_prot_dump.add_step(p)
        group = get_all_models_group(simo, po)
        m = po.add_model(group)
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
1 4 50 4.200 4.200 1.000 NO 200 1 .
#
#
loop_
_ihm_2dem_class_average_fitting.ordinal_id
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
1 1 1 0.873 -0.406503 -0.909500 -0.086975 0.379444 -0.254653 0.889480 -0.831131
0.328574 0.448622 304.187 219.586 0.000
#
""")

    def test_sas_dumper(self):
        """Test SASDumper class"""
        class DummyModel(object):
            pass
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        state = simo._protocol_output[0][1]
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))

        lp = ihm.location.InputFileLocation(repo='foo', path='baz')
        d = ihm.dataset.SASDataset(lp)
        d._id = 4
        model = DummyModel()
        model._id = 42
        po._add_foxs_restraint(model, 'Nup84', (2,3), d, 3.4, 1.2, 'test')

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
_ihm_sas_restraint.ordinal_id
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
1 4 42 2 NO 'Heavy atoms' FoXS Single 3.400 1.200 test
#
""")

    def test_add_em3d_restraint(self):
        """Test add_em3d_restraint method"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        state = simo._protocol_output[0][1]
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")
        p = IMP.atom.get_by_type(simo.hier_dict['Nup84'],
                                 IMP.atom.FRAGMENT_TYPE)[0]
        class DummyRestraint(object):
            label = 'foo'
        class DummyProtocolStep(object):
            pass
        pr = DummyRestraint()
        pr.dataset = None
        po.add_em3d_restraint(state, pmi_restraint=pr,
                              target_ps=[None, None], densities=[p])

        l = ihm.location.InputFileLocation(repo='foo', path='bar')
        d = ihm.dataset.EMDensityDataset(l)
        d._id = 4
        pr.dataset = d

        p = DummyProtocolStep()
        p.state = po._last_state
        po.model_prot_dump.add_step(p)
        group = get_all_models_group(simo, po)
        m = po.add_model(group)
        m.stats = {'GaussianEMRestraint_foo_CCC': 0.1}
        m = po.add_model(group)
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
_ihm_3dem_restraint.ordinal_id
_ihm_3dem_restraint.dataset_list_id
_ihm_3dem_restraint.fitting_method
_ihm_3dem_restraint.fitting_method_citation
_ihm_3dem_restraint.struct_assembly_id
_ihm_3dem_restraint.number_of_gaussians
_ihm_3dem_restraint.model_id
_ihm_3dem_restraint.cross_correlation_coefficient
1 4 'Gaussian mixture models' . 1 2 1 0.100
2 4 'Gaussian mixture models' . 1 2 2 0.200
#
""")

    def test_update_locations(self):
        """Test update_locations() method"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)

        with IMP.test.temporary_directory() as tmpdir:
            bar = os.path.join(tmpdir, 'bar')
            with open(bar, 'w') as f:
                f.write("")
            local = ihm.location.InputFileLocation(bar)
            # No Repository set, so cannot map local to repository location
            po._update_locations([local])
            self.assertEqual(local.repo, None)

            simo.add_metadata(ihm.Software(
                                  name='test', classification='test code',
                                  description='Some test program',
                                  version=1, location='http://salilab.org'))
            simo.add_metadata(ihm.location.Repository(doi='foo',
                                                      root=tmpdir))
            loc = ihm.location.InputFileLocation(bar)
            po._update_locations([loc])
            self.assertEqual(loc.repo.doi, 'foo')
            self.assertEqual(loc.path, 'bar')
            # Further calls shouldn't change things
            po._update_locations([loc])
            self.assertEqual(loc.repo.doi, 'foo')
            self.assertEqual(loc.path, 'bar')

    def test_seq_dif(self):
        """Test StartingModelDumper.dump_seq_dif"""
        class DummyEntity(object):
            _id = 4
        class DummyPO(object):
            def _get_chain_for_component(self, comp, output):
                return 'H'
            entities = {'nup84': DummyEntity()}
        class DummyRes(object):
            def get_index(self):
                return 42
        class DummyModel(object):
            name = 'dummy-m1'

        po = DummyPO()
        d = IMP.pmi.mmcif._StartingModelDumper(po)
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d.dump_seq_dif(w, [IMP.pmi.mmcif._MSESeqDif(DummyRes(), 'nup84',
                                                    'X', DummyModel(), 2)])
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_starting_model_seq_dif.ordinal_id
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
        m = None
        bf1 = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=0,
                                           end=10, num=2, hier=None)
        bf2 = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=11,
                                           end=30, num=3, hier=None)
        bf3 = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=31,
                                           end=50, num=4, hier=None)
        self.assertFalse(bf1.combine(None))
        self.assertFalse(bf1.combine(bf3))
        self.assertTrue(bf1.combine(bf2))
        self.assertEqual(bf1.start, 0)
        self.assertEqual(bf1.end, 30)
        self.assertEqual(bf1.num, 5)
        self.assertTrue(bf1.combine(bf3))
        self.assertEqual(bf1.start, 0)
        self.assertEqual(bf1.end, 50)
        self.assertEqual(bf1.num, 9)

    def test_model_repr_dump_add_frag(self):
        """Test ModelRepresentationDumper.add_fragment()"""
        m = None
        state1 = 'state1'
        state2 = 'state2'
        rep1 = 'rep1'
        d = IMP.pmi.mmcif._ModelRepresentationDumper(EmptyObject())
        b = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=0,
                                         end=10, num=2, hier=None)
        d.add_fragment(state1, rep1, b)
        self.assertEqual(len(d.fragments['rep1']['comp1']), 1)
        self.assertEqual(len(d.fragments['rep1']['comp1'][state1]), 1)
        frag = d.fragments['rep1']['comp1'][state1][0]
        self.assertEqual(frag.start, 0)
        self.assertEqual(frag.end, 10)

        b = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=11,
                                         end=30, num=3, hier=None)
        d.add_fragment(state1, rep1, b)
        self.assertEqual(len(d.fragments['rep1']['comp1']), 1)
        self.assertEqual(len(d.fragments['rep1']['comp1'][state1]), 1)
        frag = d.fragments['rep1']['comp1'][state1][0]
        self.assertEqual(frag.start, 0)
        self.assertEqual(frag.end, 30)

        d.add_fragment(state2, rep1, b)
        self.assertEqual(len(d.fragments['rep1']['comp1']), 2)
        self.assertEqual(len(d.fragments['rep1']['comp1'][state2]), 1)
        frag = d.fragments['rep1']['comp1'][state2][0]
        self.assertEqual(frag.start, 11)
        self.assertEqual(frag.end, 30)

    def test_model_repr_dump(self):
        """Test ModelRepresentationDumper"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        po.exclude_coordinates('Nup84', (3,4))
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")
        simo.create_transformed_component("Nup84.2", "Nup84",
                IMP.algebra.Transformation3D(IMP.algebra.Vector3D(1,2,3)))
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        # Need this to assign starting model details
        po.starting_model_dump.finalize()
        po.model_repr_dump.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_representation.ordinal_id
_ihm_model_representation.representation_id
_ihm_model_representation.segment_id
_ihm_model_representation.entity_id
_ihm_model_representation.entity_description
_ihm_model_representation.entity_asym_id
_ihm_model_representation.seq_id_begin
_ihm_model_representation.seq_id_end
_ihm_model_representation.model_object_primitive
_ihm_model_representation.starting_model_id
_ihm_model_representation.model_mode
_ihm_model_representation.model_granularity
_ihm_model_representation.model_object_count
1 1 1 1 Nup84 A 1 2 sphere Nup84-m1 flexible by-residue .
2 1 2 1 Nup84 B 1 2 sphere Nup84-m1 flexible by-residue .
#
""")

    def test_model_repr_dump_rigid(self):
        """Test ModelRepresentationDumper with rigid bodies"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")
        simo.set_rigid_body_from_hierarchies(nup84)
        simo.set_floppy_bodies()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        # Need this to assign starting model details
        po.starting_model_dump.finalize()
        po.model_repr_dump.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_representation.ordinal_id
_ihm_model_representation.representation_id
_ihm_model_representation.segment_id
_ihm_model_representation.entity_id
_ihm_model_representation.entity_description
_ihm_model_representation.entity_asym_id
_ihm_model_representation.seq_id_begin
_ihm_model_representation.seq_id_end
_ihm_model_representation.model_object_primitive
_ihm_model_representation.starting_model_id
_ihm_model_representation.model_mode
_ihm_model_representation.model_granularity
_ihm_model_representation.model_object_count
1 1 1 1 Nup84 A 1 2 sphere Nup84-m1 rigid by-residue .
2 1 2 1 Nup84 A 3 4 sphere . flexible by-feature 1
#
""")

    def test_flush(self):
        """Test ProtocolOutput.flush()"""
        class DummyDumper(IMP.pmi.mmcif._Dumper):
            def __init__(self):
                self.actions = []
            def finalize_metadata(self):
                self.actions.append('fm')
            def finalize(self):
                self.actions.append('f')
            def dump(self, cw):
                self.actions.append('d')
        dump = DummyDumper()
        fh = StringIO()
        po = IMP.pmi.mmcif.ProtocolOutput(fh)
        po._dumpers = [dump]
        po.flush()
        self.assertEqual(dump.actions, ['fm', 'f', 'd'])

    def test_struct_conf_dumper(self):
        """Test StructConfDumper"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                         self.get_input_file_name("test.nup84.helix.pdb"), "A")
        simo.set_rigid_body_from_hierarchies(nup84)
        simo.set_floppy_bodies()

        po.system.asym_units[0]._id = 'A'
        d = IMP.pmi.mmcif._StructConfDumper(po)

        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        po.starting_model_dump.finalize()
        d.dump(w)
        out = fh.getvalue().split('\n')
        # Account for the fact that _struct_conf_type items do not have a
        # guaranteed order (they are stored in a dict), by sorting them
        out = sorted(out[:3]) + out[3:]
        self.assertEqual("\n".join(out),
"""_struct_conf_type.criteria ?
_struct_conf_type.id HELX_P
_struct_conf_type.reference ?
#
loop_
_struct_conf.id
_struct_conf.conf_type_id
_struct_conf.beg_label_comp_id
_struct_conf.beg_label_asym_id
_struct_conf.beg_label_seq_id
_struct_conf.end_label_comp_id
_struct_conf.end_label_asym_id
_struct_conf.end_label_seq_id
HELX_P1 HELX_P MET A 1 GLU A 2
#
""")

    def test_state_prefix(self):
        """Test _State.get_prefixed_name()"""
        po = DummyPO(None)
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
        po = DummyPO(None)
        r = DummyRepr('short', 'long')
        state = po._add_state(r)
        self.assertEqual(state.get_postfixed_name('foo'), 'foo in state short')
        r = DummyRepr(None, None)
        state = po._add_state(r)
        self.assertEqual(state.get_postfixed_name('foo'), 'foo')

if __name__ == '__main__':
    IMP.test.main()
