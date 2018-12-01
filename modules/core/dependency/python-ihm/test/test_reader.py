import utils
import os
from collections import namedtuple
import unittest
import sys
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.reader

def cif_file_handles(cif):
    """Yield both in-memory and real-file handles for the given mmCIF text.
       This allows us to test both the pure Python reader and the C-accelerated
       version with the same input file."""
    yield StringIO(cif)
    with utils.temporary_directory() as tmpdir:
        fname = os.path.join(tmpdir, 'test')
        with open(fname, 'w') as fh:
            fh.write(cif)
        with open(fname) as fh:
            yield fh

ASYM_ENTITY = """
loop_
_entity_poly_seq.entity_id
_entity_poly_seq.num
_entity_poly_seq.mon_id
_entity_poly_seq.hetero
1 1 MET .
1 2 CYS .
1 3 MET .
1 4 SER .
#
loop_
_struct_asym.id
_struct_asym.entity_id
_struct_asym.details
A 1 foo
"""

CENTERS_TRANSFORMS = """
loop_
_ihm_geometric_object_center.id
_ihm_geometric_object_center.xcoord
_ihm_geometric_object_center.ycoord
_ihm_geometric_object_center.zcoord
1 1.000 2.000 3.000
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
1.000000 1.000 2.000 3.000
#
"""

class Tests(unittest.TestCase):
    def test_read(self):
        """Test read() function"""
        cif = "data_model\n_struct.entry_id testid\n"
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            self.assertEqual(s.id, 'testid')

    def test_system_reader(self):
        """Test SystemReader class"""
        s = ihm.reader._SystemReader(ihm.model.Model)

    def test_id_mapper(self):
        """Test IDMapper class"""
        class MockObject(object):
            def __init__(self, x, y):
                self.x, self.y = x, y

        testlist = []
        im = ihm.reader._IDMapper(testlist, MockObject, '1', y='2')
        a = im.get_by_id('ID1')
        b = im.get_by_id('ID1')
        self.assertEqual(id(a), id(b))
        self.assertEqual(a.x, '1')
        self.assertEqual(a.y, '2')
        self.assertEqual(testlist, [a])

    def test_handler(self):
        """Test Handler base class"""
        class MockObject(object):
            pass
        o = MockObject()
        o.system = 'foo'
        h = ihm.reader._Handler(o)
        self.assertEqual(h.system, 'foo')

    def test_handler_copy_if_present(self):
        """Test copy_if_present method"""
        class MockObject(object):
            pass
        Keys = namedtuple('Keys', 'foo bar t test x')
        o = MockObject()
        h = ihm.reader._Handler(None)
        h._copy_if_present(o, {'foo':'bar', 'bar':'baz', 't':'u'},
                           keys=['test', 'foo'],
                           mapkeys={'bar':'baro', 'x':'y'})
        self.assertEqual(o.foo, 'bar')
        self.assertEqual(o.baro, 'baz')
        self.assertFalse(hasattr(o, 't'))
        self.assertFalse(hasattr(o, 'x'))
        self.assertFalse(hasattr(o, 'bar'))

    def test_struct_handler(self):
        """Test StructHandler"""
        cif = "_struct.entry_id eid\n_struct.title 'Test title'"
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            self.assertEqual(s.id, 'eid')
            self.assertEqual(s.title, 'Test title')

    def test_multiple_systems(self):
        """Test multiple systems from data blocks"""
        cif = """
data_id1
_struct.entry_id id1
data_id2
_struct.entry_id id2
data_id3
_struct.entry_id id3
"""
        for fh in cif_file_handles(cif):
            s1, s2, s3 = ihm.reader.read(fh)
            self.assertEqual(s1.id, 'id1')
            self.assertEqual(s2.id, 'id2')
            self.assertEqual(s3.id, 'id3')

    def test_software_handler(self):
        """Test SoftwareHandler"""
        cif = """
loop_
_software.pdbx_ordinal
_software.name
_software.classification
_software.description
_software.version
_software.type
_software.location
1 'test software' 'test class' 'test desc' program 1.0.1 https://example.org
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            software, = s.software
            self.assertEqual(software._id, '1')
            self.assertEqual(software.name, 'test software')
            self.assertEqual(software.classification, 'test class')

    def test_citation_handler(self):
        """Test CitationHandler and CitationAuthorHandler"""
        cif = """
loop_
_citation.id
_citation.journal_abbrev
_citation.journal_volume
_citation.page_first
_citation.page_last
_citation.year
_citation.pdbx_database_id_PubMed
_citation.pdbx_database_id_DOI
2 'Mol Cell Proteomics' 9 2943 . 2014 1234 .
3 'Mol Cell Proteomics' 9 2943 2946 2014 1234 1.2.3.4
4 'Mol Cell Proteomics' 9 . . 2014 1234 1.2.3.4
#
#
loop_
_citation_author.citation_id
_citation_author.name
_citation_author.ordinal
3 'Foo A' 1
3 'Bar C' 2
3 . 3
5 'Baz X' 4
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            citation1, citation2, citation3, citation4 = s.citations
            self.assertEqual(citation1._id, '2')
            self.assertEqual(citation1.page_range, '2943')
            self.assertEqual(citation1.authors, [])
            self.assertEqual(citation1.pmid, '1234')
            self.assertEqual(citation1.doi, None)

            self.assertEqual(citation2._id, '3')
            self.assertEqual(citation2.page_range, ('2943', '2946'))
            self.assertEqual(citation2.authors, ['Foo A', 'Bar C'])
            self.assertEqual(citation2.doi, '1.2.3.4')

            self.assertEqual(citation3._id, '4')
            self.assertEqual(citation3.authors, [])
            self.assertEqual(citation3.page_range, None)

            # todo: should probably be an error, no _citation.id == 4
            self.assertEqual(citation4._id, '5')
            self.assertEqual(citation4.authors, ['Baz X'])

    def test_chem_comp_handler(self):
        """Test ChemCompHandler and EntityPolySeqHandler"""
        chem_comp_cat = """
loop_
_chem_comp.id
_chem_comp.type
_chem_comp.name
_chem_comp.formula
MET 'L-peptide linking' . .
CYS 'D-peptide linking' CYSTEINE .
MYTYPE 'D-PEPTIDE LINKING' 'MY CUSTOM COMPONENT' 'C6 H12'
"""
        entity_poly_cat = """
loop_
_entity_poly_seq.entity_id
_entity_poly_seq.num
_entity_poly_seq.mon_id
_entity_poly_seq.hetero
1 1 MET .
1 4 MYTYPE .
1 5 CYS .
1 2 MET .
"""
        cif1 = chem_comp_cat + entity_poly_cat
        cif2 = entity_poly_cat + chem_comp_cat
        # Order of the two categories shouldn't matter
        for cif in cif1, cif2:
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                e1, = s.entities
                s = e1.sequence
                self.assertEqual(len(s), 5)
                lpeptide = ihm.LPeptideAlphabet()
                self.assertEqual(id(s[0]), id(lpeptide['M']))
                self.assertEqual(id(s[1]), id(lpeptide['M']))
                self.assertEqual(id(s[4]), id(lpeptide['C']))
                self.assertEqual(s[0].name, 'METHIONINE')
                self.assertEqual(s[2], None)
                self.assertEqual(s[3].id, 'MYTYPE')
                self.assertEqual(s[3].type, 'D-peptide linking')
                self.assertEqual(s[3].name, 'MY CUSTOM COMPONENT')
                self.assertAlmostEqual(s[3].formula_weight, 84.162, places=1)
                self.assertEqual(s[3].__class__, ihm.DPeptideChemComp)
                # Class of standard type shouldn't be changed
                self.assertEqual(s[4].type, 'L-peptide linking')
                self.assertEqual(s[4].__class__, ihm.LPeptideChemComp)

    def test_chem_comp_nonpoly_handler(self):
        """Test ChemCompHandler and EntityNonPolyHandler"""
        chem_comp_cat = """
loop_
_chem_comp.id
_chem_comp.type
HEM non-polymer
HOH non-polymer
"""
        entity_nonpoly_cat = """
loop_
_pdbx_entity_nonpoly.entity_id
_pdbx_entity_nonpoly.name
_pdbx_entity_nonpoly.comp_id
1 Heme HEM
2 Water HOH
"""
        cif1 = chem_comp_cat + entity_nonpoly_cat
        cif2 = entity_nonpoly_cat + chem_comp_cat
        # Order of the two categories shouldn't matter
        for cif in cif1, cif2:
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                e1, e2 = s.entities
                s = e1.sequence
                self.assertEqual(len(s), 1)
                self.assertEqual(s[0].id, 'HEM')
                self.assertEqual(s[0].type, 'non-polymer')
                self.assertEqual(s[0].__class__, ihm.NonPolymerChemComp)
                s = e2.sequence
                self.assertEqual(len(s), 1)
                self.assertEqual(s[0].id, 'HOH')
                self.assertEqual(s[0].type, 'non-polymer')
                self.assertEqual(s[0].__class__, ihm.WaterChemComp)

    def test_entity_handler(self):
        """Test EntityHandler"""
        cif = """
loop_
_entity.id
_entity.type
_entity.pdbx_description
_entity.pdbx_number_of_molecules
_entity.formula_weight
_entity.details
1 polymer Nup84 2 100.0 .
2 polymer Nup85 3 200.0 .
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            e1, e2 = s.entities
            self.assertEqual(e1.description, 'Nup84')
            self.assertEqual(e1.number_of_molecules, '2') # todo: coerce to int

    def test_asym_unit_handler(self):
        """Test AsymUnitHandler"""
        cif = """
