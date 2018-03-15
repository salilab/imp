import utils
import os
import unittest
import sys
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.dumper
import ihm.format
import ihm.location
import ihm.representation
import ihm.startmodel
import ihm.dataset
import ihm.protocol
import ihm.analysis
import ihm.model
import ihm.restraint

def _get_dumper_output(dumper, system):
    fh = StringIO()
    writer = ihm.format.CifWriter(fh)
    dumper.dump(system, writer)
    return fh.getvalue()

class Tests(unittest.TestCase):
    def test_write(self):
        """Test write() function"""
        sys1 = ihm.System('system1')
        sys2 = ihm.System('system 2+3')
        fh = StringIO()
        ihm.dumper.write(fh, [sys1, sys2])
        lines = fh.getvalue().split('\n')
        self.assertEqual(lines[:2], ["data_system1", "_entry.id system1"])
        self.assertEqual(lines[9:11],
                         ["data_system23", "_entry.id 'system 2+3'"])

    def test_dumper(self):
        """Test Dumper base class"""
        dumper = ihm.dumper._Dumper()
        dumper.finalize(None)
        dumper.dump(None, None)

    def test_entry_dumper(self):
        """Test EntryDumper"""
        system = ihm.System(name='test_model')
        dumper = ihm.dumper._EntryDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, "data_test_model\n_entry.id test_model\n")

    def test_software(self):
        """Test SoftwareDumper"""
        system = ihm.System()
        system.software.append(ihm.Software(
                         name='test', classification='test code',
                         description='Some test program',
                         version=1, location='http://test.org'))
        system.software.append(ihm.Software(
                          name='foo', classification='test code',
                          description='Other test program',
                          location='http://test2.org'))
        dumper = ihm.dumper._SoftwareDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_software.pdbx_ordinal
_software.name
_software.classification
_software.description
_software.version
_software.type
_software.location
1 test 'test code' 'Some test program' 1 program http://test.org
2 foo 'test code' 'Other test program' . program http://test2.org
#
""")

    def test_entity_dumper(self):
        """Test EntityDumper"""
        system = ihm.System()
        system.entities.append(ihm.Entity('ABC', description='foo'))
        system.entities.append(ihm.Entity('ABCD', description='baz'))
        dumper = ihm.dumper._EntityDumper()
        dumper.finalize(system) # Assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_entity.id
_entity.type
_entity.src_method
_entity.pdbx_description
_entity.formula_weight
_entity.pdbx_number_of_molecules
_entity.details
1 polymer man foo ? 1 .
2 polymer man baz ? 1 .
#
""")

    def test_entity_duplicates(self):
        """Test EntityDumper with duplicate entities"""
        system = ihm.System()
        system.entities.append(ihm.Entity('ABC'))
        system.entities.append(ihm.Entity('ABC'))
        dumper = ihm.dumper._EntityDumper()
        self.assertRaises(ValueError, dumper.finalize, system)

    def test_chem_comp_dumper(self):
        """Test ChemCompDumper"""
        system = ihm.System()
        system.entities.append(ihm.Entity('ACGTTA'))
        dumper = ihm.dumper._ChemCompDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_chem_comp.id
_chem_comp.type
ALA 'L-peptide linking'
CYS 'L-peptide linking'
GLY 'L-peptide linking'
THR 'L-peptide linking'
#
""")

    def test_entity_poly_dumper(self):
        """Test EntityPolyDumper"""
        system = ihm.System()
        e1 = ihm.Entity('ACGT')
        e2 = ihm.Entity('ACC')
        system.entities.extend((e1, e2))
        # One entity is modeled (with an asym unit) the other not; this should
        # be reflected in pdbx_strand_id
        system.asym_units.append(ihm.AsymUnit(e1, 'foo'))
        system.asym_units.append(ihm.AsymUnit(e1, 'bar'))
        ed = ihm.dumper._EntityDumper()
        ed.finalize(system) # Assign entity IDs
        sd = ihm.dumper._StructAsymDumper()
        sd.finalize(system) # Assign asym IDs
        dumper = ihm.dumper._EntityPolyDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_entity_poly.entity_id
_entity_poly.type
_entity_poly.nstd_linkage
_entity_poly.nstd_monomer
_entity_poly.pdbx_strand_id
_entity_poly.pdbx_seq_one_letter_code
_entity_poly.pdbx_seq_one_letter_code_can
1 polypeptide(L) no no A ACGT ACGT
2 polypeptide(L) no no . ACC ACC
#
""")

    def test_entity_poly_seq_dumper(self):
        """Test EntityPolySeqDumper"""
        system = ihm.System()
        system.entities.append(ihm.Entity('ACGT'))
        system.entities.append(ihm.Entity('ACC'))
        ed = ihm.dumper._EntityDumper()
        ed.finalize(system) # Assign IDs
        dumper = ihm.dumper._EntityPolySeqDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_entity_poly_seq.entity_id
