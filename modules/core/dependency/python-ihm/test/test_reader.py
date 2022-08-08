import utils
import os
import unittest
import gzip
import sys
import operator
import warnings
if sys.version_info[0] >= 3:
    from io import StringIO, BytesIO
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
        if sys.version_info[0] >= 3:
            # Make sure we can read the file in binary mode too
            s, = ihm.reader.read(BytesIO(cif.encode('latin-1')))
            self.assertEqual(s.id, 'testid')

    def test_read_unicode(self):
        """Test that Unicode characters are handled sensibly"""
        # mmCIF files should technically be ASCII, but try not to fall over
        # if we're fed a Unicode file
        cif = "data_model\n_struct.entry_id test\u00dc\U0001f600\n"
        s, = ihm.reader.read(StringIO(cif))
        self.assertEqual(s.id, 'test\u00dc\U0001f600')
        # Full Unicode support requires Python 3
        if sys.version_info[0] >= 3:
            s, = ihm.reader.read(BytesIO(cif.encode('utf-8')))
            # Reading in binary mode should give us the raw text (latin-1)
            self.assertEqual(s.id, 'test\xc3\x9c\xf0\x9f\x98\x80')
            with utils.temporary_directory() as tmpdir:
                fname = os.path.join(tmpdir, 'test')
                with open(fname, 'w', encoding='utf-8') as fh:
                    fh.write(cif)
                # Should get the input back if we use the right UTF-8 encoding
                with open(fname, encoding='utf-8') as fh:
                    s, = ihm.reader.read(fh)
                    self.assertEqual(s.id, 'test\u00dc\U0001f600')
                # Should get a decode error if we treat it as ASCII:
                with open(fname, encoding='ascii') as fh:
                    self.assertRaises(UnicodeDecodeError, ihm.reader.read, fh)
                # A permissive 8-bit encoding should work but give us garbage
                with open(fname, encoding='latin-1') as fh:
                    s, = ihm.reader.read(fh)
                    self.assertEqual(s.id, 'test\xc3\x9c\xf0\x9f\x98\x80')

    def test_read_custom_handler(self):
        """Test read() function with custom Handler"""
        class MyHandler(ihm.reader.Handler):
            category = "_custom_category"

            def __call__(self, field1, myfield):
                self.system.custom_data = (field1, myfield)

        cif = "data_model\n_struct.entry_id testid\n" \
              "_custom_category.field1 foo\n_custom_category.myfield bar\n"
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh, handlers=[MyHandler])
            self.assertEqual(s.id, 'testid')
            self.assertEqual(s.custom_data, ('foo', 'bar'))

    def test_system_reader(self):
        """Test SystemReader class"""
        _ = ihm.reader.SystemReader(ihm.model.Model,
                                    ihm.startmodel.StartingModel)

    def test_id_mapper(self):
        """Test IDMapper class"""
        class MockObject(object):
            def __init__(self, x, y):
                self.x, self.y = x, y

        testlist = []
        im = ihm.reader.IDMapper(testlist, MockObject, '1', y='2')
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
        h = ihm.reader.Handler(o)
        self.assertEqual(h.system, 'foo')

    def test_handler_copy_if_present(self):
        """Test copy_if_present method"""
        class MockObject(object):
            pass
        # Keys = namedtuple('Keys', 'foo bar t test x')
        o = MockObject()
        h = ihm.reader.Handler(None)
        h.copy_if_present(o, {'foo': 'bar', 'bar': 'baz', 't': 'u'},
                          keys=['test', 'foo'],
                          mapkeys={'bar': 'baro', 'x': 'y'})
        self.assertEqual(o.foo, 'bar')
        self.assertEqual(o.baro, 'baz')
        self.assertFalse(hasattr(o, 't'))
        self.assertFalse(hasattr(o, 'x'))
        self.assertFalse(hasattr(o, 'bar'))

    def test_struct_handler(self):
        """Test StructHandler"""
        cif = """
_struct.entry_id eid
_struct.title 'Test title'
_struct.pdbx_model_details 'Test details'
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            self.assertEqual(s.id, 'eid')
            self.assertEqual(s.title, 'Test title')
            self.assertEqual(s.model_details, 'Test details')

    def test_multiple_systems(self):
        """Test multiple systems from data blocks"""
        cif = """
data_id1
_struct.entry_id id1
data_id2
_struct.entry_id id2
data_id3
_struct.entry_id id3
data_long-entry$#<>
_struct.entry_id id4
"""
        for fh in cif_file_handles(cif):
            s1, s2, s3, s4 = ihm.reader.read(fh)
            self.assertEqual(s1.id, 'id1')
            self.assertEqual(s2.id, 'id2')
            self.assertEqual(s3.id, 'id3')
            # Should not be tripped up by odd characters in data_ block,
            # and the system ID should match entry_id
            self.assertEqual(s4.id, 'id4')

    def test_software_handler(self):
        """Test SoftwareHandler"""
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
1 'Mol Cell Proteomics' 9 2943 . 2014 1234 .
#
loop_
_software.pdbx_ordinal
_software.name
_software.classification
_software.description
_software.version
_software.type
_software.location
_software.citation_id
1 'test software' 'test class' 'test desc' program 1.0.1 https://example.org .
2 'other software' 'oth class' 'test desc' program 1.0.1 https://example.org 1
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            software, s2 = s.software
            self.assertEqual(software._id, '1')
            self.assertEqual(software.name, 'test software')
            self.assertEqual(software.classification, 'test class')
            self.assertIsNone(software.citation)
            self.assertEqual(s2._id, '2')
            self.assertEqual(s2.name, 'other software')
            self.assertEqual(s2.classification, 'oth class')
            self.assertEqual(s2.citation.pmid, '1234')

    def test_audit_author_handler(self):
        """Test AuditAuthorHandler"""
        cif = """
loop_
_audit_author.name
_audit_author.pdbx_ordinal
auth1 1
auth2 2
auth3 3
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            self.assertEqual(s.authors, ['auth1', 'auth2', 'auth3'])

    def test_grant_handler(self):
        """Test GrantHandler"""
        cif = """
loop_
_pdbx_audit_support.funding_organization
_pdbx_audit_support.country
_pdbx_audit_support.grant_number
_pdbx_audit_support.ordinal
NIH 'United States' foo 1
NSF 'United States' bar 2
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            g1, g2 = s.grants
            self.assertEqual(g1.funding_organization, "NIH")
            self.assertEqual(g1.country, "United States")
            self.assertEqual(g1.grant_number, "foo")
            self.assertEqual(g2.funding_organization, "NSF")
            self.assertEqual(g2.country, "United States")
            self.assertEqual(g2.grant_number, "bar")

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
primary 'Mol Cell Proteomics' 9 2943 . 2014 1234 .
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
            self.assertEqual(citation1._id, 'primary')
            self.assertTrue(citation1.is_primary)
            self.assertEqual(citation1.page_range, '2943')
            self.assertEqual(citation1.authors, [])
            self.assertEqual(citation1.pmid, '1234')
            self.assertIsNone(citation1.doi)

            self.assertEqual(citation2._id, '3')
            self.assertFalse(citation2.is_primary)
            self.assertEqual(citation2.page_range, ('2943', '2946'))
            self.assertEqual(citation2.authors, ['Foo A', 'Bar C'])
            self.assertEqual(citation2.doi, '1.2.3.4')

            self.assertEqual(citation3._id, '4')
            self.assertFalse(citation3.is_primary)
            self.assertEqual(citation3.authors, [])
            self.assertIsNone(citation3.page_range)

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
                self.assertIsNone(s[2])
                self.assertEqual(s[3].id, 'MYTYPE')
                self.assertEqual(s[3].type, 'D-peptide linking')
                self.assertEqual(s[3].name, 'MY CUSTOM COMPONENT')
                self.assertAlmostEqual(s[3].formula_weight, 84.162, delta=0.1)
                self.assertEqual(s[3].__class__, ihm.DPeptideChemComp)
                # Class of standard type shouldn't be changed
                self.assertEqual(s[4].type, 'L-peptide linking')
                self.assertEqual(s[4].__class__, ihm.LPeptideChemComp)

    def test_entity_poly_handler(self):
        """Test EntityPolyHandler"""
        fh = StringIO("""
loop_
_entity_poly_seq.entity_id
_entity_poly_seq.num
_entity_poly_seq.mon_id
_entity_poly_seq.hetero
1 1 OCS .
1 2 MET .
1 3 ACE .
2 1 MET .
3 1 MET .
#
loop_
_entity_poly.entity_id
_entity_poly.type
_entity_poly.pdbx_seq_one_letter_code
_entity_poly.pdbx_seq_one_letter_code_can
1 'polypeptide(L)'
;(OCS)
M
;
SM
3 other . .
""")
        s, = ihm.reader.read(fh)
        e1, e2, e3 = s.entities
        c1, c2, c3 = e1.sequence
        self.assertEqual(c1.id, 'OCS')
        # Missing information should be filled in from entity_poly
        self.assertEqual(c1.code, 'OCS')
        self.assertEqual(c1.code_canonical, 'S')
        # No info in entity_poly for this component
        self.assertEqual(c3.id, 'ACE')
        self.assertIsNone(c3.code)
        self.assertIsNone(c3.code_canonical)

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

    def test_chem_descriptor_handler(self):
        """Test ChemDescriptorHandler"""
        for cat in ("ihm_chemical_component_descriptor",
                    "flr_chemical_descriptor"):
            cif = """
loop_
_%(cat)s.id
_%(cat)s.auth_name
_%(cat)s.chemical_name
_%(cat)s.common_name
_%(cat)s.smiles
_%(cat)s.smiles_canonical
_%(cat)s.inchi
_%(cat)s.inchi_key
1 EDC "test-chem-EDC" . "CCN=C=NCCCN(C)C" . test-inchi test-inchi-key
""" % {'cat': cat}
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                d1, = s.orphan_chem_descriptors
                self.assertEqual(d1.auth_name, 'EDC')
                self.assertIsNone(d1.chem_comp_id)
                self.assertEqual(d1.chemical_name, 'test-chem-EDC')
                self.assertEqual(d1.smiles, 'CCN=C=NCCCN(C)C')
                self.assertIsNone(d1.smiles_canonical)
                self.assertEqual(d1.inchi, 'test-inchi')
                self.assertEqual(d1.inchi_key, 'test-inchi-key')

    def test_entity_handler(self):
        """Test EntityHandler"""
        cif = """
loop_
_entity.id
_entity.type
_entity.src_method
_entity.pdbx_description
_entity.pdbx_number_of_molecules
_entity.formula_weight
_entity.details
1 polymer nat Nup84 2 100.0 .
2 polymer syn Nup85 3 200.0 .
3 polymer . Nup86 3 300.0 .
4 polymer unknown Nup87 3 300.0 .
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            e1, e2, e3, e4 = s.entities
            self.assertEqual(e1.description, 'Nup84')
            self.assertTrue(e1._force_polymer)
            self.assertEqual(
                e1.number_of_molecules, '2')  # todo: coerce to int
            self.assertEqual(e1.source.src_method, 'nat')
            self.assertEqual(e2.source.src_method, 'syn')
            self.assertIsNone(e3.source)
            self.assertIsNone(e4.source)

    def test_entity_handler_minimal(self):
        """Test EntityHandler with minimal entity category"""
        cif = """
loop_
_entity.id
1
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            e1, = s.entities
            self.assertIsNone(e1.description)

    def test_entity_src_gen_handler(self):
        """Test EntitySrcGenHandler"""
        entity = """
loop_
_entity.id
_entity.type
_entity.src_method
_entity.pdbx_description
_entity.pdbx_number_of_molecules
_entity.formula_weight
_entity.details
1 polymer man Nup84 2 100.0 .
"""
        src_gen = """
_entity_src_gen.entity_id 1
_entity_src_gen.pdbx_src_id 42
_entity_src_gen.pdbx_gene_src_scientific_name 'test latin name'
_entity_src_gen.pdbx_gene_src_ncbi_taxonomy_id 1234
_entity_src_gen.gene_src_common_name 'test common name'
_entity_src_gen.gene_src_strain 'test strain'
_entity_src_gen.pdbx_host_org_scientific_name 'host latin name'
_entity_src_gen.pdbx_host_org_ncbi_taxonomy_id 5678
_entity_src_gen.host_org_common_name 'other common name'
_entity_src_gen.pdbx_host_org_strain 'other strain'
"""
        # Order of the categories shouldn't matter
        cif1 = entity + src_gen
        cif2 = src_gen + entity
        for cif in cif1, cif2:
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                e, = s.entities
                self.assertEqual(e.source.src_method, 'man')
                self.assertEqual(e.source._id, '42')
                self.assertEqual(e.source.host.ncbi_taxonomy_id, '5678')
                self.assertEqual(e.source.host.scientific_name,
                                 'host latin name')
                self.assertEqual(e.source.host.common_name,
                                 'other common name')
                self.assertEqual(e.source.host.strain, 'other strain')
                self.assertEqual(e.source.gene.ncbi_taxonomy_id, '1234')
                self.assertEqual(e.source.gene.scientific_name,
                                 'test latin name')
                self.assertEqual(e.source.gene.common_name, 'test common name')
                self.assertEqual(e.source.gene.strain, 'test strain')

    def test_entity_src_nat_handler(self):
        """Test EntitySrcNatHandler"""
        entity = """
loop_
_entity.id
_entity.type
_entity.src_method
_entity.pdbx_description
_entity.pdbx_number_of_molecules
_entity.formula_weight
_entity.details
1 polymer man Nup84 2 100.0 .
"""
        src_nat = """
_entity_src_nat.entity_id 1
_entity_src_nat.pdbx_src_id 42
_entity_src_nat.pdbx_organism_scientific 'test latin name'
_entity_src_nat.pdbx_ncbi_taxonomy_id 5678
_entity_src_nat.common_name 'test common name'
_entity_src_nat.strain 'test strain'
"""
        # Order of the categories shouldn't matter
        cif1 = entity + src_nat
        cif2 = src_nat + entity
        for cif in cif1, cif2:
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                e, = s.entities
                self.assertEqual(e.source.src_method, 'nat')
                self.assertEqual(e.source._id, '42')
                self.assertEqual(e.source.ncbi_taxonomy_id, '5678')
                self.assertEqual(e.source.scientific_name, 'test latin name')
                self.assertEqual(e.source.common_name, 'test common name')
                self.assertEqual(e.source.strain, 'test strain')

    def test_entity_src_syn_handler(self):
        """Test EntitySrcSynHandler"""
        entity = """
loop_
_entity.id
_entity.type
_entity.src_method
_entity.pdbx_description
_entity.pdbx_number_of_molecules
_entity.formula_weight
_entity.details
1 polymer man Nup84 2 100.0 .
"""
        src_syn = """
_pdbx_entity_src_syn.entity_id 1
_pdbx_entity_src_syn.pdbx_src_id 42
_pdbx_entity_src_syn.organism_scientific 'test latin name'
_pdbx_entity_src_syn.organism_common_name 'test common name'
_pdbx_entity_src_syn.ncbi_taxonomy_id 5678
_pdbx_entity_src_syn.strain 'test strain'
"""
        # Order of the categories shouldn't matter
        cif1 = entity + src_syn
        cif2 = src_syn + entity
        for cif in cif1, cif2:
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                e, = s.entities
                self.assertEqual(e.source.src_method, 'syn')
                self.assertEqual(e.source._id, '42')
                self.assertEqual(e.source.ncbi_taxonomy_id, '5678')
                self.assertEqual(e.source.scientific_name, 'test latin name')
                self.assertEqual(e.source.common_name, 'test common name')
                # _pdbx_entity_src_syn.strain is not used in current PDB
                self.assertIsNone(e.source.strain)

    def test_struct_ref_handler(self):
        """Test StructRefHandler"""
        entity = """
loop_
_entity.id
_entity.type
_entity.src_method
_entity.pdbx_description
_entity.pdbx_number_of_molecules
_entity.formula_weight
_entity.details
1 polymer man test 1 100.0 .
"""
        struct_ref = """