loop_
_struct_asym.id
_struct_asym.entity_id
_struct_asym.details
A 1 Nup84
B 1 Nup85
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            a1, a2 = s.asym_units
            self.assertEqual(a1._id, 'A')
            self.assertEqual(a1.entity._id, '1')

            self.assertEqual(a1.details, 'Nup84')
            self.assertEqual(a2.entity._id, '1')
            self.assertEqual(a2._id, 'B')
            self.assertEqual(a2.details, 'Nup85')
            self.assertEqual(id(a1.entity), id(a2.entity))

    def test_assembly_details_handler(self):
        """Test AssemblyDetailsHandler"""
        cif = """
loop_
_ihm_struct_assembly_details.assembly_id
_ihm_struct_assembly_details.assembly_name
_ihm_struct_assembly_details.assembly_description
1 'Complete assembly' 'All known components'
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            a1, = s.orphan_assemblies
            self.assertEqual(a1._id, '1')
            self.assertEqual(a1.name, 'Complete assembly')
            self.assertEqual(a1.description, 'All known components')

    def test_assembly_handler(self):
        """Test AssemblyHandler"""
        cif = """
loop_
_ihm_struct_assembly.ordinal_id
_ihm_struct_assembly.assembly_id
_ihm_struct_assembly.parent_assembly_id
_ihm_struct_assembly.entity_description
_ihm_struct_assembly.entity_id
_ihm_struct_assembly.asym_id
_ihm_struct_assembly.seq_id_begin
_ihm_struct_assembly.seq_id_end
1 1 1 Nup84 1 A 1 726
2 1 1 Nup85 2 B 1 744
3 2 1 Nup86 2 . 1 50
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            a1, a2 = s.orphan_assemblies
            self.assertEqual(a1._id, '1')
            self.assertEqual(a1.parent, None)
            self.assertEqual(len(a1), 2)
            # AsymUnitRange
            self.assertEqual(a1[0]._id, 'A')
            self.assertEqual(a1[0].seq_id_range, (1,726))
            self.assertEqual(a1[1]._id, 'B')
            self.assertEqual(a1[1].seq_id_range, (1,744))

            self.assertEqual(a2._id, '2')
            self.assertEqual(a2.parent, a1)
            # EntityRange
            self.assertEqual(len(a2), 1)
            self.assertEqual(a2[0]._id, '2')
            self.assertEqual(a2[0].seq_id_range, (1,50))

    def test_external_file_handler(self):
        """Test ExtRef and ExtFileHandler"""
        ext_ref_cat = """
loop_
_ihm_external_reference_info.reference_id
_ihm_external_reference_info.reference_provider
_ihm_external_reference_info.reference_type
_ihm_external_reference_info.reference
_ihm_external_reference_info.refers_to
_ihm_external_reference_info.associated_url
1 Zenodo DOI 10.5281/zenodo.1218053 Archive https://example.com/foo.zip
2 . 'Supplementary Files' . Other .
3 Zenodo DOI 10.5281/zenodo.1218058 File https://example.com/foo.dcd
"""
        ext_file_cat = """
loop_
_ihm_external_files.id
_ihm_external_files.reference_id
_ihm_external_files.file_path
_ihm_external_files.content_type
_ihm_external_files.details
1 1 scripts/test.py 'Modeling workflow or script' 'Test script'
2 2 foo/bar.txt 'Input data or restraints' 'Test text'
3 3 . 'Modeling or post-processing output' 'Ensemble structures'
4 3 . . .
"""
        # Order of the categories shouldn't matter
        cif1 = ext_ref_cat + ext_file_cat
        cif2 = ext_file_cat + ext_ref_cat
        for cif in cif1, cif2:
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                l1, l2, l3, l4 = s.locations
                self.assertEqual(l1.path, 'scripts/test.py')
                self.assertEqual(l1.details, 'Test script')
                self.assertEqual(l1.repo.doi, '10.5281/zenodo.1218053')
                self.assertEqual(l1.__class__,
                                 ihm.location.WorkflowFileLocation)

                self.assertEqual(l2.path, 'foo/bar.txt')
                self.assertEqual(l2.details, 'Test text')
                self.assertEqual(l2.repo, None)
                self.assertEqual(l2.__class__, ihm.location.InputFileLocation)

                self.assertEqual(l3.path, '.')
                self.assertEqual(l3.details, 'Ensemble structures')
                self.assertEqual(l3.repo.doi, '10.5281/zenodo.1218058')
                self.assertEqual(l3.__class__, ihm.location.OutputFileLocation)

                self.assertEqual(l4.path, '.')
                self.assertEqual(l4.details, None)
                self.assertEqual(l4.repo.doi, '10.5281/zenodo.1218058')
                # Type is unspecified
                self.assertEqual(l4.__class__, ihm.location.FileLocation)

    def test_dataset_list_handler(self):
        """Test DatasetListHandler"""
        cif = """
loop_
_ihm_dataset_list.id
_ihm_dataset_list.data_type
_ihm_dataset_list.database_hosted
1 'Experimental model' YES
2 'COMPARATIVE MODEL' YES
3 'EM raw micrographs' YES
4 . YES
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            d1, d2, d3, d4 = s.orphan_datasets
            self.assertEqual(d1.__class__, ihm.dataset.PDBDataset)
            self.assertEqual(d2.__class__, ihm.dataset.ComparativeModelDataset)
            self.assertEqual(d3.__class__, ihm.dataset.EMMicrographsDataset)
            # No specified data type - use base class
            self.assertEqual(d4.__class__, ihm.dataset.Dataset)

    def test_dataset_group_handler(self):
        """Test DatasetGroupHandler"""
        cif = """