_entity_poly_seq.num
_entity_poly_seq.mon_id
_entity_poly_seq.hetero
1 1 ALA .
1 2 CYS .
1 3 GLY .
1 4 THR .
2 1 ALA .
2 2 CYS .
2 3 CYS .
#
""")

    def test_struct_asym_dumper(self):
        """Test StructAsymDumper"""
        system = ihm.System()
        e1 = ihm.Entity('ACGT')
        e2 = ihm.Entity('ACC')
        e1._id = 1
        e2._id = 2
        system.entities.extend((e1, e2))
        system.asym_units.append(ihm.AsymUnit(e1, 'foo'))
        system.asym_units.append(ihm.AsymUnit(e1, 'bar'))
        system.asym_units.append(ihm.AsymUnit(e2, 'baz'))
        dumper = ihm.dumper._StructAsymDumper()
        dumper.finalize(system) # assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_struct_asym.id
_struct_asym.entity_id
_struct_asym.details
A 1 foo
B 1 bar
C 2 baz
#
""")

    def test_assembly_all_modeled(self):
        """Test AssemblyDumper, all components modeled"""
        system = ihm.System()
        e1 = ihm.Entity('AAA', description='foo')
        e2 = ihm.Entity('AA', description='baz')
        a1 = ihm.AsymUnit(e1)
        a2 = ihm.AsymUnit(e1)
        a3 = ihm.AsymUnit(e2)
        system.entities.extend((e1, e2))
        system.asym_units.extend((a1, a2, a3))

        c = ihm.AssemblyComponent(a2, seq_id_range=(2,3))
        system.assemblies.append(ihm.Assembly((a1, c), name='foo'))
        # Out of order assembly (should be ordered on output)
        system.assemblies.append(ihm.Assembly((a3, a2), name='bar'))
        # Duplicate assembly (should be ignored)
        system.assemblies.append(ihm.Assembly((a2, a3)))

        # Assign entity and asym IDs
        ihm.dumper._EntityDumper().finalize(system)
        ihm.dumper._StructAsymDumper().finalize(system)

        d = ihm.dumper._AssemblyDumper()
        d.finalize(system)
        self.assertEqual([a._id for a in system.assemblies], [1,2,3,3])
        out = _get_dumper_output(d, system)
        self.assertEqual(out, """#
loop_
_ihm_struct_assembly_details.assembly_id
_ihm_struct_assembly_details.assembly_name
_ihm_struct_assembly_details.assembly_description
1 'Complete assembly' 'All known components'
2 foo .
3 bar .
#
#
loop_
_ihm_struct_assembly.ordinal_id
_ihm_struct_assembly.assembly_id
_ihm_struct_assembly.parent_assembly_id
_ihm_struct_assembly.entity_description
_ihm_struct_assembly.entity_id
_ihm_struct_assembly.asym_id
_ihm_struct_assembly.seq_id_begin
_ihm_struct_assembly.seq_id_end
1 1 1 foo 1 A 1 3
2 1 1 foo 1 B 1 3
3 1 1 baz 2 C 1 2
4 2 2 foo 1 A 1 3
5 2 2 foo 1 B 2 3
6 3 3 foo 1 B 1 3
7 3 3 baz 2 C 1 2
#
""")

    def test_assembly_subset_modeled(self):
        """Test AssemblyDumper, subset of components modeled"""
        system = ihm.System()
        e1 = ihm.Entity('AAA', description='foo')
        e2 = ihm.Entity('AA', description='bar')
        a1 = ihm.AsymUnit(e1)
        system.entities.extend((e1, e2))
        system.asym_units.append(a1)
        # Note that no asym unit uses entity e2, so the assembly
        # should omit the chain ID ('.')

        # Assign entity and asym IDs
        ihm.dumper._EntityDumper().finalize(system)
        ihm.dumper._StructAsymDumper().finalize(system)

        d = ihm.dumper._AssemblyDumper()
        d.finalize(system)
        out = _get_dumper_output(d, system)
        self.assertEqual(out, """#
loop_
_ihm_struct_assembly_details.assembly_id
_ihm_struct_assembly_details.assembly_name
_ihm_struct_assembly_details.assembly_description
1 'Complete assembly' 'All known components'
#
#
loop_
_ihm_struct_assembly.ordinal_id
_ihm_struct_assembly.assembly_id
_ihm_struct_assembly.parent_assembly_id
_ihm_struct_assembly.entity_description
_ihm_struct_assembly.entity_id
_ihm_struct_assembly.asym_id
_ihm_struct_assembly.seq_id_begin
_ihm_struct_assembly.seq_id_end
1 1 1 foo 1 A 1 3
2 1 1 bar 2 . 1 2
#
""")

    def test_external_reference_dumper(self):
        """Test ExternalReferenceDumper"""
        system = ihm.System()
        repo1 = ihm.location.Repository(doi="foo")
        repo2 = ihm.location.Repository(doi="10.5281/zenodo.46266",
                                        url='nup84-v1.0.zip',
                                        top_directory=os.path.join('foo',
                                                                   'bar'))
        repo3 = ihm.location.Repository(doi="10.5281/zenodo.58025",
                                        url='foo.spd')
        l = ihm.location.InputFileLocation(repo=repo1, path='bar')
        system.locations.append(l)
        # Duplicates should be ignored
        l = ihm.location.InputFileLocation(repo=repo1, path='bar')
        system.locations.append(l)
        # Different file, same repository
        l = ihm.location.InputFileLocation(repo=repo1, path='baz')
        system.locations.append(l)
        # Different repository
        l = ihm.location.OutputFileLocation(repo=repo2, path='baz')
        system.locations.append(l)
        # Repository containing a single file (not an archive)
        l = ihm.location.InputFileLocation(repo=repo3, path='foo.spd',
                                           details='EM micrographs')
        system.locations.append(l)

        with utils.temporary_directory('') as tmpdir:
            bar = os.path.join(tmpdir, 'test_mmcif_extref.tmp')
            with open(bar, 'w') as f:
                f.write("abcd")
            # Local file
            system.locations.append(ihm.location.WorkflowFileLocation(bar))
            # DatabaseLocations should be ignored
            system.locations.append(ihm.location.PDBLocation(
                                              '1abc', '1.0', 'test details'))

            d = ihm.dumper._ExternalReferenceDumper()
            d.finalize(system)
            out = _get_dumper_output(d, system)
            self.assertEqual(out, """#
loop_
_ihm_external_reference_info.reference_id
_ihm_external_reference_info.reference_provider
_ihm_external_reference_info.reference_type
_ihm_external_reference_info.reference
_ihm_external_reference_info.refers_to
_ihm_external_reference_info.associated_url
1 . DOI foo Other .
2 Zenodo DOI 10.5281/zenodo.46266 Archive nup84-v1.0.zip
3 Zenodo DOI 10.5281/zenodo.58025 File foo.spd
4 . 'Supplementary Files' . Other .
#
#
loop_
_ihm_external_files.id
_ihm_external_files.reference_id
_ihm_external_files.file_path
_ihm_external_files.content_type
_ihm_external_files.file_size_bytes
_ihm_external_files.details
1 1 bar 'Input data or restraints' . .
2 1 baz 'Input data or restraints' . .
3 2 foo/bar/baz 'Modeling or post-processing output' . .
4 3 foo.spd 'Input data or restraints' . 'EM micrographs'
5 4 %s 'Modeling workflow or script' 4 .
#
""" % bar.replace(os.sep, '/'))

    def test_dataset_dumper_duplicates_details(self):
        """DatasetDumper ignores duplicate datasets with differing details"""
        system = ihm.System()
        dump = ihm.dumper._DatasetDumper()
        l = ihm.location.PDBLocation('1abc', '1.0', 'test details')
        ds1 = ihm.dataset.PDBDataset(l)
        system.datasets.append(ds1)
        # A duplicate dataset should be ignored even if details differ
        l = ihm.location.PDBLocation('1abc', '1.0', 'other details')
        ds2 = ihm.dataset.PDBDataset(l)
        system.datasets.append(ds2)
        dump.finalize(system) # Assign IDs
        self.assertEqual(ds1._id, 1)
        self.assertEqual(ds2._id, 1)
        self.assertEqual(len(dump._dataset_by_id), 1)

    def test_dataset_dumper_duplicates_samedata_sameloc(self):
        """DatasetDumper doesn't duplicate same datasets in same location"""
        system = ihm.System()
        loc1 = ihm.location.DatabaseLocation("mydb", "abc", "1.0", "")
        loc2 = ihm.location.DatabaseLocation("mydb", "xyz", "1.0", "")

        # Identical datasets in the same location aren't duplicated
        cx1 = ihm.dataset.CXMSDataset(loc1)
        cx2 = ihm.dataset.CXMSDataset(loc1)

        dump = ihm.dumper._DatasetDumper()
        system.datasets.extend((cx1, cx2))
        dump.finalize(system) # Assign IDs
        self.assertEqual(cx1._id, 1)
        self.assertEqual(cx2._id, 1)
        self.assertEqual(len(dump._dataset_by_id), 1)

    def test_dataset_dumper_duplicates_samedata_diffloc(self):
        """DatasetDumper is OK with same datasets in different locations"""
        system = ihm.System()
        loc1 = ihm.location.DatabaseLocation("mydb", "abc", "1.0", "")
        loc2 = ihm.location.DatabaseLocation("mydb", "xyz", "1.0", "")
        cx1 = ihm.dataset.CXMSDataset(loc1)
        cx2 = ihm.dataset.CXMSDataset(loc2)
        dump = ihm.dumper._DatasetDumper()
        system.datasets.extend((cx1, cx2))
        dump.finalize(system) # Assign IDs
        self.assertEqual(cx1._id, 1)
        self.assertEqual(cx2._id, 2)
        self.assertEqual(len(dump._dataset_by_id), 2)

    def test_dataset_dumper_duplicates_diffdata_sameloc(self):
        """DatasetDumper is OK with different datasets in same location"""
        system = ihm.System()
        # Different datasets in same location are OK (but odd)
        loc2 = ihm.location.DatabaseLocation("mydb", "xyz", "1.0", "")
        cx2 = ihm.dataset.CXMSDataset(loc2)
        em3d = ihm.dataset.EMDensityDataset(loc2)
        dump = ihm.dumper._DatasetDumper()
        system.datasets.extend((cx2, em3d))
        dump.finalize(system) # Assign IDs
        self.assertEqual(cx2._id, 1)
        self.assertEqual(em3d._id, 2)
        self.assertEqual(len(dump._dataset_by_id), 2)

    def test_dataset_dumper_allow_duplicates(self):
        """DatasetDumper is OK with duplicates if allow_duplicates=True"""
        system = ihm.System()
        emloc1 = ihm.location.EMDBLocation("abc")
        emloc2 = ihm.location.EMDBLocation("abc")
        emloc1._allow_duplicates = True
        em3d_1 = ihm.dataset.EMDensityDataset(emloc1)
        em3d_2 = ihm.dataset.EMDensityDataset(emloc2)
        dump = ihm.dumper._DatasetDumper()
        system.datasets.extend((em3d_1, em3d_2))
        dump.finalize(system) # Assign IDs
        self.assertEqual(em3d_1._id, 1)
        self.assertEqual(em3d_2._id, 2)
        self.assertEqual(len(dump._dataset_by_id), 2)

    def test_dataset_dumper_group_finalize(self):
        """Test DatasetDumper finalize of dataset groups"""
        system = ihm.System()
        l = ihm.location.InputFileLocation(repo='foo', path='baz')
        ds1 = ihm.dataset.CXMSDataset(l)
        group1 = ihm.dataset.DatasetGroup([ds1])
        # Duplicate group
        group2 = ihm.dataset.DatasetGroup([ds1])

        system.datasets.append(ds1)
        system.dataset_groups.extend((group1, group2))

        d = ihm.dumper._DatasetDumper()
        d.finalize(system) # Assign IDs
        self.assertEqual(len(d._dataset_by_id), 1)
        self.assertEqual(len(d._dataset_group_by_id), 1)

    def test_dataset_dumper_dump(self):
        """Test DatasetDumper.dump()"""
        system = ihm.System()
        l = ihm.location.InputFileLocation(repo='foo', path='bar')
        l._id = 97
        ds1 = ihm.dataset.CXMSDataset(l)
        system.datasets.append(ds1)

        # group1 contains just the first dataset (but duplicated)
        group1 = ihm.dataset.DatasetGroup([ds1, ds1])
        system.dataset_groups.append(group1)

        l = ihm.location.InputFileLocation(repo='foo2', path='bar2')
        l._id = 98
        ds2 = ihm.dataset.CXMSDataset(l)
        # Don't need to add to system.datasets since ds2 is ref'd by ds3

        # group2 contains all datasets so far (ds1 & ds2)
        group2 = ihm.dataset.DatasetGroup([ds1, ds2])
        system.dataset_groups.append(group2)

        l = ihm.location.PDBLocation('1abc', '1.0', 'test details')
        ds3 = ihm.dataset.PDBDataset(l)
        system.datasets.append(ds3)
        ds3.parents.append(ds2)
        # Ignore duplicates
        ds3.parents.append(ds2)

        d = ihm.dumper._DatasetDumper()
        d.finalize(system) # Assign IDs
        out = _get_dumper_output(d, system)
        self.assertEqual(out, """#
loop_
_ihm_dataset_list.id
_ihm_dataset_list.data_type
_ihm_dataset_list.database_hosted
1 'CX-MS data' NO
2 'CX-MS data' NO
3 'Experimental model' YES
#
#
loop_
_ihm_dataset_group.ordinal_id
_ihm_dataset_group.group_id
_ihm_dataset_group.dataset_list_id
1 1 1
2 2 1
3 2 2
#
#
loop_
_ihm_dataset_external_reference.id
_ihm_dataset_external_reference.dataset_list_id
_ihm_dataset_external_reference.file_id
1 1 97
2 2 98
#
#
loop_
_ihm_dataset_related_db_reference.id
_ihm_dataset_related_db_reference.dataset_list_id
_ihm_dataset_related_db_reference.db_name
_ihm_dataset_related_db_reference.accession_code
_ihm_dataset_related_db_reference.version
_ihm_dataset_related_db_reference.details
1 3 PDB 1abc 1.0 'test details'
#
#
loop_
_ihm_related_datasets.ordinal_id
_ihm_related_datasets.dataset_list_id_derived
_ihm_related_datasets.dataset_list_id_primary
1 3 2
#
""")

    def test_model_representation_dump(self):
        """Test ModelRepresentationDumper"""
        system = ihm.System()
        e1 = ihm.Entity('AAAAAAAA', description='bar')
        system.entities.append(e1)
        asym = ihm.AsymUnit(e1, 'foo')
        system.asym_units.append(asym)

        s1 = ihm.representation.AtomicSegment(
                    asym(1,2), starting_model=None, rigid=True)
        s2 = ihm.representation.ResidueSegment(
                    asym(3,4), starting_model=None,
                    rigid=False, primitive='sphere')
        s3 = ihm.representation.MultiResidueSegment(
                    asym(1,2), starting_model=None,
                    rigid=False, primitive='gaussian')
        s4 = ihm.representation.FeatureSegment(
                    asym(3,4), starting_model=None,
                    rigid=True, primitive='other', count=3)
        r1 = ihm.representation.Representation((s1, s2))
        r2 = ihm.representation.Representation((s3, s4))
        system.representations.extend((r1, r2))

        e1._id = 42
        asym._id = 'X'

        dumper = ihm.dumper._ModelRepresentationDumper()
        dumper.finalize(system) # assign IDs
        out = _get_dumper_output(dumper, system)
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
1 1 1 42 bar X 1 2 atomistic . rigid by-atom .
2 1 2 42 bar X 3 4 sphere . flexible by-residue .
3 2 1 42 bar X 1 2 gaussian . flexible multi-residue .
4 2 2 42 bar X 3 4 other . rigid by-feature 3
#
""")

    def test_starting_model_dumper(self):
        """Test StartingModelDumper"""
        system = ihm.System()
        e1 = ihm.Entity('AAA', description='foo')
        system.entities.append(e1)
        asym = ihm.AsymUnit(e1, 'bar')
        system.asym_units.append(asym)
        l = ihm.location.PDBLocation('1abc', '1.0', 'test details')
        ds1 = ihm.dataset.PDBDataset(l)
        system.datasets.append(ds1)

        s1 = ihm.startmodel.PDBSource('1abc', 'C', [])
        sm = ihm.startmodel.StartingModel(asym, ds1, 'A', [s1], offset=10)
        system.starting_models.append(sm)

        e1._id = 42
        asym._id = 99
        ds1._id = 101
        dumper = ihm.dumper._StartingModelDumper()
        dumper.finalize(system) # assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
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
1 42 foo 99 1 3 'experimental model' A 10 101
#
""")

    def test_modeling_protocol(self):
        """Test ProtocolDumper"""
        class MockObject(object):
            pass
        system = ihm.System()
        p1 = ihm.protocol.Protocol('equilibration')
        assembly = ihm.Assembly(description='foo')
        assembly._id = 42
        dsg = MockObject()
        dsg._id = 99
        dsg2 = MockObject()
        dsg2._id = 101
        p1.steps.append(ihm.protocol.Step(assembly=assembly, dataset_group=dsg,
                               method='Monte Carlo', num_models_begin=0,
                               num_models_end=500, multi_scale=True, name='s1'))
        p1.steps.append(ihm.protocol.Step(assembly=assembly, dataset_group=dsg,
                               method='Replica exchange', num_models_begin=500,
                               num_models_end=2000, multi_scale=True))
        system.protocols.append(p1)

        p2 = ihm.protocol.Protocol('sampling')
        p2.steps.append(ihm.protocol.Step(assembly=assembly, dataset_group=dsg2,
                               method='Replica exchange', num_models_begin=2000,
                               num_models_end=1000, multi_scale=True))
        system.protocols.append(p2)

        dumper = ihm.dumper._ProtocolDumper()
        dumper.finalize(system) # assign IDs
        out = _get_dumper_output(dumper, system)
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
_ihm_modeling_protocol.time_ordered_flag
1 1 1 42 99 foo equilibration s1 'Monte Carlo' 0 500 YES NO NO
2 1 2 42 99 foo equilibration . 'Replica exchange' 500 2000 YES NO NO
3 2 1 42 101 foo sampling . 'Replica exchange' 2000 1000 YES NO NO
#
""")

    def test_post_process(self):
        """Test PostProcessDumper"""
        class MockObject(object):
            pass
        system = ihm.System()
        p1 = ihm.protocol.Protocol('refinement')
        system.protocols.append(p1)

        a1 = ihm.analysis.Analysis()
        a1.steps.append(ihm.analysis.EmptyStep())
        a2 = ihm.analysis.Analysis()
        a2.steps.append(ihm.analysis.FilterStep(
                             feature='energy/score', num_models_begin=1000,
                             num_models_end=200))
        a2.steps.append(ihm.analysis.ClusterStep(
                             feature='RMSD', num_models_begin=200,
                             num_models_end=42))
        p1.analyses.extend((a1, a2))

        dumper = ihm.dumper._ProtocolDumper()
        dumper.finalize(system) # assign protocol IDs

        dumper = ihm.dumper._PostProcessDumper()
        dumper.finalize(system) # assign analysis IDs

        out = _get_dumper_output(dumper, system)
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
1 1 1 1 none none . .
2 1 2 1 filter energy/score 1000 200
3 1 2 2 cluster RMSD 200 42
#
""")

    def test_model_dumper(self):
        """Test ModelDumper"""
        class MockObject(object):
            pass
        system = ihm.System()
        state = ihm.model.State()
        system.state_groups.append(ihm.model.StateGroup([state]))
        protocol = MockObject()
        protocol._id = 42
        assembly = MockObject()
        assembly._id = 99
        representation = MockObject()
        representation._id = 32
        model = ihm.model.Model(assembly=assembly, protocol=protocol,
                                representation=representation,
                                name='test model')
        model2 = ihm.model.Model(assembly=assembly, protocol=protocol,
                                 representation=representation,
                                 name='test model2')
        model3 = ihm.model.Model(assembly=assembly, protocol=protocol,
                                 representation=representation,
                                 name='test model3')
        # Existing IDs should be overwritten
        model3._id = 999
        # Group contains multiple copies of model - should be pruned on output
        group = ihm.model.ModelGroup([model, model, model2], name='Group1')
        state.append(group)
        group2 = ihm.model.ModelGroup([model3], name='Group 2')
        state.append(group2)

        dumper = ihm.dumper._ModelDumper()
        dumper.finalize(system) # assign model/group IDs

        out = _get_dumper_output(dumper, system)
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
1 1 1 'test model' Group1 99 42 32
2 2 1 'test model2' Group1 99 42 32
3 3 2 'test model3' 'Group 2' 99 42 32
#
""")

    def _make_test_model(self):
        class MockObject(object):
            pass
        system = ihm.System()
        state = ihm.model.State()
        system.state_groups.append(ihm.model.StateGroup([state]))
        e1 = ihm.Entity('ACGT')
        e1._id = 9
        system.entities.append(e1)
        asym = ihm.AsymUnit(e1, 'foo')
        asym._id = 'X'
        system.asym_units.append(asym)
        protocol = MockObject()
        protocol._id = 42
        assembly = MockObject()
        assembly._id = 99
        representation = MockObject()
        representation._id = 32
        model = ihm.model.Model(assembly=assembly, protocol=protocol,
                                representation=representation,
                                name='test model')

        group = ihm.model.ModelGroup([model])
        state.append(group)
        return system, model, asym

    def test_model_dumper_spheres(self):
        """Test ModelDumper with spheres"""
        system, model, asym = self._make_test_model()
        model._spheres = [ihm.model.Sphere(asym_unit=asym,
                                           seq_id_range=(1,5), x=1.0,
                                           y=2.0, z=3.0, radius=4.0),
                          ihm.model.Sphere(asym_unit=asym,
                                           seq_id_range=(6,6), x=4.0,
                                           y=5.0, z=6.0, radius=1.0, rmsf=8.0)]

        dumper = ihm.dumper._ModelDumper()
        dumper.finalize(system) # assign model/group IDs

        out = _get_dumper_output(dumper, system)
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
1 1 1 'test model' . 99 42 32
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
1 9 1 5 X 1.000 2.000 3.000 4.000 . 1
2 9 6 6 X 4.000 5.000 6.000 1.000 8.000 1
#
""")

    def test_model_dumper_atoms(self):
        """Test ModelDumper with atoms"""
        system, model, asym = self._make_test_model()
        model._atoms = [ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='C',
                                       x=1.0, y=2.0, z=3.0),
                        ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='CA',
                                       x=10.0, y=20.0, z=30.0),
                        ihm.model.Atom(asym_unit=asym, seq_id=2, atom_id='N',
                                       x=4.0, y=5.0, z=6.0)]

        dumper = ihm.dumper._ModelDumper()
        dumper.finalize(system) # assign model/group IDs

        out = _get_dumper_output(dumper, system)
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
1 1 1 'test model' . 99 42 32
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
1 C ALA 1 X 1.000 2.000 3.000 9 1
2 CA ALA 1 X 10.000 20.000 30.000 9 1
3 N CYS 2 X 4.000 5.000 6.000 9 1
#
""")

    def test_ensemble_dumper(self):
        """Test EnsembleDumper"""
        class MockObject(object):
            pass
        pp = MockObject()
        pp._id = 99
        system = ihm.System()
        m1 = ihm.model.Model(assembly='a1', protocol='p1', representation='r1')
        m2 = ihm.model.Model(assembly='a2', protocol='p2', representation='r2')
        group = ihm.model.ModelGroup([m1, m2])
        group._id = 42

        e1 = ihm.model.Ensemble(model_group=group, num_models=10,
                                post_process=pp, name='cluster1',
                                clustering_method='Hierarchical',
                                clustering_feature='RMSD',
                                precision=4.2)
        loc = ihm.location.OutputFileLocation(repo='foo', path='bar')
        loc._id = 3
        e2 = ihm.model.Ensemble(model_group=group, num_models=10,
                                file=loc)
        system.ensembles.extend((e1, e2))

        dumper = ihm.dumper._EnsembleDumper()
        dumper.finalize(system) # assign IDs

        out = _get_dumper_output(dumper, system)
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
1 cluster1 99 42 Hierarchical RMSD 10 2 4.200 .
2 . . 42 . . 10 2 . 3
#
""")

    def test_density_dumper(self):
        """Test DensityDumper"""
        class MockObject(object):
            pass
        system = ihm.System()
        e1 = ihm.Entity('ABCD')
        e1._id = 9
        asym = ihm.AsymUnit(e1)
        asym._id = 'X'

        group = MockObject()
        group._id = 42
        ens = ihm.model.Ensemble(model_group=group, num_models=10)

        loc = ihm.location.OutputFileLocation(repo='foo', path='bar')
        loc._id = 3
        ens.densities.append(ihm.model.LocalizationDensity(loc, asym(1,2)))
        ens.densities.append(ihm.model.LocalizationDensity(loc, asym))
        ens._id = 5
        system.ensembles.append(ens)

        dumper = ihm.dumper._DensityDumper()
        dumper.finalize(system) # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_localization_density_files.id