loop_
_struct_ref.id
_struct_ref.entity_id
_struct_ref.db_name
_struct_ref.db_code
_struct_ref.pdbx_db_accession
_struct_ref.pdbx_align_begin
_struct_ref.pdbx_seq_one_letter_code
_struct_ref.details
1 1 UNP NUP84_YEAST P52891 3
;LSP
TYQT
;
'test sequence'
2 1 MyDatabase testcode testacc 1 MEL 'other sequence'
3 1 MyDatabase testcode2 testacc2 1 . 'other sequence'
4 1 MyDatabase testcode3 testacc3 1 ? 'other sequence'
#
#
loop_
_struct_ref_seq.align_id
_struct_ref_seq.ref_id
_struct_ref_seq.seq_align_beg
_struct_ref_seq.seq_align_end
_struct_ref_seq.db_align_beg
_struct_ref_seq.db_align_end
1 1 1 4 3 6
2 1 5 5 8 8
#
#
loop_
_struct_ref_seq_dif.pdbx_ordinal
_struct_ref_seq_dif.align_id
_struct_ref_seq_dif.seq_num
_struct_ref_seq_dif.db_mon_id
_struct_ref_seq_dif.mon_id
_struct_ref_seq_dif.details
1 1 2 TRP SER 'Test mutation'
#
"""
        # Order of the categories shouldn't matter
        cif1 = entity + struct_ref
        cif2 = struct_ref + entity
        for cif in cif1, cif2:
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                e, = s.entities
                r1, r2, r3, r4 = e.references
                self.assertIsInstance(r1, ihm.reference.UniProtSequence)
                self.assertEqual(r1.db_name, 'UNP')
                self.assertEqual(r1.db_code, 'NUP84_YEAST')
                self.assertEqual(r1.accession, 'P52891')
                self.assertEqual(r1.sequence, '--LSPTYQT')
                self.assertEqual(r1.details, 'test sequence')
                self.assertIsNone(r3.sequence)
                self.assertEqual(r4.sequence, ihm.unknown)
                a1, a2 = r1.alignments
                self.assertEqual(a1.db_begin, 3)
                self.assertEqual(a1.db_end, 6)
                self.assertEqual(a1.entity_begin, 1)
                self.assertEqual(a1.entity_end, 4)
                sd, = a1.seq_dif
                self.assertEqual(sd.seq_id, 2)
                self.assertIsInstance(sd.db_monomer, ihm.ChemComp)
                self.assertIsInstance(sd.monomer, ihm.ChemComp)
                self.assertEqual(sd.db_monomer.id, 'TRP')
                self.assertEqual(sd.monomer.id, 'SER')
                self.assertEqual(sd.details, 'Test mutation')
                self.assertEqual(a2.db_begin, 8)
                self.assertEqual(a2.db_end, 8)
                self.assertEqual(a2.entity_begin, 5)
                self.assertEqual(a2.entity_end, 5)
                self.assertEqual(len(a2.seq_dif), 0)
                self.assertIsInstance(r2, ihm.reference.Sequence)
                self.assertEqual(r2.db_name, 'MyDatabase')
                self.assertEqual(r2.db_code, 'testcode')
                self.assertEqual(r2.accession, 'testacc')
                self.assertEqual(r2.sequence, 'MEL')
                self.assertEqual(r2.details, 'other sequence')
                self.assertEqual(len(r2.alignments), 0)

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
            self.assertEqual(a1.id, 'A')
            self.assertEqual(a1.entity._id, '1')

            self.assertEqual(a1.details, 'Nup84')
            self.assertEqual(a2.entity._id, '1')
            self.assertEqual(a2._id, 'B')
            self.assertEqual(a2.id, 'B')
            self.assertEqual(a2.details, 'Nup85')
            self.assertEqual(id(a1.entity), id(a2.entity))

    def test_assembly_handler(self):
        """Test AssemblyHandler"""
        cif = """
loop_
_ihm_struct_assembly.id
_ihm_struct_assembly.name
_ihm_struct_assembly.description
1 'Complete assembly' 'All known components'
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            a1, = s.orphan_assemblies
            self.assertEqual(a1._id, '1')
            self.assertEqual(a1.name, 'Complete assembly')
            self.assertEqual(a1.description, 'All known components')

    def test_assembly_details_handler(self):
        """Test AssemblyDetailsHandler"""
        entity_cif = """
loop_
_entity_poly_seq.entity_id
_entity_poly_seq.num
_entity_poly_seq.mon_id
1 1 ALA
1 2 ALA
2 1 ALA
2 2 ALA
#
loop_
_ihm_entity_poly_segment.id
_ihm_entity_poly_segment.entity_id
_ihm_entity_poly_segment.seq_id_begin
_ihm_entity_poly_segment.seq_id_end
1 1 1 726
2 2 1 744
3 1 1 2
4 2 1 50
5 2 1 2
"""
        assembly_cif = """
loop_
_struct_asym.id
_struct_asym.entity_id
_struct_asym.details
A 1 Nup84
B 2 Nup85
#
loop_
_ihm_struct_assembly.id
_ihm_struct_assembly.name
_ihm_struct_assembly.description
1 . .
2 . .
3 . .
4 'user-provided name' 'user-provided desc'
#
loop_
_ihm_struct_assembly_details.id
_ihm_struct_assembly_details.assembly_id
_ihm_struct_assembly_details.parent_assembly_id
_ihm_struct_assembly_details.entity_description
_ihm_struct_assembly_details.entity_id
_ihm_struct_assembly_details.asym_id
_ihm_struct_assembly_details.entity_poly_segment_id
1 1 1 Nup84 1 A 1
2 1 1 Nup85 2 B 2
3 1 1 Nup84 1 A 3
4 2 1 Nup86 2 . 4
5 2 1 Nup85 2 . 5
6 3 1 Nup84 1 A .
7 3 1 Nup85 2 . .
7 4 1 Nup84 1 A .
8 4 1 Nup85 2 B .
"""
        # Order of categories should not matter
        for cif in (entity_cif + assembly_cif, assembly_cif + entity_cif):
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                a1, a2, a3, a4 = s.orphan_assemblies
                self.assertEqual(a1._id, '1')
                self.assertIsNone(a1.parent)
                self.assertEqual(len(a1), 3)
                # AsymUnitRange
                self.assertIsInstance(a1[0], ihm.AsymUnitRange)
                self.assertEqual(a1[0]._id, 'A')
                self.assertEqual(a1[0].seq_id_range, (1, 726))
                self.assertEqual(a1[1]._id, 'B')
                self.assertEqual(a1[1].seq_id_range, (1, 744))

                self.assertEqual(a2._id, '2')
                self.assertEqual(a2.parent, a1)
                # AsymUnit
                self.assertIsInstance(a1[2], ihm.AsymUnit)
                # EntityRange
                self.assertEqual(len(a2), 2)
                self.assertIsInstance(a2[0], ihm.EntityRange)
                self.assertEqual(a2[0]._id, '2')
                self.assertEqual(a2[0].seq_id_range, (1, 50))
                # Entity
                self.assertIsInstance(a2[1], ihm.Entity)

                # Assembly with no ranges given
                self.assertEqual(len(a3), 2)
                self.assertIsInstance(a3[0], ihm.AsymUnit)
                self.assertIsInstance(a3[1], ihm.Entity)

                # "Complete" assembly that covers all AsymUnits
                self.assertEqual(len(a4), 2)
                self.assertIsInstance(a4[0], ihm.AsymUnit)
                self.assertIsInstance(a4[1], ihm.AsymUnit)
                self.assertEqual(a4.name, 'user-provided name')
                self.assertEqual(a4.description, 'user-provided desc')
                # Should set name, description of system.complete_assembly
                self.assertEqual(s.complete_assembly.name, a4.name)
                self.assertEqual(s.complete_assembly.description,
                                 a4.description)

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
_ihm_external_reference_info.details
1 Zenodo DOI 10.5281/zenodo.1218053 Archive https://example.com/foo.zip
'test repo'
2 . 'Supplementary Files' . Other . .
3 Zenodo DOI 10.5281/zenodo.1218058 File https://example.com/foo.dcd .
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
                self.assertEqual(l1.repo.details, 'test repo')
                self.assertEqual(l1.__class__,
                                 ihm.location.WorkflowFileLocation)

                self.assertEqual(l2.path, 'foo/bar.txt')
                self.assertEqual(l2.details, 'Test text')
                self.assertIsNone(l2.repo)
                self.assertEqual(l2.__class__, ihm.location.InputFileLocation)

                self.assertEqual(l3.path, '.')
                self.assertEqual(l3.details, 'Ensemble structures')
                self.assertEqual(l3.repo.doi, '10.5281/zenodo.1218058')
                self.assertEqual(l3.__class__, ihm.location.OutputFileLocation)

                self.assertEqual(l4.path, '.')
                self.assertIsNone(l4.details)
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
_ihm_dataset_list.details
1 'Experimental model' YES .
2 'COMPARATIVE MODEL' YES .
3 'EM raw micrographs' YES 'test details'
4 . YES .
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            d1, d2, d3, d4 = s.orphan_datasets
            self.assertEqual(d1.__class__, ihm.dataset.PDBDataset)
            self.assertEqual(d2.__class__, ihm.dataset.ComparativeModelDataset)
            self.assertEqual(d3.__class__, ihm.dataset.EMMicrographsDataset)
            # No specified data type - use base class
            self.assertEqual(d4.__class__, ihm.dataset.Dataset)
            self.assertIsNone(d1.details)
            self.assertEqual(d3.details, 'test details')

    def test_dataset_group_handler(self):
        """Test DatasetGroupHandler"""
        cif = """
loop_
_ihm_dataset_group.id
_ihm_dataset_group.name
_ihm_dataset_group.application
_ihm_dataset_group.details
1 "foo" "foo app" "foo details"
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            g1, = s.orphan_dataset_groups
            self.assertEqual(len(g1), 0)  # no datasets read yet
            self.assertEqual(g1.name, 'foo')
            self.assertEqual(g1.application, 'foo app')
            self.assertEqual(g1.details, 'foo details')

    def test_dataset_group_link_handler(self):
        """Test DatasetGroupLinkHandler"""
        cif = """
loop_
_ihm_dataset_group_link.group_id
_ihm_dataset_group_link.dataset_list_id
1 1
1 2
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
            self.assertIsNone(d3.location.version)
            self.assertIsNone(d3.location.details)
            self.assertIsNone(d4.location.db_name)
            self.assertEqual(d4.location.__class__,
                             ihm.location.DatabaseLocation)
            self.assertIsNone(d4.location.access_code)

    def test_related_datasets_handler(self):
        """Test RelatedDatasetsHandler"""
        cif = """
loop_
_ihm_data_transformation.id
_ihm_data_transformation.rot_matrix[1][1]
_ihm_data_transformation.rot_matrix[2][1]
_ihm_data_transformation.rot_matrix[3][1]
_ihm_data_transformation.rot_matrix[1][2]
_ihm_data_transformation.rot_matrix[2][2]
_ihm_data_transformation.rot_matrix[3][2]
_ihm_data_transformation.rot_matrix[1][3]
_ihm_data_transformation.rot_matrix[2][3]
_ihm_data_transformation.rot_matrix[3][3]
_ihm_data_transformation.tr_vector[1]
_ihm_data_transformation.tr_vector[2]
_ihm_data_transformation.tr_vector[3]
42 -0.637588 0.089507 0.765160 0.755616 -0.120841 0.643771 0.150085
0.988628 0.009414 327.161 83.209 -227.800
#
loop_
_ihm_related_datasets.dataset_list_id_derived
_ihm_related_datasets.dataset_list_id_primary
_ihm_related_datasets.transformation_id
4 1 .
7 1 42
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            d1, d2, d3 = s.orphan_datasets
            self.assertEqual(d1._id, '4')
            self.assertEqual(d2._id, '1')
            self.assertEqual(d3._id, '7')
            self.assertEqual(d1.parents, [d2])
            self.assertEqual(d2.parents, [])
            self.assertEqual(len(d3.parents), 1)
            self.assertIsInstance(d3.parents[0],
                                  ihm.dataset.TransformedDataset)
            self.assertEqual(d3.parents[0].dataset._id, '1')
            t = d3.parents[0].transform
            self.assertEqual(t._id, '42')
            self.assertAlmostEqual(t.tr_vector[0], 327.161, delta=0.01)
            self.assertAlmostEqual(t.rot_matrix[1][2], 0.988628, delta=0.01)

    def test_model_representation_handler(self):
        """Test ModelRepresentationHandler"""
        cif = """
loop_
_ihm_model_representation.id
_ihm_model_representation.name
_ihm_model_representation.details
1 "rep A" "rep A details"
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            r1, = s.orphan_representations
            self.assertEqual(len(r1), 0)  # no segments read yet
            self.assertEqual(r1.name, 'rep A')
            self.assertEqual(r1.details, 'rep A details')

    def test_model_representation_details_handler(self):
        """Test ModelRepresentationDetailsHandler"""
        range_cif = """
loop_
_ihm_entity_poly_segment.id
_ihm_entity_poly_segment.entity_id
_ihm_entity_poly_segment.seq_id_begin
_ihm_entity_poly_segment.seq_id_end
1 1 1 6
2 1 7 20
"""
        repr_cif = """
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
1 1 1 Nup84 A 1 sphere . flexible by-feature 1 'test segment'
2 1 1 Nup84 A 2 sphere 1 rigid by-residue . .
3 2 1 Nup84 A . atomistic . flexible by-atom . .
4 3 2 Nup85 B . sphere . . multi-residue . .
"""
        # Order of categories should not matter
        for cif in (range_cif + repr_cif, repr_cif + range_cif):
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                r1, r2, r3 = s.orphan_representations
                self.assertEqual(len(r1), 2)
                s1, s2 = r1
                self.assertEqual(s1.__class__,
                                 ihm.representation.FeatureSegment)
                self.assertEqual(s1.primitive, 'sphere')
                self.assertEqual(s1.count, 1)
                self.assertEqual(s1.rigid, False)
                self.assertIsNone(s1.starting_model)
                self.assertEqual(s1.asym_unit.seq_id_range, (1, 6))
                self.assertEqual(s1.description, 'test segment')

                self.assertEqual(s2.__class__,
                                 ihm.representation.ResidueSegment)
                self.assertEqual(s2.primitive, 'sphere')
                self.assertIsNone(s2.count)
                self.assertEqual(s2.rigid, True)
                self.assertEqual(s2.starting_model._id, '1')
                self.assertEqual(s2.asym_unit.seq_id_range, (7, 20))
                self.assertIsNone(s2.description)

                self.assertEqual(len(r2), 1)
                s1, = r2
                self.assertEqual(s1.__class__,
                                 ihm.representation.AtomicSegment)

                self.assertEqual(len(r3), 1)
                s1, = r3
                self.assertEqual(s1.__class__,
                                 ihm.representation.MultiResidueSegment)

    def test_starting_model_details_handler(self):
        """Test StartingModelDetailsHandler"""
        ps_cif = """
loop_
_ihm_entity_poly_segment.id
_ihm_entity_poly_segment.entity_id
_ihm_entity_poly_segment.seq_id_begin
_ihm_entity_poly_segment.seq_id_end
1 1 7 483
"""
        sm_cif = """
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
1 1 Nup84 A 1 'comparative model' Q 8 4 .
2 1 Nup84 A . 'comparative model' X . 6 'test desc'
"""
        # Order of the two categories shouldn't matter
        for cif in ps_cif + sm_cif, sm_cif + ps_cif:
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                m1, m2 = s.orphan_starting_models
                self.assertEqual(m1.asym_unit._id, 'A')
                self.assertEqual(m1.asym_unit.seq_id_range, (7, 483))
                self.assertEqual(m1.asym_id, 'Q')
                self.assertEqual(m1.offset, 8)
                self.assertEqual(m1.dataset._id, '4')
                self.assertIsNone(m1.description)

                self.assertEqual(m2.asym_unit._id, 'A')
                self.assertEqual(m2.asym_id, 'X')
                self.assertEqual(m2.offset, 0)
                self.assertEqual(m2.dataset._id, '6')
                self.assertEqual(m2.description, 'test desc')

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
            self.assertIsNone(m2.script_file)
            self.assertIsNone(m2.software)

    def test_starting_comparative_models_handler(self):
        """Test StartingComparativeModelsHandler"""
        cif = """
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
1 1 A 7 436 C 9 438 90.000 1 3 2
2 1 A 33 424 C 33 424 100.000 1 1 .
3 1 A 33 424 C . ? 100.000 1 1 .
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            m1, = s.orphan_starting_models
            t1, t2, t3 = m1.templates
            self.assertEqual(t1.dataset._id, '3')
            self.assertEqual(t1.asym_id, 'C')
            self.assertEqual(t1.seq_id_range, (7, 436))
            self.assertEqual(t1.template_seq_id_range, (9, 438))
            self.assertAlmostEqual(t1.sequence_identity.value, 90.0,
                                   delta=0.1)
            self.assertEqual(t1.sequence_identity.denominator, 1)
            self.assertEqual(t1.alignment_file._id, '2')
            self.assertIsNone(t2.alignment_file)
            self.assertEqual(t3.template_seq_id_range, (None, ihm.unknown))

    def test_protocol_handler(self):
        """Test ProtocolHandler"""
        cif = """