loop_
_ihm_dataset_group.ordinal_id
_ihm_dataset_group.group_id
_ihm_dataset_group.dataset_list_id
1 1 1
2 1 2
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            d1, d2 = s.orphan_datasets
            g1, = s.orphan_dataset_groups
            self.assertEqual(len(g1), 2)
            self.assertEqual(g1[0], d1)
            self.assertEqual(g1[1], d2)
            # No type specified yet
            self.assertEqual(d1.__class__, ihm.dataset.Dataset)
            self.assertEqual(d2.__class__, ihm.dataset.Dataset)

    def test_dataset_extref_handler(self):
        """Test DatasetExtRefHandler"""
        cif = """
loop_
_ihm_dataset_external_reference.id
_ihm_dataset_external_reference.dataset_list_id
_ihm_dataset_external_reference.file_id
1 4 11
2 6 12
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            d1, d2 = s.orphan_datasets
            self.assertEqual(d1._id, '4')
            self.assertEqual(d1.location._id, '11')
            self.assertEqual(d2._id, '6')
            self.assertEqual(d2.location._id, '12')

    def test_dataset_dbref_handler(self):
        """Test DatasetDBRefHandler"""
        cif = """
loop_
_ihm_dataset_related_db_reference.id
_ihm_dataset_related_db_reference.dataset_list_id
_ihm_dataset_related_db_reference.db_name
_ihm_dataset_related_db_reference.accession_code
_ihm_dataset_related_db_reference.version
_ihm_dataset_related_db_reference.details
1 1 PDB 3JRO . .
2 3 PDB 3F3F 30-OCT-08 'CRYSTAL STRUCTURE'
3 5 emdb EMD-123 . .
4 6 . . . .
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            d1, d2, d3, d4 = s.orphan_datasets
            self.assertEqual(d1.location.db_name, 'PDB')
            self.assertEqual(d1.location.__class__, ihm.location.PDBLocation)
            self.assertEqual(d1.location.access_code, '3JRO')
            self.assertEqual(d2.location.db_name, 'PDB')
            self.assertEqual(d2.location.__class__, ihm.location.PDBLocation)
            self.assertEqual(d2.location.access_code, '3F3F')
            self.assertEqual(d2.location.version, '30-OCT-08')
            self.assertEqual(d2.location.details, 'CRYSTAL STRUCTURE')
            self.assertEqual(d3.location.db_name, 'EMDB')
            self.assertEqual(d3.location.__class__, ihm.location.EMDBLocation)
            self.assertEqual(d3.location.access_code, 'EMD-123')
            self.assertEqual(d3.location.version, None)
            self.assertEqual(d3.location.details, None)
            self.assertEqual(d4.location.db_name, None)
            self.assertEqual(d4.location.__class__,
                             ihm.location.DatabaseLocation)
            self.assertEqual(d4.location.access_code, None)

    def test_related_datasets_handler(self):
        """Test RelatedDatasetsHandler"""
        cif = """
loop_
_ihm_related_datasets.ordinal_id
_ihm_related_datasets.dataset_list_id_derived
_ihm_related_datasets.dataset_list_id_primary
1 4 1
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            d1, d2 = s.orphan_datasets
            self.assertEqual(d1._id, '4')
            self.assertEqual(d2._id, '1')
            self.assertEqual(d1.parents, [d2])
            self.assertEqual(d2.parents, [])

    def test_model_representation_handler(self):
        """Test ModelRepresentationHandler"""
        cif = """
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
1 1 1 1 Nup84 A 1 6 sphere . flexible by-feature 1
2 1 2 1 Nup84 A 7 20 sphere 1 rigid by-residue .
3 2 1 1 Nup84 A . . atomistic . flexible by-atom .
4 3 1 2 Nup85 B . . sphere . . multi-residue .
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            r1, r2, r3 = s.orphan_representations
            self.assertEqual(len(r1), 2)
            s1, s2 = r1
            self.assertEqual(s1.__class__, ihm.representation.FeatureSegment)
            self.assertEqual(s1.primitive, 'sphere')
            self.assertEqual(s1.count, 1)
            self.assertEqual(s1.rigid, False)
            self.assertEqual(s1.starting_model, None)
            self.assertEqual(s1.asym_unit.seq_id_range, (1,6))

            self.assertEqual(s2.__class__, ihm.representation.ResidueSegment)
            self.assertEqual(s2.primitive, 'sphere')
            self.assertEqual(s2.count, None)
            self.assertEqual(s2.rigid, True)
            self.assertEqual(s2.starting_model._id, '1')
            self.assertEqual(s2.asym_unit.seq_id_range, (7,20))

            self.assertEqual(len(r2), 1)
            s1, = r2
            self.assertEqual(s1.__class__, ihm.representation.AtomicSegment)

            self.assertEqual(len(r3), 1)
            s1, = r3
            self.assertEqual(s1.__class__,
                             ihm.representation.MultiResidueSegment)

    def test_starting_model_details_handler(self):
        """Test StartingModelDetailsHandler"""
        cif = """
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
1 1 Nup84 A 7 483 'comparative model' Q 8 4
2 1 Nup84 A . . 'comparative model' X . 6
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            m1, m2 = s.orphan_starting_models
            self.assertEqual(m1.asym_unit._id, 'A')
            self.assertEqual(m1.asym_unit.seq_id_range, (7,483))
            self.assertEqual(m1.asym_id, 'Q')
            self.assertEqual(m1.offset, 8)
            self.assertEqual(m1.dataset._id, '4')

            self.assertEqual(m2.asym_unit._id, 'A')
            self.assertEqual(m2.asym_id, 'X')
            self.assertEqual(m2.offset, 0)
            self.assertEqual(m2.dataset._id, '6')

    def test_starting_computational_models_handler(self):
        """Test StartingComputationModelsHandler"""
        cif = """