_ihm_localization_density_files.file_id
_ihm_localization_density_files.ensemble_id
_ihm_localization_density_files.entity_id
_ihm_localization_density_files.asym_id
_ihm_localization_density_files.seq_id_begin
_ihm_localization_density_files.seq_id_end
1 3 5 9 X 1 2
2 3 5 9 X 1 4
#
""")

    def test_single_state(self):
        """Test MultiStateDumper with a single state"""
        system = ihm.System()
        state = ihm.model.State()
        system.state_groups.append(ihm.model.StateGroup([state]))

        dumper = ihm.dumper._MultiStateDumper()
        dumper.finalize(system) # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, "")

    def test_multi_state(self):
        """Test MultiStateDumper with multiple states"""
        system = ihm.System()
        state = ihm.model.State()
        sg1 = ihm.model.StateGroup()
        sg2 = ihm.model.StateGroup()
        system.state_groups.extend((sg1, sg2))

        state1 = ihm.model.State(type='complex formation', name='unbound',
                                 experiment_type="Fraction of bulk",
                                 details="Unbound molecule 1")
        state1.append(ihm.model.ModelGroup(name="group1"))
        state1.append(ihm.model.ModelGroup(name="group2"))

        state2 = ihm.model.State(type='complex formation', name='bound',
                                 experiment_type="Fraction of bulk",
                                 details="Unbound molecule 2")
        state2.append(ihm.model.ModelGroup(name="group3"))
        sg1.extend((state1, state2))

        state3 = ihm.model.State(population_fraction=0.4)
        state3.append(ihm.model.ModelGroup(name="group4"))
        sg2.append(state3)

        dumper = ihm.dumper._ModelDumper()
        dumper.finalize(system) # assign model group IDs

        dumper = ihm.dumper._MultiStateDumper()
        dumper.finalize(system) # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
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
1 1 1 . 'complex formation' unbound 1 'Fraction of bulk' 'Unbound molecule 1'
2 1 1 . 'complex formation' unbound 2 'Fraction of bulk' 'Unbound molecule 1'
3 2 1 . 'complex formation' bound 3 'Fraction of bulk' 'Unbound molecule 2'
4 3 2 0.400 . . 4 . .
#
""")

    def test_em3d_restraint_dumper(self):
        """Test EM3DRestraintDumper"""
        class MockObject(object):
            pass
        system = ihm.System()

        dataset = MockObject()
        dataset._id = 97
        assembly = MockObject()
        assembly._id = 99
        r = ihm.restraint.EM3DRestraint(dataset=dataset, assembly=assembly,
                       segment=False, fitting_method='Gaussian mixture model',
                       number_of_gaussians=40, details='GMM fitting')
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m._id = 42
        m2 = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m2._id = 44
        system.restraints.extend((r, MockObject()))

        r.fits[m] = ihm.restraint.EM3DRestraintFit(0.4)
        r.fits[m2] = ihm.restraint.EM3DRestraintFit()

        dumper = ihm.dumper._EM3DDumper()
        dumper.finalize(system) # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_3dem_restraint.ordinal_id