loop_
_ihm_modeling_protocol.id
_ihm_modeling_protocol.protocol_name
_ihm_modeling_protocol.num_steps
1 Prot1 5
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            p1, = s.orphan_protocols
            self.assertEqual(p1.name, "Prot1")
            # no step objects read yet, num_steps ignored
            self.assertEqual(len(p1.steps), 0)

    def test_protocol_details_handler(self):
        """Test ProtocolDetailsHandler"""
        cif = """
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
_ihm_modeling_protocol_details.ensemble_flag
_ihm_modeling_protocol_details.software_id
_ihm_modeling_protocol_details.script_file_id
_ihm_modeling_protocol_details.description
1 1 1 1 1 . Sampling 'Monte Carlo' 0 500 YES NO NO NO . . .
2 1 2 1 2 . Sampling 'Monte Carlo' 500 5000 YES . NO YES 401 501 'test step'
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            p1, = s.orphan_protocols
            self.assertEqual(len(p1.steps), 2)
            self.assertEqual(p1.steps[0]._id, '1')
            self.assertEqual(p1.steps[0].assembly._id, '1')
            self.assertEqual(p1.steps[0].dataset_group._id, '1')
            self.assertEqual(p1.steps[0].name, 'Sampling')
            self.assertEqual(p1.steps[0].method, 'Monte Carlo')
            self.assertEqual(p1.steps[0].num_models_begin, 0)
            self.assertEqual(p1.steps[0].num_models_end, 500)
            self.assertEqual(p1.steps[0].multi_scale, True)
            self.assertEqual(p1.steps[0].multi_state, False)
            self.assertEqual(p1.steps[0].ordered, False)
            self.assertEqual(p1.steps[0].ensemble, False)
            self.assertIsNone(p1.steps[0].software)
            self.assertIsNone(p1.steps[0].script_file)
            self.assertIsNone(p1.steps[0].description)
            self.assertEqual(p1.steps[1]._id, '2')
            self.assertEqual(p1.steps[1].multi_scale, True)
            self.assertIsNone(p1.steps[1].multi_state)
            self.assertEqual(p1.steps[1].ordered, False)
            self.assertEqual(p1.steps[1].ensemble, True)
            self.assertEqual(p1.steps[1].software._id, '401')
            self.assertEqual(p1.steps[1].script_file._id, '501')
            self.assertEqual(p1.steps[1].description, 'test step')

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
_ihm_modeling_post_process.details
1  1   1   1   'filter'  'energy/score'  15000   6520 . . 401 501 .
2  1   1   2   'cluster' 'dRMSD'         6520    6520 . . . . .
3  1   2   1   'filter'  'energy/score'  15000   6520 . . . . .
4  1   2   2   'filter'  'composition'   6520    6520 . . . . .
5  1   2   3   'cluster' 'dRMSD'         6520    6520 . . . . .
6  2   3   1   'none' .         .    . . . . . 'empty step'
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
            self.assertIsNone(a1.steps[1].software)
            self.assertIsNone(a1.steps[1].script_file)
            self.assertIsNone(a1.steps[1].details)
            self.assertEqual(len(a2.steps), 3)

            a1, = p2.analyses
            self.assertEqual(len(a1.steps), 1)
            self.assertEqual(a1.steps[0].__class__, ihm.analysis.EmptyStep)
            self.assertEqual(a1.steps[0].feature, 'none')
            self.assertIsNone(a1.steps[0].num_models_begin)
            self.assertIsNone(a1.steps[0].num_models_end)
            self.assertEqual(a1.steps[0].details, 'empty step')

    def test_model_list_handler(self):
        """Test ModelListHandler and ModelGroupHandler"""
        cif = """
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 'Best scoring model' 1 2 3
2 'Best scoring model' 1 1 1
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 "Cluster 1" .
2 "Cluster 2" .
#
loop_
_ihm_model_group_link.group_id
_ihm_model_group_link.model_id
1 1
2 2
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
ATOM 1 N N . MET 1 A 14.326 -2.326 8.122 1.000 1 A 0.000 42 42
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            sg, sg2 = s.state_groups
            # sg should contain all models in groups but not explicitly
            # put in a State
            state, = sg
            self.assertIsNone(state.name)  # auto-created state
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
            # sg2 should contain all models referenced by the file but not
            # put in groups (in this case, model ID 42 from atom_site)
            state, = sg2
            self.assertIsNone(state.name)  # auto-created state
            mg1, = state
            self.assertIsNone(mg1.name)  # auto-created group
            m, = mg1
            self.assertEqual(m._id, '42')

    def test_multi_state_handler(self):
        """Test MultiStateHandler and MultiStateLinkHandler"""
        cif = """
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 'Best scoring model' 1 2 3
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 "Cluster 1" .
#
loop_
_ihm_model_group_link.group_id
_ihm_model_group_link.model_id
1 1
#
loop_
_ihm_multi_state_modeling.state_id
_ihm_multi_state_modeling.state_group_id
_ihm_multi_state_modeling.population_fraction
_ihm_multi_state_modeling.state_type
_ihm_multi_state_modeling.state_name
_ihm_multi_state_modeling.experiment_type
_ihm_multi_state_modeling.details
1 1 0.4 'complex formation' 'unbound' 'Fraction of bulk'  'unbound molecule 1'
2 1 .  'complex formation' 'unbound' 'Fraction of bulk'  'unbound molecule 2'
3 1 .  'complex formation' 'bound' 'Fraction of bulk'
'bound molecules 1 and 2'
#
loop_
_ihm_multi_state_model_group_link.state_id
_ihm_multi_state_model_group_link.model_group_id
1 1
2 2
3 3
3 4
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            sg, = s.state_groups
            s1, s2, s3, = sg
            self.assertAlmostEqual(s1.population_fraction, 0.4, delta=0.1)
            self.assertEqual(s1.type, 'complex formation')
            self.assertEqual(s1.name, 'unbound')
            self.assertEqual(len(s1), 1)
            mg1, = s1
            self.assertEqual(mg1.name, 'Cluster 1')
            self.assertEqual(s1.experiment_type, 'Fraction of bulk')
            self.assertEqual(s1.details, 'unbound molecule 1')

            self.assertIsNone(s2.population_fraction)
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
_ihm_ensemble_info.details
_ihm_ensemble_info.sub_sample_flag
_ihm_ensemble_info.sub_sampling_type
1 'Cluster 1' 2 3 . dRMSD 1257 1 15.400 9 . . .
2 'Cluster 2' 2 . . dRMSD 1257 1 15.400 9 'cluster details' YES independent
3 'Cluster 3' . . invalid_cluster invalid_feature 1 1 15.400 9 . . .
#
#
loop_
_ihm_ensemble_sub_sample.id
_ihm_ensemble_sub_sample.name
_ihm_ensemble_sub_sample.ensemble_id
_ihm_ensemble_sub_sample.num_models
_ihm_ensemble_sub_sample.num_models_deposited
_ihm_ensemble_sub_sample.model_group_id
_ihm_ensemble_sub_sample.file_id
1 ss1 2 5 0 . .
2 ss2 2 5 2 42 3
#
"""
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh)
            e, e2, e3 = s.ensembles
            self.assertEqual(e.model_group._id, '3')
            self.assertEqual(e.num_models, 1257)
            self.assertEqual(e.post_process._id, '2')
            self.assertIsNone(e.clustering_method)
            self.assertEqual(e.clustering_feature, 'dRMSD')
            self.assertEqual(e.name, 'Cluster 1')
            self.assertIsNone(e.details)
            self.assertAlmostEqual(e.precision, 15.4, delta=0.1)
            self.assertEqual(e.file._id, '9')
            self.assertIsNone(e2.model_group)
            self.assertEqual(e2.details, 'cluster details')
            s1, s2 = e2.subsamples
            self.assertEqual(s1.name, 'ss1')
            self.assertEqual(s1.num_models, 5)
            self.assertIsNone(s1.model_group)
            self.assertIsNone(s1.file)
            self.assertIsInstance(s1, ihm.model.IndependentSubsample)
            self.assertEqual(s2.name, 'ss2')
            self.assertEqual(s2.num_models, 5)
            self.assertEqual(s2.model_group._id, '42')
            self.assertEqual(s2.file._id, '3')
            self.assertIsInstance(s2, ihm.model.IndependentSubsample)
            # invalid cluster/feature should be mapped to default
            self.assertEqual(e3.clustering_method, 'Other')
            self.assertEqual(e3.clustering_feature, 'other')

    def test_density_handler(self):
        """Test DensityHandler"""
        segment_cif = """
loop_
_ihm_entity_poly_segment.id
_ihm_entity_poly_segment.entity_id
_ihm_entity_poly_segment.seq_id_begin
_ihm_entity_poly_segment.seq_id_end
1 1 1 726
"""
        density_cif = """
loop_
_ihm_localization_density_files.id
_ihm_localization_density_files.file_id
_ihm_localization_density_files.ensemble_id
_ihm_localization_density_files.entity_id
_ihm_localization_density_files.asym_id
_ihm_localization_density_files.entity_poly_segment_id
1 22 9 1 A 1
2 23 9 2 B .
"""
        # Order should not matter
        for cif in (segment_cif + density_cif, density_cif + segment_cif):
            for fh in cif_file_handles(cif):
                s, = ihm.reader.read(fh)
                e, = s.ensembles
                self.assertEqual(e._id, '9')
                d1, d2 = e.densities
                self.assertEqual(d1._id, '1')
                self.assertEqual(d1.file._id, '22')
                self.assertEqual(d1.asym_unit.__class__, ihm.AsymUnitRange)
                self.assertEqual(d1.asym_unit.seq_id_range, (1, 726))
                self.assertEqual(d2._id, '2')
                self.assertEqual(d2.asym_unit.__class__, ihm.AsymUnit)

    def test_em3d_restraint_handler(self):
        """Test EM3DRestraintHandler"""
        fh = StringIO("""
loop_
_ihm_3dem_restraint.id
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
        fits = sorted(r.fits.items(), key=lambda x: x[0]._id)
        self.assertEqual(len(fits), 2)
        self.assertEqual(fits[0][0]._id, '1')
        self.assertIsNone(fits[0][1].cross_correlation_coefficient)
        self.assertEqual(fits[1][0]._id, '2')
        self.assertAlmostEqual(fits[1][1].cross_correlation_coefficient,
                               0.9, delta=0.1)

    def test_get_int(self):
        """Test _get_int method"""
        h = ihm.reader.Handler(None)
        self.assertEqual(h.get_int('45'), 45)
        self.assertIsNone(h.get_int(None))
        self.assertEqual(h.get_int(ihm.unknown), ihm.unknown)
        self.assertRaises(ValueError, h.get_int, ".")
        self.assertRaises(ValueError, h.get_int, "?")

    def test_get_int_or_string(self):
        """Test _get_int_or_string method"""
        h = ihm.reader.Handler(None)
        self.assertEqual(h.get_int_or_string('45A'), '45A')
        self.assertEqual(h.get_int_or_string('45'), 45)
        self.assertIsNone(h.get_int_or_string(None))
        self.assertEqual(h.get_int_or_string(ihm.unknown), ihm.unknown)
        self.assertEqual(h.get_int_or_string('.'), '.')
        self.assertEqual(h.get_int_or_string('?'), '?')
        self.assertEqual(h.get_int_or_string(45), 45)

    def test_get_float(self):
        """Test _get_float method"""
        h = ihm.reader.Handler(None)
        self.assertAlmostEqual(h.get_float('45.3'), 45.3, delta=0.1)
        self.assertIsNone(h.get_float(None))
        self.assertEqual(h.get_float(ihm.unknown), ihm.unknown)
        self.assertRaises(ValueError, h.get_float, ".")
        self.assertRaises(ValueError, h.get_float, "?")

    def test_get_bool(self):
        """Test _get_bool method"""
        h = ihm.reader.Handler(None)
        self.assertEqual(h.get_bool('YES'), True)
        self.assertEqual(h.get_bool('NO'), False)
        self.assertIsNone(h.get_bool('something else'))
        self.assertIsNone(h.get_bool(None))
        self.assertEqual(h.get_bool(ihm.unknown), ihm.unknown)

    def test_get_lower(self):
        """Test _get_lower method"""
        h = ihm.reader.Handler(None)
        self.assertEqual(h.get_lower('Test String'), 'test string')
        self.assertIsNone(h.get_lower(None))
        self.assertEqual(h.get_lower(ihm.unknown), ihm.unknown)
        self.assertEqual(h.get_lower('.'), '.')
        self.assertEqual(h.get_lower('?'), '?')

    def test_get_vector3(self):
        """Test _get_vector3 function"""
        d = {'tr_vector1': 4.0, 'tr_vector2': 6.0, 'tr_vector3': 9.0,
             'omitted1': None, 'unknown1': ihm.unknown}
        r = ihm.reader._get_vector3(d, 'tr_vector')
        # Coerce to int so we can compare exactly
        self.assertEqual([int(x) for x in r], [4, 6, 9])

        self.assertIsNone(ihm.reader._get_vector3(d, 'omitted'))
        self.assertEqual(ihm.reader._get_vector3(d, 'unknown'), ihm.unknown)

    def test_get_matrix33(self):
        """Test _get_matrix33 function"""
        d = {'m11': 4.0, 'm12': 6.0, 'm13': 9.0,
             'm21': 1.0, 'm22': 2.0, 'm23': 3.0,
             'm31': 8.0, 'm32': 1.0, 'm33': 7.0,
             'omitted11': None, 'unknown11': ihm.unknown}
        r = ihm.reader._get_matrix33(d, 'm')
        # Coerce to int so we can compare exactly
        self.assertEqual([[int(x) for x in row] for row in r],
                         [[4, 6, 9],
                          [1, 2, 3],
                          [8, 1, 7]])

        self.assertIsNone(ihm.reader._get_matrix33(d, 'omitted'))
        self.assertEqual(ihm.reader._get_matrix33(d, 'unknown'), ihm.unknown)

    def test_unknown_omitted(self):
        """Test that Handlers handle unknown/omitted values correctly"""
        fh = StringIO("""
loop_
_audit_author.name
_audit_author.pdbx_ordinal
. 1
? 2
'.' 3
'?' 4
#
""")
        s, = ihm.reader.read(fh)
        self.assertEqual(s.authors, [None, ihm.unknown, '.', '?'])

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
1 1 9 0.853 -0.637588 0.089507 0.765160 0.755616 -0.120841 0.643771 0.150085
0.988628 0.009414 327.161 83.209 -227.800
""")
        s, = ihm.reader.read(fh)
        r, = s.restraints
        self.assertEqual(r._id, '1')
        self.assertEqual(r.dataset._id, '65')
        self.assertEqual(r.number_raw_micrographs, 800)
        self.assertAlmostEqual(r.pixel_size_width, 2.030, delta=0.01)
        self.assertAlmostEqual(r.pixel_size_height, 4.030, delta=0.01)
        self.assertAlmostEqual(r.image_resolution, 35.0, delta=0.1)
        self.assertEqual(r.segment, False)
        self.assertEqual(r.number_of_projections, 10000)
        self.assertEqual(r.assembly._id, '42')
        fit, = list(r.fits.items())
        self.assertEqual(fit[0]._id, '9')
        self.assertAlmostEqual(fit[1].cross_correlation_coefficient, 0.853,
                               delta=0.01)
        self.assertAlmostEqual(fit[1].tr_vector[0], 327.161, delta=0.01)
        self.assertAlmostEqual(fit[1].rot_matrix[1][2], 0.988628, delta=0.01)
        self.assertEqual([int(x) for x in fit[1].tr_vector], [327, 83, -227])

    def test_sas_restraint_handler(self):
        """Test SASRestraintHandler"""
        fh = StringIO("""
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
        self.assertAlmostEqual(r.radius_of_gyration, 27.9, delta=0.1)
        fit, = list(r.fits.items())
        self.assertEqual(fit[0]._id, '8')
        self.assertAlmostEqual(fit[1].chi_value, 1.36, delta=0.01)

    def test_sphere_obj_site_handler(self):
        """Test SphereObjSiteHandler"""
        class MyModel(ihm.model.Model):
            def add_sphere(self, sphere):
                super(MyModel, self).add_sphere(sphere)
                self.sphere_count = len(self._spheres)

        fh = StringIO("""
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 . 1 1 1
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 "Cluster 1" .
#
loop_
_ihm_model_group_link.group_id
_ihm_model_group_link.model_id
1 1
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
1 1 1 6 A 389.993 145.089 134.782 4.931 . 1
2 1 7 7 B 406.895 142.176 135.653 3.318 1.34 1
""")
        s, = ihm.reader.read(fh, model_class=MyModel)
        m = s.state_groups[0][0][0][0]
        self.assertEqual(m.sphere_count, 2)
        s1, s2 = m._spheres
        self.assertEqual(s1.asym_unit._id, 'A')
        self.assertEqual(s1.seq_id_range, (1, 6))
        self.assertAlmostEqual(s1.x, 389.993, delta=0.01)
        self.assertAlmostEqual(s1.y, 145.089, delta=0.01)
        self.assertAlmostEqual(s1.z, 134.782, delta=0.01)
        self.assertAlmostEqual(s1.radius, 4.931, delta=0.01)
        self.assertIsNone(s1.rmsf)
        self.assertAlmostEqual(s2.rmsf, 1.34, delta=0.1)

    def test_atom_site_handler(self):
        """Test AtomSiteHandler"""
        fh = StringIO("""
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 . 1 1 1
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 "Cluster 1" .
#
loop_
_ihm_model_group_link.group_id
_ihm_model_group_link.model_id
1 1
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
ATOM 1 N N . SER 1 A 54.401 -49.984 -35.287 . 1 A . 1 1
HETATM 2 C CA . SER . B 54.452 -48.492 -35.210 0.200 1 A 42.0 1 1
""")
        s, = ihm.reader.read(fh)
        m = s.state_groups[0][0][0][0]
        a1, a2 = m._atoms
        self.assertEqual(a1.asym_unit._id, 'A')
        self.assertEqual(a1.seq_id, 1)
        self.assertEqual(a1.atom_id, 'N')
        self.assertEqual(a1.type_symbol, 'N')
        self.assertAlmostEqual(a1.x, 54.401, delta=0.01)
        self.assertAlmostEqual(a1.y, -49.984, delta=0.01)
        self.assertAlmostEqual(a1.z, -35.287, delta=0.01)
        self.assertEqual(a1.het, False)
        self.assertIsNone(a1.biso)
        self.assertIsNone(a1.occupancy)

        self.assertEqual(a2.asym_unit._id, 'B')
        self.assertIsNone(a2.seq_id)
        self.assertEqual(a2.atom_id, 'CA')
        self.assertEqual(a2.type_symbol, 'C')
        self.assertEqual(a2.het, True)
        self.assertAlmostEqual(a2.biso, 42.0, delta=1.0)
        self.assertAlmostEqual(a2.occupancy, 0.2, delta=0.1)

    def test_atom_site_handler_auth_seq_id(self):
        """Test AtomSiteHandler handling of auth_seq_id and ins_code"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 . 1 1 1
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 "Cluster 1" .
#
loop_
_ihm_model_group_link.group_id
_ihm_model_group_link.model_id
1 1
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
_atom_site.pdbx_PDB_ins_code
_atom_site.label_asym_id
_atom_site.Cartn_x
_atom_site.Cartn_y
_atom_site.Cartn_z
_atom_site.label_entity_id
_atom_site.auth_asym_id
_atom_site.B_iso_or_equiv
_atom_site.pdbx_PDB_model_num
_atom_site.ihm_model_id
ATOM 1 N N . SER 1 2 A A 54.401 -49.984 -35.287 1 A . 1 1
HETATM 2 C CA . SER 2 20A . A 54.452 -48.492 -35.210 1 A 42.0 1 1
ATOM 3 N N . SER 3 3 . A 54.401 -49.984 -35.287 1 A . 1 1
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map, {1: (2, 'A'), 2: ('20A', None)})

    def test_atom_site_handler_no_asym_id(self):
        """Test AtomSiteHandler with missing asym_id"""
        fh = StringIO("""
loop_
_entity_poly_seq.entity_id
_entity_poly_seq.num
_entity_poly_seq.mon_id
_entity_poly_seq.hetero
5 1 MET .
5 2 CYS .
5 3 MET .
5 4 SER .
#
loop_
_struct_asym.id
_struct_asym.entity_id
_struct_asym.details
D 5 foo
#
loop_
_atom_site.group_PDB
_atom_site.id
_atom_site.type_symbol
_atom_site.label_atom_id
_atom_site.label_alt_id
_atom_site.label_comp_id
_atom_site.label_asym_id
_atom_site.label_entity_id
_atom_site.label_seq_id
_atom_site.pdbx_PDB_ins_code
_atom_site.Cartn_x
_atom_site.Cartn_y
_atom_site.Cartn_z
_atom_site.occupancy
_atom_site.B_iso_or_equiv
_atom_site.pdbx_formal_charge
_atom_site.auth_asym_id
_atom_site.pdbx_PDB_model_num
ATOM   1  C CA  . MET . . 1 ?  1.000 1.000 1.000 1.00   2.95 0 A 1
ATOM   2  C CA  . ASP . . 2 ?  2.000 2.000 2.000 1.00   0.95 0 A 1
ATOM   3  C CA  . CYS . . 3 ?  3.000 3.000 3.000 1.00   0.95 0 A 1
ATOM   4  C CA  . MET . . 1 ?  1.000 1.000 1.000 1.00   2.95 0 B 1
ATOM   5  C CA  . ASP . . 2 ?  2.000 2.000 2.000 1.00   0.95 0 B 1
ATOM   6  C CA  . CYS . . 3 ?  3.000 3.000 3.000 1.00   0.95 0 B 1
ATOM   7  C CA  . CYS . . 2 ?  1.000 1.000 1.000 1.00   0.95 0 C 1
ATOM   8  C CA  . CYS . . 5 ?  3.000 3.000 3.000 1.00   0.95 0 C 1
ATOM   9  C CA  . MET . . 1 ?  3.000 3.000 3.000 1.00   0.95 0 D 1
""")
        s, = ihm.reader.read(fh)
        # No asym_id, so use auth_asym_id
        a1, a2, a3, a4 = s.asym_units
        self.assertEqual(a1._id, 'D')
        self.assertEqual(a2._id, 'A')
        self.assertEqual(a3._id, 'B')
        self.assertEqual(a4._id, 'C')
        # A and B should have same sequence, thus same entity
        self.assertIs(a2.entity, a3.entity)
        # Sequence should have been populated from comp_ids
        self.assertEqual("".join(c.code for c in a2.entity.sequence), "MDC")
        # C has different entity and sequence, with gaps
        self.assertEqual("".join(c.code_canonical for c in a4.entity.sequence),
                         "XCXXC")
        # D is defined in struct_asym and entity_poly so should use that
        # sequence
        self.assertEqual("".join(c.code_canonical for c in a1.entity.sequence),
                         "MCMS")

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
2 1 1 . 1 ALA CB
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
2 2 1 . 2 CYS 3 GLY
#
loop_
_ihm_non_poly_feature.ordinal_id
_ihm_non_poly_feature.feature_id
_ihm_non_poly_feature.entity_id
_ihm_non_poly_feature.asym_id
_ihm_non_poly_feature.comp_id
_ihm_non_poly_feature.atom_id
1 3 3 C HEM FE
2 3 3 . HEM FE
3 4 3 C HEM .
4 4 3 . HEM .
#
loop_
_ihm_pseudo_site.id
_ihm_pseudo_site.Cartn_x
_ihm_pseudo_site.Cartn_y
_ihm_pseudo_site.Cartn_z
_ihm_pseudo_site.radius
_ihm_pseudo_site.description
55 10.000 20.000 30.000 4.0 'centroid'
#
loop_
_ihm_pseudo_site_feature.feature_id
_ihm_pseudo_site_feature.pseudo_site_id
5 55
"""
        rsr = """