loop_
_ihm_starting_computational_models.starting_model_id
_ihm_starting_computational_models.software_id
_ihm_starting_computational_models.script_file_id
1 99 8
2 . .
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            m1, m2 = s.orphan_starting_models
            self.assertEqual(m1.script_file._id, '8')
            self.assertEqual(m1.software._id, '99')
            self.assertEqual(m2.script_file, None)
            self.assertEqual(m2.software, None)

    def test_starting_comparative_models_handler(self):
        """Test StartingComparativeModelsHandler"""
        cif = """
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
1 1 A 7 436 C 9 438 90.000 1 3 2
2 1 A 33 424 C 33 424 100.000 1 1 .
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            m1, = s.orphan_starting_models
            t1, t2 = m1.templates
            self.assertEqual(t1.dataset._id, '3')
            self.assertEqual(t1.asym_id, 'C')
            self.assertEqual(t1.seq_id_range, (7,436))
            self.assertEqual(t1.template_seq_id_range, (9,438))
            self.assertAlmostEqual(float(t1.sequence_identity), 90.0, places=1)
            self.assertEqual(t1.sequence_identity.denominator, 1)
            self.assertEqual(t1.alignment_file._id, '2')
            self.assertEqual(t2.alignment_file, None)

    def test_protocol_handler(self):
        """Test ProtocolHandler"""
        cif = """
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
_ihm_modeling_protocol.software_id
_ihm_modeling_protocol.script_file_id
1 1 1 1 1 . Prot1 Sampling 'Monte Carlo' 0 500 YES NO NO . .
2 1 2 1 2 . Prot1 Sampling 'Monte Carlo' 500 5000 YES . NO 401 501
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            p1, = s.orphan_protocols
            self.assertEqual(p1.name, "Prot1")
            self.assertEqual(len(p1.steps), 2)
            self.assertEqual(p1.steps[0].assembly._id, '1')
            self.assertEqual(p1.steps[0].dataset_group._id, '1')
            self.assertEqual(p1.steps[0].name, 'Sampling')
            self.assertEqual(p1.steps[0].method, 'Monte Carlo')
            self.assertEqual(p1.steps[0].num_models_begin, 0)
            self.assertEqual(p1.steps[0].num_models_end, 500)
            self.assertEqual(p1.steps[0].multi_scale, True)
            self.assertEqual(p1.steps[0].multi_state, False)
            self.assertEqual(p1.steps[0].ordered, False)
            self.assertEqual(p1.steps[0].software, None)
            self.assertEqual(p1.steps[0].script_file, None)
            self.assertEqual(p1.steps[1].multi_scale, True)
            self.assertEqual(p1.steps[1].multi_state, None)
            self.assertEqual(p1.steps[1].ordered, False)
            self.assertEqual(p1.steps[1].software._id, '401')
            self.assertEqual(p1.steps[1].script_file._id, '501')

    def test_post_process_handler(self):
        """Test PostProcessHandler"""
        cif = """
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
1  1   1   1   'filter'  'energy/score'  15000   6520 . . 401 501
2  1   1   2   'cluster' 'dRMSD'         6520    6520 . . . .
3  1   2   1   'filter'  'energy/score'  15000   6520 . . . .
4  1   2   2   'filter'  'composition'   6520    6520 . . . .
5  1   2   3   'cluster' 'dRMSD'         6520    6520 . . . .
6  2   3   1   'none' .         .    . . . . .
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            p1, p2 = s.orphan_protocols
            self.assertEqual(len(p1.analyses), 2)
            self.assertEqual(len(p2.analyses), 1)
            a1, a2 = p1.analyses
            self.assertEqual(len(a1.steps), 2)
            self.assertEqual(a1.steps[0].__class__, ihm.analysis.FilterStep)
            self.assertEqual(a1.steps[0].feature, 'energy/score')
            self.assertEqual(a1.steps[0].num_models_begin, 15000)
            self.assertEqual(a1.steps[0].num_models_end, 6520)
            self.assertEqual(a1.steps[0].software._id, '401')
            self.assertEqual(a1.steps[0].script_file._id, '501')
            self.assertEqual(a1.steps[1].__class__, ihm.analysis.ClusterStep)
            self.assertEqual(a1.steps[1].software, None)
            self.assertEqual(a1.steps[1].script_file, None)
            self.assertEqual(len(a2.steps), 3)

            a1, = p2.analyses
            self.assertEqual(len(a1.steps), 1)
            self.assertEqual(a1.steps[0].__class__, ihm.analysis.EmptyStep)
            self.assertEqual(a1.steps[0].feature, 'none')
            self.assertEqual(a1.steps[0].num_models_begin, None)
            self.assertEqual(a1.steps[0].num_models_end, None)

    def test_model_list_handler(self):
        """Test ModelListHandler"""
        cif = """
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_name
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 1 1 'Best scoring model' 'Cluster 1' 1 2 3
2 2 2 'Best scoring model' 'Cluster 2' 1 1 1
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            sg, = s.state_groups
            state, = sg
            self.assertEqual(state.name, None) # auto-created state
            mg1, mg2 = state
            self.assertEqual(mg1.name, 'Cluster 1')
            self.assertEqual(mg1._id, '1')
            m, = mg1
            self.assertEqual(m._id, '1')
            self.assertEqual(m.name, 'Best scoring model')
            self.assertEqual(m.assembly._id, '1')
            self.assertEqual(m.protocol._id, '2')
            self.assertEqual(m.representation._id, '3')
            self.assertEqual(mg2.name, 'Cluster 2')
            self.assertEqual(mg2._id, '2')
            m, = mg2
            self.assertEqual(m._id, '2')

    def test_multi_state_handler(self):
        """Test MultiStateHandler"""
        cif = """
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_name
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 1 1 'Best scoring model' 'Cluster 1' 1 2 3
#
#
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
1 1 1 0.4 'complex formation' 'unbound' 1  'Fraction of bulk'  'unbound molecule 1'
2 2 1 .  'complex formation' 'unbound' 2  'Fraction of bulk'  'unbound molecule 2'
3 3 1 .  'complex formation' 'bound'   3  'Fraction of bulk'  'bound molecules 1 and 2'
4 3 1 .  'complex formation' 'bound'   4  'Fraction of bulk'  'bound molecules 1 and 2'
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            sg, = s.state_groups
            s1, s2, s3, = sg
            self.assertAlmostEqual(s1.population_fraction, 0.4, places=1)
            self.assertEqual(s1.type, 'complex formation')
            self.assertEqual(s1.name, 'unbound')
            self.assertEqual(len(s1), 1)
            mg1, = s1
            self.assertEqual(mg1.name, 'Cluster 1')
            self.assertEqual(s1.experiment_type, 'Fraction of bulk')
            self.assertEqual(s1.details, 'unbound molecule 1')

            self.assertEqual(s2.population_fraction, None)
            self.assertEqual(len(s2), 1)
            self.assertEqual(len(s3), 2)

    def test_ensemble_handler(self):
        """Test EnsembleHandler"""
        cif = """
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
1 'Cluster 1' 2 3 . dRMSD 1257 1 15.400 9
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            e, = s.ensembles
            self.assertEqual(e.model_group._id, '3')
            self.assertEqual(e.num_models, 1257)
            self.assertEqual(e.post_process._id, '2')
            self.assertEqual(e.clustering_method, None)
            self.assertEqual(e.clustering_feature, 'dRMSD')
            self.assertEqual(e.name, 'Cluster 1')
            self.assertAlmostEqual(e.precision, 15.4, places=1)
            self.assertEqual(e.file._id, '9')

    def test_density_handler(self):
        """Test DensityHandler"""
        cif = """
loop_
_ihm_localization_density_files.id
_ihm_localization_density_files.file_id
_ihm_localization_density_files.ensemble_id
_ihm_localization_density_files.entity_id
_ihm_localization_density_files.asym_id
_ihm_localization_density_files.seq_id_begin
_ihm_localization_density_files.seq_id_end
1 22 9 1 A 1 726
2 23 9 2 B . .
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            e, = s.ensembles
            self.assertEqual(e._id, '9')
            d1, d2 = e.densities
            self.assertEqual(d1._id, '1')
            self.assertEqual(d1.file._id, '22')
            self.assertEqual(d1.asym_unit.__class__, ihm.AsymUnitRange)
            self.assertEqual(d1.asym_unit.seq_id_range, (1,726))
            self.assertEqual(d2._id, '2')
            self.assertEqual(d2.asym_unit.__class__, ihm.AsymUnit)

    def test_em3d_restraint_handler(self):
        """Test EM3DRestraintHandler"""
        fh = StringIO("""