_ihm_3dem_restraint.dataset_list_id
_ihm_3dem_restraint.fitting_method
_ihm_3dem_restraint.struct_assembly_id
_ihm_3dem_restraint.number_of_gaussians
_ihm_3dem_restraint.model_id
_ihm_3dem_restraint.cross_correlation_coefficient
1 97 'Gaussian mixture model' 99 40 42 0.400
2 97 'Gaussian mixture model' 99 40 44 .
#
""")

    def test_sas_restraint_dumper(self):
        """Test SASRestraintDumper"""
        class MockObject(object):
            pass
        system = ihm.System()

        dataset = MockObject()
        dataset._id = 97
        assembly = MockObject()
        assembly._id = 99
        r = ihm.restraint.SASRestraint(dataset=dataset, assembly=assembly,
                       segment=False, fitting_method='FoXS',
                       fitting_atom_type='Heavy atoms',
                       multi_state=False,
                       radius_of_gyration=21.07, details='FoXS fitting')
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m._id = 42
        m2 = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m2._id = 44
        system.restraints.extend((r, MockObject()))

        r.fits[m] = ihm.restraint.SASRestraintFit(4.69)
        r.fits[m2] = ihm.restraint.SASRestraintFit()

        dumper = ihm.dumper._SASDumper()
        dumper.finalize(system) # assign IDs

        out = _get_dumper_output(dumper, system)
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
1 97 42 99 NO 'Heavy atoms' FoXS Single 21.070 4.690 'FoXS fitting'
2 97 44 99 NO 'Heavy atoms' FoXS Single 21.070 . 'FoXS fitting'
#
""")

    def test_em2d_restraint_dumper(self):
        """Test EM2DRestraintDumper"""
        class MockObject(object):
            pass
        system = ihm.System()

        dataset = MockObject()
        dataset._id = 97
        assembly = MockObject()
        assembly._id = 99
        r = ihm.restraint.EM2DRestraint(dataset=dataset, assembly=assembly,
                       segment=False, number_raw_micrographs=400,
                       pixel_size_width=0.6, pixel_size_height=0.5,
                       image_resolution=30.0, number_of_projections=100,
                       details='Test fit')
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m._id = 42
        m2 = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m2._id = 44
        system.restraints.extend((r, MockObject()))

        r.fits[m] = ihm.restraint.EM2DRestraintFit(
                      cross_correlation_coefficient=0.4,
                      rot_matrix=[[-0.64,0.09,0.77],[0.76,-0.12,0.64],
                                  [0.15,0.99,0.01]],
                      tr_vector=[1.,2.,3.])
        r.fits[m2] = ihm.restraint.EM2DRestraintFit()

        dumper = ihm.dumper._EM2DDumper()
        dumper.finalize(system) # assign IDs

        out = _get_dumper_output(dumper, system)
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
1 97 400 0.600 0.500 30.000 NO 100 99 'Test fit'
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
1 1 42 0.400 -0.640000 0.760000 0.150000 0.090000 -0.120000 0.990000 0.770000
0.640000 0.010000 1.000 2.000 3.000
2 1 44 . . . . . . . . . . . . .
#
""")


if __name__ == '__main__':
    unittest.main()