loop_
_ihm_feature_list.feature_id
_ihm_feature_list.feature_type
_ihm_feature_list.entity_type
_ihm_feature_list.details
1 atom polymer 'test feature'
2 'residue range' polymer .
3 atom non-polymer .
4 atom non-polymer .
5 'pseudo site' other .
#
loop_
_ihm_derived_distance_restraint.id
_ihm_derived_distance_restraint.group_id
_ihm_derived_distance_restraint.feature_id_1
_ihm_derived_distance_restraint.feature_id_2
_ihm_derived_distance_restraint.restraint_type
_ihm_derived_distance_restraint.distance_lower_limit
_ihm_derived_distance_restraint.distance_upper_limit
_ihm_derived_distance_restraint.probability
_ihm_derived_distance_restraint.mic_value
_ihm_derived_distance_restraint.group_conditionality
_ihm_derived_distance_restraint.dataset_list_id
1 . 1 2 'lower bound' 25.000 . 0.800 0.400 . 97
2 . 1 4 'upper bound' . 45.000 0.800 . ALL 98
3 1 1 2 'lower and upper bound' 22.000 45.000 0.800 . ANY 99
4 1 5 3 'harmonic' 35.000 35.000 0.800 . ALL .
5 . 5 3 . ? ? ? . ALL .
"""
        # Test both ways to make sure features still work if they are
        # referenced by ID before their type is known
        for text in (feats + rsr, rsr + feats):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            self.assertEqual(len(s.orphan_features), 5)
            r1, r2, r3, r4, r5 = s.restraints
            rg1, = s.restraint_groups
            self.assertEqual([r for r in rg1], [r3, r4])
            self.assertEqual(r1.dataset._id, '97')
            self.assertIsInstance(r1.feature1,
                                  ihm.restraint.AtomFeature)
            self.assertEqual(len(r1.feature1.atoms), 2)
            self.assertEqual(r1.feature1.atoms[0].id, 'CA')
            self.assertEqual(r1.feature1.atoms[0].residue.seq_id, 1)
            self.assertIsNone(r1.feature1.atoms[0].residue.entity)
            self.assertEqual(r1.feature1.atoms[1].id, 'CB')
            self.assertEqual(r1.feature1.atoms[1].residue.seq_id, 1)
            self.assertIsNone(r1.feature1.atoms[1].residue.asym)
            self.assertEqual(r1.feature1.details, 'test feature')
            self.assertIsInstance(r1.feature2,
                                  ihm.restraint.ResidueFeature)
            self.assertEqual(len(r1.feature2.ranges), 2)
            self.assertEqual(r1.feature2.ranges[0].seq_id_range, (2, 3))
            self.assertIsInstance(r1.feature2.ranges[0], ihm.AsymUnitRange)
            self.assertEqual(r1.feature2.ranges[1].seq_id_range, (2, 3))
            self.assertIsInstance(r1.feature2.ranges[1], ihm.EntityRange)
            self.assertIsInstance(r1.distance,
                                  ihm.restraint.LowerBoundDistanceRestraint)
            self.assertAlmostEqual(r1.distance.distance, 25.000, delta=0.1)
            self.assertAlmostEqual(r1.probability, 0.8000, delta=0.1)
            self.assertAlmostEqual(r1.mic_value, 0.4000, delta=0.1)
            self.assertIsNone(r1.restrain_all)
            self.assertEqual(r2.restrain_all, True)
            self.assertEqual(r3.restrain_all, False)
            self.assertIsInstance(r2.feature2,
                                  ihm.restraint.NonPolyFeature)
            self.assertEqual(len(r2.feature2.objs), 2)
            self.assertIsInstance(r2.feature2.objs[0], ihm.AsymUnit)
            self.assertEqual(r2.feature2.objs[0]._id, 'C')
            self.assertIsInstance(r2.feature2.objs[1], ihm.Entity)
            self.assertIsInstance(r2.distance,
                                  ihm.restraint.UpperBoundDistanceRestraint)
            self.assertIsNone(r2.mic_value)
            self.assertIsInstance(
                r3.distance, ihm.restraint.LowerUpperBoundDistanceRestraint)
            self.assertIsInstance(r4.distance,
                                  ihm.restraint.HarmonicDistanceRestraint)
            self.assertIsInstance(r4.feature2,
                                  ihm.restraint.AtomFeature)
            self.assertIsNone(r4.feature2.atoms[0].residue.entity)
            self.assertIsNone(r4.feature2.atoms[1].residue.asym)
            self.assertIsInstance(r4.feature1,
                                  ihm.restraint.PseudoSiteFeature)
            self.assertAlmostEqual(r4.feature1.site.x, 10.0, delta=0.1)
            self.assertAlmostEqual(r4.feature1.site.y, 20.0, delta=0.1)
            self.assertAlmostEqual(r4.feature1.site.z, 30.0, delta=0.1)
            self.assertAlmostEqual(r4.feature1.site.radius, 4.0, delta=0.1)
            self.assertEqual(r4.feature1.site.description, 'centroid')

    def test_sphere_handler(self):
        """Test SphereHandler"""
        obj_list = CENTERS_TRANSFORMS + """
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
1 sphere 'my sphere' 'a test sphere'
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
        for text in (obj_list + spheres, spheres + obj_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            s1, s2 = s.orphan_geometric_objects
            self.assertIsInstance(s1, ihm.geometry.Sphere)
            self.assertIsInstance(s2, ihm.geometry.Sphere)
            self.assertEqual(s1.name, 'my sphere')
            self.assertEqual(s1.description, 'a test sphere')
            self.assertAlmostEqual(s1.center.x, 1.000, delta=0.1)
            self.assertAlmostEqual(s1.center.y, 2.000, delta=0.1)
            self.assertAlmostEqual(s1.center.z, 3.000, delta=0.1)
            self.assertAlmostEqual(s1.transformation.tr_vector[1], 2.000,
                                   delta=0.1)
            self.assertIsNone(s2.name)
            self.assertIsNone(s2.center)
            self.assertIsNone(s2.transformation)

    def test_torus_handler(self):
        """Test TorusHandler"""
        obj_list = CENTERS_TRANSFORMS + """
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
1 torus . .
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
        for text in (obj_list + tori, tori + obj_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            t1, t2 = s.orphan_geometric_objects
            self.assertIsInstance(t1, ihm.geometry.Torus)
            self.assertIsInstance(t2, ihm.geometry.Torus)
            self.assertAlmostEqual(t1.center.x, 1.000, delta=0.1)
            self.assertAlmostEqual(t1.transformation.tr_vector[1], 2.000,
                                   delta=0.1)
            self.assertAlmostEqual(t1.major_radius, 5.600, delta=0.1)
            self.assertAlmostEqual(t1.minor_radius, 1.200, delta=0.1)
            self.assertIsNone(t2.center)
            self.assertIsNone(t2.transformation)

    def test_half_torus_handler(self):
        """Test HalfTorusHandler"""
        obj_list = CENTERS_TRANSFORMS + """
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
1 half-torus . .
2 half-torus . .
3 half-torus . .
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
        for text in (obj_list + tori + half_tori, tori + half_tori + obj_list,
                     obj_list + half_tori + tori, half_tori + tori + obj_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            t1, t2, t3 = s.orphan_geometric_objects
            self.assertIsInstance(t1, ihm.geometry.HalfTorus)
            self.assertIsInstance(t2, ihm.geometry.HalfTorus)
            self.assertIsInstance(t3, ihm.geometry.HalfTorus)
            self.assertAlmostEqual(t1.center.x, 1.000, delta=0.1)
            self.assertAlmostEqual(t1.transformation.tr_vector[1], 2.000,
                                   delta=0.1)
            self.assertAlmostEqual(t1.major_radius, 5.600, delta=0.1)
            self.assertAlmostEqual(t1.minor_radius, 1.200, delta=0.1)
            self.assertAlmostEqual(t1.thickness, 0.100, delta=0.1)
            self.assertEqual(t1.inner, True)
            self.assertIsNone(t2.center)
            self.assertIsNone(t2.transformation)
            self.assertEqual(t2.inner, False)
            self.assertIsNone(t3.inner)

    def test_axis_handler(self):
        """Test AxisHandler"""
        obj_list = CENTERS_TRANSFORMS + """
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
1 axis . .
2 axis . .
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
        for text in (obj_list + axes, axes + obj_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            a1, a2 = s.orphan_geometric_objects
            self.assertIsInstance(a1, ihm.geometry.XAxis)
            self.assertIsInstance(a2, ihm.geometry.YAxis)
            self.assertAlmostEqual(a1.transformation.tr_vector[1], 2.000,
                                   delta=0.1)
            self.assertIsNone(a2.transformation)

    def test_plane_handler(self):
        """Test PlaneHandler"""
        obj_list = CENTERS_TRANSFORMS + """
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
1 plane . .
2 plane . .
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
        for text in (obj_list + planes, planes + obj_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            p1, p2 = s.orphan_geometric_objects
            self.assertIsInstance(p1, ihm.geometry.XYPlane)
            self.assertIsInstance(p2, ihm.geometry.YZPlane)
            self.assertAlmostEqual(p1.transformation.tr_vector[1], 2.000,
                                   delta=0.1)
            self.assertIsNone(p2.transformation)

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
        self.assertIsInstance(r1, ihm.restraint.GeometricRestraint)
        self.assertEqual(r1.dataset._id, '97')
        self.assertEqual(r1.geometric_object._id, '23')
        self.assertEqual(r1.feature._id, '44')
        self.assertIsInstance(r1.distance,
                              ihm.restraint.UpperBoundDistanceRestraint)
        self.assertAlmostEqual(r1.distance.distance, 25.000, delta=0.1)
        self.assertAlmostEqual(r1.harmonic_force_constant, 2.000, delta=0.1)
        self.assertEqual(r1.restrain_all, False)
        self.assertEqual(r2.restrain_all, True)
        self.assertIsNone(r3.restrain_all)

        self.assertIsInstance(r2, ihm.restraint.CenterGeometricRestraint)
        self.assertIsInstance(r3, ihm.restraint.InnerSurfaceGeometricRestraint)
        self.assertIsInstance(r4, ihm.restraint.OuterSurfaceGeometricRestraint)

        self.assertIsInstance(r2.distance,
                              ihm.restraint.LowerBoundDistanceRestraint)
        self.assertIsInstance(r3.distance,
                              ihm.restraint.LowerUpperBoundDistanceRestraint)
        self.assertIsInstance(r4.distance,
                              ihm.restraint.HarmonicDistanceRestraint)

    def test_poly_seq_scheme_handler_offset(self):
        """Test PolySeqSchemeHandler with constant offset"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_pdbx_poly_seq_scheme.asym_id
_pdbx_poly_seq_scheme.entity_id
_pdbx_poly_seq_scheme.seq_id
_pdbx_poly_seq_scheme.auth_seq_num
_pdbx_poly_seq_scheme.pdb_strand_id
A 1 1 6 A
A 1 2 7 A
A 1 3 8 A
A 1 4 9 A
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map, 5)
        self.assertIsNone(asym._strand_id)
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1, 5)],
                         [6, 7, 8, 9])

    def test_poly_seq_scheme_handler_offset_ins_code(self):
        """Test PolySeqSchemeHandler with constant offset but inscodes"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_pdbx_poly_seq_scheme.asym_id
_pdbx_poly_seq_scheme.entity_id
_pdbx_poly_seq_scheme.seq_id
_pdbx_poly_seq_scheme.auth_seq_num
_pdbx_poly_seq_scheme.pdb_strand_id
_pdbx_poly_seq_scheme.pdb_ins_code
A 1 1 6 A .
A 1 2 7 A .
A 1 3 8 A .
A 1 4 9 A A
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map,
                         {1: (6, None), 2: (7, None), 3: (8, None),
                          4: (9, 'A')})
        self.assertIsNone(asym._strand_id)
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1, 5)],
                         [6, 7, 8, 9])
        self.assertIsNone(asym.residue(1).ins_code)
        self.assertEqual(asym.residue(4).ins_code, 'A')

    def test_poly_seq_scheme_handler_empty(self):
        """Test PolySeqSchemeHandler with no poly_seq_scheme"""
        fh = StringIO(ASYM_ENTITY)
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym.auth_seq_id_map, 0)
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1, 5)],
                         [1, 2, 3, 4])

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
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1, 5)],
                         [1, 2, 3, 4])

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
        # No mapping for residue 4 (and no insertion codes at all)
        self.assertEqual(asym.auth_seq_id_map, {1: (6, None), 2: (7, None),
                                                3: (8, None)})
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1, 5)],
                         [6, 7, 8, 4])
        self.assertIsNone(asym.residue(1).ins_code)

    def test_poly_seq_scheme_handler_incon_off(self):
        """Test PolySeqSchemeHandler with inconsistent offset"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_pdbx_poly_seq_scheme.asym_id
_pdbx_poly_seq_scheme.entity_id
_pdbx_poly_seq_scheme.seq_id
_pdbx_poly_seq_scheme.auth_seq_num
_pdbx_poly_seq_scheme.pdb_strand_id
A 1 1 6 X
A 1 2 7 X
A 1 3 8 X
A 1 4 10 X
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertEqual(asym._strand_id, 'X')
        self.assertEqual(asym.auth_seq_id_map, {1: (6, None), 2: (7, None),
                                                3: (8, None), 4: (10, None)})
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1, 5)],
                         [6, 7, 8, 10])
        self.assertIsNone(asym.residue(1).ins_code)

    def test_poly_seq_scheme_handler_str_seq_id(self):
        """Test PolySeqSchemeHandler with a non-integer auth_seq_num"""
        fh = StringIO(ASYM_ENTITY + """