loop_
_ihm_3dem_restraint.ordinal_id
_ihm_3dem_restraint.dataset_list_id
_ihm_3dem_restraint.fitting_method
_ihm_3dem_restraint.fitting_method_citation_id
_ihm_3dem_restraint.struct_assembly_id
_ihm_3dem_restraint.number_of_gaussians
_ihm_3dem_restraint.model_id
_ihm_3dem_restraint.cross_correlation_coefficient
1 26 'Gaussian mixture models' 9 2 400 1 .
2 26 'Gaussian mixture models' 9 2 400 2 0.9
""")
        s, = ihm.reader.read(fh)
        r, = s.restraints
        self.assertEqual(r.dataset._id, '26')
        self.assertEqual(r.fitting_method, 'Gaussian mixture models')
        self.assertEqual(r.fitting_method_citation._id, '9')
        self.assertEqual(r.assembly._id, '2')
        self.assertEqual(r.number_of_gaussians, 400)
        # Sort fits by model ID
        fits = sorted(r.fits.items(), key=lambda x:x[0]._id)
        self.assertEqual(len(fits), 2)
        self.assertEqual(fits[0][0]._id, '1')
        self.assertEqual(fits[0][1].cross_correlation_coefficient, None)
        self.assertEqual(fits[1][0]._id, '2')
        self.assertAlmostEqual(fits[1][1].cross_correlation_coefficient,
                               0.9, places=1)

    def test_get_int_or_string(self):
        """Test _get_int_or_string function"""
        self.assertEqual(ihm.reader._get_int_or_string('45A'), '45A')
        self.assertEqual(ihm.reader._get_int_or_string('45'), 45)
        self.assertEqual(ihm.reader._get_int_or_string(None), None)
        self.assertEqual(ihm.reader._get_int_or_string(45), 45)

    def test_get_vector3(self):
        """Test _get_vector3 function"""
        d = {'tr_vector1':4.0, 'tr_vector2':6.0, 'tr_vector3':9.0,
             'not_there1':None}
        r = ihm.reader._get_vector3(d, 'tr_vector')
        # Coerce to int so we can compare exactly
        self.assertEqual([int(x) for x in r], [4,6,9])

        self.assertEqual(ihm.reader._get_vector3(d, 'not_there'), None)

    def test_get_matrix33(self):
        """Test _get_matrix33 function"""
        d = {'m11':4.0, 'm12':6.0, 'm13':9.0,
             'm21':1.0, 'm22':2.0, 'm23':3.0,
             'm31':8.0, 'm32':1.0, 'm33':7.0, 'not_there11':None}
        r = ihm.reader._get_matrix33(d, 'm')
        # Coerce to int so we can compare exactly
        self.assertEqual([[int(x) for x in row] for row in r],
                         [[4,6,9],
                          [1,2,3],
                          [8,1,7]])

        self.assertEqual(ihm.reader._get_matrix33(d, 'not_there'), None)

    def test_em2d_restraint_handler(self):
        """Test EM2DRestraintHandler"""
        fh = StringIO("""
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
1 65 800 2.030 4.030 35.000 NO 10000 42 .
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
1 1 9 0.853 -0.637588 0.089507 0.765160 0.755616 -0.120841 0.643771 0.150085
0.988628 0.009414 327.161 83.209 -227.800
""")
        s, = ihm.reader.read(fh)
        r, = s.restraints
        self.assertEqual(r._id, '1')
        self.assertEqual(r.dataset._id, '65')
        self.assertEqual(r.number_raw_micrographs, 800)
        self.assertAlmostEqual(r.pixel_size_width, 2.030, places=2)
        self.assertAlmostEqual(r.pixel_size_height, 4.030, places=2)
        self.assertAlmostEqual(r.image_resolution, 35.0, places=1)
        self.assertEqual(r.segment, False)
        self.assertEqual(r.number_of_projections, 10000)
        self.assertEqual(r.assembly._id, '42')
        fit, = list(r.fits.items())
        self.assertEqual(fit[0]._id, '9')
        self.assertAlmostEqual(fit[1].cross_correlation_coefficient, 0.853,
                               places=2)
        self.assertAlmostEqual(fit[1].tr_vector[0], 327.161, places=2)
        self.assertAlmostEqual(fit[1].rot_matrix[1][2], 0.988628, places=2)
        self.assertEqual([int(x) for x in fit[1].tr_vector], [327, 83, -227])

    def test_sas_restraint_handler(self):
        """Test SASRestraintHandler"""
        fh = StringIO("""
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
1 27 8 3 NO 'Heavy atoms' FoXS Single 27.9 1.36 .
""")
        s, = ihm.reader.read(fh)
        r, = s.restraints
        self.assertEqual(r.dataset._id, '27')
        self.assertEqual(r.assembly._id, '3')
        self.assertEqual(r.segment, False)
        self.assertEqual(r.fitting_method, 'FoXS')
        self.assertEqual(r.fitting_atom_type, 'Heavy atoms')
        self.assertEqual(r.multi_state, False)
        self.assertAlmostEqual(r.radius_of_gyration, 27.9, places=1)
        fit, = list(r.fits.items())
        self.assertEqual(fit[0]._id, '8')
        self.assertAlmostEqual(fit[1].chi_value, 1.36, places=2)

    def test_sphere_obj_site_handler(self):
        """Test SphereObjSiteHandler"""
        class MyModel(ihm.model.Model):
            def add_sphere(self, sphere):
                super(MyModel, self).add_sphere(sphere)
                self.sphere_count = len(self._spheres)

        fh = StringIO("""
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_name
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 1 1 . 'Cluster 1' 1 1 1
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
1 1 1 6 A 389.993 145.089 134.782 4.931 . 1
2 1 7 7 B 406.895 142.176 135.653 3.318 1.34 1
""")
        s, = ihm.reader.read(fh, model_class=MyModel)
        m = s.state_groups[0][0][0][0]
        self.assertEqual(m.sphere_count, 2)
        s1, s2 = m._spheres
        self.assertEqual(s1.asym_unit._id, 'A')
        self.assertEqual(s1.seq_id_range, (1,6))
        self.assertAlmostEqual(s1.x, 389.993, places=2)
        self.assertAlmostEqual(s1.y, 145.089, places=2)
        self.assertAlmostEqual(s1.z, 134.782, places=2)
        self.assertAlmostEqual(s1.radius, 4.931, places=2)
        self.assertEqual(s1.rmsf, None)
        self.assertAlmostEqual(s2.rmsf, 1.34, places=1)

    def test_atom_site_handler(self):
        """Test AtomSiteHandler"""
        fh = StringIO("""
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_name
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 1 1 . 'Cluster 1' 1 1 1
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
_atom_site.label_entity_id
_atom_site.auth_asym_id
_atom_site.B_iso_or_equiv
_atom_site.pdbx_PDB_model_num
_atom_site.ihm_model_id
ATOM 1 N N . SER 1 A 54.401 -49.984 -35.287 1 A . 1 1
HETATM 2 C CA . SER . B 54.452 -48.492 -35.210 1 A 42.0 1 1
""")
        s, = ihm.reader.read(fh)
        m = s.state_groups[0][0][0][0]
        a1, a2 = m._atoms
        self.assertEqual(a1.asym_unit._id, 'A')
        self.assertEqual(a1.seq_id, 1)
        self.assertEqual(a1.atom_id, 'N')
        self.assertEqual(a1.type_symbol, 'N')
        self.assertAlmostEqual(a1.x, 54.401, places=2)
        self.assertAlmostEqual(a1.y, -49.984, places=2)
        self.assertAlmostEqual(a1.z, -35.287, places=2)
        self.assertEqual(a1.het, False)
        self.assertEqual(a1.biso, None)

        self.assertEqual(a2.asym_unit._id, 'B')
        self.assertEqual(a2.seq_id, None)
        self.assertEqual(a2.atom_id, 'CA')
        self.assertEqual(a2.type_symbol, 'C')
        self.assertEqual(a2.het, True)
        self.assertAlmostEqual(a2.biso, 42.0, places=0)

    def test_atom_site_handler_auth_seq_id(self):
        """Test AtomSiteHandler handling of auth_seq_id"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_name
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 1 1 . 'Cluster 1' 1 1 1
#
loop_
_atom_site.group_PDB
_atom_site.id
_atom_site.type_symbol
_atom_site.label_atom_id
_atom_site.label_alt_id
_atom_site.label_comp_id
_atom_site.label_seq_id
_atom_site.auth_seq_id
_atom_site.label_asym_id
_atom_site.Cartn_x
_atom_site.Cartn_y
_atom_site.Cartn_z
_atom_site.label_entity_id
_atom_site.auth_asym_id
_atom_site.B_iso_or_equiv
_atom_site.pdbx_PDB_model_num
_atom_site.ihm_model_id
ATOM 1 N N . SER 1 2 A 54.401 -49.984 -35.287 1 A . 1 1
HETATM 2 C CA . SER 2 20A A 54.452 -48.492 -35.210 1 A 42.0 1 1
ATOM 3 N N . SER 3 3 A 54.401 -49.984 -35.287 1 A . 1 1
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map, {1: 2, 2: '20A'})

    def test_derived_distance_restraint_handler(self):
        """Test DerivedDistanceRestraintHandler"""
        feats = """
loop_
_ihm_poly_atom_feature.ordinal_id
_ihm_poly_atom_feature.feature_id
_ihm_poly_atom_feature.entity_id
_ihm_poly_atom_feature.asym_id
_ihm_poly_atom_feature.seq_id
_ihm_poly_atom_feature.comp_id
_ihm_poly_atom_feature.atom_id
1 1 1 A 1 ALA CA
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
1 2 1 B 2 CYS 3 GLY
#
loop_
_ihm_non_poly_feature.ordinal_id
_ihm_non_poly_feature.feature_id
_ihm_non_poly_feature.entity_id
_ihm_non_poly_feature.asym_id
_ihm_non_poly_feature.comp_id
_ihm_non_poly_feature.atom_id
1 3 3 C HEM FE
2 4 3 C HEM .
#
"""
        rsr = """
loop_
_ihm_derived_distance_restraint.id
_ihm_derived_distance_restraint.group_id
_ihm_derived_distance_restraint.feature_id_1
_ihm_derived_distance_restraint.feature_id_2
_ihm_derived_distance_restraint.restraint_type
_ihm_derived_distance_restraint.distance_lower_limit
_ihm_derived_distance_restraint.distance_upper_limit
_ihm_derived_distance_restraint.probability
_ihm_derived_distance_restraint.group_conditionality
_ihm_derived_distance_restraint.dataset_list_id
1 . 1 2 'lower bound' 25.000 . 0.800 . 97
2 . 1 4 'upper bound' . 45.000 0.800 ALL 98
3 1 1 2 'lower and upper bound' 22.000 45.000 0.800 ANY 99
4 1 2 3 'harmonic' 35.000 35.000 0.800 ALL .
"""
        # Test both ways to make sure features still work if they are
        # referenced by ID before their type is known
        for text in (feats+rsr, rsr+feats):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            self.assertEqual(len(s.orphan_features), 4)
            r1, r2, r3, r4 = s.restraints
            rg1, = s.restraint_groups
            self.assertEqual([r for r in rg1], [r3, r4])
            self.assertEqual(r1.dataset._id, '97')
            self.assertTrue(isinstance(r1.feature1,
                                       ihm.restraint.AtomFeature))
            self.assertEqual(len(r1.feature1.atoms), 1)
            self.assertEqual(r1.feature1.atoms[0].id, 'CA')
            self.assertEqual(r1.feature1.atoms[0].residue.seq_id, 1)
            self.assertTrue(isinstance(r1.feature2,
                                       ihm.restraint.ResidueFeature))
            self.assertEqual(len(r1.feature2.ranges), 1)
            self.assertEqual(r1.feature2.ranges[0].seq_id_range, (2,3))
            self.assertTrue(isinstance(r1.distance,
                                 ihm.restraint.LowerBoundDistanceRestraint))
            self.assertAlmostEqual(r1.distance.distance, 25.000, places=1)
            self.assertAlmostEqual(r1.probability, 0.8000, places=1)
            self.assertEqual(r1.restrain_all, None)
            self.assertEqual(r2.restrain_all, True)
            self.assertEqual(r3.restrain_all, False)
            self.assertTrue(isinstance(r2.feature2,
                                       ihm.restraint.NonPolyFeature))
            self.assertEqual(len(r2.feature2.asyms), 1)
            self.assertEqual(r2.feature2.asyms[0]._id, 'C')
            self.assertTrue(isinstance(r2.distance,
                                 ihm.restraint.UpperBoundDistanceRestraint))
            self.assertTrue(isinstance(r3.distance,
                             ihm.restraint.LowerUpperBoundDistanceRestraint))
            self.assertTrue(isinstance(r4.distance,
                                 ihm.restraint.HarmonicDistanceRestraint))
            self.assertTrue(isinstance(r4.feature2,
                                       ihm.restraint.AtomFeature))

    def test_sphere_handler(self):
        """Test SphereHandler"""
        obj_list = CENTERS_TRANSFORMS + """
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
_ihm_geometric_object_list.other_details
1 sphere 'my sphere' 'a test sphere' 'some details'
"""
        spheres = """
loop_
_ihm_geometric_object_sphere.object_id
_ihm_geometric_object_sphere.center_id
_ihm_geometric_object_sphere.transformation_id
_ihm_geometric_object_sphere.radius_r
1 1 1 2.200
2 . . 3.200
"""
        # Order of categories shouldn't matter
        for text in (obj_list+spheres, spheres+obj_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            s1, s2 = s.orphan_geometric_objects
            self.assertTrue(isinstance(s1, ihm.geometry.Sphere))
            self.assertTrue(isinstance(s2, ihm.geometry.Sphere))
            self.assertEqual(s1.name, 'my sphere')
            self.assertEqual(s1.description, 'a test sphere')
            self.assertEqual(s1.details, 'some details')
            self.assertAlmostEqual(s1.center.x, 1.000, places=1)
            self.assertAlmostEqual(s1.center.y, 2.000, places=1)
            self.assertAlmostEqual(s1.center.z, 3.000, places=1)
            self.assertAlmostEqual(s1.transformation.tr_vector[1], 2.000,
                                   places=1)
            self.assertEqual(s2.name, None)
            self.assertEqual(s2.center, None)
            self.assertEqual(s2.transformation, None)

    def test_torus_handler(self):
        """Test TorusHandler"""
        obj_list = CENTERS_TRANSFORMS + """
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
_ihm_geometric_object_list.other_details
1 torus . . .
"""
        tori = """
loop_
_ihm_geometric_object_torus.object_id
_ihm_geometric_object_torus.center_id
_ihm_geometric_object_torus.transformation_id
_ihm_geometric_object_torus.major_radius_R
_ihm_geometric_object_torus.minor_radius_r
1 1 1 5.600 1.200
2 . . 3.600 2.200
"""
        # Order of categories shouldn't matter
        for text in (obj_list+tori, tori+obj_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            t1, t2 = s.orphan_geometric_objects
            self.assertTrue(isinstance(t1, ihm.geometry.Torus))
            self.assertTrue(isinstance(t2, ihm.geometry.Torus))
            self.assertAlmostEqual(t1.center.x, 1.000, places=1)
            self.assertAlmostEqual(t1.transformation.tr_vector[1], 2.000,
                                   places=1)
            self.assertAlmostEqual(t1.major_radius, 5.600, places=1)
            self.assertAlmostEqual(t1.minor_radius, 1.200, places=1)
            self.assertEqual(t2.center, None)
            self.assertEqual(t2.transformation, None)

    def test_half_torus_handler(self):
        """Test HalfTorusHandler"""
        obj_list = CENTERS_TRANSFORMS + """
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
_ihm_geometric_object_list.other_details
1 half-torus . . .
2 half-torus . . .
3 half-torus . . .
"""
        tori = """
loop_
_ihm_geometric_object_torus.object_id
_ihm_geometric_object_torus.center_id
_ihm_geometric_object_torus.transformation_id
_ihm_geometric_object_torus.major_radius_R
_ihm_geometric_object_torus.minor_radius_r
1 1 1 5.600 1.200
2 . . 3.600 2.200
3 . . 3.600 2.200
"""
        half_tori = """
loop_
_ihm_geometric_object_half_torus.object_id
_ihm_geometric_object_half_torus.thickness_th
_ihm_geometric_object_half_torus.section
1 0.100 'inner half'
2 0.200 'outer half'
3 0.200 .
"""

        # Order of categories shouldn't matter
        for text in (obj_list+tori+half_tori, tori+half_tori+obj_list,
                     obj_list+half_tori+tori, half_tori+tori+obj_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            t1, t2, t3 = s.orphan_geometric_objects
            self.assertTrue(isinstance(t1, ihm.geometry.HalfTorus))
            self.assertTrue(isinstance(t2, ihm.geometry.HalfTorus))
            self.assertTrue(isinstance(t3, ihm.geometry.HalfTorus))
            self.assertAlmostEqual(t1.center.x, 1.000, places=1)
            self.assertAlmostEqual(t1.transformation.tr_vector[1], 2.000,
                                   places=1)
            self.assertAlmostEqual(t1.major_radius, 5.600, places=1)
            self.assertAlmostEqual(t1.minor_radius, 1.200, places=1)
            self.assertAlmostEqual(t1.thickness, 0.100, places=1)
            self.assertEqual(t1.inner, True)
            self.assertEqual(t2.center, None)
            self.assertEqual(t2.transformation, None)
            self.assertEqual(t2.inner, False)
            self.assertEqual(t3.inner, None)

    def test_axis_handler(self):
        """Test AxisHandler"""
        obj_list = CENTERS_TRANSFORMS + """
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
_ihm_geometric_object_list.other_details
1 axis . . .
2 axis . . .
"""
        axes = """
loop_
_ihm_geometric_object_axis.object_id
_ihm_geometric_object_axis.axis_type
_ihm_geometric_object_axis.transformation_id
1 x-axis 1
2 y-axis .
"""
        # Order of categories shouldn't matter
        for text in (obj_list+axes, axes+obj_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            a1, a2 = s.orphan_geometric_objects
            self.assertTrue(isinstance(a1, ihm.geometry.XAxis))
            self.assertTrue(isinstance(a2, ihm.geometry.YAxis))
            self.assertAlmostEqual(a1.transformation.tr_vector[1], 2.000,
                                   places=1)
            self.assertEqual(a2.transformation, None)

    def test_plane_handler(self):
        """Test PlaneHandler"""
        obj_list = CENTERS_TRANSFORMS + """
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
_ihm_geometric_object_list.other_details
1 plane . . .
2 plane . . .
"""
        planes = """
loop_
_ihm_geometric_object_plane.object_id
_ihm_geometric_object_plane.plane_type
_ihm_geometric_object_plane.transformation_id
1 xy-plane 1
2 yz-plane .
"""
        # Order of categories shouldn't matter
        for text in (obj_list+planes, planes+obj_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            p1, p2 = s.orphan_geometric_objects
            self.assertTrue(isinstance(p1, ihm.geometry.XYPlane))
            self.assertTrue(isinstance(p2, ihm.geometry.YZPlane))
            self.assertAlmostEqual(p1.transformation.tr_vector[1], 2.000,
                                   places=1)
            self.assertEqual(p2.transformation, None)

    def test_geometric_restraint_handler(self):
        """Test GeometricRestraintHandler"""
        fh = StringIO("""
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
1 23 44 other 'upper bound' 2.000 . 25.000 ANY 97
2 23 44 center 'lower bound' 2.000 15.000 . ALL .
3 23 44 'inner surface' 'lower and upper bound' 2.000 10.000 25.000 . 97
4 23 44 'outer surface' 'harmonic' 2.000 . 25.000 . 97
#
""")
        s, = ihm.reader.read(fh)
        r1, r2, r3, r4 = s.restraints
        self.assertTrue(isinstance(r1,
                             ihm.restraint.GeometricRestraint))
        self.assertEqual(r1.dataset._id, '97')
        self.assertEqual(r1.geometric_object._id, '23')
        self.assertEqual(r1.feature._id, '44')
        self.assertTrue(isinstance(r1.distance,
                             ihm.restraint.UpperBoundDistanceRestraint))
        self.assertAlmostEqual(r1.distance.distance, 25.000, places=1)
        self.assertAlmostEqual(r1.harmonic_force_constant, 2.000, places=1)
        self.assertEqual(r1.restrain_all, False)
        self.assertEqual(r2.restrain_all, True)
        self.assertEqual(r3.restrain_all, None)

        self.assertTrue(isinstance(r2,
                             ihm.restraint.CenterGeometricRestraint))
        self.assertTrue(isinstance(r3,
                             ihm.restraint.InnerSurfaceGeometricRestraint))
        self.assertTrue(isinstance(r4,
                             ihm.restraint.OuterSurfaceGeometricRestraint))

        self.assertTrue(isinstance(r2.distance,
                             ihm.restraint.LowerBoundDistanceRestraint))
        self.assertTrue(isinstance(r3.distance,
                             ihm.restraint.LowerUpperBoundDistanceRestraint))
        self.assertTrue(isinstance(r4.distance,
                             ihm.restraint.HarmonicDistanceRestraint))

    def test_poly_seq_scheme_handler_offset(self):
        """Test PolySeqSchemeHandler with constant offset"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_pdbx_poly_seq_scheme.asym_id
_pdbx_poly_seq_scheme.entity_id
_pdbx_poly_seq_scheme.seq_id
_pdbx_poly_seq_scheme.auth_seq_num
A 1 1 6
A 1 2 7
A 1 3 8
A 1 4 9
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map, 5)
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1,5)],
                         [6,7,8,9])

    def test_poly_seq_scheme_handler_empty(self):
        """Test PolySeqSchemeHandler with no poly_seq_scheme"""
        fh = StringIO(ASYM_ENTITY)
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map, 0)
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1,5)],
                         [1,2,3,4])

    def test_poly_seq_scheme_handler_nop(self):
        """Test PolySeqSchemeHandler with a do-nothing poly_seq_scheme"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_pdbx_poly_seq_scheme.asym_id
_pdbx_poly_seq_scheme.entity_id
_pdbx_poly_seq_scheme.seq_id
_pdbx_poly_seq_scheme.auth_seq_num
A 1 1 1
A 1 2 2
A 1 3 3
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map, 0)
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1,5)],
                         [1,2,3,4])

    def test_poly_seq_scheme_handler_partial(self):
        """Test PolySeqSchemeHandler with partial information"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_pdbx_poly_seq_scheme.asym_id
_pdbx_poly_seq_scheme.entity_id
_pdbx_poly_seq_scheme.seq_id
_pdbx_poly_seq_scheme.auth_seq_num
A 1 1 6
A 1 2 7
A 1 3 8
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        # No mapping for residue 4
        self.assertEqual(asym.auth_seq_id_map, {1:6, 2:7, 3:8})
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1,5)],
                         [6,7,8,4])

    def test_poly_seq_scheme_handler_incon_off(self):
        """Test PolySeqSchemeHandler with inconsistent offset"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_pdbx_poly_seq_scheme.asym_id
_pdbx_poly_seq_scheme.entity_id
_pdbx_poly_seq_scheme.seq_id
_pdbx_poly_seq_scheme.auth_seq_num
A 1 1 6
A 1 2 7
A 1 3 8
A 1 4 10
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map, {1:6, 2:7, 3:8, 4:10})
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1,5)],
                         [6,7,8,10])

    def test_poly_seq_scheme_handler_str_seq_id(self):
        """Test PolySeqSchemeHandler with a non-integer auth_seq_num"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_pdbx_poly_seq_scheme.asym_id
_pdbx_poly_seq_scheme.entity_id
_pdbx_poly_seq_scheme.seq_id
_pdbx_poly_seq_scheme.auth_seq_num
A 1 1 6
A 1 2 7
A 1 3 8
A 1 4 9A
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map, {1:6, 2:7, 3:8, 4:'9A'})
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1,5)],
                         [6,7,8,'9A'])

    def test_nonpoly_scheme_handler(self):
        """Test NonPolySchemeHandler"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_pdbx_nonpoly_scheme.asym_id