loop_
_pdbx_poly_seq_scheme.asym_id
_pdbx_poly_seq_scheme.entity_id
_pdbx_poly_seq_scheme.seq_id
_pdbx_poly_seq_scheme.auth_seq_num
_pdbx_poly_seq_scheme.pdb_strand_id
_pdbx_poly_seq_scheme.pdb_ins_code
A 1 1 6 ? .
A 1 2 7 ? .
A 1 3 8 ? X
A 1 4 9A ? .
""")
        s, = ihm.reader.read(fh)
        asym, = s.asym_units
        self.assertIsNone(asym._strand_id)
        self.assertEqual(asym.auth_seq_id_map, {1: (6, None), 2: (7, None),
                                                3: (8, 'X'), 4: ('9A', None)})
        self.assertEqual([asym.residue(i).auth_seq_id for i in range(1, 5)],
                         [6, 7, 8, '9A'])
        self.assertIsNone(asym.residue(1).ins_code)
        self.assertEqual(asym.residue(3).ins_code, 'X')

    def test_nonpoly_scheme_handler(self):
        """Test NonPolySchemeHandler"""
        fh = StringIO("""
loop_
_chem_comp.id
_chem_comp.type
_chem_comp.name
CA non-polymer 'CALCIUM ION'
#
loop_
_entity.id
_entity.type
_entity.pdbx_description
1 non-polymer 'CALCIUM ION entity'
2 non-polymer 'no-chem-comp entity'
3 water       'no-chem-comp water'
#
loop_
_pdbx_entity_nonpoly.entity_id
_pdbx_entity_nonpoly.name
_pdbx_entity_nonpoly.comp_id
1 'CALCIUM ION'             CA
#
loop_
_struct_asym.id
_struct_asym.entity_id
_struct_asym.details
A 1 foo
B 2 bar
C 3 baz
#
loop_
_pdbx_nonpoly_scheme.asym_id
_pdbx_nonpoly_scheme.entity_id
_pdbx_nonpoly_scheme.mon_id
_pdbx_nonpoly_scheme.auth_seq_num
_pdbx_nonpoly_scheme.pdb_strand_id
_pdbx_nonpoly_scheme.pdb_ins_code
A 1 FOO 1 . .
A 1 BAR 101 . .
B 2 BAR 1 Q X
C 3 HOH 1 . .
""")
        s, = ihm.reader.read(fh)
        e1, e2, e3 = s.entities
        # e1 should have sequence filled in by pdbx_entity_nonpoly
        self.assertEqual([cc.name for cc in e1.sequence], ['CALCIUM ION'])
        # e2,e3 should have sequence filled in by pdbx_nonpoly_scheme
        self.assertEqual([(cc.id, cc.name) for cc in e2.sequence],
                         [('BAR', 'no-chem-comp entity')])
        self.assertEqual([(cc.id, cc.name) for cc in e3.sequence],
                         [('HOH', 'WATER')])
        asym, a2, a3 = s.asym_units
        # non-polymers have no seq_id_range
        self.assertEqual(asym.seq_id_range, (None, None))
        self.assertEqual(asym.auth_seq_id_map, {1: (101, None)})
        self.assertEqual(asym.residue(1).auth_seq_id, 101)
        self.assertIsNone(asym.residue(1).ins_code)
        self.assertEqual(asym.strand_id, asym._id)
        self.assertIsNone(asym._strand_id)

        self.assertEqual(a2.auth_seq_id_map, {1: (1, 'X')})
        self.assertEqual(a2.residue(1).auth_seq_id, 1)
        self.assertEqual(a2.residue(1).ins_code, 'X')
        self.assertEqual(a2.strand_id, 'Q')
        self.assertEqual(a2._strand_id, 'Q')

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
_ihm_cross_link_list.linker_chem_comp_descriptor_id
_ihm_cross_link_list.dataset_list_id
_ihm_cross_link_list.details
1 1 foo 1 2 THR foo 1 3 CYS 44 97 .
2 2 foo 1 2 THR bar 2 3 PHE 44 97 'test xl'
3 2 foo 1 2 THR bar 2 2 GLU 44 97 .
4 3 foo 1 1 ALA bar 2 1 ASP 44 97 .
5 4 foo 1 1 ALA bar 2 1 ASP 88 97 .
6 5 foo 1 1 ALA bar 2 1 ASP 44 98 .
""")
        s, = ihm.reader.read(fh)
        # Check grouping
        self.assertEqual([[len(g) for g in r.experimental_cross_links]
                          for r in s.restraints], [[1, 2, 1], [1], [1]])
        r1, r2, r3 = s.restraints
        self.assertEqual(r1.dataset._id, '97')
        self.assertEqual(r1.linker._id, '44')
        xl = r1.experimental_cross_links[1][0]
        self.assertEqual(xl.residue1.entity._id, '1')
        self.assertEqual(xl.residue2.entity._id, '2')
        self.assertEqual(xl.residue1.seq_id, 2)
        self.assertEqual(xl.residue2.seq_id, 3)
        self.assertEqual(xl.details, 'test xl')

    def test_cross_link_list_handler_linker_type(self):
        """Test CrossLinkListHandler with old-style linker_type"""
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
5 4 foo 1 1 ALA bar 2 1 ASP TST 97
6 5 foo 1 1 ALA bar 2 1 ASP DSS 98
""")
        s, = ihm.reader.read(fh)
        # Check grouping
        r1, r2, r3 = s.restraints
        self.assertEqual(r1.linker.auth_name, 'DSS')
        self.assertEqual(r1.linker.chemical_name, 'disuccinimidyl suberate')
        self.assertEqual(r2.linker.auth_name, 'TST')
        self.assertIsNone(r2.linker.chemical_name)
        self.assertEqual(r3.linker.auth_name, 'DSS')
        self.assertEqual(r3.linker.chemical_name, 'disuccinimidyl suberate')

    def test_cross_link_list_handler_empty_descriptor(self):
        """Test CrossLinkListHandler with empty descriptor name"""
        fh = StringIO("""
loop_
_ihm_chemical_component_descriptor.id
_ihm_chemical_component_descriptor.auth_name
1 DSS
2 .
3 .
#
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
1 1 foo 1 2 THR foo 1 3 CYS 1 NOTDSS 97
2 2 foo 1 2 THR bar 2 3 PHE 2 EDC 97
3 2 foo 1 2 THR bar 2 3 PHE 3 . 97
""")
        s, = ihm.reader.read(fh)
        d1, d2, d3 = s.orphan_chem_descriptors
        # Descriptor name (DSS) should take precedence over
        # linker_type (NOTDSS)
        self.assertEqual(d1.auth_name, 'DSS')
        # If descriptor name is empty, fill it in using linker_type
        self.assertEqual(d2.auth_name, 'EDC')
        # If both names are empty, name is None
        self.assertIsNone(d3.auth_name)

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
#
loop_
_ihm_cross_link_pseudo_site.id
_ihm_cross_link_pseudo_site.restraint_id
_ihm_cross_link_pseudo_site.cross_link_partner
_ihm_cross_link_pseudo_site.pseudo_site_id
_ihm_cross_link_pseudo_site.model_id
1 2 1 44 .
2 2 2 88 99
3 2 2 880 990
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
        for text in (xl_list + xl_rsr, xl_rsr + xl_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            r, = s.restraints
            xl1, xl2 = sorted(r.cross_links, key=operator.attrgetter('_id'))
            self.assertIsInstance(xl1, ihm.restraint.ResidueCrossLink)
            self.assertEqual(xl1.experimental_cross_link.residue1.seq_id, 2)
            self.assertEqual(xl1.experimental_cross_link.residue2.seq_id, 3)
            self.assertEqual(xl1.fits, {})
            self.assertEqual(xl1.asym1._id, 'A')
            self.assertEqual(xl1.asym2._id, 'B')
            self.assertIsInstance(xl1.distance,
                                  ihm.restraint.UpperBoundDistanceRestraint)
            self.assertAlmostEqual(xl1.distance.distance, 25.000, delta=0.1)
            self.assertAlmostEqual(xl1.psi, 0.500, delta=0.1)
            self.assertAlmostEqual(xl1.sigma1, 1.000, delta=0.1)
            self.assertAlmostEqual(xl1.sigma2, 2.000, delta=0.1)
            self.assertIsNone(xl1.pseudo1)
            self.assertIsNone(xl1.pseudo2)

            self.assertIsInstance(xl2, ihm.restraint.AtomCrossLink)
            self.assertEqual(xl2.fits, {})
            self.assertIsInstance(xl2.distance,
                                  ihm.restraint.LowerBoundDistanceRestraint)
            self.assertTrue(xl2.atom1, 'C')
            self.assertTrue(xl2.atom2, 'N')
            self.assertAlmostEqual(xl2.distance.distance, 34.000, delta=0.1)
            self.assertIsNone(xl2.psi)
            self.assertIsNone(xl2.sigma1)
            self.assertIsNone(xl2.sigma2)
            ps1, = xl2.pseudo1
            self.assertEqual(ps1.site._id, '44')
            self.assertIsNone(ps1.model)
            ps21, ps22 = xl2.pseudo2
            self.assertEqual(ps21.site._id, '88')
            self.assertEqual(ps21.model._id, '99')
            self.assertEqual(ps22.site._id, '880')
            self.assertEqual(ps22.model._id, '990')

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
_ihm_cross_link_result_parameters.id
_ihm_cross_link_result_parameters.restraint_id
_ihm_cross_link_result_parameters.model_id
_ihm_cross_link_result_parameters.psi
_ihm_cross_link_result_parameters.sigma_1
_ihm_cross_link_result_parameters.sigma_2
1 1 201 0.100 4.200 2.100
2 1 301 . . .
"""
        # Order of categories shouldn't matter
        for text in (xl_list + xl_rsr + xl_fit, xl_fit + xl_rsr + xl_list):
            fh = StringIO(text)
            s, = ihm.reader.read(fh)
            r, = s.restraints
            xl, = r.cross_links
            # Sort fits by model ID
            fits = sorted(xl.fits.items(), key=lambda x: x[0]._id)
            self.assertEqual(len(fits), 2)
            self.assertEqual(fits[0][0]._id, '201')
            self.assertAlmostEqual(fits[0][1].psi, 0.100, delta=0.1)
            self.assertAlmostEqual(fits[0][1].sigma1, 4.200, delta=0.1)
            self.assertAlmostEqual(fits[0][1].sigma2, 2.100, delta=0.1)

            self.assertEqual(fits[1][0]._id, '301')
            self.assertIsNone(fits[1][1].psi)
            self.assertIsNone(fits[1][1].sigma1)
            self.assertIsNone(fits[1][1].sigma2)

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
        self.assertIsNone(e1.description)
        self.assertEqual(e1.group_begin._id, '1')
        self.assertEqual(e1.group_end._id, '2')

        self.assertEqual(s2.description, 'step 2 desc')
        self.assertEqual(len(s2), 2)
        e1 = s2[0]
        self.assertEqual(e1.description, 'edge 2 desc')
        self.assertEqual(e1.group_begin._id, '1')
        self.assertEqual(e1.group_end._id, '3')
        e2 = s2[1]
        self.assertIsNone(e2.description)
        self.assertEqual(e2.group_begin._id, '1')
        self.assertEqual(e2.group_end._id, '4')

    def test_read_full_pdbx(self):
        """Test reading a full PDBx file"""
        fname = utils.get_input_file_name(TOPDIR, '6ep0.cif.gz')
        # We can't use 'with' here because that requires Python >= 2.7
        f = gzip.open(fname, 'rt' if sys.version_info[0] >= 3 else 'rb')
        s, = ihm.reader.read(f)
        f.close()

    def test_old_file_read_default(self):
        """Test default handling of old files"""
        cif = """
loop_
_audit_conform.dict_name
_audit_conform.dict_version
mmcif_pdbx.dic     5.311
ihm-extension.dic  0.14
"""
        s, = ihm.reader.read(StringIO(cif))

    def test_old_file_read_fail(self):
        """Test failure reading old files"""
        cif = """
loop_
_audit_conform.dict_name
_audit_conform.dict_version
mmcif_pdbx.dic     5.311
ihm-extension.dic  0.14
"""
        self.assertRaises(ihm.reader.OldFileError,
                          ihm.reader.read, StringIO(cif), reject_old_file=True)

    def test_new_file_read_ok(self):
        """Test success reading not-old files"""
        # File read is OK if version is new enough, or version cannot be parsed
        # because it is non-int or has too many elements
        for ver in ('1.0', '0.0.4', '0.0a'):
            cif = """
loop_
_audit_conform.dict_name
_audit_conform.dict_version
mmcif_pdbx.dic     5.311
ihm-extension.dic  %s
""" % ver
            s, = ihm.reader.read(StringIO(cif), reject_old_file=True)

    def test_warn_unknown_category(self):
        """Test warnings for unknown categories"""
        cif = """
_cat1.foo baz
_cat1.bar baz
#
loop_
_cat2.foo
_cat2.bar
x y
"""
        with warnings.catch_warnings(record=True) as w:
            warnings.simplefilter("always")
            # Test with no warnings
            s, = ihm.reader.read(StringIO(cif))
            self.assertEqual(len(w), 0)
            s, = ihm.reader.read(StringIO(cif), warn_unknown_category=True)
            # Should only warn once per category
            self.assertEqual(len(w), 2)
            self.assertEqual(w[0].category, ihm.reader.UnknownCategoryWarning)
            self.assertTrue('Unknown category _cat1 encountered on line 2'
                            in str(w[0].message))
            self.assertEqual(w[1].category, ihm.reader.UnknownCategoryWarning)
            self.assertTrue('Unknown category _cat2 encountered on line 6'
                            in str(w[1].message))

    def test_warn_unknown_keyword(self):
        """Test warnings for unknown keywords"""
        cif = """
_cat1.foo baz
_struct.unknown foo
#
loop_
_struct_asym.id
_struct_asym.bar
1 y
loop_
_audit_author.pdbx_ordinal
_audit_author.name
1 "Smith J"
"""
        with warnings.catch_warnings(record=True) as w:
            warnings.simplefilter("always")
            # Test with no warnings
            s, = ihm.reader.read(StringIO(cif))
            self.assertEqual(len(w), 0)
            s, = ihm.reader.read(StringIO(cif), warn_unknown_keyword=True)
            # pdbx_ordinal is explicitly ignored, so should not trigger
            # a warning
            self.assertEqual(len(w), 2)
            self.assertEqual(w[0].category, ihm.reader.UnknownKeywordWarning)
            self.assertTrue('keyword _struct.unknown encountered on line 3'
                            in str(w[0].message))
            self.assertEqual(w[1].category, ihm.reader.UnknownKeywordWarning)
            self.assertTrue('keyword _struct_asym.bar encountered on line 7'
                            in str(w[1].message))

    def test_predicted_contact_restraint_handler(self):
        """Test PredictedContactRestraintHandler"""
        fh = StringIO("""
loop_
_ihm_predicted_contact_restraint.id
_ihm_predicted_contact_restraint.group_id
_ihm_predicted_contact_restraint.entity_id_1
_ihm_predicted_contact_restraint.asym_id_1
_ihm_predicted_contact_restraint.comp_id_1
_ihm_predicted_contact_restraint.seq_id_1
_ihm_predicted_contact_restraint.rep_atom_1
_ihm_predicted_contact_restraint.entity_id_2
_ihm_predicted_contact_restraint.asym_id_2
_ihm_predicted_contact_restraint.comp_id_2
_ihm_predicted_contact_restraint.seq_id_2
_ihm_predicted_contact_restraint.rep_atom_2
_ihm_predicted_contact_restraint.restraint_type
_ihm_predicted_contact_restraint.distance_lower_limit
_ihm_predicted_contact_restraint.distance_upper_limit
_ihm_predicted_contact_restraint.probability
_ihm_predicted_contact_restraint.model_granularity
_ihm_predicted_contact_restraint.dataset_list_id
_ihm_predicted_contact_restraint.software_id
1 . 1 A ALA 1 . 2 B TRP 2 . 'lower bound' 25.000 . 0.800 by-residue 97 34
2 1 1 A ALA 1 CA 2 B TRP 2 CB 'lower bound' 25.000 . 0.400 by-residue 97 .
3 1 1 A ALA 1 . 2 B TRP 2 . 'upper bound' . 14.000 0.600 by-feature 97 .
""")
        s, = ihm.reader.read(fh)
        r1, r2, r3 = s.restraints
        rg1, = s.restraint_groups
        self.assertEqual([r for r in rg1], [r2, r3])
        self.assertEqual(r1.dataset._id, '97')
        self.assertIsInstance(r1.resatom1, ihm.Residue)
        self.assertEqual(r1.resatom1.seq_id, 1)
        self.assertEqual(r1.resatom1.asym._id, 'A')
        self.assertIsInstance(r1.resatom2, ihm.Residue)
        self.assertEqual(r1.resatom2.seq_id, 2)
        self.assertEqual(r1.resatom2.asym._id, 'B')
        self.assertIsInstance(r1.distance,
                              ihm.restraint.LowerBoundDistanceRestraint)
        self.assertAlmostEqual(r1.distance.distance, 25.000, delta=0.1)
        self.assertAlmostEqual(r1.probability, 0.8000, delta=0.1)
        self.assertEqual(r1.by_residue, True)
        self.assertEqual(r1.software._id, '34')

        self.assertIsInstance(r2.resatom1, ihm.Atom)
        self.assertEqual(r2.resatom1.seq_id, 1)
        self.assertEqual(r2.resatom1.asym._id, 'A')
        self.assertEqual(r2.resatom1.id, 'CA')
        self.assertIsInstance(r2.resatom2, ihm.Atom)
        self.assertEqual(r2.resatom2.seq_id, 2)
        self.assertEqual(r2.resatom2.asym._id, 'B')
        self.assertEqual(r2.resatom2.id, 'CB')
        self.assertIsInstance(r3.distance,
                              ihm.restraint.UpperBoundDistanceRestraint)
        self.assertAlmostEqual(r3.distance.distance, 14.000, delta=0.1)
        self.assertIsNone(r3.software)

    def get_starting_model_coord(self):
        return """
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
1 ATOM 1 N N TYR 1 A 7 8.436 112.871 97.789 . 1
1 HETATM 2 C CA TYR 1 B . 7.951 111.565 97.289 91.820 2
"""

    def test_starting_model_coord_handler(self):
        """Test StartingModelCoordHandler"""
        fh = StringIO(self.get_starting_model_coord())
        s, = ihm.reader.read(fh)
        sm, = s.orphan_starting_models
        a1, a2 = sm._atoms
        self.assertEqual(a1.asym_unit._id, 'A')
        self.assertEqual(a1.seq_id, 7)
        self.assertEqual(a1.atom_id, 'N')
        self.assertEqual(a1.type_symbol, 'N')
        self.assertAlmostEqual(a1.x, 8.436, delta=0.01)
        self.assertAlmostEqual(a1.y, 112.871, delta=0.01)
        self.assertAlmostEqual(a1.z, 97.789, delta=0.01)
        self.assertEqual(a1.het, False)
        self.assertIsNone(a1.biso)

        self.assertEqual(a2.asym_unit._id, 'B')
        self.assertIsNone(a2.seq_id)
        self.assertEqual(a2.atom_id, 'CA')
        self.assertEqual(a2.type_symbol, 'C')
        self.assertEqual(a2.het, True)
        self.assertAlmostEqual(a2.biso, 91.820, delta=0.1)

    def test_starting_model_coord_ignored(self):
        """Test read, ignoring starting model coordinates"""
        fh = StringIO(self.get_starting_model_coord())
        s, = ihm.reader.read(fh, read_starting_model_coord=False)
        self.assertEqual(len(s.orphan_starting_models), 0)

    def test_starting_model_seq_dif_handler(self):
        """Test StartingModelSeqDifHandler"""
        fh = StringIO("""
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
1 7 G 11 LEU 9 D 12 MSE 'Mutation of MSE to LEU'
2 7 G 17 LEU 9 D 18 MSE 'Mutation of MSE to LEU'
""")
        s, = ihm.reader.read(fh)
        sm, = s.orphan_starting_models
        sd1, sd2 = sm._seq_difs
        self.assertEqual(sd1.seq_id, 11)
        self.assertEqual(sd1.db_seq_id, 12)
        self.assertEqual(sd1.db_comp_id, "MSE")
        self.assertEqual(sd1.details, "Mutation of MSE to LEU")

    def test_flr_experiment_handler(self):
        """Test FLRExperimentHandler"""
        fh = StringIO("""
loop_
_flr_experiment.ordinal_id
_flr_experiment.id
_flr_experiment.instrument_id
_flr_experiment.inst_setting_id
_flr_experiment.exp_condition_id
_flr_experiment.sample_id
_flr_experiment.details
1 1 1 12 22 42 "exp 1"
2 1 1 2 2 2 .
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        experiment, = list(flr._collection_flr_experiment.values())
        self.assertIsInstance(experiment, ihm.flr.Experiment)
        self.assertIsInstance(experiment.instrument_list[0],
                              ihm.flr.Instrument)
        self.assertIsInstance(experiment.inst_setting_list[0],
                              ihm.flr.InstSetting)
        self.assertIsInstance(experiment.exp_condition_list[0],
                              ihm.flr.ExpCondition)
        self.assertIsInstance(experiment.sample_list[0], ihm.flr.Sample)
        self.assertEqual([i._id for i in experiment.instrument_list],
                         ['1', '1'])
        self.assertEqual([i._id for i in experiment.inst_setting_list],
                         ['12', '2'])
        self.assertEqual([i._id for i in experiment.exp_condition_list],
                         ['22', '2'])
        self.assertEqual([i._id for i in experiment.sample_list],
                         ['42', '2'])
        self.assertEqual(experiment.details_list, ["exp 1", None])

    def test_flr_inst_setting_handler(self):
        """Test FLRInstSettingHandler"""
        fh = StringIO("""
loop_
_flr_inst_setting.id
_flr_inst_setting.details
1 My_Inst_setting_1
2 .
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_inst_setting.keys()),
                         ['1', '2'])
        is1 = flr._collection_flr_inst_setting['1']
        self.assertIsInstance(is1, ihm.flr.InstSetting)
        self.assertEqual(is1.details, 'My_Inst_setting_1')
        is2 = flr._collection_flr_inst_setting['2']
        self.assertIsInstance(is2, ihm.flr.InstSetting)
        self.assertIsNone(is2.details)

    def test_flr_exp_condition_handler(self):
        """Test FLRExpConditionHandler"""
        fh = StringIO("""
loop_
_flr_exp_condition.id
_flr_exp_condition.details
1 My_Exp_condition_1
2 .
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_exp_condition.keys()),
                         ['1', '2'])
        ec1 = flr._collection_flr_exp_condition['1']
        self.assertIsInstance(ec1, ihm.flr.ExpCondition)
        self.assertEqual(ec1.details, 'My_Exp_condition_1')
        ec2 = flr._collection_flr_exp_condition['2']
        self.assertIsInstance(ec2, ihm.flr.ExpCondition)
        self.assertIsNone(ec2.details)

    def test_flr_instrument_handler(self):
        """Test FLRInstrumentHandler"""
        fh = StringIO("""
loop_
_flr_instrument.id
_flr_instrument.details
1 test
2 .
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_instrument.keys()),
                         ['1', '2'])
        i1 = flr._collection_flr_instrument['1']
        self.assertIsInstance(i1, ihm.flr.Instrument)
        self.assertEqual(i1.details, 'test')
        i2 = flr._collection_flr_instrument['2']
        self.assertIsInstance(i2, ihm.flr.Instrument)
        self.assertIsNone(i2.details)

    def test_flr_entity_assembly_handler(self):
        """Test FLREntityAssemblyHandler"""
        fh = StringIO("""
loop_
_flr_entity_assembly.ordinal_id
_flr_entity_assembly.assembly_id
_flr_entity_assembly.entity_id
_flr_entity_assembly.num_copies
_flr_entity_assembly.entity_description
1 1 1 1 Entity_1
2 1 2 4 Entity_2
""")
        s, = ihm.reader.read(fh)
        self.assertEqual(len(s.entities), 2)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_entity_assembly.keys()),
                         ['1'])
        a1 = flr._collection_flr_entity_assembly['1']
        self.assertIsInstance(a1, ihm.flr.EntityAssembly)
        self.assertEqual([x._id for x in a1.entity_list], ['1', '2'])
        self.assertEqual(a1.num_copies_list, [1, 4])

    def test_flr_sample_condition_handler(self):
        """Test FLRSampleConditionHandler"""
        fh = StringIO("""
loop_
_flr_sample_condition.id
_flr_sample_condition.details
1 test
2 .
#
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_sample_condition.keys()),
                         ['1', '2'])
        s1 = flr._collection_flr_sample_condition['1']
        self.assertIsInstance(s1, ihm.flr.SampleCondition)
        self.assertEqual(s1.details, 'test')
        s2 = flr._collection_flr_sample_condition['2']
        self.assertIsInstance(s2, ihm.flr.SampleCondition)
        self.assertIsNone(s2.details)

    def test_flr_sample_handler(self):
        """Test FLRSampleHandler"""
        fh = StringIO("""
loop_
_flr_sample.id
_flr_sample.entity_assembly_id
_flr_sample.num_of_probes
_flr_sample.sample_condition_id
_flr_sample.sample_description
_flr_sample.sample_details
_flr_sample.solvent_phase
1 1 2 1 Sample_1 'Details sample 1' liquid
2 1 4 2 Sample_2 'Details sample 2' vitrified
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_sample.keys()),
                         ['1', '2'])
        s1 = flr._collection_flr_sample['1']
        self.assertIsInstance(s1, ihm.flr.Sample)
        self.assertEqual(s1.entity_assembly._id, '1')
        self.assertEqual(s1.num_of_probes, 2)
        self.assertEqual(s1.condition._id, '1')
        self.assertEqual(s1.description, 'Sample_1')
        self.assertEqual(s1.details, 'Details sample 1')
        self.assertEqual(s1.solvent_phase, 'liquid')

    def test_flr_probe_list_handler(self):
        """Test FLRProbeListHandler"""
        fh = StringIO("""
loop_
_flr_probe_list.probe_id
_flr_probe_list.chromophore_name
_flr_probe_list.reactive_probe_flag
_flr_probe_list.reactive_probe_name
_flr_probe_list.probe_origin
_flr_probe_list.probe_link_type
1 Donor1 NO . extrinsic covalent
2 Acceptor2 YES 'Acceptor1 reactive' extrinsic covalent
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_probe.keys()),
                         ['1', '2'])
        p1 = flr._collection_flr_probe['1'].probe_list_entry
        self.assertIsInstance(p1, ihm.flr.ProbeList)
        self.assertEqual(p1.chromophore_name, 'Donor1')
        self.assertEqual(p1.reactive_probe_flag, False)
        self.assertIsNone(p1.reactive_probe_name)
        self.assertEqual(p1.probe_origin, 'extrinsic')
        self.assertEqual(p1.probe_link_type, 'covalent')
        p2 = flr._collection_flr_probe['2'].probe_list_entry
        self.assertIsInstance(p2, ihm.flr.ProbeList)
        self.assertEqual(p2.chromophore_name, 'Acceptor2')
        self.assertEqual(p2.reactive_probe_flag, True)
        self.assertEqual(p2.reactive_probe_name, 'Acceptor1 reactive')
        self.assertEqual(p2.probe_origin, 'extrinsic')
        self.assertEqual(p2.probe_link_type, 'covalent')

    def test_flr_probe_descriptor_handler(self):
        """Test FLRProbeDescriptorHandler"""
        fh = StringIO("""
loop_
_flr_probe_descriptor.probe_id
_flr_probe_descriptor.reactive_probe_chem_descriptor_id
_flr_probe_descriptor.chromophore_chem_descriptor_id
_flr_probe_descriptor.chromophore_center_atom
1 . 1 CB
2 3 2 CB
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_probe.keys()),
                         ['1', '2'])
        p1 = flr._collection_flr_probe['1'].probe_descriptor
        self.assertIsInstance(p1, ihm.flr.ProbeDescriptor)
        self.assertIsNone(p1.reactive_probe_chem_descriptor)
        self.assertIsInstance(p1.chromophore_chem_descriptor,
                              ihm.ChemDescriptor)
        self.assertEqual(p1.chromophore_chem_descriptor._id, '1')
        self.assertEqual(p1.chromophore_center_atom, 'CB')

        p2 = flr._collection_flr_probe['2'].probe_descriptor
        self.assertIsInstance(p2, ihm.flr.ProbeDescriptor)
        self.assertIsInstance(p2.reactive_probe_chem_descriptor,
                              ihm.ChemDescriptor)
        self.assertEqual(p2.reactive_probe_chem_descriptor._id, '3')
        self.assertIsInstance(p2.chromophore_chem_descriptor,
                              ihm.ChemDescriptor)
        self.assertEqual(p2.chromophore_chem_descriptor._id, '2')
        self.assertEqual(p2.chromophore_center_atom, 'CB')

    def test_flr_sample_probe_details_handler(self):
        """Test FLRSampleProbeDetailsHandler"""
        fh = StringIO("""
loop_
_flr_sample_probe_details.sample_probe_id
_flr_sample_probe_details.sample_id
_flr_sample_probe_details.probe_id
_flr_sample_probe_details.fluorophore_type
_flr_sample_probe_details.description
_flr_sample_probe_details.poly_probe_position_id
1 42 99 donor 'Donor in position1-position3' 34
2 1 2 acceptor 'Acceptor in position1-position3' 2
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(
            sorted(flr._collection_flr_sample_probe_details.keys()),
            ['1', '2'])
        p1 = flr._collection_flr_sample_probe_details['1']
        self.assertIsInstance(p1, ihm.flr.SampleProbeDetails)
        self.assertIsInstance(p1.sample, ihm.flr.Sample)
        self.assertEqual(p1.sample._id, '42')
        self.assertIsInstance(p1.probe, ihm.flr.Probe)
        self.assertEqual(p1.probe._id, '99')
        self.assertEqual(p1.fluorophore_type, 'donor')
        self.assertEqual(p1.description, 'Donor in position1-position3')
        self.assertIsInstance(p1.poly_probe_position,
                              ihm.flr.PolyProbePosition)
        self.assertEqual(p1.poly_probe_position._id, '34')

    def test_flr_poly_probe_position_handler(self):
        """Test FLRPolyProbePositionHandler"""
        fh = StringIO("""
loop_
_flr_poly_probe_position.id
_flr_poly_probe_position.entity_id
_flr_poly_probe_position.entity_description
_flr_poly_probe_position.asym_id
_flr_poly_probe_position.seq_id
_flr_poly_probe_position.comp_id
_flr_poly_probe_position.atom_id
_flr_poly_probe_position.mutation_flag
_flr_poly_probe_position.modification_flag
_flr_poly_probe_position.auth_name
1 1 Entity_1 . 1 ALA . NO YES Position_1
2 2 Entity_2 C 10 CYS CB NO YES Position_3
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(len(s.entities), 2)
        self.assertEqual(
            sorted(flr._collection_flr_poly_probe_position.keys()), ['1', '2'])
        p1 = flr._collection_flr_poly_probe_position['1']
        self.assertIsInstance(p1, ihm.flr.PolyProbePosition)
        self.assertIsInstance(p1.resatom, ihm.Residue)
        self.assertEqual(p1.resatom.seq_id, 1)
        self.assertEqual(p1.resatom.entity._id, '1')
        self.assertIsNone(p1.resatom.asym)

        p2 = flr._collection_flr_poly_probe_position['2']
        self.assertIsInstance(p2, ihm.flr.PolyProbePosition)
        self.assertIsInstance(p2.resatom, ihm.Atom)
        self.assertIsInstance(p2.resatom.asym, ihm.AsymUnit)
        self.assertEqual(p2.resatom.id, 'CB')
        self.assertEqual(p2.resatom.seq_id, 10)
        self.assertEqual(p2.resatom.entity._id, '2')
        self.assertEqual(p2.resatom.asym.entity._id, '2')
        self.assertEqual(p2.resatom.asym.id, 'C')

    def test_flr_poly_probe_position_modified_handler(self):
        """Test FLRPolyProbePositionModifiedHandler"""
        fh = StringIO("""
loop_
_flr_poly_probe_position_modified.id
_flr_poly_probe_position_modified.chem_descriptor_id
_flr_poly_probe_position_modified.atom_id
1 4 .
2 4 CB
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(
            sorted(flr._collection_flr_poly_probe_position.keys()), ['1', '2'])
        p1 = flr._collection_flr_poly_probe_position['1']
        self.assertIsInstance(p1.modified_chem_descriptor,
                              ihm.ChemDescriptor)
        self.assertEqual(p1.modified_chem_descriptor._id, '4')

        p2 = flr._collection_flr_poly_probe_position['2']
        self.assertIsInstance(p2.modified_chem_descriptor,
                              ihm.ChemDescriptor)
        self.assertEqual(p2.modified_chem_descriptor._id, '4')

    def test_flr_poly_probe_position_mutated_handler(self):
        """Test FLRPolyProbePositionMutatedHandler"""
        fh = StringIO("""
loop_
_flr_poly_probe_position_mutated.id
_flr_poly_probe_position_mutated.chem_comp_id
_flr_poly_probe_position_mutated.atom_id
1 Ala .
2 Cys CB
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(
            sorted(flr._collection_flr_poly_probe_position.keys()), ['1', '2'])
        p1 = flr._collection_flr_poly_probe_position['1']
        self.assertIsInstance(p1.mutated_chem_comp_id,
                              ihm.ChemComp)
        self.assertEqual(p1.mutated_chem_comp_id.id, 'Ala')

        p2 = flr._collection_flr_poly_probe_position['2']
        self.assertIsInstance(p2.mutated_chem_comp_id,
                              ihm.ChemComp)
        self.assertEqual(p2.mutated_chem_comp_id.id, 'Cys')

    def test_flr_poly_probe_conjugate_handler(self):
        """Test FLRPolyProbeConjugateHandler"""
        fh = StringIO("""
loop_
_flr_poly_probe_conjugate.id
_flr_poly_probe_conjugate.sample_probe_id
_flr_poly_probe_conjugate.chem_descriptor_id
_flr_poly_probe_conjugate.ambiguous_stoichiometry_flag
_flr_poly_probe_conjugate.probe_stoichiometry
1 1 5 NO .
2 2 5 YES 2
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(
            sorted(flr._collection_flr_poly_probe_conjugate.keys()),
            ['1', '2'])
        p1, p2 = s.flr_data[0].poly_probe_conjugates
        self.assertIsInstance(p1.sample_probe,
                              ihm.flr.SampleProbeDetails)
        self.assertEqual(p1.sample_probe._id, '1')
        self.assertIsInstance(p1.chem_descriptor, ihm.ChemDescriptor)
        self.assertEqual(p1.chem_descriptor._id, '5')
        self.assertEqual(p1.ambiguous_stoichiometry, False)
        self.assertIsNone(p1.probe_stoichiometry)

        self.assertIsInstance(p2.sample_probe,
                              ihm.flr.SampleProbeDetails)
        self.assertEqual(p2.sample_probe._id, '2')
        self.assertIsInstance(p2.chem_descriptor, ihm.ChemDescriptor)
        self.assertEqual(p2.chem_descriptor._id, '5')
        self.assertEqual(p2.ambiguous_stoichiometry, True)
        self.assertAlmostEqual(p2.probe_stoichiometry, 2.0, delta=1.0)

    def test_flr_fret_forster_radius_handler(self):
        """Test FLRFretForsterRadiusHandler"""
        fh = StringIO("""
loop_
_flr_fret_forster_radius.id
_flr_fret_forster_radius.donor_probe_id
_flr_fret_forster_radius.acceptor_probe_id
_flr_fret_forster_radius.forster_radius
_flr_fret_forster_radius.reduced_forster_radius
1 9 10 252.000 53.200
2 11 12 52.000 .
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(
            sorted(flr._collection_flr_fret_forster_radius.keys()), ['1', '2'])
        r1 = flr._collection_flr_fret_forster_radius['1']
        self.assertIsInstance(r1.donor_probe, ihm.flr.Probe)
        self.assertEqual(r1.donor_probe._id, '9')
        self.assertIsInstance(r1.acceptor_probe, ihm.flr.Probe)
        self.assertEqual(r1.acceptor_probe._id, '10')
        self.assertAlmostEqual(r1.forster_radius, 252.000, delta=0.1)
        self.assertAlmostEqual(r1.reduced_forster_radius, 53.200, delta=0.1)

        r2 = flr._collection_flr_fret_forster_radius['2']
        self.assertIsNone(r2.reduced_forster_radius)

    def test_flr_fret_calibration_parameters_handler(self):
        """Test FLRFretCalibrationParametersHandler"""
        fh = StringIO("""
loop_
_flr_fret_calibration_parameters.id
_flr_fret_calibration_parameters.phi_acceptor
_flr_fret_calibration_parameters.alpha
_flr_fret_calibration_parameters.alpha_sd
_flr_fret_calibration_parameters.gG_gR_ratio
_flr_fret_calibration_parameters.beta
_flr_fret_calibration_parameters.gamma
_flr_fret_calibration_parameters.delta
_flr_fret_calibration_parameters.a_b
1 0.350 2.400 0.1 0.400 1.0 2.0 3.0 0.800
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        p1 = flr._collection_flr_fret_calibration_parameters['1']
        self.assertAlmostEqual(p1.phi_acceptor, 0.350, delta=0.01)
        self.assertAlmostEqual(p1.alpha, 2.400, delta=0.1)
        self.assertAlmostEqual(p1.alpha_sd, 0.1, delta=0.1)
        self.assertAlmostEqual(p1.gg_gr_ratio, 0.4, delta=0.1)
        self.assertAlmostEqual(p1.beta, 1.0, delta=0.1)
        self.assertAlmostEqual(p1.gamma, 2.0, delta=0.1)
        self.assertAlmostEqual(p1.delta, 3.0, delta=0.1)
        self.assertAlmostEqual(p1.a_b, 0.8, delta=0.1)

    def test_flr_fret_analysis_handler(self):
        """Test FLRFretAnalysisHandler"""
        fh = StringIO("""
loop_
_flr_fret_analysis.id
_flr_fret_analysis.experiment_id
_flr_fret_analysis.type
_flr_fret_analysis.sample_probe_id_1
_flr_fret_analysis.sample_probe_id_2
_flr_fret_analysis.forster_radius_id
_flr_fret_analysis.dataset_list_id
_flr_fret_analysis.external_file_id
_flr_fret_analysis.software_id
1 8 intensity-based 9 2 11 18 42 99
2 13 lifetime-based 24 5 19 32 81 98
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        a = flr._collection_flr_fret_analysis['1']
        self.assertIsInstance(a.experiment, ihm.flr.Experiment)
        self.assertEqual(a.experiment._id, '8')
        self.assertIsInstance(a.sample_probe_1, ihm.flr.SampleProbeDetails)
        self.assertEqual(a.sample_probe_1._id, '9')
        self.assertIsInstance(a.sample_probe_2, ihm.flr.SampleProbeDetails)
        self.assertEqual(a.sample_probe_2._id, '2')
        self.assertIsInstance(a.forster_radius, ihm.flr.FRETForsterRadius)
        self.assertEqual(a.forster_radius._id, '11')
        self.assertEqual(a.type, 'intensity-based')
        self.assertIsInstance(a.dataset, ihm.dataset.Dataset)
        self.assertEqual(a.dataset._id, '18')
        self.assertIsInstance(a.external_file, ihm.location.Location)
        self.assertEqual(a.external_file._id, '42')
        self.assertIsInstance(a.software, ihm.Software)
        self.assertEqual(a.software._id, '99')
        b = flr._collection_flr_fret_analysis['2']
        self.assertIsInstance(b.experiment, ihm.flr.Experiment)
        self.assertEqual(b.experiment._id, '13')
        self.assertIsInstance(b.sample_probe_1, ihm.flr.SampleProbeDetails)
        self.assertEqual(b.sample_probe_1._id, '24')
        self.assertIsInstance(b.sample_probe_2, ihm.flr.SampleProbeDetails)
        self.assertEqual(b.sample_probe_2._id, '5')
        self.assertIsInstance(b.forster_radius, ihm.flr.FRETForsterRadius)
        self.assertEqual(b.forster_radius._id, '19')
        self.assertEqual(b.type, 'lifetime-based')
        self.assertIsInstance(b.dataset, ihm.dataset.Dataset)
        self.assertEqual(b.dataset._id, '32')
        self.assertIsInstance(b.external_file, ihm.location.Location)
        self.assertEqual(b.external_file._id, '81')
        self.assertIsInstance(b.software, ihm.Software)
        self.assertEqual(b.software._id, '98')

    def test_flr_fret_analysis_intensity_handler(self):
        """Test FLRFretAnalysisIntensityHandler"""
        fh = StringIO("""
loop_
_flr_fret_analysis_intensity.ordinal_id
_flr_fret_analysis_intensity.analysis_id
_flr_fret_analysis_intensity.calibration_parameters_id
_flr_fret_analysis_intensity.donor_only_fraction
_flr_fret_analysis_intensity.chi_square_reduced
_flr_fret_analysis_intensity.method_name
_flr_fret_analysis_intensity.details
2 5 3 0.200 1.400 PDA Details
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        a = flr._collection_flr_fret_analysis['5']
        self.assertEqual(a.type, 'intensity-based')
        self.assertIsInstance(a.calibration_parameters,
                              ihm.flr.FRETCalibrationParameters)
        self.assertEqual(a.calibration_parameters._id, '3')
        self.assertAlmostEqual(a.donor_only_fraction, 0.2, delta=0.1)
        self.assertAlmostEqual(a.chi_square_reduced, 1.4, delta=0.1)
        self.assertEqual(a.method_name, 'PDA')
        self.assertEqual(a.details, 'Details')

    def test_flr_fret_analysis_lifetime_handler(self):
        """Test FLRFretAnalysisLifetimeHandler"""
        fh = StringIO("""
loop_
_flr_fret_analysis_lifetime.ordinal_id
_flr_fret_analysis_lifetime.analysis_id
_flr_fret_analysis_lifetime.reference_measurement_group_id
_flr_fret_analysis_lifetime.lifetime_fit_model_id
_flr_fret_analysis_lifetime.donor_only_fraction
_flr_fret_analysis_lifetime.chi_square_reduced
_flr_fret_analysis_lifetime.method_name
_flr_fret_analysis_lifetime.details
4 2 19 23 0.300 1.500 'Lifetime fit' 'Details on lifetime fit'
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        a = flr._collection_flr_fret_analysis['2']
        self.assertEqual(a.type, 'lifetime-based')
        self.assertIsInstance(a.ref_measurement_group,
                              ihm.flr.RefMeasurementGroup)
        self.assertEqual(a.ref_measurement_group._id, '19')
        self.assertIsInstance(a.lifetime_fit_model, ihm.flr.LifetimeFitModel)
        self.assertEqual(a.lifetime_fit_model._id, '23')
        self.assertAlmostEqual(a.donor_only_fraction, 0.3, delta=0.1)
        self.assertAlmostEqual(a.chi_square_reduced, 1.5, delta=0.1)
        self.assertEqual(a.method_name, 'Lifetime fit')
        self.assertEqual(a.details, 'Details on lifetime fit')

    def test_flr_lifetime_fit_model_handler(self):
        """Test FLRLifetimeFitModelHandler"""
        fh = StringIO("""
loop_
_flr_lifetime_fit_model.id
_flr_lifetime_fit_model.name
_flr_lifetime_fit_model.description
_flr_lifetime_fit_model.external_file_id
_flr_lifetime_fit_model.citation_id
1 'FitModel 15' 'Description of the fit model' 3 8
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        f = flr._collection_flr_lifetime_fit_model['1']
        self.assertEqual(f.name, 'FitModel 15')
        self.assertEqual(f.description, 'Description of the fit model')
        self.assertIsInstance(f.external_file, ihm.location.Location)
        self.assertEqual(f.external_file._id, '3')
        self.assertIsInstance(f.citation, ihm.Citation)
        self.assertEqual(f.citation._id, '8')

    def test_flr_ref_measurement_handler(self):
        """Test FLRRefMeasurementHandler"""
        fh = StringIO("""
loop_
_flr_reference_measurement.id
_flr_reference_measurement.reference_sample_probe_id
_flr_reference_measurement.num_species
_flr_reference_measurement.details
4 9 2 Details1
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        r = flr._collection_flr_ref_measurement['4']
        self.assertIsInstance(r.ref_sample_probe, ihm.flr.SampleProbeDetails)
        self.assertEqual(r.ref_sample_probe._id, '9')
        self.assertEqual(r.details, 'Details1')
        # num_species is set automatically when adding lifetimes to the object
        self.assertEqual(r.num_species, 0)
        r.add_lifetime('1')
        r.add_lifetime('2')
        self.assertEqual(r.num_species, 2)

    def test_flr_ref_measurement_group_handler(self):
        """Test FLRRefMeasurementGroupHandler"""
        fh = StringIO("""
loop_
_flr_reference_measurement_group.id
_flr_reference_measurement_group.num_measurements
_flr_reference_measurement_group.details
5 3 Details
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        r = flr._collection_flr_ref_measurement_group['5']
        self.assertEqual(r.details, 'Details')
        # num_measurements is set automatically when adding measurements
        # to the object
        self.assertEqual(r.num_measurements, 0)
        r.add_ref_measurement('1')
        self.assertEqual(r.num_measurements, 1)
        r.add_ref_measurement('2')
        self.assertEqual(r.num_measurements, 2)

    def test_flr_ref_measurement_group_link_handler(self):
        """Test FLRRefMeasurementGroupLinkHandler"""
        fh = StringIO("""
loop_
_flr_reference_measurement_group_link.group_id
_flr_reference_measurement_group_link.reference_measurement_id
3 12
3 25
5 19
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        g1 = flr._collection_flr_ref_measurement_group['3']
        self.assertEqual(g1.num_measurements, 2)
        self.assertIsInstance(g1.ref_measurement_list[0],
                              ihm.flr.RefMeasurement)
        self.assertEqual(g1.ref_measurement_list[0]._id, '12')
        self.assertIsInstance(g1.ref_measurement_list[1],
                              ihm.flr.RefMeasurement)
        self.assertEqual(g1.ref_measurement_list[1]._id, '25')
        g2 = flr._collection_flr_ref_measurement_group['5']
        self.assertEqual(g2.num_measurements, 1)
        self.assertIsInstance(g2.ref_measurement_list[0],
                              ihm.flr.RefMeasurement)
        self.assertEqual(g2.ref_measurement_list[0]._id, '19')

    def test_flr_ref_measurement_lifetime_handler(self):
        """Test FLRRefMeasurementLifetimeHandler"""
        fh = StringIO("""
loop_
_flr_reference_measurement_lifetime.ordinal_id
_flr_reference_measurement_lifetime.reference_measurement_id
_flr_reference_measurement_lifetime.species_name
_flr_reference_measurement_lifetime.species_fraction
_flr_reference_measurement_lifetime.lifetime
1 15 species1 0.300 4.100
2 15 species2 0.700 2.100
3 12 species1 1.000 3.800
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        # Check the lifetime objects themselves
        f1 = flr._collection_flr_ref_measurement_lifetime['1']
        self.assertEqual(f1.species_name, 'species1')
        self.assertAlmostEqual(f1.species_fraction, 0.3, delta=0.1)
        self.assertAlmostEqual(f1.lifetime, 4.1, delta=0.1)
        f2 = flr._collection_flr_ref_measurement_lifetime['2']
        self.assertEqual(f2.species_name, 'species2')
        self.assertAlmostEqual(f2.species_fraction, 0.7, delta=0.1)
        self.assertAlmostEqual(f2.lifetime, 2.1, delta=0.1)
        f3 = flr._collection_flr_ref_measurement_lifetime['3']
        self.assertEqual(f3.species_name, 'species1')
        self.assertAlmostEqual(f3.species_fraction, 1.0, delta=0.1)
        self.assertAlmostEqual(f3.lifetime, 3.8, delta=0.1)
        # And check the respective reference measurement objects
        r1 = flr._collection_flr_ref_measurement['15']
        self.assertIsInstance(r1.list_of_lifetimes[0],
                              ihm.flr.RefMeasurementLifetime)
        self.assertEqual(r1.list_of_lifetimes[0].species_name, 'species1')
        self.assertAlmostEqual(r1.list_of_lifetimes[0].species_fraction,
                               0.3, delta=0.1)
        self.assertAlmostEqual(r1.list_of_lifetimes[0].lifetime, 4.1,
                               delta=0.1)
        self.assertIsInstance(r1.list_of_lifetimes[1],
                              ihm.flr.RefMeasurementLifetime)
        self.assertEqual(r1.list_of_lifetimes[1].species_name, 'species2')
        self.assertAlmostEqual(r1.list_of_lifetimes[1].species_fraction,
                               0.7, delta=0.1)
        self.assertAlmostEqual(r1.list_of_lifetimes[1].lifetime, 2.1,
                               delta=0.1)
        r2 = flr._collection_flr_ref_measurement['12']
        self.assertIsInstance(r2.list_of_lifetimes[0],
                              ihm.flr.RefMeasurementLifetime)
        self.assertEqual(r2.list_of_lifetimes[0].species_name, 'species1')
        self.assertAlmostEqual(r2.list_of_lifetimes[0].species_fraction,
                               1.0, delta=0.1)
        self.assertAlmostEqual(r2.list_of_lifetimes[0].lifetime, 3.8,
                               delta=0.1)

    def test_flr_peak_assignment_handler(self):
        """Test FLRPeakAssignmentHandler"""
        fh = StringIO("""
loop_
_flr_peak_assignment.id
_flr_peak_assignment.method_name
_flr_peak_assignment.details
1 Population 'Test details'
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        a = flr._collection_flr_peak_assignment['1']
        self.assertEqual(a.method_name, 'Population')
        self.assertEqual(a.details, 'Test details')

    def test_flr_fret_distance_restraint_handler(self):
        """Test FLRFretDistanceRestraintHandler"""
        fh = StringIO("""
loop_
_flr_fret_distance_restraint.ordinal_id
_flr_fret_distance_restraint.id
_flr_fret_distance_restraint.group_id
_flr_fret_distance_restraint.sample_probe_id_1
_flr_fret_distance_restraint.sample_probe_id_2
_flr_fret_distance_restraint.state_id
_flr_fret_distance_restraint.analysis_id
_flr_fret_distance_restraint.distance
_flr_fret_distance_restraint.distance_error_plus
_flr_fret_distance_restraint.distance_error_minus
_flr_fret_distance_restraint.distance_type
_flr_fret_distance_restraint.population_fraction
_flr_fret_distance_restraint.peak_assignment_id
1 1 1 1 2 9 19 53.500 2.500 2.300 <R_DA>_E 0.800 42
2 2 1 3 4 8 18 49.000 2.000 2.100 <R_DA>_E 0.800 42
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(
            sorted(flr._collection_flr_fret_distance_restraint_group.keys()),
            ['1'])
        self.assertEqual(
            sorted(flr._collection_flr_fret_distance_restraint.keys()),
            ['1', '2'])
        rg1, = flr.distance_restraint_groups
        r1, r2 = rg1.get_info()
        self.assertIsInstance(r1.sample_probe_1, ihm.flr.SampleProbeDetails)
        self.assertEqual(r1.sample_probe_1._id, '1')
        self.assertIsInstance(r1.sample_probe_2, ihm.flr.SampleProbeDetails)
        self.assertEqual(r1.sample_probe_2._id, '2')
        self.assertIsInstance(r1.state, ihm.model.State)
        self.assertEqual(r1.state._id, '9')
        self.assertIsInstance(r1.analysis, ihm.flr.FRETAnalysis)
        self.assertEqual(r1.analysis._id, '19')
        self.assertAlmostEqual(r1.distance, 53.500, delta=0.1)
        self.assertAlmostEqual(r1.distance_error_plus, 2.500, delta=0.1)
        self.assertAlmostEqual(r1.distance_error_minus, 2.300, delta=0.1)
        self.assertEqual(r1.distance_type, "<R_DA>_E")
        self.assertAlmostEqual(r1.population_fraction, 0.800, delta=0.1)
        self.assertIsInstance(r1.peak_assignment, ihm.flr.PeakAssignment)
        self.assertEqual(r1.peak_assignment._id, '42')

        self.assertEqual(rg1.distance_restraint_list, [r1, r2])

    def test_flr_fret_model_quality_handler(self):
        """Test FLRFretModelQualityHandler"""
        fh = StringIO("""
loop_
_flr_fret_model_quality.model_id
_flr_fret_model_quality.chi_square_reduced
_flr_fret_model_quality.dataset_group_id
_flr_fret_model_quality.method
_flr_fret_model_quality.details
1 1.300 42 foo bar
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_fret_model_quality.keys()),
                         ['1'])
        q1, = flr.fret_model_qualities
        self.assertIsInstance(q1.model, ihm.model.Model)
        self.assertEqual(q1.model._id, '1')
        self.assertAlmostEqual(q1.chi_square_reduced, 1.300, delta=0.1)
        self.assertIsInstance(q1.dataset_group, ihm.dataset.DatasetGroup)
        self.assertEqual(q1.dataset_group._id, '42')
        self.assertEqual(q1.method, 'foo')
        self.assertEqual(q1.details, 'bar')

    def test_flr_fret_model_distance_handler(self):
        """Test FLRFretModelDistanceHandler"""
        fh = StringIO("""
loop_
_flr_fret_model_distance.id
_flr_fret_model_distance.restraint_id
_flr_fret_model_distance.model_id
_flr_fret_model_distance.distance
_flr_fret_model_distance.distance_deviation
1 42 34 52.000 1.500
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(
            sorted(flr._collection_flr_fret_model_distance.keys()), ['1'])
        d1, = flr.fret_model_distances
        self.assertIsInstance(d1.restraint, ihm.flr.FRETDistanceRestraint)
        self.assertEqual(d1.restraint._id, '42')
        self.assertIsInstance(d1.model, ihm.model.Model)
        self.assertEqual(d1.model._id, '34')
        self.assertAlmostEqual(d1.distance, 52.000, delta=0.1)
        self.assertAlmostEqual(d1.distance_deviation, 1.500, delta=0.1)

    def test_flr_fps_global_parameter_handler(self):
        """Test FLRFPSGlobalParameterHandler"""
        fh = StringIO("""
loop_
_flr_FPS_global_parameter.id
_flr_FPS_global_parameter.forster_radius_value
_flr_FPS_global_parameter.conversion_function_polynom_order
_flr_FPS_global_parameter.repetition
_flr_FPS_global_parameter.AV_grid_rel
_flr_FPS_global_parameter.AV_min_grid_A
_flr_FPS_global_parameter.AV_allowed_sphere
_flr_FPS_global_parameter.AV_search_nodes
_flr_FPS_global_parameter.AV_E_samples_k
_flr_FPS_global_parameter.sim_viscosity_adjustment
_flr_FPS_global_parameter.sim_dt_adjustment
_flr_FPS_global_parameter.sim_max_iter_k
_flr_FPS_global_parameter.sim_max_force
_flr_FPS_global_parameter.sim_clash_tolerance_A
_flr_FPS_global_parameter.sim_reciprocal_kT
_flr_FPS_global_parameter.sim_clash_potential
_flr_FPS_global_parameter.convergence_E
_flr_FPS_global_parameter.convergence_K
_flr_FPS_global_parameter.convergence_F
_flr_FPS_global_parameter.convergence_T
1 52 3 1000 0.200 0.400 0.500 3 200 1 1 200 400 1 10 ^2 100 0.001 0.001 0.002
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(
            sorted(flr._collection_flr_fps_global_parameters.keys()), ['1'])
        p1 = flr._collection_flr_fps_global_parameters['1']
        self.assertAlmostEqual(p1.forster_radius, 52.000, delta=0.1)
        self.assertEqual(p1.conversion_function_polynom_order, 3)
        self.assertEqual(p1.repetition, 1000)
        self.assertAlmostEqual(p1.av_grid_rel, 0.200, delta=0.1)
        self.assertAlmostEqual(p1.av_min_grid_a, 0.400, delta=0.1)
        self.assertAlmostEqual(p1.av_allowed_sphere, 0.500, delta=0.1)
        self.assertEqual(p1.av_search_nodes, 3)
        self.assertAlmostEqual(p1.av_e_samples_k, 200, delta=0.1)
        self.assertAlmostEqual(p1.sim_viscosity_adjustment, 1, delta=0.1)
        self.assertAlmostEqual(p1.sim_dt_adjustment, 1, delta=0.1)
        self.assertEqual(p1.sim_max_iter_k, 200)
        self.assertAlmostEqual(p1.sim_max_force, 400, delta=0.1)
        self.assertAlmostEqual(p1.sim_clash_tolerance_a, 1, delta=0.1)
        self.assertAlmostEqual(p1.sim_reciprocal_kt, 10, delta=0.1)
        self.assertEqual(p1.sim_clash_potential, "^2")
        self.assertAlmostEqual(p1.convergence_e, 100, delta=0.1)
        self.assertAlmostEqual(p1.convergence_k, 0.001, delta=0.001)
        self.assertAlmostEqual(p1.convergence_f, 0.001, delta=0.001)
        self.assertAlmostEqual(p1.convergence_t, 0.002, delta=0.001)

    def test_flr_fps_modeling_handler(self):
        """Test FLRFPSModelingHandler"""
        fh = StringIO("""
loop_
_flr_FPS_modeling.id
_flr_FPS_modeling.ihm_modeling_protocol_ordinal_id
_flr_FPS_modeling.restraint_group_id
_flr_FPS_modeling.global_parameter_id
_flr_FPS_modeling.probe_modeling_method
_flr_FPS_modeling.details
1 8 9 10 AV3 "test details"
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_fps_modeling.keys()),
                         ['1'])
        m1 = flr._collection_flr_fps_modeling['1']
        self.assertIsInstance(m1.protocol, ihm.protocol.Protocol)
        self.assertEqual(m1.protocol._id, '8')
        self.assertIsInstance(m1.restraint_group,
                              ihm.flr.FRETDistanceRestraintGroup)
        self.assertEqual(m1.restraint_group._id, '9')
        self.assertIsInstance(m1.global_parameter, ihm.flr.FPSGlobalParameters)
        self.assertEqual(m1.global_parameter._id, '10')
        self.assertEqual(m1.probe_modeling_method, 'AV3')
        self.assertEqual(m1.details, 'test details')

    def test_flr_fps_av_parameter_handler(self):
        """Test FLRFPSAVParameterHandler"""
        fh = StringIO("""
loop_
_flr_FPS_AV_parameter.id
_flr_FPS_AV_parameter.num_linker_atoms
_flr_FPS_AV_parameter.linker_length
_flr_FPS_AV_parameter.linker_width
_flr_FPS_AV_parameter.probe_radius_1
_flr_FPS_AV_parameter.probe_radius_2
_flr_FPS_AV_parameter.probe_radius_3
1 15 20.000 3.500 10.000 5.000 4.000
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_fps_av_parameter.keys()),
                         ['1'])
        p = flr._collection_flr_fps_av_parameter['1']
        self.assertEqual(p.num_linker_atoms, 15)
        self.assertAlmostEqual(p.linker_length, 20.000, delta=0.1)
        self.assertAlmostEqual(p.linker_width, 3.500, delta=0.1)
        self.assertAlmostEqual(p.probe_radius_1, 10.000, delta=0.1)
        self.assertAlmostEqual(p.probe_radius_2, 5.000, delta=0.1)
        self.assertAlmostEqual(p.probe_radius_3, 4.000, delta=0.1)

    def test_flr_fps_av_modeling_handler(self):
        """Test FLRFPSAVModelingHandler"""
        fh = StringIO("""
loop_
_flr_FPS_modeling.id
_flr_FPS_modeling.ihm_modeling_protocol_ordinal_id
_flr_FPS_modeling.restraint_group_id
_flr_FPS_modeling.global_parameter_id
_flr_FPS_modeling.probe_modeling_method
_flr_FPS_modeling.details
3 4 5 6 AV3 .
#
loop_
_flr_FPS_AV_modeling.id
_flr_FPS_AV_modeling.sample_probe_id
_flr_FPS_AV_modeling.FPS_modeling_id
_flr_FPS_AV_modeling.parameter_id
1 2 3 4
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_fps_av_modeling.keys()),
                         ['1'])
        m = flr._collection_flr_fps_av_modeling['1']
        self.assertIsInstance(m.sample_probe, ihm.flr.SampleProbeDetails)
        self.assertEqual(m.sample_probe._id, '2')
        self.assertIsInstance(m.fps_modeling, ihm.flr.FPSModeling)
        self.assertEqual(m.fps_modeling._id, '3')
        self.assertIsInstance(m.parameter, ihm.flr.FPSAVParameter)
        self.assertEqual(m.parameter._id, '4')

    def test_flr_fps_mean_probe_position_handler(self):
        """Test FLRFPSMPPHandler"""
        fh = StringIO("""
loop_
_flr_FPS_mean_probe_position.id
_flr_FPS_mean_probe_position.sample_probe_id
_flr_FPS_mean_probe_position.mpp_xcoord
_flr_FPS_mean_probe_position.mpp_ycoord
_flr_FPS_mean_probe_position.mpp_zcoord
1 2 1.000 2.000 3.000
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(
            flr._collection_flr_fps_mean_probe_position.keys()), ['1'])
        p = flr._collection_flr_fps_mean_probe_position['1']
        self.assertIsInstance(p.sample_probe, ihm.flr.SampleProbeDetails)
        self.assertEqual(p.sample_probe._id, '2')
        self.assertAlmostEqual(p.x, 1.0, delta=0.1)
        self.assertAlmostEqual(p.y, 2.0, delta=0.1)
        self.assertAlmostEqual(p.z, 3.0, delta=0.1)

    def test_flr_fps_mpp_atom_position_handler(self):
        """Test FLRFPSMPPAtomPositionHandler"""
        fh = StringIO("""
loop_
_flr_FPS_MPP_atom_position.id
_flr_FPS_MPP_atom_position.entity_id
_flr_FPS_MPP_atom_position.seq_id
_flr_FPS_MPP_atom_position.comp_id
_flr_FPS_MPP_atom_position.atom_id
_flr_FPS_MPP_atom_position.asym_id
_flr_FPS_MPP_atom_position.xcoord
_flr_FPS_MPP_atom_position.ycoord
_flr_FPS_MPP_atom_position.zcoord
_flr_FPS_MPP_atom_position.group_id
1 1 4 ALA CA A 1.000 2.000 3.000 1
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(
            sorted(flr._collection_flr_fps_mpp_atom_position.keys()), ['1'])
        p = flr._collection_flr_fps_mpp_atom_position['1']
        self.assertIsInstance(p.atom, ihm.Atom)
        self.assertEqual(p.atom.id, 'CA')
        self.assertEqual(p.atom.seq_id, 4)
        self.assertEqual(p.atom.asym._id, 'A')
        self.assertAlmostEqual(p.x, 1.0, delta=0.1)
        self.assertAlmostEqual(p.y, 2.0, delta=0.1)
        self.assertAlmostEqual(p.z, 3.0, delta=0.1)

    def test_flr_fps_mpp_modeling_handler(self):
        """Test FLRFPSMPPModelingHandler"""
        fh = StringIO("""
loop_
_flr_FPS_modeling.id
_flr_FPS_modeling.ihm_modeling_protocol_ordinal_id
_flr_FPS_modeling.restraint_group_id
_flr_FPS_modeling.global_parameter_id
_flr_FPS_modeling.probe_modeling_method
_flr_FPS_modeling.details
3 4 5 6 AV3 .
#
loop_
_flr_FPS_MPP_modeling.ordinal_id
_flr_FPS_MPP_modeling.FPS_modeling_id
_flr_FPS_MPP_modeling.mpp_id
_flr_FPS_MPP_modeling.mpp_atom_position_group_id
1 3 4 5
""")
        s, = ihm.reader.read(fh)
        flr, = s.flr_data
        self.assertEqual(sorted(flr._collection_flr_fps_mpp_modeling.keys()),
                         ['1'])
        m = flr._collection_flr_fps_mpp_modeling['1']
        self.assertIsInstance(m.fps_modeling, ihm.flr.FPSModeling)
        self.assertEqual(m.fps_modeling._id, '3')
        self.assertIsInstance(m.mpp, ihm.flr.FPSMeanProbePosition)
        self.assertEqual(m.mpp._id, '4')
        self.assertIsInstance(m.mpp_atom_position_group,
                              ihm.flr.FPSMPPAtomPositionGroup)
        self.assertEqual(m.mpp_atom_position_group._id, '5')

    def test_variant_base(self):
        """Test Variant base class"""
        v = ihm.reader.Variant()
        self.assertIsNone(v.get_handlers(None))
        self.assertIsNone(v.get_audit_conform_handler(None))

    def test_write_variant(self):
        """Test write() function with Variant object"""
        cif = "data_model\n_struct.entry_id testid\n"
        for fh in cif_file_handles(cif):
            s, = ihm.reader.read(fh, variant=ihm.reader.IHMVariant())
            self.assertEqual(s.id, 'testid')


if __name__ == '__main__':
    unittest.main()