_pdbx_nonpoly_scheme.entity_id
_pdbx_nonpoly_scheme.auth_seq_num
A 1 1
A 1 101
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map, {1:101})
        self.assertEqual(asym.residue(1).auth_seq_id, 101)

    def test_cross_link_list_handler(self):
        """Test CrossLinkListHandler"""
        fh = StringIO("""
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
1 1 foo 1 2 THR foo 1 3 CYS DSS 97
2 2 foo 1 2 THR bar 2 3 PHE DSS 97
3 2 foo 1 2 THR bar 2 2 GLU DSS 97
4 3 foo 1 1 ALA bar 2 1 ASP DSS 97
5 4 foo 1 1 ALA bar 2 1 ASP EDC 97
6 5 foo 1 1 ALA bar 2 1 ASP DSS 98
""")
        s, = ihm.reader.read(fh)
        # Check grouping
        self.assertEqual([[len(g) for g in r.experimental_cross_links]
                          for r in s.restraints], [[1, 2, 1], [1], [1]])
        r1, r2, r3 = s.restraints
        self.assertEqual(r1.dataset._id, '97')
        self.assertEqual(r1.linker_type, 'DSS')
        xl = r1.experimental_cross_links[1][0]
        self.assertEqual(xl.residue1.entity._id, '1')
        self.assertEqual(xl.residue2.entity._id, '2')
        self.assertEqual(xl.residue1.seq_id, 2)
        self.assertEqual(xl.residue2.seq_id, 3)

    def test_cross_link_restraint_handler(self):
        """Test CrossLinkRestraintHandler"""
        xl_list = """
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
1 1 foo 1 2 THR foo 1 3 CYS DSS 97
2 2 foo 1 2 THR bar 2 3 PHE DSS 97
"""
        xl_rsr = """
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
1 1 1 A 2 THR 1 B 3 CYS . . 'upper bound' ALL by-residue 25.000 0.500 1.000
2.000
2 2 1 A 2 THR 2 B 2 GLU C N 'lower bound' ANY by-atom 34.000 . . .
"""
        # Order of categories shouldn't matter
        for text in (xl_list+xl_rsr, xl_rsr+xl_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            r, = s.restraints
            xl1, xl2 = r.cross_links
            self.assertTrue(isinstance(xl1, ihm.restraint.ResidueCrossLink))
            self.assertEqual(xl1.experimental_cross_link.residue1.seq_id, 2)
            self.assertEqual(xl1.experimental_cross_link.residue2.seq_id, 3)
            self.assertEqual(xl1.fits, {})
            self.assertEqual(xl1.asym1._id, 'A')
            self.assertEqual(xl1.asym2._id, 'B')
            self.assertTrue(isinstance(xl1.distance,
                                 ihm.restraint.UpperBoundDistanceRestraint))
            self.assertAlmostEqual(xl1.distance.distance, 25.000, places=1)
            self.assertAlmostEqual(xl1.psi, 0.500, places=1)
            self.assertAlmostEqual(xl1.sigma1, 1.000, places=1)
            self.assertAlmostEqual(xl1.sigma2, 2.000, places=1)

            self.assertTrue(isinstance(xl2, ihm.restraint.AtomCrossLink))
            self.assertEqual(xl2.fits, {})
            self.assertTrue(isinstance(xl2.distance,
                                 ihm.restraint.LowerBoundDistanceRestraint))
            self.assertTrue(xl2.atom1, 'C')
            self.assertTrue(xl2.atom2, 'N')
            self.assertAlmostEqual(xl2.distance.distance, 34.000, places=1)
            self.assertEqual(xl2.psi, None)
            self.assertEqual(xl2.sigma1, None)
            self.assertEqual(xl2.sigma2, None)

    def test_cross_link_result_handler(self):
        """Test CrossLinkResultHandler"""
        xl_list = """
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
1 1 foo 1 2 THR foo 1 3 CYS DSS 97
"""
        xl_rsr = """
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
1 1 1 A 2 THR 1 B 3 CYS . . 'upper bound' ALL by-residue 25.000 0.500 1.000
2.000
"""
        xl_fit = """
loop_
_ihm_cross_link_result_parameters.ordinal_id
_ihm_cross_link_result_parameters.restraint_id
_ihm_cross_link_result_parameters.model_id
_ihm_cross_link_result_parameters.psi
_ihm_cross_link_result_parameters.sigma_1
_ihm_cross_link_result_parameters.sigma_2
1 1 201 0.100 4.200 2.100
2 1 301 . . .
"""
        # Order of categories shouldn't matter
        for text in (xl_list+xl_rsr+xl_fit, xl_fit+xl_rsr+xl_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            r, = s.restraints
            xl, = r.cross_links
            # Sort fits by model ID
            fits = sorted(xl.fits.items(), key=lambda x:x[0]._id)
            self.assertEqual(len(fits), 2)
            self.assertEqual(fits[0][0]._id, '201')
            self.assertAlmostEqual(fits[0][1].psi, 0.100, places=1)
            self.assertAlmostEqual(fits[0][1].sigma1, 4.200, places=1)
            self.assertAlmostEqual(fits[0][1].sigma2, 2.100, places=1)

            self.assertEqual(fits[1][0]._id, '301')
            self.assertEqual(fits[1][1].psi, None)
            self.assertEqual(fits[1][1].sigma1, None)
            self.assertEqual(fits[1][1].sigma2, None)

    def test_ordered_ensemble_handler(self):
        """Test OrderedEnsembleHandler"""
        fh = StringIO("""
loop_
_ihm_ordered_ensemble.process_id
_ihm_ordered_ensemble.process_description
_ihm_ordered_ensemble.ordered_by
_ihm_ordered_ensemble.step_id
_ihm_ordered_ensemble.step_description
_ihm_ordered_ensemble.edge_id
_ihm_ordered_ensemble.edge_description
_ihm_ordered_ensemble.model_group_id_begin
_ihm_ordered_ensemble.model_group_id_end
1 pdesc 'steps in a reaction pathway' 1 'step 1 desc' 1 .  1 2
1 pdesc 'steps in a reaction pathway' 2 'step 2 desc' 2 'edge 2 desc'  1 3
1 pdesc 'steps in a reaction pathway' 2 'step 2 desc' 3 .  1 4
""")
        s, = ihm.reader.read(fh)
        op, = s.ordered_processes
        self.assertEqual(op.description, 'pdesc')
        self.assertEqual(len(op.steps), 2)
        s1, s2 = op.steps
        self.assertEqual(s1.description, 'step 1 desc')
        self.assertEqual(len(s1), 1)
        e1 = s1[0]
        self.assertEqual(e1.description, None)
        self.assertEqual(e1.group_begin._id, '1')
        self.assertEqual(e1.group_end._id, '2')

        self.assertEqual(s2.description, 'step 2 desc')
        self.assertEqual(len(s2), 2)
        e1 = s2[0]
        self.assertEqual(e1.description, 'edge 2 desc')
        self.assertEqual(e1.group_begin._id, '1')
        self.assertEqual(e1.group_end._id, '3')
        e2 = s2[1]
        self.assertEqual(e2.description, None)
        self.assertEqual(e2.group_begin._id, '1')
        self.assertEqual(e2.group_end._id, '4')


if __name__ == '__main__':
    unittest.main()
