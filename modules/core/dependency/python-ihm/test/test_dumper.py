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
import ihm.reference
import ihm.restraint
import ihm.geometry
import ihm.source
import ihm.flr
from test_format_bcif import MockFh, MockMsgPack


def _get_dumper_output(dumper, system):
    fh = StringIO()
    writer = ihm.format.CifWriter(fh)
    dumper.dump(system, writer)
    return fh.getvalue()


def _get_dumper_bcif_output(dumper, system):
    fh = MockFh()
    writer = ihm.format_bcif.BinaryCifWriter(fh)
    sys.modules['msgpack'] = MockMsgPack
    dumper.dump(system, writer)
    writer.flush()
    return fh.data


class Tests(unittest.TestCase):
    def test_write(self):
        """Test write() function"""
        sys1 = ihm.System(id='system1')
        sys2 = ihm.System(id='system 2+3')
        fh = StringIO()
        ihm.dumper.write(fh, [sys1, sys2])
        lines = fh.getvalue().split('\n')
        self.assertEqual(lines[:2], ["data_system1", "_entry.id system1"])
        self.assertEqual(lines[16:18],
                         ["data_system23", "_entry.id 'system 2+3'"])

    def test_write_custom_dumper(self):
        """Test write() function with custom dumper"""
        class MyDumper(ihm.dumper.Dumper):
            def dump(self, system, writer):
                with writer.category("_custom_category") as loc:
                    loc.write(myfield="foo", field2="bar")
        sys1 = ihm.System(id='system1')
        fh = StringIO()
        ihm.dumper.write(fh, [sys1], dumpers=[MyDumper])
        lines = fh.getvalue().split('\n')
        self.assertEqual(sorted(lines[-3:-1]),
                         ['_custom_category.field2 bar',
                          '_custom_category.myfield foo'])

    def test_dumper(self):
        """Test Dumper base class"""
        dumper = ihm.dumper.Dumper()
        dumper.finalize(None)
        dumper.dump(None, None)

    def test_prettyprint_seq(self):
        """Test _prettyprint_seq() function"""
        seq = ['x' * 30, 'y' * 20, 'z' * 10]
        # No line breaks
        self.assertEqual(list(ihm.dumper._prettyprint_seq(seq, 100)),
                         ['x' * 30 + 'y' * 20 + 'z' * 10])
        # Break inserted between sequence items
        self.assertEqual(list(ihm.dumper._prettyprint_seq(seq, 55)),
                         ['x' * 30 + 'y' * 20, 'z' * 10])
        # Items longer than width will exceed line length
        self.assertEqual(list(ihm.dumper._prettyprint_seq(seq, 25)),
                         ['x' * 30, 'y' * 20, 'z' * 10])
        # Empty sequence
        self.assertEqual(list(ihm.dumper._prettyprint_seq([], 25)), [])

    def test_entry_dumper(self):
        """Test EntryDumper"""
        system = ihm.System(id='test_model')
        dumper = ihm.dumper._EntryDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, "data_test_model\n_entry.id test_model\n")

    def test_entry_dumper_data_chars(self):
        """Test allowed characters in data_ block with EntryDumper"""
        system = ihm.System(id='foo99-bar94_ABC $#% x')
        dumper = ihm.dumper._EntryDumper()
        out = _get_dumper_output(dumper, system).split('\n')[0]
        # Whitespace and special characters (except - _) should be removed
        self.assertEqual(out, "data_foo99-bar94_ABCx")

    def test_audit_conform_dumper(self):
        """Test AuditConformDumper"""
        system = ihm.System()
        dumper = ihm.dumper._AuditConformDumper()
        out = _get_dumper_output(dumper, system)
        lines = sorted(out.split('\n'))
        self.assertEqual(lines[1].split()[0], "_audit_conform.dict_location")
        self.assertEqual(lines[2].rstrip('\r\n'),
                         "_audit_conform.dict_name ihm-extension.dic")
        self.assertEqual(lines[3].split()[0], "_audit_conform.dict_version")

    def test_struct_dumper(self):
        """Test StructDumper"""
        system = ihm.System(title='test model', model_details="test details")
        dumper = ihm.dumper._StructDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """_struct.entry_id model
_struct.pdbx_model_details 'test details'
_struct.pdbx_structure_determination_methodology integrative
_struct.title 'test model'
""")

    def test_comment_dumper(self):
        """Test CommentDumper"""
        system = ihm.System()
        system.comments.extend(("Comment 1", "Comment 2"))
        dumper = ihm.dumper._CommentDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """# Comment 1
# Comment 2
""")
        # Comments should be ignored in BinaryCIF output
        out = _get_dumper_bcif_output(dumper, system)
        self.assertEqual(out[u'dataBlocks'], [])

    def test_software(self):
        """Test SoftwareDumper"""
        system = ihm.System()
        c1 = ihm.Citation(
            pmid='25161197', title='foo',
            journal="Mol Cell Proteomics", volume=13, page_range=(2927, 2943),
            year=2014, authors=['auth1', 'auth2', 'auth3'], doi='doi1')
        system.software.append(ihm.Software(
            name='test', classification='test code',
            description='Some test program',
            version=1, location='http://test.org'))
        system.software.append(ihm.Software(
            name='foo', classification='test code',
            description='Other test program', location='http://test2.org',
            citation=c1))
        # Duplicate should be removed
        system.software.append(ihm.Software(
            name='foo', classification='x', description='y', location='z'))
        dumper = ihm.dumper._CitationDumper()
        dumper.finalize(system)
        dumper = ihm.dumper._SoftwareDumper()
        dumper.finalize(system)
        self.assertEqual(len(dumper._software_by_id), 2)
        # Repeated calls to finalize() should yield identical results
        dumper.finalize(system)
        self.assertEqual(len(dumper._software_by_id), 2)
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
_software.citation_id
1 test 'test code' 'Some test program' 1 program http://test.org .
2 foo 'test code' 'Other test program' . program http://test2.org 1
#
""")

    def test_citation(self):
        """Test CitationDumper"""
        system = ihm.System()
        c1 = ihm.Citation(
            pmid='25161197',
            title="Structural characterization by cross-linking reveals the\n"
                  "detailed architecture of a coatomer-related heptameric\n"
                  "module from the nuclear pore complex.",
            journal="Mol Cell Proteomics", volume=13, page_range=(2927, 2943),
            year=2014,
            authors=['Shi Y', 'Fernandez-Martinez J', 'Tjioe E', 'Pellarin R',
                     'Kim SJ', 'Williams R', 'Schneidman-Duhovny D', 'Sali A',
                     'Rout MP', 'Chait BT'],
            doi='10.1074/mcp.M114.041673')
        system.citations.extend((c1, c1))  # duplicates should be removed
        dumper = ihm.dumper._CitationDumper()
        dumper.finalize(system)  # Assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_citation.id
_citation.title
_citation.journal_abbrev
_citation.journal_volume
_citation.page_first
_citation.page_last
_citation.year
_citation.pdbx_database_id_PubMed
_citation.pdbx_database_id_DOI
1
;Structural characterization by cross-linking reveals the
detailed architecture of a coatomer-related heptameric
module from the nuclear pore complex.
;
'Mol Cell Proteomics' 13 2927 2943 2014 25161197 10.1074/mcp.M114.041673
#
#
loop_
_citation_author.citation_id
_citation_author.name
_citation_author.ordinal
1 'Shi Y' 1
1 'Fernandez-Martinez J' 2
1 'Tjioe E' 3
1 'Pellarin R' 4
1 'Kim SJ' 5
1 'Williams R' 6
1 'Schneidman-Duhovny D' 7
1 'Sali A' 8
1 'Rout MP' 9
1 'Chait BT' 10
#
""")
        # Handle no last page
        c1.page_range = 'e1637'
        dumper = ihm.dumper._CitationDumper()
        dumper.finalize(system)  # Assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertIn("'Mol Cell Proteomics' 13 e1637 . 2014 ", out)

    def test_citation_primary(self):
        """Test CitationDumper with a primary citation"""
        system = ihm.System()
        c1 = ihm.Citation(pmid='x', title='y', journal='z', year=2014,
                          authors=[], volume=1, page_range=1, doi='d')
        c2 = ihm.Citation(pmid='x2', title='y2', journal='z2', year=2015,
                          authors=[], volume=1, page_range=1, doi='e',
                          is_primary=True)
        system.citations.extend((c1, c2))
        dumper = ihm.dumper._CitationDumper()
        dumper.finalize(system)  # Assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_citation.id
_citation.title
_citation.journal_abbrev
_citation.journal_volume
_citation.page_first
_citation.page_last
_citation.year
_citation.pdbx_database_id_PubMed
_citation.pdbx_database_id_DOI
primary y2 z2 1 1 . 2015 x2 e
2 y z 1 1 . 2014 x d
#
""")

    def test_citation_multiple_primary(self):
        """Test CitationDumper with multiple primary citations"""
        system = ihm.System()
        c1 = ihm.Citation(pmid='x', title='y', journal='z', year=2014,
                          authors=[], volume=1, page_range=1, doi='d',
                          is_primary=True)
        c2 = ihm.Citation(pmid='x2', title='y2', journal='z2', year=2015,
                          authors=[], volume=1, page_range=1, doi='e',
                          is_primary=True)
        system.citations.extend((c1, c2))
        dumper = ihm.dumper._CitationDumper()
        self.assertRaises(ValueError, dumper.finalize, system)

    def test_audit_author_empty(self):
        """Test AuditAuthorDumper with empty list"""
        system = ihm.System()

        c1 = ihm.Citation(
            pmid='25161197', title='foo',
            journal="Mol Cell Proteomics", volume=13, page_range=(2927, 2943),
            year=2014, authors=['auth1', 'auth2', 'auth3'], doi='doi1')
        c2 = ihm.Citation(
            pmid='45161197', title='bar',
            journal="Mol Cell Proteomics", volume=13, page_range=(2927, 2943),
            year=2014, authors=['auth2', 'auth4'], doi='doi2')
        system.citations.extend((c1, c2))

        # Citations indirectly referenced by software should *not* be used
        c3 = ihm.Citation(
            pmid='455', title='baz',
            journal="Mol Cell Proteomics", volume=13, page_range=(2927, 2943),
            year=2014, authors=['auth5', 'auth6', 'auth7'], doi='doi3')
        software = ihm.Software(name='test', classification='test code',
                                description='Some test program',
                                version=1, location='http://test.org',
                                citation=c3)
        system.software.append(software)

        dumper = ihm.dumper._AuditAuthorDumper()
        out = _get_dumper_output(dumper, system)
        # auth2 is repeated in the input; we should see it only once in the
        # output
        self.assertEqual(out, """#
loop_
_audit_author.name
_audit_author.pdbx_ordinal
auth1 1
auth2 2
auth3 3
auth4 4
#
""")

    def test_omitted_unknown(self):
        """Test that Dumpers handle omitted/unknown values correctly"""
        system = ihm.System()
        system.authors.extend((None, ihm.unknown, '.', '?'))
        dumper = ihm.dumper._AuditAuthorDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_audit_author.name
_audit_author.pdbx_ordinal
. 1
? 2
'.' 3
'?' 4
#
""")

    def test_audit_author(self):
        """Test AuditAuthorDumper"""
        system = ihm.System()
        system.authors.extend(('auth1', 'auth2', 'auth3'))

        dumper = ihm.dumper._AuditAuthorDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_audit_author.name
_audit_author.pdbx_ordinal
auth1 1
auth2 2
auth3 3
#
""")

    def test_grant(self):
        """Test GrantDumper"""
        system = ihm.System()
        g1 = ihm.Grant(funding_organization="NIH", country="United States",
                       grant_number="foo")
        g2 = ihm.Grant(funding_organization="NSF", country="United States",
                       grant_number="bar")
        system.grants.extend((g1, g2))

        dumper = ihm.dumper._GrantDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_pdbx_audit_support.funding_organization
_pdbx_audit_support.country
_pdbx_audit_support.grant_number
_pdbx_audit_support.ordinal
NIH 'United States' foo 1
NSF 'United States' bar 2
#
""")

    def test_entity_dumper(self):
        """Test EntityDumper"""
        system = ihm.System()
        system.entities.append(ihm.Entity('AHC', description='foo',
                                          source=ihm.source.Manipulated()))
        system.entities.append(ihm.Entity('AHCD', description='baz',
                                          source=ihm.source.Natural()))
        system.entities.append(ihm.Entity('AHD', description='bar',
                                          source=ihm.source.Synthetic()))
        dumper = ihm.dumper._EntityDumper()
        dumper.finalize(system)  # Assign IDs
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
1 polymer man foo 366.413 1 .
2 polymer nat baz 499.516 1 .
3 polymer syn bar 378.362 1 .
#
""")

    def test_entity_duplicates(self):
        """Test EntityDumper with duplicate entities"""
        system = ihm.System()
        system.entities.append(ihm.Entity('AHC'))
        system.entities.append(ihm.Entity('AHC'))
        dumper = ihm.dumper._EntityDumper()
        self.assertRaises(ValueError, dumper.finalize, system)

    def test_entity_src_nat_dumper(self):
        """Test EntitySrcNatDumper"""
        system = ihm.System()
        system.entities.append(ihm.Entity('AHC', description='foo',
                                          source=ihm.source.Manipulated()))
        s = ihm.source.Natural(ncbi_taxonomy_id='1234',
                               scientific_name='Test latin name',
                               common_name='Test common name',
                               strain='test strain')
        system.entities.append(ihm.Entity('AHCD', description='baz',
                                          source=s))
        ihm.dumper._EntityDumper().finalize(system)
        dumper = ihm.dumper._EntitySrcNatDumper()
        dumper.finalize(system)  # Assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_entity_src_nat.entity_id
_entity_src_nat.pdbx_src_id
_entity_src_nat.pdbx_ncbi_taxonomy_id
_entity_src_nat.pdbx_organism_scientific
_entity_src_nat.common_name
_entity_src_nat.strain
2 1 1234 'Test latin name' 'Test common name' 'test strain'
#
""")

    def test_entity_src_syn_dumper(self):
        """Test EntitySrcSynDumper"""
        system = ihm.System()
        system.entities.append(ihm.Entity('AHC', description='foo',
                                          source=ihm.source.Manipulated()))
        s = ihm.source.Synthetic(ncbi_taxonomy_id='1234',
                                 scientific_name='Test latin name',
                                 common_name='Test common name',
                                 strain='test strain')
        system.entities.append(ihm.Entity('AHCD', description='baz',
                                          source=s))
        ihm.dumper._EntityDumper().finalize(system)
        dumper = ihm.dumper._EntitySrcSynDumper()
        dumper.finalize(system)  # Assign IDs
        out = _get_dumper_output(dumper, system)
        # _pdbx_entity_src_syn.strain is not used in current PDB entries
        self.assertEqual(out, """#
loop_
_pdbx_entity_src_syn.entity_id
_pdbx_entity_src_syn.pdbx_src_id
_pdbx_entity_src_syn.ncbi_taxonomy_id
_pdbx_entity_src_syn.organism_scientific
_pdbx_entity_src_syn.organism_common_name
2 1 1234 'Test latin name' 'Test common name'
#
""")

    def test_entity_src_gen_dumper(self):
        """Test EntitySrcGenDumper"""
        system = ihm.System()
        system.entities.append(ihm.Entity('AHC', description='foo',
                                          source=ihm.source.Natural()))
        system.entities.append(
            ihm.Entity('AHCG', description='bar',
                       source=ihm.source.Manipulated(gene=None, host=None)))
        gene = ihm.source.Details(ncbi_taxonomy_id='1234',
                                  scientific_name='Test latin name',
                                  common_name='Test common name',
                                  strain='test strain')
        host = ihm.source.Details(ncbi_taxonomy_id='5678',
                                  scientific_name='Other latin name',
                                  common_name='Other common name',
                                  strain='other strain')
        s = ihm.source.Manipulated(gene=gene, host=host)
        system.entities.append(ihm.Entity('AHCD', description='baz',
                                          source=s))
        ihm.dumper._EntityDumper().finalize(system)
        dumper = ihm.dumper._EntitySrcGenDumper()
        dumper.finalize(system)  # Assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_entity_src_gen.entity_id
_entity_src_gen.pdbx_src_id
_entity_src_gen.pdbx_gene_src_ncbi_taxonomy_id
_entity_src_gen.pdbx_gene_src_scientific_name
_entity_src_gen.gene_src_common_name
_entity_src_gen.gene_src_strain
_entity_src_gen.pdbx_host_org_ncbi_taxonomy_id
_entity_src_gen.pdbx_host_org_scientific_name
_entity_src_gen.host_org_common_name
_entity_src_gen.pdbx_host_org_strain
3 2 1234 'Test latin name' 'Test common name' 'test strain' 5678
'Other latin name' 'Other common name' 'other strain'
#
""")

    def test_struct_ref(self):
        """Test StructRefDumper"""
        system = ihm.System()
        lpep = ihm.LPeptideAlphabet()
        sd = ihm.reference.SeqDif(seq_id=2, db_monomer=lpep['W'],
                                  monomer=lpep['S'], details='Test mutation')
        r1 = ihm.reference.UniProtSequence(
            db_code='NUP84_YEAST', accession='P52891', sequence='MELWPTYQT',
            details='test sequence')
        r1.alignments.append(ihm.reference.Alignment(db_begin=3, seq_dif=[sd]))
        r2 = ihm.reference.UniProtSequence(
            db_code='testcode', accession='testacc', sequence='MELSPTYQT',
            details='test2')
        r2.alignments.append(ihm.reference.Alignment(
            db_begin=4, db_end=5, entity_begin=2, entity_end=3))
        r2.alignments.append(ihm.reference.Alignment(
            db_begin=9, db_end=9, entity_begin=4, entity_end=4))
        r3 = ihm.reference.UniProtSequence(
            db_code='testcode2', accession='testacc2', sequence=None)
        r3.alignments.append(ihm.reference.Alignment(
            db_begin=4, db_end=5, entity_begin=2, entity_end=3))
        r4 = ihm.reference.UniProtSequence(
            db_code='testcode3', accession='testacc3', sequence=ihm.unknown)
        r4.alignments.append(ihm.reference.Alignment(
            db_begin=4, db_end=5, entity_begin=2, entity_end=3))
        system.entities.append(ihm.Entity('LSPT', references=[r1, r2, r3, r4]))
        dumper = ihm.dumper._EntityDumper()
        dumper.finalize(system)  # Assign entity IDs

        dumper = ihm.dumper._StructRefDumper()
        dumper.finalize(system)  # Assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_struct_ref.id
_struct_ref.entity_id
_struct_ref.db_name
_struct_ref.db_code
_struct_ref.pdbx_db_accession
_struct_ref.pdbx_align_begin
_struct_ref.pdbx_seq_one_letter_code
_struct_ref.details
1 1 UNP NUP84_YEAST P52891 3 LWPTYQT 'test sequence'
2 1 UNP testcode testacc 4 SPTYQT test2
3 1 UNP testcode2 testacc2 4 . .
4 1 UNP testcode3 testacc3 4 ? .
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
2 2 2 3 4 5
3 2 4 4 9 9
4 3 2 3 4 5
5 4 2 3 4 5
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
""")

    def test_struct_ref_bad_align(self):
        """Test StructRefDumper with bad entity align"""
        system = ihm.System()
        r = ihm.reference.UniProtSequence(
            db_code='NUP84_YEAST', accession='P52891', sequence='MELSPTYQT',
            details='test sequence')
        r.alignments.append(ihm.reference.Alignment(entity_begin=90))
        system.entities.append(ihm.Entity('LSPT', references=[r]))
        dumper = ihm.dumper._EntityDumper()
        dumper.finalize(system)  # Assign entity IDs

        dumper = ihm.dumper._StructRefDumper()
        dumper.finalize(system)  # Assign IDs
        with self.assertRaises(IndexError) as cm:
            _get_dumper_output(dumper, system)
        self.assertIn('is (90-4), out of range 1-4', str(cm.exception))

    def test_struct_ref_bad_db_align(self):
        """Test StructRefDumper with bad db align"""
        system = ihm.System()
        r = ihm.reference.UniProtSequence(
            db_code='NUP84_YEAST', accession='P52891', sequence='MELSPTYQT',
            details='test sequence')
        r.alignments.append(ihm.reference.Alignment(db_begin=90))
        system.entities.append(ihm.Entity('LSPT', references=[r]))
        dumper = ihm.dumper._EntityDumper()
        dumper.finalize(system)  # Assign entity IDs

        dumper = ihm.dumper._StructRefDumper()
        dumper.finalize(system)  # Assign IDs
        with self.assertRaises(IndexError) as cm:
            _get_dumper_output(dumper, system)
        self.assertIn('is (90-9), out of range 1-9', str(cm.exception))

    def test_struct_ref_seq_mismatch(self):
        """Test StructRefDumper with sequence mismatch"""
        system = ihm.System()
        r = ihm.reference.UniProtSequence(
            db_code='NUP84_YEAST', accession='P52891', sequence='MELSPTYQT',
            details='test sequence')
        system.entities.append(ihm.Entity('LSPT', references=[r]))
        dumper = ihm.dumper._EntityDumper()
        dumper.finalize(system)  # Assign entity IDs

        dumper = ihm.dumper._StructRefDumper()
        dumper.finalize(system)  # Assign IDs
        with self.assertRaises(ValueError) as cm:
            _get_dumper_output(dumper, system)
        self.assertIn('does not match entity canonical sequence',
                      str(cm.exception))

    def test_struct_ref_seq_dif_outrange(self):
        """Test StructRefDumper with SeqDif out of range"""
        system = ihm.System()
        lpep = ihm.LPeptideAlphabet()
        sd = ihm.reference.SeqDif(seq_id=40, db_monomer=lpep['W'],
                                  monomer=lpep['S'], details='Test mutation')
        r = ihm.reference.UniProtSequence(
            db_code='NUP84_YEAST', accession='P52891', sequence='MELSPTYQT',
            details='test sequence')
        r.alignments.append(ihm.reference.Alignment(seq_dif=[sd]))
        system.entities.append(ihm.Entity('LSPT', references=[r]))
        dumper = ihm.dumper._EntityDumper()
        dumper.finalize(system)  # Assign entity IDs

        dumper = ihm.dumper._StructRefDumper()
        dumper.finalize(system)  # Assign IDs
        with self.assertRaises(IndexError) as cm:
            _get_dumper_output(dumper, system)
        self.assertIn('is 40, out of range 1-4', str(cm.exception))

    def test_struct_ref_seq_dif_mismatch(self):
        """Test StructRefDumper with SeqDif code mismatch"""
        system = ihm.System()
        lpep = ihm.LPeptideAlphabet()
        sd = ihm.reference.SeqDif(seq_id=2, db_monomer=lpep['W'],
                                  monomer=lpep['Y'], details='Test mutation')
        r = ihm.reference.UniProtSequence(
            db_code='NUP84_YEAST', accession='P52891', sequence='MELWPTYQT',
            details='test sequence')
        r.alignments.append(ihm.reference.Alignment(seq_dif=[sd]))
        system.entities.append(ihm.Entity('LSPT', references=[r]))
        dumper = ihm.dumper._EntityDumper()
        dumper.finalize(system)  # Assign entity IDs

        dumper = ihm.dumper._StructRefDumper()
        dumper.finalize(system)  # Assign IDs
        with self.assertRaises(ValueError) as cm:
            _get_dumper_output(dumper, system)
        self.assertIn('one-letter code (Y) does not match', str(cm.exception))
        self.assertIn('(S at position 2)', str(cm.exception))

    def test_chem_comp_dumper(self):
        """Test ChemCompDumper"""
        system = ihm.System()
        system.entities.append(ihm.Entity('ACGTTA'))
        system.entities.append(ihm.Entity('ACGA', alphabet=ihm.RNAAlphabet))
        system.entities.append(ihm.Entity(('DA', 'DC'),
                                          alphabet=ihm.DNAAlphabet))
        dumper = ihm.dumper._ChemCompDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_chem_comp.id
_chem_comp.type
_chem_comp.name
_chem_comp.formula
_chem_comp.formula_weight
A 'RNA linking' "ADENOSINE-5'-MONOPHOSPHATE" 'C10 H14 N5 O7 P' 347.224
ALA 'L-peptide linking' ALANINE 'C3 H7 N O2' 89.094
C 'RNA linking' "CYTIDINE-5'-MONOPHOSPHATE" 'C9 H14 N3 O8 P' 323.198
CYS 'L-peptide linking' CYSTEINE 'C3 H7 N O2 S' 121.154
DA 'DNA linking' "2'-DEOXYADENOSINE-5'-MONOPHOSPHATE" 'C10 H14 N5 O6 P' 331.225
DC 'DNA linking' "2'-DEOXYCYTIDINE-5'-MONOPHOSPHATE" 'C9 H14 N3 O7 P' 307.199
G 'RNA linking' "GUANOSINE-5'-MONOPHOSPHATE" 'C10 H14 N5 O8 P' 363.223
GLY 'peptide linking' GLYCINE 'C2 H5 N O2' 75.067
THR 'L-peptide linking' THREONINE 'C4 H9 N O3' 119.120
#
""")

    def test_chem_comp_ccd_descriptors(self):
        """Test ChemCompDumper with ccd or descriptors"""
        system = ihm.System()

        comp1 = ihm.NonPolymerChemComp("C1", name='C1', ccd='MA')
        comp2 = ihm.NonPolymerChemComp("C2", name='C2',
                                       descriptors=['foo', 'bar'])
        system.entities.append(ihm.Entity([comp1, comp2]))
        dumper = ihm.dumper._ChemCompDumper()
        # Cannot output ChemComp with ccd or descriptors to IHM files
        self.assertRaises(ValueError, _get_dumper_output, dumper, system)

    def test_chem_descriptor_dumper(self):
        """Test ChemDescriptorDumper"""
        system = ihm.System()
        d1 = ihm.ChemDescriptor('EDC', smiles='CCN=C=NCCCN(C)C',
                                inchi_key='LMDZBCPBFSXMTL-UHFFFAOYSA-N')
        system.orphan_chem_descriptors.append(d1)
        dumper = ihm.dumper._ChemDescriptorDumper()
        dumper.finalize(system)  # Assign descriptor IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_chemical_component_descriptor.id
_ihm_chemical_component_descriptor.auth_name
_ihm_chemical_component_descriptor.chemical_name
_ihm_chemical_component_descriptor.common_name
_ihm_chemical_component_descriptor.smiles
_ihm_chemical_component_descriptor.smiles_canonical
_ihm_chemical_component_descriptor.inchi
_ihm_chemical_component_descriptor.inchi_key
1 EDC . . CCN=C=NCCCN(C)C . . LMDZBCPBFSXMTL-UHFFFAOYSA-N
#
""")

    def test_entity_poly_dumper(self):
        """Test EntityPolyDumper"""
        system = ihm.System()
        e1 = ihm.Entity('ACGT')  # sequence containing glycine
        e2 = ihm.Entity(('A', 'C', 'C', 'UNK', 'MSE'))  # no glycine
        # All D-peptides (with glycine)
        e3 = ihm.Entity(('DAL', 'DCY', 'G'), alphabet=ihm.DPeptideAlphabet)
        # All D-peptides (without glycine)
        e4 = ihm.Entity(('DAL', 'DCY'), alphabet=ihm.DPeptideAlphabet)
        # Mix of L- and D-peptides
        dpep_al = ihm.DPeptideAlphabet()
        e5 = ihm.Entity(('A', dpep_al['DCY'], 'G'))
        # Non-polymeric entity
        e6 = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        # Sequence containing a non-standard residue
        e7 = ihm.Entity((ihm.NonPolymerChemComp('ACE'), 'C', 'C'))
        system.entities.extend((e1, e2, e3, e4, e5, e6, e7))
        # One protein entity is modeled (with an asym unit) the other not;
        # this should be reflected in pdbx_strand_id
        system.asym_units.append(ihm.AsymUnit(e1, 'foo', strand_id='a'))
        system.asym_units.append(ihm.AsymUnit(e1, 'bar', strand_id='b'))

        rna = ihm.Entity('AC', alphabet=ihm.RNAAlphabet)
        dna = ihm.Entity(('DA', 'DC'), alphabet=ihm.DNAAlphabet)
        hybrid = ihm.Entity(rna.sequence + dna.sequence)
        system.entities.extend((rna, dna, hybrid))

        ed = ihm.dumper._EntityDumper()
        ed.finalize(system)  # Assign entity IDs
        sd = ihm.dumper._StructAsymDumper()
        sd.finalize(system)  # Assign asym IDs
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
1 polypeptide(L) no no a,b ACGT ACGT
2 polypeptide(L) no no . ACC(UNK)(MSE) ACCXM
3 polypeptide(D) no no . (DAL)(DCY)G ACG
4 polypeptide(D) no no . (DAL)(DCY) AC
5 polypeptide(L) no no . A(DCY)G ACG
7 polypeptide(L) no yes . (ACE)CC XCC
8 polyribonucleotide no no . AC AC
9 polydeoxyribonucleotide no no . (DA)(DC) AC
10 'polydeoxyribonucleotide/polyribonucleotide hybrid' no no . AC(DA)(DC) ACAC
#
""")

    def test_entity_nonpoly_dumper(self):
        """Test EntityNonPolyDumper"""
        system = ihm.System()
        # Polymeric entity
        e1 = ihm.Entity('ACGT')
        # Non-polymeric entity
        e2 = ihm.Entity([ihm.NonPolymerChemComp('HEM')], description='heme')
        e3 = ihm.Entity([ihm.WaterChemComp()])
        system.entities.extend((e1, e2, e3))

        ed = ihm.dumper._EntityDumper()
        ed.finalize(system)  # Assign entity IDs
        dumper = ihm.dumper._EntityNonPolyDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_pdbx_entity_nonpoly.entity_id
_pdbx_entity_nonpoly.name
_pdbx_entity_nonpoly.comp_id
2 heme HEM
3 . HOH
#
""")

    def test_entity_poly_seq_dumper(self):
        """Test EntityPolySeqDumper"""
        system = ihm.System()
        system.entities.append(ihm.Entity('ACGT'))
        system.entities.append(ihm.Entity('ACC'))
        system.entities.append(ihm.Entity('AC', alphabet=ihm.RNAAlphabet))
        system.entities.append(ihm.Entity(('DA', 'DC'),
                                          alphabet=ihm.DNAAlphabet))
        # Non-polymeric entity
        system.entities.append(ihm.Entity([ihm.NonPolymerChemComp('HEM')]))
        ed = ihm.dumper._EntityDumper()
        ed.finalize(system)  # Assign IDs
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
3 1 A .
3 2 C .
4 1 DA .
4 2 DC .
#
""")

    def test_poly_seq_scheme_dumper(self):
        """Test PolySeqSchemeDumper"""
        system = ihm.System()
        e1 = ihm.Entity('ACGT')
        e2 = ihm.Entity('ACC')
        e3 = ihm.Entity('AC', alphabet=ihm.RNAAlphabet)
        e4 = ihm.Entity(('DA', 'DC'), alphabet=ihm.DNAAlphabet)
        # Non-polymeric entity
        e5 = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        system.entities.extend((e1, e2, e3, e4, e5))
        system.asym_units.append(ihm.AsymUnit(e1, 'foo'))
        system.asym_units.append(ihm.AsymUnit(e2, 'bar', auth_seq_id_map=5))
        system.asym_units.append(ihm.AsymUnit(e3, 'baz'))
        system.asym_units.append(ihm.AsymUnit(e4, 'test', strand_id='X',
                                              auth_seq_id_map={1: (1, 'A'),
                                                               2: (1, 'B')}))
        system.asym_units.append(ihm.AsymUnit(e5, 'heme'))
        ihm.dumper._EntityDumper().finalize(system)
        ihm.dumper._StructAsymDumper().finalize(system)
        dumper = ihm.dumper._PolySeqSchemeDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_pdbx_poly_seq_scheme.asym_id
_pdbx_poly_seq_scheme.entity_id
_pdbx_poly_seq_scheme.seq_id
_pdbx_poly_seq_scheme.mon_id
_pdbx_poly_seq_scheme.pdb_seq_num
_pdbx_poly_seq_scheme.auth_seq_num
_pdbx_poly_seq_scheme.pdb_mon_id
_pdbx_poly_seq_scheme.auth_mon_id
_pdbx_poly_seq_scheme.pdb_strand_id
_pdbx_poly_seq_scheme.pdb_ins_code
A 1 1 ALA 1 1 ALA ALA A .
A 1 2 CYS 2 2 CYS CYS A .
A 1 3 GLY 3 3 GLY GLY A .
A 1 4 THR 4 4 THR THR A .
B 2 1 ALA 6 6 ALA ALA B .
B 2 2 CYS 7 7 CYS CYS B .
B 2 3 CYS 8 8 CYS CYS B .
C 3 1 A 1 1 A A C .
C 3 2 C 2 2 C C C .
D 4 1 DA 1 1 DA DA X A
D 4 2 DC 1 1 DC DC X B
#
""")

    def test_nonpoly_scheme_dumper(self):
        """Test NonPolySchemeDumper"""
        system = ihm.System()
        e1 = ihm.Entity('ACGT')
        e2 = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        e3 = ihm.Entity([ihm.NonPolymerChemComp('ZN')])
        system.entities.extend((e1, e2, e3))
        system.asym_units.append(ihm.AsymUnit(e1, 'foo'))
        system.asym_units.append(ihm.AsymUnit(e2, 'baz', strand_id='Q'))
        system.asym_units.append(ihm.AsymUnit(e3, 'bar', auth_seq_id_map=5))
        ihm.dumper._EntityDumper().finalize(system)
        ihm.dumper._StructAsymDumper().finalize(system)
        dumper = ihm.dumper._NonPolySchemeDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_pdbx_nonpoly_scheme.asym_id
_pdbx_nonpoly_scheme.entity_id
_pdbx_nonpoly_scheme.mon_id
_pdbx_nonpoly_scheme.ndb_seq_num
_pdbx_nonpoly_scheme.pdb_seq_num
_pdbx_nonpoly_scheme.auth_seq_num
_pdbx_nonpoly_scheme.auth_mon_id
_pdbx_nonpoly_scheme.pdb_strand_id
_pdbx_nonpoly_scheme.pdb_ins_code
B 2 HEM 1 1 1 HEM Q .
C 3 ZN 1 6 6 ZN C .
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
        system.asym_units.append(ihm.AsymUnit(e1, 'foo', id='Z'))
        system.asym_units.append(ihm.AsymUnit(e1, 'bar'))
        system.asym_units.append(ihm.AsymUnit(e2, 'baz', id='A'))
        system.asym_units.append(ihm.AsymUnit(e2, 'tmp'))
        dumper = ihm.dumper._StructAsymDumper()
        dumper.finalize(system)  # assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_struct_asym.id
_struct_asym.entity_id
_struct_asym.details
Z 1 foo
B 1 bar
A 2 baz
C 2 tmp
#
""")

    def test_struct_asym_dumper_duplicate_ids(self):
        """Test StructAsymDumper detection of duplicate user IDs"""
        system = ihm.System()
        e1 = ihm.Entity('ACGT')
        system.entities.append(e1)
        a1 = ihm.AsymUnit(e1, 'foo', id='Z')
        a2 = ihm.AsymUnit(e1, 'baz', id='Z')
        system.asym_units.extend((a1, a2))
        dumper = ihm.dumper._StructAsymDumper()
        self.assertRaises(ValueError, dumper.finalize, system)

    def test_assembly_all_modeled(self):
        """Test AssemblyDumper, all components modeled"""
        system = ihm.System()
        e1 = ihm.Entity('ACG', description='foo')
        e2 = ihm.Entity('AW', description='baz')
        a1 = ihm.AsymUnit(e1)
        a2 = ihm.AsymUnit(e1)
        a3 = ihm.AsymUnit(e2)
        system.entities.extend((e1, e2))
        system.asym_units.extend((a1, a2, a3))

        system.orphan_assemblies.append(
            ihm.Assembly((a1, a2(2, 3)), name='foo'))
        # Out of order assembly (should be ordered on output)
        system.orphan_assemblies.append(ihm.Assembly((a3, a2), name='bar',
                                                     description='desc1'))
        # Duplicate (equal) assembly (should be ignored, but description
        # merged in)
        a = ihm.Assembly((a2, a3), description='desc2')
        system.orphan_assemblies.append(a)
        # Another duplicate with duplicate description (should be ignored)
        a = ihm.Assembly((a2, a3), description='desc2')
        system.orphan_assemblies.append(a)

        # Duplicate (identical) assembly (should be ignored, including
        # description)
        system.orphan_assemblies.append(a)

        # Assign entity, asym and range IDs
        ihm.dumper._EntityDumper().finalize(system)
        ihm.dumper._StructAsymDumper().finalize(system)

        system._make_complete_assembly()

        # Assign and check segment IDs
        dumper = ihm.dumper._EntityPolySegmentDumper()
        dumper.finalize(system)
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_entity_poly_segment.id
_ihm_entity_poly_segment.entity_id
_ihm_entity_poly_segment.seq_id_begin
_ihm_entity_poly_segment.seq_id_end
_ihm_entity_poly_segment.comp_id_begin
_ihm_entity_poly_segment.comp_id_end
1 1 1 3 ALA GLY
2 2 1 2 ALA TRP
3 1 2 3 CYS GLY
#
""")

        d = ihm.dumper._AssemblyDumper()
        d.finalize(system)
        self.assertEqual(system.complete_assembly._id, 1)
        self.assertEqual([asmb._id for asmb in system.orphan_assemblies],
                         [2, 3, 3, 3, 3])
        out = _get_dumper_output(d, system)
        self.assertEqual(out, """#
loop_
_ihm_struct_assembly.id
_ihm_struct_assembly.name
_ihm_struct_assembly.description
1 'Complete assembly' 'All known components'
2 foo .
3 bar 'desc1 & desc2'
#
#
loop_
_ihm_struct_assembly_details.id
_ihm_struct_assembly_details.assembly_id
_ihm_struct_assembly_details.parent_assembly_id
_ihm_struct_assembly_details.entity_description
_ihm_struct_assembly_details.entity_id
_ihm_struct_assembly_details.asym_id
_ihm_struct_assembly_details.entity_poly_segment_id
1 1 1 foo 1 A 1
2 1 1 foo 1 B 1
3 1 1 baz 2 C 2
4 2 2 foo 1 A 1
5 2 2 foo 1 B 3
6 3 3 foo 1 B 1
7 3 3 baz 2 C 2
#
""")

    def test_assembly_subset_modeled(self):
        """Test AssemblyDumper, subset of components modeled"""
        system = ihm.System()
        e1 = ihm.Entity('ACG', description='foo')
        e2 = ihm.Entity('EW', description='bar')
        a1 = ihm.AsymUnit(e1)
        system.entities.extend((e1, e2))
        system.asym_units.append(a1)
        # Note that no asym unit uses entity e2, so it won't be included
        # in the assembly

        # Assign entity and asym IDs
        ihm.dumper._EntityDumper().finalize(system)
        ihm.dumper._StructAsymDumper().finalize(system)

        system._make_complete_assembly()

        # Assign and check segment IDs
        dumper = ihm.dumper._EntityPolySegmentDumper()
        dumper.finalize(system)
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_entity_poly_segment.id
_ihm_entity_poly_segment.entity_id
_ihm_entity_poly_segment.seq_id_begin
_ihm_entity_poly_segment.seq_id_end
_ihm_entity_poly_segment.comp_id_begin
_ihm_entity_poly_segment.comp_id_end
1 1 1 3 ALA GLY
#
""")

        d = ihm.dumper._AssemblyDumper()
        d.finalize(system)
        out = _get_dumper_output(d, system)
        self.assertEqual(out, """#
loop_
_ihm_struct_assembly.id
_ihm_struct_assembly.name
_ihm_struct_assembly.description
1 'Complete assembly' 'All known components'
#
#
loop_
_ihm_struct_assembly_details.id
_ihm_struct_assembly_details.assembly_id
_ihm_struct_assembly_details.parent_assembly_id
_ihm_struct_assembly_details.entity_description
_ihm_struct_assembly_details.entity_id
_ihm_struct_assembly_details.asym_id
_ihm_struct_assembly_details.entity_poly_segment_id
1 1 1 foo 1 A 1
#
""")

    def test_external_reference_dumper(self):
        """Test ExternalReferenceDumper"""
        system = ihm.System()
        repo1 = ihm.location.Repository(doi="foo", details='test repo')
        repo2 = ihm.location.Repository(doi="10.5281/zenodo.46266",
                                        url='nup84-v1.0.zip',
                                        top_directory=os.path.join('foo',
                                                                   'bar'))
        repo3 = ihm.location.Repository(doi="10.5281/zenodo.58025",
                                        url='foo.spd')
        loc = ihm.location.InputFileLocation(repo=repo1, path='bar')
        system.locations.append(loc)
        # Duplicates should be ignored
        loc = ihm.location.InputFileLocation(repo=repo1, path='bar')
        system.locations.append(loc)
        # Different file, same repository
        loc = ihm.location.InputFileLocation(repo=repo1, path='baz')
        system.locations.append(loc)
        # Different repository
        loc = ihm.location.OutputFileLocation(repo=repo2, path='baz')
        system.locations.append(loc)
        # Repository containing a single file (not an archive)
        loc = ihm.location.InputFileLocation(repo=repo3, path='foo.spd',
                                             details='EM micrographs')
        system.locations.append(loc)
        # Path can also be None for Repository containing a single file
        loc = ihm.location.InputFileLocation(repo=repo3, path=None,
                                             details='EM micrographs')
        system.locations.append(loc)

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
            self.assertEqual(len(d._ref_by_id), 6)
            self.assertEqual(len(d._repo_by_id), 4)
            # Repeated calls to finalize() should yield identical results
            d.finalize(system)
            self.assertEqual(len(d._ref_by_id), 6)
            self.assertEqual(len(d._repo_by_id), 4)
            out = _get_dumper_output(d, system)
            self.assertEqual(out, """#
loop_
_ihm_external_reference_info.reference_id
_ihm_external_reference_info.reference_provider
_ihm_external_reference_info.reference_type
_ihm_external_reference_info.reference
_ihm_external_reference_info.refers_to
_ihm_external_reference_info.associated_url
_ihm_external_reference_info.details
1 . DOI foo Other . 'test repo'
2 Zenodo DOI 10.5281/zenodo.46266 Archive nup84-v1.0.zip .
3 Zenodo DOI 10.5281/zenodo.58025 File foo.spd .
4 . 'Supplementary Files' . Other . .
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
5 3 . 'Input data or restraints' . 'EM micrographs'
6 4 %s 'Modeling workflow or script' 4 .
#
""" % bar.replace(os.sep, '/'))

    def test_dataset_dumper_duplicates_details(self):
        """DatasetDumper ignores duplicate datasets with differing details"""
        system = ihm.System()
        dump = ihm.dumper._DatasetDumper()
        loc = ihm.location.PDBLocation('1abc', '1.0', 'test details')
        ds1 = ihm.dataset.PDBDataset(loc)
        system.orphan_datasets.append(ds1)
        # A duplicate dataset should be ignored even if details differ
        loc = ihm.location.PDBLocation('1abc', '1.0', 'other details')
        ds2 = ihm.dataset.PDBDataset(loc)
        system.orphan_datasets.append(ds2)
        ds3 = ihm.dataset.PDBDataset(loc, details='other dataset details')
        system.orphan_datasets.append(ds3)
        dump.finalize(system)  # Assign IDs
        self.assertEqual(ds1._id, 1)
        self.assertEqual(ds2._id, 1)
        self.assertEqual(ds3._id, 1)
        self.assertEqual(len(dump._dataset_by_id), 1)

    def test_dataset_dumper_duplicates_samedata_sameloc(self):
        """DatasetDumper doesn't duplicate same datasets in same location"""
        system = ihm.System()
        loc1 = ihm.location.DatabaseLocation("mydb", "abc", "1.0", "")

        # Identical datasets in the same location aren't duplicated
        cx1 = ihm.dataset.CXMSDataset(loc1)
        cx2 = ihm.dataset.CXMSDataset(loc1)

        dump = ihm.dumper._DatasetDumper()
        system.orphan_datasets.extend((cx1, cx2))
        dump.finalize(system)  # Assign IDs
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
        system.orphan_datasets.extend((cx1, cx2))
        dump.finalize(system)  # Assign IDs
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
        system.orphan_datasets.extend((cx2, em3d))
        dump.finalize(system)  # Assign IDs
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
        system.orphan_datasets.extend((em3d_1, em3d_2))
        dump.finalize(system)  # Assign IDs
        self.assertEqual(em3d_1._id, 1)
        self.assertEqual(em3d_2._id, 2)
        self.assertEqual(len(dump._dataset_by_id), 2)

    def test_dataset_dumper_group_finalize(self):
        """Test DatasetDumper finalize of dataset groups"""
        system = ihm.System()
        loc = ihm.location.InputFileLocation(repo='foo', path='baz')
        ds1 = ihm.dataset.CXMSDataset(loc)
        group1 = ihm.dataset.DatasetGroup([ds1])
        # Duplicate group
        group2 = ihm.dataset.DatasetGroup([ds1])

        system.orphan_datasets.append(ds1)
        system.orphan_dataset_groups.extend((group1, group2))

        d = ihm.dumper._DatasetDumper()
        d.finalize(system)  # Assign IDs
        self.assertEqual(len(d._dataset_by_id), 1)
        self.assertEqual(len(d._dataset_group_by_id), 1)

        # Repeated calls to finalize should yield identical results
        d.finalize(system)
        self.assertEqual(len(d._dataset_by_id), 1)
        self.assertEqual(len(d._dataset_group_by_id), 1)

    def test_dataset_dumper_dump(self):
        """Test DatasetDumper.dump()"""
        system = ihm.System()
        loc = ihm.location.InputFileLocation(repo='foo', path='bar')
        loc._id = 97
        ds1 = ihm.dataset.CXMSDataset(loc)
        system.orphan_datasets.append(ds1)

        # group1 contains just the first dataset (but duplicated)
        group1 = ihm.dataset.DatasetGroup([ds1, ds1], name="first")
        system.orphan_dataset_groups.append(group1)

        loc = ihm.location.InputFileLocation(repo='foo2', path='bar2')
        loc._id = 98
        ds2 = ihm.dataset.CXMSDataset(loc)

        # group2 contains all datasets so far (ds1 & ds2)
        group2 = ihm.dataset.DatasetGroup([ds1, ds2], name="all so far")
        system.orphan_dataset_groups.append(group2)

        loc = ihm.location.PDBLocation('1abc', '1.0', 'test details')
        ds3 = ihm.dataset.PDBDataset(loc, details='test dataset details')
        system.orphan_datasets.append(ds3)
        ds3.parents.append(ds2)
        # Ignore duplicates
        ds3.parents.append(ds2)

        # Derived dataset with (shared) transformation
        loc = ihm.location.PDBLocation('1cde', version='foo', details='bar')
        dst = ihm.dataset.Dataset(loc, details='bar')
        t = ihm.geometry.Transformation(
            rot_matrix=[[-0.64, 0.09, 0.77], [0.76, -0.12, 0.64],
                        [0.15, 0.99, 0.01]],
            tr_vector=[1., 2., 3.])
        td = ihm.dataset.TransformedDataset(dst, transform=t)
        ds3.parents.append(td)

        loc = ihm.location.PDBLocation('1cdf', version='foo', details='bar')
        dst = ihm.dataset.Dataset(loc, details='baz')
        # Same transformation as before
        td = ihm.dataset.TransformedDataset(dst, transform=t)
        ds3.parents.append(td)

        # Dataset with no location
        ds4 = ihm.dataset.PDBDataset(None)
        system.orphan_datasets.append(ds4)

        d = ihm.dumper._DatasetDumper()
        d.finalize(system)  # Assign IDs
        out = _get_dumper_output(d, system)
        self.assertEqual(out, """#
loop_
_ihm_dataset_list.id
_ihm_dataset_list.data_type
_ihm_dataset_list.database_hosted
_ihm_dataset_list.details
1 'CX-MS data' NO .
2 'CX-MS data' NO .
3 Other YES bar
4 Other YES baz
5 'Experimental model' YES 'test dataset details'
6 'Experimental model' NO .
#
#
loop_
_ihm_dataset_group.id
_ihm_dataset_group.name
_ihm_dataset_group.application
_ihm_dataset_group.details
1 first . .
2 'all so far' . .
#
#
loop_
_ihm_dataset_group_link.group_id
_ihm_dataset_group_link.dataset_list_id
1 1
2 1
2 2
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
1 3 PDB 1cde foo bar
2 4 PDB 1cdf foo bar
3 5 PDB 1abc 1.0 'test details'
#
#
loop_
_ihm_related_datasets.dataset_list_id_derived
_ihm_related_datasets.dataset_list_id_primary
_ihm_related_datasets.transformation_id
5 2 .
5 3 1
5 4 1
#
#
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
1 -0.640000 0.760000 0.150000 0.090000 -0.120000 0.990000 0.770000 0.640000
0.010000 1.000 2.000 3.000
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
            asym(1, 2), starting_model=None, rigid=True)
        s2 = ihm.representation.ResidueSegment(
            asym(3, 4), starting_model=None,
            rigid=False, primitive='sphere')
        s3 = ihm.representation.MultiResidueSegment(
            asym(1, 2), starting_model=None,
            rigid=False, primitive='gaussian')
        s4 = ihm.representation.FeatureSegment(
            asym(3, 4), starting_model=None,
            rigid=True, primitive='other', count=3,
            description='test segment')
        r1 = ihm.representation.Representation((s1, s2), name='foo',
                                               details='foo details')
        r2 = ihm.representation.Representation((s3, s4), name='bar')
        system.orphan_representations.extend((r1, r2))

        e1._id = 42
        asym._id = 'X'

        # Assign segment IDs
        ihm.dumper._EntityPolySegmentDumper().finalize(system)

        dumper = ihm.dumper._ModelRepresentationDumper()
        dumper.finalize(system)  # assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_model_representation.id
_ihm_model_representation.name
_ihm_model_representation.details
1 foo 'foo details'
2 bar .
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
1 1 42 bar X 1 atomistic . rigid by-atom . .
2 1 42 bar X 2 sphere . flexible by-residue . .
3 2 42 bar X 1 gaussian . flexible multi-residue . .
4 2 42 bar X 2 other . rigid by-feature 3 'test segment'
#
""")

    def test_starting_model_dumper(self):
        """Test StartingModelDumper"""
        class TestStartingModel(ihm.startmodel.StartingModel):
            def get_atoms(self):
                asym = self.asym_unit
                return [ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='CA',
                                       type_symbol='C', x=-8.0, y=-5.0, z=91.0,
                                       biso=42.)]

            def get_seq_dif(self):
                return [ihm.startmodel.MSESeqDif(db_seq_id=5, seq_id=7),
                        ihm.startmodel.SeqDif(db_seq_id=6, seq_id=8,
                                              db_comp_id='LEU',
                                              details='LEU -> GLY')]

        system = ihm.System()
        e1 = ihm.Entity('A' * 6 + 'MG' + 'A' * 12, description='foo')
        system.entities.append(e1)
        asym = ihm.AsymUnit(e1, 'bar')
        system.asym_units.append(asym)
        loc = ihm.location.PDBLocation('1abc', '1.0', 'test details')
        dstemplate = ihm.dataset.PDBDataset(loc)
        loc = ihm.location.PDBLocation('2xyz', '1.0', 'test details')
        dstarget = ihm.dataset.PDBDataset(loc)
        ali = ihm.location.InputFileLocation(repo='foo', path='test.ali')
        script = ihm.location.WorkflowFileLocation(repo='foo', path='test.py')
        software = ihm.Software(name='test', classification='test code',
                                description='Some test program',
                                version=1, location='http://test.org')

        s1 = ihm.startmodel.Template(
            dataset=dstemplate, asym_id='C',
            seq_id_range=(-9, 0),  # 1,10 in IHM numbering
            template_seq_id_range=(101, 110),
            sequence_identity=30.)
        s2 = ihm.startmodel.Template(
            dataset=dstemplate, asym_id='D',
            seq_id_range=(-5, 2),  # 5,12 in IHM numbering
            template_seq_id_range=(201, 210),
            sequence_identity=ihm.startmodel.SequenceIdentity(40., None),
            alignment_file=ali)
        s3 = ihm.startmodel.Template(
            dataset=dstemplate, asym_id='D',
            seq_id_range=(-5, 2),  # 5,12 in IHM numbering
            template_seq_id_range=(201, 210),
            sequence_identity=ihm.startmodel.SequenceIdentity(ihm.unknown,
                                                              ihm.unknown),
            alignment_file=ali)

        sm = TestStartingModel(asym(1, 12), dstarget, 'A', [s1, s2, s3],
                               offset=10, script_file=script,
                               software=software)
        system.orphan_starting_models.append(sm)

        sm = TestStartingModel(asym(1, 15), dstarget, 'A', [],
                               description="test desc")
        system.orphan_starting_models.append(sm)

        e1._id = 42
        asym._id = 99
        dstemplate._id = 101
        dstarget._id = 102
        ali._id = 5
        script._id = 8
        software._id = 99
        # Assign and check segment IDs
        dumper = ihm.dumper._EntityPolySegmentDumper()
        dumper.finalize(system)
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_entity_poly_segment.id
_ihm_entity_poly_segment.entity_id
_ihm_entity_poly_segment.seq_id_begin
_ihm_entity_poly_segment.seq_id_end
_ihm_entity_poly_segment.comp_id_begin
_ihm_entity_poly_segment.comp_id_end
1 42 1 12 ALA ALA
2 42 1 15 ALA ALA
#
""")

        dumper = ihm.dumper._StartingModelDumper()
        dumper.finalize(system)  # assign IDs
        out = _get_dumper_output(dumper, system)
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
1 42 foo 99 1 'experimental model' A 10 102 .
2 42 foo 99 2 'experimental model' A 0 102 'test desc'
#
#
loop_
_ihm_starting_computational_models.starting_model_id
_ihm_starting_computational_models.software_id
_ihm_starting_computational_models.script_file_id
1 99 8
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
1 1 A 1 10 C 101 110 30.000 1 101 .
2 1 A 5 12 D 201 210 40.000 . 101 5
3 1 A 5 12 D 201 210 ? ? 101 5
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
1 ATOM 1 C CA ALA 42 99 1 -8.000 -5.000 91.000 42.000 1
2 ATOM 1 C CA ALA 42 99 1 -8.000 -5.000 91.000 42.000 2
#
#
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
1 42 99 7 MET 1 A 5 MSE 'Conversion of modified residue MSE to MET'
2 42 99 8 GLY 1 A 6 LEU 'LEU -> GLY'
3 42 99 7 MET 2 A 5 MSE 'Conversion of modified residue MSE to MET'
4 42 99 8 GLY 2 A 6 LEU 'LEU -> GLY'
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
        software = MockObject()
        software._id = 80
        script = MockObject()
        script._id = 90
        p1.steps.append(ihm.protocol.Step(
            assembly=assembly, dataset_group=dsg,
            method='Monte Carlo', num_models_begin=0,
            num_models_end=500, multi_scale=True, ensemble=True, name='s1'))
        p1.steps.append(ihm.protocol.Step(
            assembly=assembly, dataset_group=dsg,
            method='Replica exchange', num_models_begin=500,
            num_models_end=2000, multi_scale=True, ensemble=False))
        system.orphan_protocols.append(p1)

        p2 = ihm.protocol.Protocol('sampling')
        p2.steps.append(ihm.protocol.Step(
            assembly=assembly, dataset_group=dsg2,
            method='Replica exchange', num_models_begin=2000,
            num_models_end=1000, multi_scale=True,
            software=software, script_file=script,
            description='test step'))
        system.orphan_protocols.append(p2)

        dumper = ihm.dumper._ProtocolDumper()
        dumper.finalize(system)  # assign IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_modeling_protocol.id
_ihm_modeling_protocol.protocol_name
_ihm_modeling_protocol.num_steps
1 equilibration 2
2 sampling 1
#
#
loop_
_ihm_modeling_protocol_details.id
_ihm_modeling_protocol_details.protocol_id
_ihm_modeling_protocol_details.step_id
_ihm_modeling_protocol_details.struct_assembly_id
_ihm_modeling_protocol_details.dataset_group_id
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
1 1 1 42 99 s1 'Monte Carlo' 0 500 YES NO NO YES . . .
2 1 2 42 99 . 'Replica exchange' 500 2000 YES NO NO NO . . .
3 2 1 42 101 . 'Replica exchange' 2000 1000 YES NO NO NO 80 90 'test step'
#
""")

    def test_post_process(self):
        """Test PostProcessDumper"""
        class MockObject(object):
            pass
        system = ihm.System()
        p1 = ihm.protocol.Protocol('refinement')
        system.orphan_protocols.append(p1)

        a1 = ihm.analysis.Analysis()
        a1.steps.append(ihm.analysis.EmptyStep())
        a2 = ihm.analysis.Analysis()
        a2.steps.append(ihm.analysis.FilterStep(
            feature='energy/score', num_models_begin=1000,
            num_models_end=200))
        a2.steps.append(ihm.analysis.ClusterStep(
            feature='RMSD', num_models_begin=200, num_models_end=42))
        asmb1 = MockObject()
        asmb1._id = 101
        dg1 = MockObject()
        dg1._id = 301
        software = MockObject()
        software._id = 401
        script = MockObject()
        script._id = 501
        a2.steps.append(ihm.analysis.ValidationStep(
            feature='energy/score', num_models_begin=42,
            num_models_end=42, assembly=asmb1, dataset_group=dg1,
            software=software, script_file=script, details='test step'))
        p1.analyses.extend((a1, a2))

        dumper = ihm.dumper._ProtocolDumper()
        dumper.finalize(system)  # assign protocol IDs

        dumper = ihm.dumper._PostProcessDumper()
        dumper.finalize(system)  # assign analysis IDs

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
_ihm_modeling_post_process.struct_assembly_id
_ihm_modeling_post_process.dataset_group_id
_ihm_modeling_post_process.software_id
_ihm_modeling_post_process.script_file_id
_ihm_modeling_post_process.details
1 1 1 1 none none . . . . . . .
2 1 2 1 filter energy/score 1000 200 . . . . .
3 1 2 2 cluster RMSD 200 42 . . . . .
4 1 2 3 validation energy/score 42 42 101 301 401 501 'test step'
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
        assembly = ihm.Assembly()
        assembly._id = 99
        representation = ihm.representation.Representation()
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
        dumper.finalize(system)  # assign model/group IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 'test model' 99 42 32
2 'test model2' 99 42 32
3 'test model3' 99 42 32
#
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 Group1 .
2 'Group 2' .
#
#
loop_
_ihm_model_group_link.group_id
_ihm_model_group_link.model_id
1 1
1 2
2 3
#
""")

    def _make_test_model(self, water=False):
        class MockObject(object):
            pass
        system = ihm.System()
        state = ihm.model.State()
        system.state_groups.append(ihm.model.StateGroup([state]))
        if water:
            e1 = ihm.Entity([ihm.WaterChemComp()] * 3)
        else:
            e1 = ihm.Entity('ACGT')
        e1._id = 9
        system.entities.append(e1)
        asym = ihm.AsymUnit(e1, 'foo')
        asym._id = 'X'
        system.asym_units.append(asym)
        protocol = MockObject()
        protocol._id = 42
        assembly = ihm.Assembly([asym])
        assembly._id = 99
        s = ihm.representation.ResidueSegment(asym, True, 'sphere')
        representation = ihm.representation.Representation([s])
        representation._id = 32
        model = ihm.model.Model(assembly=assembly, protocol=protocol,
                                representation=representation,
                                name='test model')

        group = ihm.model.ModelGroup([model])
        state.append(group)
        return system, model, asym

    def test_range_checker_asmb_asym(self):
        """Test RangeChecker class checking assembly asym ID match"""
        system, model, asym = self._make_test_model()
        asym2 = ihm.AsymUnit(asym.entity, 'bar')
        asym2._id = 'Y'
        system.asym_units.append(asym2)
        # Handle multiple ranges for a given asym
        model.assembly.append(asym(1, 2))
        # RangeChecker should ignore entities in the assembly
        model.assembly.append(asym.entity)

        # Everything is represented
        for a in asym, asym2:
            s = ihm.representation.AtomicSegment(a, rigid=True)
            model.representation.append(s)
            s = ihm.representation.FeatureSegment(a, rigid=False,
                                                  primitive='sphere', count=2)
            model.representation.append(s)

        rngcheck = ihm.dumper._RangeChecker(model)
        # Atom is OK (good asym)
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)
        # Sphere is OK (good asym)
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        # Atom is not OK (bad asym)
        atom = ihm.model.Atom(asym_unit=asym2, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Sphere is not OK (bad asym)
        sphere = ihm.model.Sphere(asym_unit=asym2, seq_id_range=(1, 2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

    def test_range_checker_asmb_seq_id(self):
        """Test RangeChecker class checking assembly seq_id range"""
        system, model, asym = self._make_test_model()
        # Only part of asym is in the assembly
        asmb = ihm.Assembly([asym(1, 2)])
        model.assembly = asmb

        # Everything is represented
        s = ihm.representation.AtomicSegment(asym, rigid=True)
        model.representation.append(s)
        s = ihm.representation.FeatureSegment(asym, rigid=False,
                                              primitive='sphere', count=2)
        model.representation.append(s)

        rngcheck = ihm.dumper._RangeChecker(model)
        self.assertIsNone(rngcheck._last_asmb_range_matched)
        self.assertIsNone(rngcheck._last_asmb_asym_matched)
        # Atom is OK (good range)
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)
        # Cache should now be set
        self.assertEqual(rngcheck._last_asmb_range_matched, (1, 2))
        self.assertEqual(rngcheck._last_asmb_asym_matched, 'X')
        # 2nd check with same seq_id should use the cache
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='CA',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)
        # Sphere is OK (good range)
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        # Atom is not OK (bad range)
        atom = ihm.model.Atom(asym_unit=asym, seq_id=10, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Sphere is not OK (bad range)
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 10),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

    def test_range_checker_duplicate_atoms(self):
        """Test RangeChecker class checking duplicate atoms"""
        system, model, asym = self._make_test_model()
        asmb = ihm.Assembly([asym])
        model.assembly = asmb

        # Everything is represented
        s = ihm.representation.AtomicSegment(asym, rigid=True)
        model.representation.append(s)

        rngcheck = ihm.dumper._RangeChecker(model)
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='CA',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)
        # Error to write another atom with same atom_id to same seq_id
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='CA',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

    def test_range_checker_duplicate_atoms_water(self):
        """Test RangeChecker class checking duplicate water atoms"""
        system, model, asym = self._make_test_model(water=True)
        asmb = ihm.Assembly([asym])
        model.assembly = asmb

        # Everything is represented
        s = ihm.representation.AtomicSegment(asym, rigid=True)
        model.representation.append(s)

        rngcheck = ihm.dumper._RangeChecker(model)
        atom = ihm.model.Atom(asym_unit=asym, seq_id=None, atom_id='O',
                              type_symbol='O', x=1.0, y=2.0, z=3.0, het=True)
        rngcheck(atom)
        atom = ihm.model.Atom(asym_unit=asym, seq_id=None, atom_id='O',
                              type_symbol='O', x=1.0, y=2.0, z=3.0, het=True)
        rngcheck(atom)

    def test_range_checker_repr_asym(self):
        """Test RangeChecker class checking representation asym ID match"""
        system, model, asym = self._make_test_model()
        asym2 = ihm.AsymUnit(asym.entity, 'bar')
        asym2._id = 'Y'
        system.asym_units.append(asym2)
        model.assembly.append(asym2)

        # Add multiple representation segments for asym
        s = ihm.representation.AtomicSegment(asym(1, 2), rigid=True)
        model.representation.append(s)
        s = ihm.representation.FeatureSegment(asym, rigid=False,
                                              primitive='sphere', count=2)
        model.representation.append(s)

        rngcheck = ihm.dumper._RangeChecker(model)
        # Atom is OK (good asym)
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)
        # Sphere is OK (good asym)
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        # Atom is not OK (bad asym)
        atom = ihm.model.Atom(asym_unit=asym2, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Sphere is not OK (bad asym)
        sphere = ihm.model.Sphere(asym_unit=asym2, seq_id_range=(1, 2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

    def test_range_checker_repr_seq_id(self):
        """Test RangeChecker class checking representation seq_id range"""
        system, model, asym = self._make_test_model()
        asym2 = ihm.AsymUnit(asym.entity, 'bar')
        asym2._id = 'Y'
        system.asym_units.append(asym2)
        model.assembly.append(asym2)

        heme = ihm.NonPolymerChemComp("HEM", name='heme',
                                      formula='C34 H32 Fe N4 O4')
        entity_heme = ihm.Entity([heme], description='Heme')
        entity_heme._id = 99
        system.entities.append(entity_heme)
        asym_nonpol = ihm.AsymUnit(entity_heme, 'baz')
        asym_nonpol._id = 'Z'
        system.asym_units.append(asym_nonpol)
        model.assembly.append(asym_nonpol)

        # Add multiple representation segments for asym2
        s = ihm.representation.AtomicSegment(asym2(1, 2), rigid=True)
        model.representation.append(s)
        s = ihm.representation.FeatureSegment(asym2(1, 2), rigid=False,
                                              primitive='sphere', count=2)
        model.representation.append(s)

        s = ihm.representation.AtomicSegment(asym_nonpol, rigid=True)
        model.representation.append(s)

        rngcheck = ihm.dumper._RangeChecker(model)
        self.assertIsNone(rngcheck._last_repr_segment_matched)
        # Atom is OK (good range)
        atom = ihm.model.Atom(asym_unit=asym2, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)
        # Cache should now be set
        self.assertEqual(
            rngcheck._last_repr_segment_matched.asym_unit.seq_id_range, (1, 2))
        # 2nd check with same seq_id should use the cache
        atom = ihm.model.Atom(asym_unit=asym2, seq_id=1, atom_id='CA',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)
        # Sphere is OK (good range)
        sphere = ihm.model.Sphere(asym_unit=asym2, seq_id_range=(1, 2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        # Atom is not OK (bad range)
        atom = ihm.model.Atom(asym_unit=asym2, seq_id=4, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Sphere is not OK (bad range)
        sphere = ihm.model.Sphere(asym_unit=asym2, seq_id_range=(1, 4),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

        # Atom in a nonpolymer must have no seq_id
        atom = ihm.model.Atom(asym_unit=asym_nonpol, seq_id=None, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)
        atom = ihm.model.Atom(asym_unit=asym2, seq_id=None, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)
        atom = ihm.model.Atom(asym_unit=asym_nonpol, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

    def test_range_checker_repr_type_atomic(self):
        """Test RangeChecker class type checking against AtomicSegments"""
        system, model, asym = self._make_test_model()
        # Replace test model's residue representation with atomic
        s = ihm.representation.AtomicSegment(asym, rigid=False)
        model.representation = ihm.representation.Representation([s])

        rngcheck = ihm.dumper._RangeChecker(model)

        # Atom is OK
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)

        # Sphere is not OK
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 1),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

    def test_range_checker_repr_type_residue(self):
        """Test RangeChecker class type checking against ResidueSegments"""
        system, model, asym = self._make_test_model()
        # Test model already has ResidueSegment representation

        rngcheck = ihm.dumper._RangeChecker(model)

        # Atom is not OK
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Multi-residue Sphere is not OK
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

        # Single-residue Sphere is OK
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 1),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

    def test_range_checker_repr_type_multi_residue(self):
        """Test RangeChecker class type checking against
           MultiResidueSegments"""
        system, model, asym = self._make_test_model()
        # Replace test model's residue representation with multi-residue
        s = ihm.representation.MultiResidueSegment(asym, rigid=False,
                                                   primitive='sphere')
        model.representation = ihm.representation.Representation([s])

        rngcheck = ihm.dumper._RangeChecker(model)

        # Atom is not OK
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Sphere is OK if it matches the asym range exactly
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 4),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

    def test_range_checker_repr_type_feature(self):
        """Test RangeChecker class type checking against FeatureSegments"""
        system, model, asym = self._make_test_model()
        # Replace test model's residue representation with feature
        s = ihm.representation.FeatureSegment(asym, rigid=False,
                                              primitive='sphere', count=2)
        model.representation = ihm.representation.Representation([s])

        rngcheck = ihm.dumper._RangeChecker(model)

        # Atom is not OK
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Sphere is OK if it falls entirely within the segment range
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1, 20),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

    def test_model_dumper_spheres(self):
        """Test ModelDumper with spheres"""
        system, model, asym = self._make_test_model()

        # Replace test model's residue representation with feature
        s = ihm.representation.FeatureSegment(asym, rigid=False,
                                              primitive='sphere', count=2)
        r = ihm.representation.Representation([s])
        r._id = 32
        model.representation = r

        model._spheres = [ihm.model.Sphere(asym_unit=asym,
                                           seq_id_range=(1, 3), x=1.0,
                                           y=2.0, z=3.0, radius=4.0),
                          ihm.model.Sphere(asym_unit=asym,
                                           seq_id_range=(4, 4), x=4.0,
                                           y=5.0, z=6.0, radius=1.0, rmsf=8.0)]

        dumper = ihm.dumper._ModelDumper()
        dumper.finalize(system)  # assign model/group IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 'test model' 99 42 32
#
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 . .
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
1 9 1 3 X 1.000 2.000 3.000 4.000 . 1
2 9 4 4 X 4.000 5.000 6.000 1.000 8.000 1
#
""")

    def test_model_dumper_atoms(self):
        """Test ModelDumper with atoms"""
        system, model, asym = self._make_test_model()

        # Replace test model's residue representation with atomic
        s = ihm.representation.AtomicSegment(asym, rigid=False)
        r = ihm.representation.Representation([s])
        r._id = 32
        model.representation = r

        model._atoms = [ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='C',
                                       type_symbol='C', x=1.0, y=2.0, z=3.0),
                        ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='CA',
                                       type_symbol='C', x=10.0, y=20.0, z=30.0,
                                       het=True),
                        ihm.model.Atom(asym_unit=asym, seq_id=2, atom_id='N',
                                       type_symbol='N', x=4.0, y=5.0, z=6.0,
                                       biso=42.0, occupancy=0.2)]

        dumper = ihm.dumper._ModelDumper()
        dumper.finalize(system)  # assign model/group IDs

        # With auth_seq_id == seq_id
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 'test model' 99 42 32
#
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 . .
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
_atom_site.auth_seq_id
_atom_site.pdbx_PDB_ins_code
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
ATOM 1 C C . ALA 1 1 ? X 1.000 2.000 3.000 . 9 X . 1 1
HETATM 2 C CA . ALA 1 1 ? X 10.000 20.000 30.000 . 9 X . 1 1
ATOM 3 N N . CYS 2 2 ? X 4.000 5.000 6.000 0.200 9 X 42.000 1 1
#
#
loop_
_atom_type.symbol
C
N
#
""")
        # Test dump_atoms with add_ihm=False
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        dumper.dump_atoms(system, writer, add_ihm=False)
        self.assertNotIn('ihm_model_id', fh.getvalue())

        # With auth_seq_id == seq_id-1
        asym.auth_seq_id_map = -1
        out = _get_dumper_output(dumper, system)
        self.assertEqual(
            out.split('\n')[43:46:2],
            ["ATOM 1 C C . ALA 1 0 ? X 1.000 2.000 3.000 . 9 X . 1 1",
             "ATOM 3 N N . CYS 2 1 ? X 4.000 5.000 6.000 "
             "0.200 9 X 42.000 1 1"])

        # With auth_seq_id map
        asym.auth_seq_id_map = {1: 42, 2: 99}
        out = _get_dumper_output(dumper, system)
        self.assertEqual(
            out.split('\n')[43:46:2],
            ["ATOM 1 C C . ALA 1 42 ? X 1.000 2.000 3.000 . 9 X . 1 1",
             "ATOM 3 N N . CYS 2 99 ? X 4.000 5.000 6.000 "
             "0.200 9 X 42.000 1 1"])

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
                                file=loc, details='test details')

        ss1 = ihm.model.IndependentSubsample(name='ss1', num_models=5)
        ss2 = ihm.model.IndependentSubsample(name='ss2', num_models=5,
                                             model_group=group, file=loc)
        ss3 = ihm.model.RandomSubsample(name='ss3', num_models=5)
        e2.subsamples.extend((ss1, ss2, ss3))
        system.ensembles.extend((e1, e2))

        dumper = ihm.dumper._EnsembleDumper()
        dumper.finalize(system)  # assign IDs

        # Should raise an error since ss3 is not the same type as ss1/ss2
        self.assertRaises(TypeError, _get_dumper_output, dumper, system)
        del e2.subsamples[2]

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
_ihm_ensemble_info.details
_ihm_ensemble_info.sub_sample_flag
_ihm_ensemble_info.sub_sampling_type
1 cluster1 99 42 Hierarchical RMSD 10 2 4.200 . . NO .
2 . . 42 . . 10 2 . 3 'test details' YES independent
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
""")

    def test_density_dumper(self):
        """Test DensityDumper"""
        class MockObject(object):
            pass
        system = ihm.System()
        e1 = ihm.Entity('AHCD')
        e1._id = 9
        asym = ihm.AsymUnit(e1)
        asym._id = 'X'

        group = MockObject()
        group._id = 42
        ens = ihm.model.Ensemble(model_group=group, num_models=10)

        loc = ihm.location.OutputFileLocation(repo='foo', path='bar')
        loc._id = 3
        ens.densities.append(ihm.model.LocalizationDensity(loc, asym(1, 2)))
        ens.densities.append(ihm.model.LocalizationDensity(loc, asym))
        ens._id = 5
        system.ensembles.append(ens)

        # Assign segment IDs
        ihm.dumper._EntityPolySegmentDumper().finalize(system)

        dumper = ihm.dumper._DensityDumper()
        dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_localization_density_files.id
_ihm_localization_density_files.file_id
_ihm_localization_density_files.ensemble_id
_ihm_localization_density_files.entity_id
_ihm_localization_density_files.asym_id
_ihm_localization_density_files.entity_poly_segment_id
1 3 5 9 X 1
2 3 5 9 X 2
#
""")

    def test_entity_poly_segment_dumper(self):
        """Test EntityPolySegmentDumper"""
        system = ihm.System()
        e1 = ihm.Entity('AHCD')
        e2 = ihm.Entity('ACG')
        e3 = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        a1 = ihm.AsymUnit(e1)
        a1._id = 'X'
        system.entities.extend((e1, e2, e3))
        system.asym_units.append(a1)
        res1 = e2.residue(1)
        res2 = e2.residue(2)
        system.orphan_features.append(ihm.restraint.ResidueFeature([e2]))
        system.orphan_features.append(ihm.restraint.ResidueFeature([res2]))
        system.orphan_features.append(ihm.restraint.NonPolyFeature([e3]))

        system._make_complete_assembly()

        ihm.dumper._EntityDumper().finalize(system)  # assign entity IDs

        dumper = ihm.dumper._EntityPolySegmentDumper()
        dumper.finalize(system)  # assign IDs

        # e1 isn't directly used in anything (a1 is used instead, in the
        # assembly) so should have no range ID
        self.assertFalse(hasattr(e1, '_range_id'))
        self.assertEqual(a1._range_id, 1)
        # e2 is use, in a ResidueFeature, so should have a range ID
        self.assertEqual(e2._range_id, 2)
        # non-polymers don't have ranges
        self.assertEqual(e3._range_id, None)
        # res2 should have been assigned a range, but not res1
        self.assertFalse(hasattr(res1, '_range_id'))
        self.assertEqual(res2._range_id, 3)

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_entity_poly_segment.id
_ihm_entity_poly_segment.entity_id
_ihm_entity_poly_segment.seq_id_begin
_ihm_entity_poly_segment.seq_id_end
_ihm_entity_poly_segment.comp_id_begin
_ihm_entity_poly_segment.comp_id_end
1 1 1 4 ALA ASP
2 2 1 3 ALA GLY
3 2 2 2 CYS CYS
#
""")

    def test_single_state(self):
        """Test MultiStateDumper with a single state"""
        system = ihm.System()
        state = ihm.model.State()
        system.state_groups.append(ihm.model.StateGroup([state]))

        dumper = ihm.dumper._MultiStateDumper()
        dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, "")

    def test_multi_state(self):
        """Test MultiStateDumper with multiple states"""
        system = ihm.System()
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
        dumper.finalize(system)  # assign model group IDs

        dumper = ihm.dumper._MultiStateDumper()
        dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_multi_state_modeling.state_id
_ihm_multi_state_modeling.state_group_id
_ihm_multi_state_modeling.population_fraction
_ihm_multi_state_modeling.state_type
_ihm_multi_state_modeling.state_name
_ihm_multi_state_modeling.experiment_type
_ihm_multi_state_modeling.details
1 1 . 'complex formation' unbound 'Fraction of bulk' 'Unbound molecule 1'
2 1 . 'complex formation' bound 'Fraction of bulk' 'Unbound molecule 2'
3 2 0.400 . . . .
#
#
loop_
_ihm_multi_state_model_group_link.state_id
_ihm_multi_state_model_group_link.model_group_id
1 1
1 2
2 3
3 4
#
""")

    def test_orphan_model_groups(self):
        """Test detection of ModelGroups not in States"""
        system = ihm.System()
        m1 = ihm.model.Model(assembly='a1', protocol='p1', representation='r1')
        group = ihm.model.ModelGroup([m1])
        group._id = 42

        e1 = ihm.model.Ensemble(model_group=group, num_models=10,
                                post_process=None, name='cluster1',
                                clustering_method='Hierarchical',
                                clustering_feature='RMSD',
                                precision=4.2)
        system.ensembles.append(e1)

        dumper = ihm.dumper._ModelDumper()
        self.assertRaises(ValueError, dumper.finalize, system)

    def test_ordered(self):
        """Test OrderedDumper"""
        system = ihm.System()
        mg1 = ihm.model.ModelGroup(name="group1")
        mg1._id = 42
        mg2 = ihm.model.ModelGroup(name="group2")
        mg2._id = 82
        mg3 = ihm.model.ModelGroup(name="group3")
        mg3._id = 92

        proc = ihm.model.OrderedProcess("time steps")
        edge = ihm.model.ProcessEdge(mg1, mg2)
        step = ihm.model.ProcessStep([edge], "Linear reaction")
        proc.steps.append(step)
        system.ordered_processes.append(proc)

        proc = ihm.model.OrderedProcess("time steps", "Proc 2")
        edge1 = ihm.model.ProcessEdge(mg1, mg2)
        edge2 = ihm.model.ProcessEdge(mg1, mg3)
        step = ihm.model.ProcessStep([edge1, edge2], "Branched reaction")
        proc.steps.append(step)
        system.ordered_processes.append(proc)

        dumper = ihm.dumper._OrderedDumper()
        dumper.finalize(system)

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
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
1 . 'time steps' 1 'Linear reaction' 1 . 42 82
2 'Proc 2' 'time steps' 1 'Branched reaction' 1 . 42 82
2 'Proc 2' 'time steps' 1 'Branched reaction' 2 . 42 92
#
""")

    def test_em3d_restraint_dumper(self):
        """Test EM3DRestraintDumper"""
        class MockObject(object):
            pass
        system = ihm.System()

        dataset = MockObject()
        dataset._id = 97
        dataset2 = MockObject()
        dataset2._id = 87
        assembly = MockObject()
        assembly._id = 99
        citation = MockObject()
        citation._id = 8
        r = ihm.restraint.EM3DRestraint(
            dataset=dataset, assembly=assembly,
            segment=False, fitting_method='Gaussian mixture model',
            number_of_gaussians=40, details='GMM fitting')
        r2 = ihm.restraint.EM3DRestraint(
            dataset=dataset2, assembly=assembly,
            segment=False, fitting_method='Gaussian mixture model',
            fitting_method_citation=citation,
            number_of_gaussians=30, details='GMM fitting')
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m._id = 42
        m2 = ihm.model.Model(assembly='foo', protocol='bar',
                             representation='baz')
        m2._id = 44
        system.restraints.extend((r, r2, MockObject()))

        r.fits[m] = ihm.restraint.EM3DRestraintFit(0.4)
        r.fits[m2] = ihm.restraint.EM3DRestraintFit()
        r2.fits[m2] = ihm.restraint.EM3DRestraintFit()

        dumper = ihm.dumper._EM3DDumper()
        dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(dumper, system)
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
1 97 'Gaussian mixture model' . 99 40 42 0.400
2 97 'Gaussian mixture model' . 99 40 44 .
3 87 'Gaussian mixture model' 8 99 30 44 .
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
        r = ihm.restraint.SASRestraint(
            dataset=dataset, assembly=assembly, segment=False,
            fitting_method='FoXS', fitting_atom_type='Heavy atoms',
            multi_state=False, radius_of_gyration=21.07,
            details='FoXS fitting')
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
        dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(dumper, system)
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
        r = ihm.restraint.EM2DRestraint(
            dataset=dataset, assembly=assembly, segment=False,
            number_raw_micrographs=400, pixel_size_width=0.6,
            pixel_size_height=0.5, image_resolution=30.0,
            number_of_projections=100, details='Test fit')
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m._id = 42
        m2 = ihm.model.Model(assembly='foo', protocol='bar',
                             representation='baz')
        m2._id = 44
        system.restraints.extend((r, MockObject()))

        r.fits[m] = ihm.restraint.EM2DRestraintFit(
            cross_correlation_coefficient=0.4,
            rot_matrix=[[-0.64, 0.09, 0.77], [0.76, -0.12, 0.64],
                        [0.15, 0.99, 0.01]],
            tr_vector=[1., 2., 3.])
        r.fits[m2] = ihm.restraint.EM2DRestraintFit()

        dumper = ihm.dumper._EM2DDumper()
        dumper.finalize(system)  # assign IDs

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
1 1 42 0.400 -0.640000 0.760000 0.150000 0.090000 -0.120000 0.990000 0.770000
0.640000 0.010000 1.000 2.000 3.000
2 1 44 . . . . . . . . . . . . .
#
""")

    def test_cross_link_restraint_dumper(self):
        """Test CrossLinkRestraintDumper"""
        class MockObject(object):
            pass
        system = ihm.System()
        e1 = ihm.Entity('ATC', description='foo')
        e2 = ihm.Entity('DEF', description='bar')
        system.entities.extend((e1, e2))
        asym1 = ihm.AsymUnit(e1)
        asym2 = ihm.AsymUnit(e2)
        system.asym_units.extend((asym1, asym2))

        dataset = MockObject()
        dataset._id = 97
        dss = ihm.ChemDescriptor('DSS')
        r = ihm.restraint.CrossLinkRestraint(dataset=dataset, linker=dss)
        # intra, unambiguous
        xxl1 = ihm.restraint.ExperimentalCrossLink(
            e1.residue(2), e1.residue(3))
        # inter, ambiguous
        xxl2 = ihm.restraint.ExperimentalCrossLink(
            e1.residue(2), e2.residue(3))
        xxl3 = ihm.restraint.ExperimentalCrossLink(
            e1.residue(2), e2.residue(2))
        # duplicate crosslink, should be combined with the original (xxl2)
        xxl4 = ihm.restraint.ExperimentalCrossLink(
            e1.residue(2), e2.residue(3))
        # should end up in own group, not with xxl4 (since xxl4==xxl2)
        xxl5 = ihm.restraint.ExperimentalCrossLink(
            e1.residue(1), e2.residue(1), details='test xl')
        r.experimental_cross_links.extend(([xxl1], [xxl2, xxl3], [xxl4, xxl5]))
        system.restraints.extend((r, MockObject()))

        d = ihm.restraint.UpperBoundDistanceRestraint(25.0)
        xl1 = ihm.restraint.ResidueCrossLink(
            xxl1, asym1, asym1, d, psi=0.5, sigma1=1.0, sigma2=2.0,
            restrain_all=True)
        d = ihm.restraint.LowerBoundDistanceRestraint(34.0)
        xl2 = ihm.restraint.AtomCrossLink(
            xxl3, asym1, asym2, 'C', 'N', d, restrain_all=False)
        # Duplicates should be ignored
        xl3 = ihm.restraint.AtomCrossLink(
            xxl3, asym1, asym2, 'C', 'N', d, restrain_all=False)
        # Restraints on pseudo sites
        ps = ihm.restraint.PseudoSite(x=10., y=20., z=30.)
        ps._id = 89
        psxl = ihm.restraint.CrossLinkPseudoSite(site=ps)
        xl4 = ihm.restraint.ResidueCrossLink(
            xxl5, asym1, asym2, d, psi=0.5, sigma1=1.0, sigma2=2.0,
            restrain_all=True, pseudo2=[psxl])
        m = MockObject()
        m._id = 99
        psxl = ihm.restraint.CrossLinkPseudoSite(site=ps, model=m)
        m = MockObject()
        m._id = 990
        psxl2 = ihm.restraint.CrossLinkPseudoSite(site=ps, model=m)
        xl5 = ihm.restraint.ResidueCrossLink(
            xxl2, asym1, asym2, d, psi=0.5, sigma1=1.0, sigma2=2.0,
            restrain_all=True, pseudo2=[psxl, psxl2])
        r.cross_links.extend((xl1, xl2, xl3, xl4, xl5))

        model = MockObject()
        model._id = 201
        xl1.fits[model] = ihm.restraint.CrossLinkFit(psi=0.1, sigma1=4.2,
                                                     sigma2=2.1)

        ihm.dumper._EntityDumper().finalize(system)  # assign entity IDs
        ihm.dumper._StructAsymDumper().finalize(system)  # assign asym IDs
        ihm.dumper._ChemDescriptorDumper().finalize(system)  # descriptor IDs
        dumper = ihm.dumper._CrossLinkDumper()
        dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(dumper, system)
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
1 1 foo 1 2 THR foo 1 3 CYS 1 DSS 97 .
2 2 foo 1 2 THR bar 2 3 PHE 1 DSS 97 .
3 2 foo 1 2 THR bar 2 2 GLU 1 DSS 97 .
4 3 foo 1 1 ALA bar 2 1 ASP 1 DSS 97 'test xl'
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
1 1 1 A 2 THR 1 A 3 CYS . . 'upper bound' ALL by-residue 25.000 0.500 1.000
2.000 NO
2 3 1 A 2 THR 2 B 2 GLU C N 'lower bound' ANY by-atom 34.000 . . . NO
3 4 1 A 1 ALA 2 B 1 ASP . . 'lower bound' ALL by-residue 34.000 0.500 1.000
2.000 YES
4 2 1 A 2 THR 2 B 3 PHE . . 'lower bound' ALL by-residue 34.000 0.500 1.000
2.000 YES
#
#
loop_
_ihm_cross_link_pseudo_site.id
_ihm_cross_link_pseudo_site.restraint_id
_ihm_cross_link_pseudo_site.cross_link_partner
_ihm_cross_link_pseudo_site.pseudo_site_id
_ihm_cross_link_pseudo_site.model_id
1 3 2 89 .
2 4 2 89 99
3 4 2 89 990
#
#
loop_
_ihm_cross_link_result_parameters.id
_ihm_cross_link_result_parameters.restraint_id
_ihm_cross_link_result_parameters.model_id
_ihm_cross_link_result_parameters.psi
_ihm_cross_link_result_parameters.sigma_1
_ihm_cross_link_result_parameters.sigma_2
1 1 201 0.100 4.200 2.100
#
""")

    def test_geometric_object_dumper(self):
        """Test GeometricObjectDumper"""
        system = ihm.System()
        center = ihm.geometry.Center(1., 2., 3.)
        trans = ihm.geometry.Transformation([[1, 0, 0], [0, 1, 0], [0, 0, 1]],
                                            [1., 2., 3.])

        sphere = ihm.geometry.Sphere(center=center, transformation=trans,
                                     radius=2.2, name='my sphere',
                                     description='a test sphere')
        torus = ihm.geometry.Torus(center=center, transformation=trans,
                                   major_radius=5.6, minor_radius=1.2)
        half_torus = ihm.geometry.HalfTorus(
            center=center, transformation=trans, major_radius=5.6,
            minor_radius=1.2, thickness=0.1, inner=True)
        axis = ihm.geometry.XAxis()
        plane = ihm.geometry.XYPlane()

        system.orphan_geometric_objects.extend((sphere, torus, half_torus,
                                                axis, plane))

        dumper = ihm.dumper._GeometricObjectDumper()
        dumper.finalize(system)  # assign IDs
        self.assertEqual(len(dumper._objects_by_id), 5)
        self.assertEqual(len(dumper._centers_by_id), 1)
        self.assertEqual(len(dumper._transformations_by_id), 1)
        # Repeated calls to finalize should yield identical results
        dumper.finalize(system)
        self.assertEqual(len(dumper._objects_by_id), 5)
        self.assertEqual(len(dumper._centers_by_id), 1)
        self.assertEqual(len(dumper._transformations_by_id), 1)
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
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
#
loop_
_ihm_geometric_object_list.object_id
_ihm_geometric_object_list.object_type
_ihm_geometric_object_list.object_name
_ihm_geometric_object_list.object_description
1 sphere 'my sphere' 'a test sphere'
2 torus . .
3 half-torus . .
4 axis . .
5 plane . .
#
#
loop_
_ihm_geometric_object_sphere.object_id
_ihm_geometric_object_sphere.center_id
_ihm_geometric_object_sphere.transformation_id
_ihm_geometric_object_sphere.radius_r
1 1 1 2.200
#
#
loop_
_ihm_geometric_object_torus.object_id
_ihm_geometric_object_torus.center_id
_ihm_geometric_object_torus.transformation_id
_ihm_geometric_object_torus.major_radius_R
_ihm_geometric_object_torus.minor_radius_r
2 1 1 5.600 1.200
3 1 1 5.600 1.200
#
#
loop_
_ihm_geometric_object_half_torus.object_id
_ihm_geometric_object_half_torus.thickness_th
_ihm_geometric_object_half_torus.section
3 0.100 'inner half'
#
#
loop_
_ihm_geometric_object_axis.object_id
_ihm_geometric_object_axis.axis_type
_ihm_geometric_object_axis.transformation_id
4 x-axis .
#
#
loop_
_ihm_geometric_object_plane.object_id
_ihm_geometric_object_plane.plane_type
_ihm_geometric_object_plane.transformation_id
5 xy-plane .
#
""")

    def test_feature_dumper(self):
        """Test FeatureDumper"""
        system = ihm.System()
        e1 = ihm.Entity('ACGT')
        e2 = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        system.entities.extend((e1, e2))
        a1 = ihm.AsymUnit(e1, 'foo')
        a2 = ihm.AsymUnit(e1, 'baz')
        a3 = ihm.AsymUnit(e2, 'heme')
        system.asym_units.extend((a1, a2, a3))

        f = ihm.restraint.ResidueFeature([a1, a2(2, 3), e1, e1(2, 3)],
                                         details='test feature')
        system.orphan_features.append(f)
        # Cannot make a ResidueFeature that includes a non-polymer 'residue'
        self.assertRaises(ValueError, ihm.restraint.ResidueFeature, [a1, a3])

        # Polymeric atom feature
        f = ihm.restraint.AtomFeature([a1.residue(1).atom('CA'),
                                       a2.residue(2).atom('N'),
                                       e1.residue(1).atom('CB')])
        system.orphan_features.append(f)
        # Nonpolymeric atom feature
        f = ihm.restraint.AtomFeature([a3.residue(1).atom('FE'),
                                       e2.residue(1).atom('FE')])
        system.orphan_features.append(f)
        # Cannot make one feature that selects both polymer and nonpolymer
        self.assertRaises(ValueError, ihm.restraint.AtomFeature,
                          [a1.residue(1).atom('CA'), a2.residue(2).atom('N'),
                           a3.residue(1).atom('FE')])
        # Nonpolymeric feature
        f = ihm.restraint.NonPolyFeature([a3, e2])
        system.orphan_features.append(f)
        # Cannot make a NonPolyFeature that includes a polymer 'residue'
        self.assertRaises(ValueError, ihm.restraint.NonPolyFeature, [a1, a3])

        # Pseudo site feature
        ps = ihm.restraint.PseudoSite(x=10., y=20., z=30.)
        ps._id = 89
        f = ihm.restraint.PseudoSiteFeature(site=ps)
        system.orphan_features.append(f)

        ihm.dumper._EntityDumper().finalize(system)  # assign entity IDs
        ihm.dumper._StructAsymDumper().finalize(system)  # assign asym IDs

        dumper = ihm.dumper._FeatureDumper()
        dumper.finalize(system)  # assign IDs
        self.assertEqual(len(dumper._features_by_id), 5)
        # Repeated calls to finalize should yield identical results
        dumper.finalize(system)
        self.assertEqual(len(dumper._features_by_id), 5)
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_feature_list.feature_id
_ihm_feature_list.feature_type
_ihm_feature_list.entity_type
_ihm_feature_list.details
1 'residue range' polymer 'test feature'
2 atom polymer .
3 atom non-polymer .
4 ligand non-polymer .
5 'pseudo site' other .
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
1 1 1 A 1 ALA 4 THR
2 1 1 B 2 CYS 3 GLY
3 1 1 . 1 ALA 4 THR
4 1 1 . 2 CYS 3 GLY
#
#
loop_
_ihm_poly_atom_feature.ordinal_id
_ihm_poly_atom_feature.feature_id
_ihm_poly_atom_feature.entity_id
_ihm_poly_atom_feature.asym_id
_ihm_poly_atom_feature.seq_id
_ihm_poly_atom_feature.comp_id
_ihm_poly_atom_feature.atom_id
1 2 1 A 1 ALA CA
2 2 1 B 2 CYS N
3 2 1 . 1 ALA CB
#
#
loop_
_ihm_non_poly_feature.ordinal_id
_ihm_non_poly_feature.feature_id
_ihm_non_poly_feature.entity_id
_ihm_non_poly_feature.asym_id
_ihm_non_poly_feature.comp_id
_ihm_non_poly_feature.atom_id
1 3 2 C HEM FE
2 3 2 . HEM FE
3 4 2 C HEM .
4 4 2 . HEM .
#
#
loop_
_ihm_pseudo_site_feature.feature_id
_ihm_pseudo_site_feature.pseudo_site_id
5 89
#
""")

    def test_pseudo_site_dumper(self):
        """Test PseudoSiteDumper"""
        system = ihm.System()
        ps1 = ihm.restraint.PseudoSite(x=10., y=20., z=30.)
        ps2 = ihm.restraint.PseudoSite(x=10., y=20., z=30.,
                                       radius=40., description="test pseudo")
        system.orphan_pseudo_sites.extend((ps1, ps2))

        dumper = ihm.dumper._PseudoSiteDumper()
        dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_pseudo_site.id
_ihm_pseudo_site.Cartn_x
_ihm_pseudo_site.Cartn_y
_ihm_pseudo_site.Cartn_z
_ihm_pseudo_site.radius
_ihm_pseudo_site.description
1 10.000 20.000 30.000 . .
2 10.000 20.000 30.000 40.000 'test pseudo'
#
""")

    def test_geometric_restraint_dumper(self):
        """Test GeometricRestraintDumper"""
        class MockObject(object):
            pass
        system = ihm.System()

        feat = MockObject()
        feat._id = 44
        geom = MockObject()
        geom._id = 23
        dataset = MockObject()
        dataset._id = 97

        dist = ihm.restraint.UpperBoundDistanceRestraint(25.0)
        r = ihm.restraint.CenterGeometricRestraint(
            dataset=dataset, geometric_object=geom, feature=feat,
            distance=dist, harmonic_force_constant=2.0, restrain_all=False)
        system.restraints.append(r)

        dumper = ihm.dumper._GeometricRestraintDumper()
        dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(dumper, system)
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
1 23 44 center 'upper bound' 2.000 . 25.000 ANY 97
#
""")

    def test_derived_distance_restraint_dumper(self):
        """Test DerivedDistanceRestraintDumper"""
        class MockObject(object):
            pass
        system = ihm.System()

        feat1 = MockObject()
        feat1._id = 44
        feat2 = MockObject()
        feat2._id = 84
        dataset = MockObject()
        dataset._id = 97

        dist = ihm.restraint.LowerBoundDistanceRestraint(25.0)
        unkdist = ihm.restraint.DistanceRestraint()
        r1 = ihm.restraint.DerivedDistanceRestraint(
            dataset=dataset, feature1=feat1, feature2=feat2, distance=dist,
            probability=0.8)
        r2 = ihm.restraint.DerivedDistanceRestraint(
            dataset=dataset, feature1=feat1, feature2=feat2, distance=dist,
            probability=0.4)
        r3 = ihm.restraint.DerivedDistanceRestraint(
            dataset=dataset, feature1=feat1, feature2=feat2, distance=unkdist,
            probability=0.6, mic_value=0.4)
        rg = ihm.restraint.RestraintGroup((r2, r3))
        system.restraints.extend((r1, r2))  # r2 is in restraints and groups
        system.restraint_groups.append(rg)

        dumper = ihm.dumper._DerivedDistanceRestraintDumper()
        dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
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
1 . 44 84 'lower bound' 25.000 . 0.800 . . 97
2 1 44 84 'lower bound' 25.000 . 0.400 . . 97
3 1 44 84 . . . 0.600 0.400 . 97
#
""")

    def test_derived_distance_restraint_dumper_fail(self):
        """Test DerivedDistanceRestraintDumper multi-group failure"""
        class MockObject(object):
            pass
        system = ihm.System()

        feat1 = MockObject()
        feat2 = MockObject()
        dataset = MockObject()

        dist = ihm.restraint.LowerBoundDistanceRestraint(25.0)
        r1 = ihm.restraint.DerivedDistanceRestraint(
            dataset=dataset, feature1=feat1, feature2=feat2, distance=dist,
            probability=0.8)
        rg1 = ihm.restraint.RestraintGroup([r1])
        rg2 = ihm.restraint.RestraintGroup([r1])
        system.restraint_groups.extend((rg1, rg2))

        dumper = ihm.dumper._DerivedDistanceRestraintDumper()
        # r1 cannot be in multiple groups (rg1 and rg2)
        self.assertRaises(ValueError, dumper.finalize, system)

    def test_bad_restraint_groups(self):
        """Test RestraintGroups containing unsupported restraints"""
        class MockObject(object):
            pass

        s = ihm.System()
        dataset = MockObject()
        dataset.parents = []
        assembly = ihm.Assembly()

        # Empty restraint groups are OK (even though they don't get IDs)
        rg = ihm.restraint.RestraintGroup([])
        s.restraint_groups.append(rg)
        fh = StringIO()
        ihm.dumper.write(fh, [s])

        r = ihm.restraint.SASRestraint(
            dataset=dataset, assembly=assembly, segment=False,
            fitting_method='FoXS', fitting_atom_type='Heavy atoms',
            multi_state=False, radius_of_gyration=21.07,
            details='FoXS fitting')

        rg = ihm.restraint.RestraintGroup([r])
        s.restraint_groups.append(rg)

        fh = StringIO()
        # SASRestraint is an unsupported type in RestraintGroup
        self.assertRaises(TypeError, ihm.dumper.write, fh, [s])

    def test_predicted_contact_restraint_dumper(self):
        """Test PredictedContactRestraintDumper"""
        class MockObject(object):
            pass
        system = ihm.System()
        e1 = ihm.Entity('AHC')
        a1 = ihm.AsymUnit(e1)
        e2 = ihm.Entity('GWT')
        a2 = ihm.AsymUnit(e2)
        system.entities.extend((e1, e2))
        system.asym_units.extend((a1, a2))

        dataset = MockObject()
        dataset._id = 97
        software = MockObject()
        software._id = 34

        dist = ihm.restraint.LowerBoundDistanceRestraint(25.0)
        dist2 = ihm.restraint.UpperBoundDistanceRestraint(14.0)
        r1 = ihm.restraint.PredictedContactRestraint(
            dataset=dataset, resatom1=a1.residue(1), resatom2=a2.residue(2),
            distance=dist, probability=0.8, by_residue=True, software=software)
        r2 = ihm.restraint.PredictedContactRestraint(
            dataset=dataset, resatom1=a1.residue(1).atom('CA'),
            resatom2=a2.residue(2).atom('CB'), by_residue=True,
            distance=dist, probability=0.4)
        r3 = ihm.restraint.PredictedContactRestraint(
            dataset=dataset, resatom1=a1.residue(1), resatom2=a2.residue(2),
            distance=dist2, probability=0.6, by_residue=False)
        rg = ihm.restraint.RestraintGroup((r2, r3))
        system.restraints.extend((r1, r2))  # r2 is in restraints and groups
        system.restraint_groups.append(rg)

        ihm.dumper._EntityDumper().finalize(system)  # assign entity IDs
        ihm.dumper._StructAsymDumper().finalize(system)  # assign asym IDs
        dumper = ihm.dumper._PredictedContactRestraintDumper()
        dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
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
#
""")

    def test_FLRDumper(self):
        """Test FLR dumpers"""

        class MockObject(object):
            pass

        cur_state = MockObject()
        cur_state._id = 1

        cur_model_1 = MockObject()
        cur_model_1._id = 1
        cur_model_2 = MockObject()
        cur_model_2._id = 2
        dataset_1 = MockObject()
        dataset_1._id = 1
        dataset_group_1 = MockObject()
        dataset_group_1._id = 1
        cur_ihm_modeling_protocol = MockObject()
        cur_ihm_modeling_protocol._id = 1

        system = ihm.System()
        # Fill the system
        cur_flr_data = ihm.flr.FLRData()

        cur_entity_1 = ihm.Entity("AG", description='Entity_1')
        cur_entity_2 = ihm.Entity("CCCCCCCCCC", description='Entity_2')

        system.entities.extend([cur_entity_1, cur_entity_2])

        asym1 = ihm.AsymUnit(cur_entity_1, id='C')
        system.asym_units.append(asym1)

        # FLR
        cur_entity_assembly = ihm.flr.EntityAssembly()
        cur_entity_assembly.add_entity(entity=cur_entity_1, num_copies=1)
        cur_entity_assembly.add_entity(entity=cur_entity_2, num_copies=2)

        cur_instrument = ihm.flr.Instrument(details='My_Instrument')
        cur_inst_setting_1 = ihm.flr.InstSetting(details='My_Inst_setting_1')
        cur_inst_setting_2 = ihm.flr.InstSetting(details='My_Inst_setting_2')
        cur_exp_condition_1 = ihm.flr.ExpCondition(
            details='My_Exp_condition_1')
        cur_exp_condition_2 = ihm.flr.ExpCondition(
            details='My_Exp_condition_2')

        cur_sample_condition_1 = ihm.flr.SampleCondition(
            details='My_Sample_condition_1')
        cur_sample_condition_2 = ihm.flr.SampleCondition(
            details='My_Sample_condition_2')

        cur_sample_1 = ihm.flr.Sample(entity_assembly=cur_entity_assembly,
                                      num_of_probes=2,
                                      condition=cur_sample_condition_1,
                                      description='Sample_1',
                                      details='Details sample 1',
                                      solvent_phase='liquid')
        cur_sample_2 = ihm.flr.Sample(entity_assembly=cur_entity_assembly,
                                      num_of_probes=2,
                                      condition=cur_sample_condition_2,
                                      description='Sample_2',
                                      details='Details sample 2',
                                      solvent_phase='liquid')
        # Reference sample
        cur_sample_3 = ihm.flr.Sample(entity_assembly=cur_entity_assembly,
                                      num_of_probes=1,
                                      condition=cur_sample_condition_1,
                                      description='Reference Sample',
                                      details='Details Reference Sample',
                                      solvent_phase='liquid')

        cur_experiment = ihm.flr.Experiment()
        cur_experiment.add_entry(instrument=cur_instrument,
                                 inst_setting=cur_inst_setting_1,
                                 exp_condition=cur_exp_condition_1,
                                 sample=cur_sample_1)
        cur_experiment.add_entry(instrument=cur_instrument,
                                 inst_setting=cur_inst_setting_2,
                                 exp_condition=cur_exp_condition_2,
                                 sample=cur_sample_2)
        cur_experiment.add_entry(instrument=cur_instrument,
                                 inst_setting=cur_inst_setting_1,
                                 exp_condition=cur_exp_condition_1,
                                 sample=cur_sample_3)
        # Probes
        cur_probe_1 = ihm.flr.Probe()
        cur_probe_2 = ihm.flr.Probe()
        cur_probe_list_1 = ihm.flr.ProbeList(chromophore_name='Donor1',
                                             reactive_probe_flag=False,
                                             probe_origin='extrinsic',
                                             probe_link_type='covalent')
        cur_probe_list_2 = ihm.flr.ProbeList(
            chromophore_name='Acceptor2',
            reactive_probe_flag=True, reactive_probe_name='Acceptor1 reactive',
            probe_origin='extrinsic', probe_link_type='covalent')
        # Chem descriptor ID 1
        cur_chem_desc_probe_1_chromophore = ihm.ChemDescriptor(
            auth_name='Donor1_chromophore_chem_desc',
            chem_comp_id=None, common_name=None, smiles='C1')
        cur_chem_desc_probe_1_chromophore._id = 1
        # Chem descriptor ID 2
        cur_chem_desc_probe_2_chromophore = ihm.ChemDescriptor(
            auth_name='Donor2_chromophore_chem_desc',
            chem_comp_id=None, common_name=None, smiles='C2')
        cur_chem_desc_probe_2_chromophore._id = 2
        # Chem descriptor ID 3
        cur_chem_desc_probe_2_reactive = ihm.ChemDescriptor(
            auth_name='Donor1_reactive_chem_desc',
            chem_comp_id=None, common_name=None, smiles='R1')
        cur_chem_desc_probe_2_reactive._id = 3
        cur_probe_descriptor_1 = ihm.flr.ProbeDescriptor(
            reactive_probe_chem_descriptor=None,
            chromophore_chem_descriptor=cur_chem_desc_probe_1_chromophore,
            chromophore_center_atom='CB')
        cur_probe_descriptor_2 = ihm.flr.ProbeDescriptor(
            reactive_probe_chem_descriptor=cur_chem_desc_probe_2_reactive,
            chromophore_chem_descriptor=cur_chem_desc_probe_2_chromophore,
            chromophore_center_atom='CB')
        cur_probe_1.probe_descriptor = cur_probe_descriptor_1
        cur_probe_1.probe_list_entry = cur_probe_list_1
        cur_probe_2.probe_descriptor = cur_probe_descriptor_2
        cur_probe_2.probe_list_entry = cur_probe_list_2

        # Modified residue
        # Chem descriptor ID 4
        cur_chem_descriptor_modified_residue = ihm.ChemDescriptor(
            auth_name='Modified_residue', smiles='Modified')
        cur_chem_comp_mutated_residue = ihm.ChemComp(
            id='Cys', code='C', code_canonical='C')

        cur_chem_descriptor_modified_residue._id = 4
#        cur_chem_comp_mutated_residue._id = 5
        # Poly_probe_position
        cur_poly_probe_position_1 = ihm.flr.PolyProbePosition(
            resatom=cur_entity_1.residue(1),  # no atom ID given
            mutation_flag=True,
            modification_flag=True, auth_name='Position_1',
            mutated_chem_comp_id=cur_chem_comp_mutated_residue,
            modified_chem_descriptor=cur_chem_descriptor_modified_residue)
        cur_poly_probe_position_2 = ihm.flr.PolyProbePosition(
            # using asym instead of only entity
            resatom=asym1.residue(2).atom('CB'),
            mutation_flag=False,
            modification_flag=False, auth_name='Position_2')
        cur_poly_probe_position_3 = ihm.flr.PolyProbePosition(
            resatom=cur_entity_2.residue(10).atom('CB'),
            mutation_flag=True,
            modification_flag=True, auth_name='Position_3',
            mutated_chem_comp_id=cur_chem_comp_mutated_residue,
            modified_chem_descriptor=cur_chem_descriptor_modified_residue)
        # Sample_probe_details
        cur_sample_probe_details_1 = ihm.flr.SampleProbeDetails(
            sample=cur_sample_1, probe=cur_probe_1, fluorophore_type='donor',
            poly_probe_position=cur_poly_probe_position_1,
            description='Donor in position1-position3')
        cur_sample_probe_details_2 = ihm.flr.SampleProbeDetails(
            sample=cur_sample_1, probe=cur_probe_2,
            fluorophore_type='acceptor',
            poly_probe_position=cur_poly_probe_position_3,
            description='Acceptor in position1-position3')
        cur_sample_probe_details_3 = ihm.flr.SampleProbeDetails(
            sample=cur_sample_2, probe=cur_probe_1, fluorophore_type='donor',
            poly_probe_position=cur_poly_probe_position_2,
            description='Donor in position2-position3')
        cur_sample_probe_details_4 = ihm.flr.SampleProbeDetails(
            sample=cur_sample_2, probe=cur_probe_2,
            fluorophore_type='acceptor',
            poly_probe_position=cur_poly_probe_position_3,
            description='Acceptor in position2-position3')
        cur_sample_probe_details_5 = ihm.flr.SampleProbeDetails(
            sample=cur_sample_3, probe=cur_probe_1, fluorophore_type='donor',
            poly_probe_position=cur_poly_probe_position_1,
            description='Donor-only on reference sample')
        # Poly_probe_conjugate
        # Chem Descriptor ID 5
        cur_poly_probe_conjugate_chem_descriptor = ihm.ChemDescriptor(
            auth_name='Conjugate', smiles='Conj1')
        cur_poly_probe_conjugate_chem_descriptor._id = 5
        cur_poly_probe_conjugate_1 = ihm.flr.PolyProbeConjugate(
            sample_probe=cur_sample_probe_details_1,
            chem_descriptor=cur_poly_probe_conjugate_chem_descriptor,
            ambiguous_stoichiometry=False)
        cur_poly_probe_conjugate_2 = ihm.flr.PolyProbeConjugate(
            sample_probe=cur_sample_probe_details_2,
            chem_descriptor=cur_poly_probe_conjugate_chem_descriptor,
            ambiguous_stoichiometry=False)
        cur_poly_probe_conjugate_3 = ihm.flr.PolyProbeConjugate(
            sample_probe=cur_sample_probe_details_3,
            chem_descriptor=cur_poly_probe_conjugate_chem_descriptor,
            ambiguous_stoichiometry=False)
        cur_poly_probe_conjugate_4 = ihm.flr.PolyProbeConjugate(
            sample_probe=cur_sample_probe_details_4,
            chem_descriptor=cur_poly_probe_conjugate_chem_descriptor,
            ambiguous_stoichiometry=False)
        cur_poly_probe_conjugate_5 = ihm.flr.PolyProbeConjugate(
            sample_probe=cur_sample_probe_details_5,
            chem_descriptor=cur_poly_probe_conjugate_chem_descriptor,
            ambiguous_stoichiometry=False)
        cur_flr_data.poly_probe_conjugates.extend(
            (cur_poly_probe_conjugate_1, cur_poly_probe_conjugate_2,
             cur_poly_probe_conjugate_3, cur_poly_probe_conjugate_4,
             cur_poly_probe_conjugate_5))

        # Forster_radius
        cur_forster_radius = ihm.flr.FRETForsterRadius(
            donor_probe=cur_probe_1, acceptor_probe=cur_probe_2,
            forster_radius=52.0, reduced_forster_radius=53.2)

        # Fret_calibration_parameters
        cur_fret_calibration_parameters_1 = ihm.flr.FRETCalibrationParameters(
            phi_acceptor=0.35, alpha=2.4, gg_gr_ratio=0.4, a_b=0.8)
        cur_fret_calibration_parameters_2 = ihm.flr.FRETCalibrationParameters(
            phi_acceptor=0.35, alpha=2.4, gg_gr_ratio=0.38, a_b=0.8)

        # LifetimeFitModel
        cur_lifetime_fit_model = ihm.flr.LifetimeFitModel(
            name='Lifetime fit model 1', description='Description of model')

        # RefMeasurementLifetime
        cur_lifetime_1 = ihm.flr.RefMeasurementLifetime(species_fraction=0.6,
                                                        lifetime=3.2)
        cur_lifetime_2 = ihm.flr.RefMeasurementLifetime(species_fraction=0.4,
                                                        lifetime=1.4)
        # RefMeasurement
        cur_ref_measurement_1 = ihm.flr.RefMeasurement(
            ref_sample_probe=cur_sample_probe_details_5,
            details='Reference Measurement 1')
        cur_ref_measurement_1.add_lifetime(cur_lifetime_1)
        cur_ref_measurement_1.add_lifetime(cur_lifetime_2)
        # RefMeasurementGroup
        cur_lifetime_ref_measurement_group = ihm.flr.RefMeasurementGroup(
            details='Reference measurement group 1')
        cur_lifetime_ref_measurement_group.add_ref_measurement(
            cur_ref_measurement_1)

        # FretAnalysis
        cur_fret_analysis_1 = ihm.flr.FRETAnalysis(
            experiment=cur_experiment,
            sample_probe_1=cur_sample_probe_details_1,
            sample_probe_2=cur_sample_probe_details_2,
            forster_radius=cur_forster_radius,
            type='intensity-based',
            calibration_parameters=cur_fret_calibration_parameters_1,
            method_name='PDA', chi_square_reduced=1.5, dataset=dataset_1)
        cur_fret_analysis_2 = ihm.flr.FRETAnalysis(
            experiment=cur_experiment,
            sample_probe_1=cur_sample_probe_details_3,
            sample_probe_2=cur_sample_probe_details_4,
            forster_radius=cur_forster_radius, type='intensity-based',
            calibration_parameters=cur_fret_calibration_parameters_2,
            method_name='PDA', chi_square_reduced=1.8, dataset=dataset_1)
        # lifetime-based FRETAnalysis
        cur_fret_analysis_3 = ihm.flr.FRETAnalysis(
            experiment=cur_experiment,
            sample_probe_1=cur_sample_probe_details_1,
            sample_probe_2=cur_sample_probe_details_2,
            forster_radius=cur_forster_radius, type='lifetime-based',
            lifetime_fit_model=cur_lifetime_fit_model,
            ref_measurement_group=cur_lifetime_ref_measurement_group,
            method_name='Lifetime fit', chi_square_reduced=1.6,
            dataset=dataset_1)

        # Peak_assignment
        cur_peak_assignment = ihm.flr.PeakAssignment(
            method_name='Population',
            details='Peaks were assigned by population fractions.')

        # Fret_distance_restraints
        cur_fret_distance_restraint_1 = ihm.flr.FRETDistanceRestraint(
            sample_probe_1=cur_sample_probe_details_1,
            sample_probe_2=cur_sample_probe_details_2,
            analysis=cur_fret_analysis_1, distance=53.5,
            distance_error_plus=2.5, distance_error_minus=2.3,
            distance_type='<R_DA>_E', state=cur_state,
            population_fraction=0.80, peak_assignment=cur_peak_assignment)
        cur_fret_distance_restraint_2 = ihm.flr.FRETDistanceRestraint(
            sample_probe_1=cur_sample_probe_details_3,
            sample_probe_2=cur_sample_probe_details_4,
            analysis=cur_fret_analysis_2, distance=49.0,
            distance_error_plus=2.0, distance_error_minus=2.1,
            distance_type='<R_DA>_E', state=cur_state,
            population_fraction=0.80, peak_assignment=cur_peak_assignment)
        cur_fret_distance_restraint_3 = ihm.flr.FRETDistanceRestraint(
            sample_probe_1=cur_sample_probe_details_1,
            sample_probe_2=cur_sample_probe_details_2,
            analysis=cur_fret_analysis_3, distance=53.5,
            distance_error_plus=2.5, distance_error_minus=2.3,
            distance_type='<R_DA>_E', state=cur_state,
            population_fraction=0.80, peak_assignment=cur_peak_assignment)

        cur_fret_dist_restraint_group = ihm.flr.FRETDistanceRestraintGroup()
        cur_fret_dist_restraint_group.add_distance_restraint(
            cur_fret_distance_restraint_1)
        cur_fret_dist_restraint_group.add_distance_restraint(
            cur_fret_distance_restraint_2)
        cur_fret_dist_restraint_group.add_distance_restraint(
            cur_fret_distance_restraint_3)

        cur_flr_data.distance_restraint_groups.append(
            cur_fret_dist_restraint_group)

        # fret_model_quality
        cur_fret_model_quality_1 = ihm.flr.FRETModelQuality(
            model=cur_model_1, chi_square_reduced=1.3,
            dataset_group=dataset_group_1, method=None)
        cur_fret_model_quality_2 = ihm.flr.FRETModelQuality(
            model=cur_model_2, chi_square_reduced=1.9,
            dataset_group=dataset_group_1, method=None)
        cur_flr_data.fret_model_qualities.extend(
            (cur_fret_model_quality_1, cur_fret_model_quality_2))
        # fret_model_distance
        cur_fret_model_distance_1_1 = ihm.flr.FRETModelDistance(
            restraint=cur_fret_distance_restraint_1, model=cur_model_1,
            distance=52.0)
        cur_fret_model_distance_1_2 = ihm.flr.FRETModelDistance(
            restraint=cur_fret_distance_restraint_2, model=cur_model_1,
            distance=50.0)
        cur_fret_model_distance_2_1 = ihm.flr.FRETModelDistance(
            restraint=cur_fret_distance_restraint_1, model=cur_model_2,
            distance=53.8)
        cur_fret_model_distance_2_2 = ihm.flr.FRETModelDistance(
            restraint=cur_fret_distance_restraint_2, model=cur_model_2,
            distance=49.4)
        cur_flr_data.fret_model_distances.extend(
            (cur_fret_model_distance_1_1, cur_fret_model_distance_1_2,
             cur_fret_model_distance_2_1, cur_fret_model_distance_2_2))

        # FPS modeling
        cur_FPS_global_parameters = ihm.flr.FPSGlobalParameters(
            forster_radius=52, conversion_function_polynom_order=3,
            repetition=1000, av_grid_rel=0.2, av_min_grid_a=0.4,
            av_allowed_sphere=0.5, av_search_nodes=3, av_e_samples_k=200,
            sim_viscosity_adjustment=1, sim_dt_adjustment=1,
            sim_max_iter_k=200, sim_max_force=400, sim_clash_tolerance_a=1,
            sim_reciprocal_kt=10, sim_clash_potential='^2',
            convergence_e=100, convergence_k=0.001, convergence_f=0.001,
            convergence_t=0.002)

        cur_FPS_modeling_1 = ihm.flr.FPSModeling(
            protocol=cur_ihm_modeling_protocol,
            restraint_group=cur_fret_dist_restraint_group,
            global_parameter=cur_FPS_global_parameters,
            probe_modeling_method="AV3")
        cur_FPS_modeling_2 = ihm.flr.FPSModeling(
            protocol=cur_ihm_modeling_protocol,
            restraint_group=cur_fret_dist_restraint_group,
            global_parameter=cur_FPS_global_parameters,
            probe_modeling_method="MPP")
        # Modeling by AV
        cur_FPS_AV_parameters_1 = ihm.flr.FPSAVParameter(
            num_linker_atoms=15, linker_length=20.0, linker_width=3.5,
            probe_radius_1=10.0, probe_radius_2=5.0, probe_radius_3=3.5)

        cur_FPS_AV_modeling_1 = ihm.flr.FPSAVModeling(
            fps_modeling=cur_FPS_modeling_1,
            sample_probe=cur_sample_probe_details_1,
            parameter=cur_FPS_AV_parameters_1)
        cur_FPS_AV_modeling_3 = ihm.flr.FPSAVModeling(
            fps_modeling=cur_FPS_modeling_1,
            sample_probe=cur_sample_probe_details_3,
            parameter=cur_FPS_AV_parameters_1)
        cur_flr_data.fps_modeling.append(cur_FPS_AV_modeling_1)
        cur_flr_data.fps_modeling.append(cur_FPS_AV_modeling_3)

        # Modeling by mean probe position
        cur_mpp_atom_position_1 = ihm.flr.FPSMPPAtomPosition(
            atom=asym1.residue(1).atom('CA'), x=1.0, y=1.0, z=1.0)
        cur_mpp_atom_position_2 = ihm.flr.FPSMPPAtomPosition(
            atom=asym1.residue(2).atom('CA'), x=2.0, y=2.0, z=2.0)
        cur_mpp_atom_position_group = ihm.flr.FPSMPPAtomPositionGroup()
        cur_mpp_atom_position_group.add_atom_position(cur_mpp_atom_position_1)
        cur_mpp_atom_position_group.add_atom_position(cur_mpp_atom_position_2)
        cur_mean_probe_position_2 = ihm.flr.FPSMeanProbePosition(
            sample_probe=cur_sample_probe_details_2, x=1.0, y=2.0, z=3.0)
        cur_mean_probe_position_4 = ihm.flr.FPSMeanProbePosition(
            sample_probe=cur_sample_probe_details_4, x=1.0, y=2.0, z=3.0)
        cur_FPS_MPP_modeling_2 = ihm.flr.FPSMPPModeling(
            fps_modeling=cur_FPS_modeling_2, mpp=cur_mean_probe_position_2,
            mpp_atom_position_group=cur_mpp_atom_position_group)
        cur_FPS_MPP_modeling_4 = ihm.flr.FPSMPPModeling(
            fps_modeling=cur_FPS_modeling_2, mpp=cur_mean_probe_position_4,
            mpp_atom_position_group=cur_mpp_atom_position_group)
        cur_flr_data.fps_modeling.append(cur_FPS_MPP_modeling_2)
        cur_flr_data.fps_modeling.append(cur_FPS_MPP_modeling_4)

        system.flr_data = [cur_flr_data]

        ihm.dumper._EntityDumper().finalize(system)  # assign entity IDs
        ihm.dumper._StructAsymDumper().finalize(system)  # assign asym IDs

        ihm.dumper._ChemCompDumper().finalize(system)

        experiment_dumper = ihm.dumper._FLRExperimentDumper()
        experiment_dumper.finalize(system)

        inst_setting_dumper = ihm.dumper._FLRInstSettingDumper()
        inst_setting_dumper.finalize(system)

        exp_condition_dumper = ihm.dumper._FLR_ExpConditionDumper()
        exp_condition_dumper.finalize(system)

        instrument_dumper = ihm.dumper._FLRInstrumentDumper()
        instrument_dumper.finalize(system)

        entity_assembly_dumper = ihm.dumper._FLREntityAssemblyDumper()
        entity_assembly_dumper.finalize(system)

        sample_condition_dumper = ihm.dumper._FLRSampleConditionDumper()
        sample_condition_dumper.finalize(system)

        sample_dumper = ihm.dumper._FLRSampleDumper()
        sample_dumper.finalize(system)

        probe_dumper = ihm.dumper._FLRProbeDumper()
        probe_dumper.finalize(system)

        sample_probe_details_dumper = ihm.dumper._FLRSampleProbeDetailsDumper()
        sample_probe_details_dumper.finalize(system)

        poly_probe_pos_dumper = ihm.dumper._FLRPolyProbePositionDumper()
        poly_probe_pos_dumper.finalize(system)

        conjugate_dumper = ihm.dumper._FLRConjugateDumper()
        conjugate_dumper.finalize(system)

        radii_dumper = ihm.dumper._FLRForsterRadiusDumper()
        radii_dumper.finalize(system)

        parameters_dumper = ihm.dumper._FLRCalibrationParametersDumper()
        parameters_dumper.finalize(system)

        lifetime_fit_model_dumper = ihm.dumper._FLRLifetimeFitModelDumper()
        lifetime_fit_model_dumper.finalize(system)

        ref_measurement_dumper = ihm.dumper._FLRRefMeasurementDumper()
        ref_measurement_dumper.finalize(system)

        analysis_dumper = ihm.dumper._FLRAnalysisDumper()
        analysis_dumper.finalize(system)

        peak_assignment_dumper = ihm.dumper._FLRPeakAssignmentDumper()
        peak_assignment_dumper.finalize(system)

        distance_restraint_dumper = ihm.dumper._FLRDistanceRestraintDumper()
        distance_restraint_dumper.finalize(system)

        model_quality_dumper = ihm.dumper._FLRModelQualityDumper()
        model_quality_dumper.finalize(system)

        model_distance_dumper = ihm.dumper._FLRModelDistanceDumper()
        model_distance_dumper.finalize(system)

        fps_modeling_dumper = ihm.dumper._FLRFPSModelingDumper()
        fps_modeling_dumper.finalize(system)

        av_dumper = ihm.dumper._FLRFPSAVModelingDumper()
        av_dumper.finalize(system)

        mpp_dumper = ihm.dumper._FLRFPSMPPModelingDumper()
        mpp_dumper.finalize(system)  # assign IDs

        out = _get_dumper_output(experiment_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_experiment.ordinal_id
_flr_experiment.id
_flr_experiment.instrument_id
_flr_experiment.inst_setting_id
_flr_experiment.exp_condition_id
_flr_experiment.sample_id
_flr_experiment.details
1 1 1 1 1 1 .
2 1 1 2 2 2 .
3 1 1 1 1 3 .
#
""")

        out = _get_dumper_output(inst_setting_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_inst_setting.id
_flr_inst_setting.details
1 My_Inst_setting_1
2 My_Inst_setting_2
#
""")

        out = _get_dumper_output(exp_condition_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_exp_condition.id
_flr_exp_condition.details
1 My_Exp_condition_1
2 My_Exp_condition_2
#
""")

        out = _get_dumper_output(instrument_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_instrument.id
_flr_instrument.details
1 My_Instrument
#
""")

        out = _get_dumper_output(entity_assembly_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_entity_assembly.ordinal_id
_flr_entity_assembly.assembly_id
_flr_entity_assembly.entity_id
_flr_entity_assembly.num_copies
_flr_entity_assembly.entity_description
1 1 1 1 Entity_1
2 1 2 2 Entity_2
#
""")

        out = _get_dumper_output(sample_condition_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_sample_condition.id
_flr_sample_condition.details
1 My_Sample_condition_1
2 My_Sample_condition_2
#
""")

        out = _get_dumper_output(sample_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_sample.id
_flr_sample.entity_assembly_id
_flr_sample.num_of_probes
_flr_sample.sample_condition_id
_flr_sample.sample_description
_flr_sample.sample_details
_flr_sample.solvent_phase
1 1 2 1 Sample_1 'Details sample 1' liquid
2 1 2 2 Sample_2 'Details sample 2' liquid
3 1 1 1 'Reference Sample' 'Details Reference Sample' liquid
#
""")

        out = _get_dumper_output(probe_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_probe_list.probe_id
_flr_probe_list.chromophore_name
_flr_probe_list.reactive_probe_flag
_flr_probe_list.reactive_probe_name
_flr_probe_list.probe_origin
_flr_probe_list.probe_link_type
1 Donor1 NO . extrinsic covalent
2 Acceptor2 YES 'Acceptor1 reactive' extrinsic covalent
#
#
loop_
_flr_probe_descriptor.probe_id
_flr_probe_descriptor.reactive_probe_chem_descriptor_id
_flr_probe_descriptor.chromophore_chem_descriptor_id
_flr_probe_descriptor.chromophore_center_atom
1 . 1 CB
2 3 2 CB
#
""")

        out = _get_dumper_output(sample_probe_details_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_sample_probe_details.sample_probe_id
_flr_sample_probe_details.sample_id
_flr_sample_probe_details.probe_id
_flr_sample_probe_details.fluorophore_type
_flr_sample_probe_details.description
_flr_sample_probe_details.poly_probe_position_id
1 1 1 donor 'Donor in position1-position3' 1
2 1 2 acceptor 'Acceptor in position1-position3' 2
3 2 1 donor 'Donor in position2-position3' 3
4 2 2 acceptor 'Acceptor in position2-position3' 2
5 3 1 donor 'Donor-only on reference sample' 1
#
""")

        out = _get_dumper_output(poly_probe_pos_dumper, system)
        self.assertEqual(out, """#
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
1 1 Entity_1 . 1 ALA . YES YES Position_1
2 2 Entity_2 . 10 CYS CB YES YES Position_3
3 1 Entity_1 C 2 GLY CB NO NO Position_2
#
#
loop_
_flr_poly_probe_position_mutated.id
_flr_poly_probe_position_mutated.chem_comp_id
_flr_poly_probe_position_mutated.atom_id
1 Cys .
2 Cys CB
#
#
loop_
_flr_poly_probe_position_modified.id
_flr_poly_probe_position_modified.chem_descriptor_id
_flr_poly_probe_position_modified.atom_id
1 4 .
2 4 CB
#
""")

        out = _get_dumper_output(conjugate_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_poly_probe_conjugate.id
_flr_poly_probe_conjugate.sample_probe_id
_flr_poly_probe_conjugate.chem_descriptor_id
_flr_poly_probe_conjugate.ambiguous_stoichiometry_flag
_flr_poly_probe_conjugate.probe_stoichiometry
1 1 5 NO .
2 2 5 NO .
3 3 5 NO .
4 4 5 NO .
5 5 5 NO .
#
""")

        out = _get_dumper_output(radii_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_fret_forster_radius.id
_flr_fret_forster_radius.donor_probe_id
_flr_fret_forster_radius.acceptor_probe_id
_flr_fret_forster_radius.forster_radius
_flr_fret_forster_radius.reduced_forster_radius
1 1 2 52.000 53.200
#
""")

        out = _get_dumper_output(parameters_dumper, system)
        self.assertEqual(out, """#
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
1 0.350 2.400 . 0.400 . . . 0.800
2 0.350 2.400 . 0.380 . . . 0.800
#
""")

        out = _get_dumper_output(ref_measurement_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_reference_measurement_group.id
_flr_reference_measurement_group.num_measurements
_flr_reference_measurement_group.details
1 1 'Reference measurement group 1'
#
#
loop_
_flr_reference_measurement_group_link.group_id
_flr_reference_measurement_group_link.reference_measurement_id
1 1
#
#
loop_
_flr_reference_measurement.id
_flr_reference_measurement.reference_sample_probe_id
_flr_reference_measurement.num_species
_flr_reference_measurement.details
1 5 2 'Reference Measurement 1'
#
#
loop_
_flr_reference_measurement_lifetime.ordinal_id
_flr_reference_measurement_lifetime.reference_measurement_id
_flr_reference_measurement_lifetime.species_name
_flr_reference_measurement_lifetime.species_fraction
_flr_reference_measurement_lifetime.lifetime
1 1 . 0.600 3.200
2 1 . 0.400 1.400
#
""")

        out = _get_dumper_output(lifetime_fit_model_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_lifetime_fit_model.id
_flr_lifetime_fit_model.name
_flr_lifetime_fit_model.description
_flr_lifetime_fit_model.external_file_id
_flr_lifetime_fit_model.citation_id
1 'Lifetime fit model 1' 'Description of model' . .
#
""")

        out = _get_dumper_output(analysis_dumper, system)
        self.assertEqual(out, """#
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
1 1 intensity-based 1 2 1 1 . .
2 1 intensity-based 3 4 1 1 . .
3 1 lifetime-based 1 2 1 1 . .
#
#
loop_
_flr_fret_analysis_intensity.ordinal_id
_flr_fret_analysis_intensity.analysis_id
_flr_fret_analysis_intensity.calibration_parameters_id
_flr_fret_analysis_intensity.donor_only_fraction
_flr_fret_analysis_intensity.chi_square_reduced
_flr_fret_analysis_intensity.method_name
_flr_fret_analysis_intensity.details
1 1 1 . 1.500 PDA .
2 2 2 . 1.800 PDA .
#
#
loop_
_flr_fret_analysis_lifetime.ordinal_id
_flr_fret_analysis_lifetime.analysis_id
_flr_fret_analysis_lifetime.reference_measurement_group_id
_flr_fret_analysis_lifetime.lifetime_fit_model_id
_flr_fret_analysis_lifetime.donor_only_fraction
_flr_fret_analysis_lifetime.chi_square_reduced
_flr_fret_analysis_lifetime.method_name
_flr_fret_analysis_lifetime.details
1 3 1 1 . 1.600 'Lifetime fit' .
#
""")

        out = _get_dumper_output(peak_assignment_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_peak_assignment.id
_flr_peak_assignment.method_name
_flr_peak_assignment.details
1 Population 'Peaks were assigned by population fractions.'
#
""")

        out = _get_dumper_output(distance_restraint_dumper, system)
        self.assertEqual(out, """#
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
1 1 1 1 2 1 1 53.500 2.500 2.300 <R_DA>_E 0.800 1
2 2 1 3 4 1 2 49.000 2.000 2.100 <R_DA>_E 0.800 1
3 3 1 1 2 1 3 53.500 2.500 2.300 <R_DA>_E 0.800 1
#
""")

        out = _get_dumper_output(model_quality_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_fret_model_quality.model_id
_flr_fret_model_quality.chi_square_reduced
_flr_fret_model_quality.dataset_group_id
_flr_fret_model_quality.method
_flr_fret_model_quality.details
1 1.300 1 . .
2 1.900 1 . .
#
""")

        out = _get_dumper_output(model_distance_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_fret_model_distance.id
_flr_fret_model_distance.restraint_id
_flr_fret_model_distance.model_id
_flr_fret_model_distance.distance
_flr_fret_model_distance.distance_deviation
1 1 1 52.000 1.500
2 2 1 50.000 -1.000
3 1 2 53.800 -0.300
4 2 2 49.400 -0.400
#
""")

        out = _get_dumper_output(fps_modeling_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_FPS_modeling.id
_flr_FPS_modeling.ihm_modeling_protocol_ordinal_id
_flr_FPS_modeling.restraint_group_id
_flr_FPS_modeling.global_parameter_id
_flr_FPS_modeling.probe_modeling_method
_flr_FPS_modeling.details
1 1 1 1 AV3 .
2 1 1 1 MPP .
#
#
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
#
""")

        out = _get_dumper_output(av_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_FPS_AV_parameter.id
_flr_FPS_AV_parameter.num_linker_atoms
_flr_FPS_AV_parameter.linker_length
_flr_FPS_AV_parameter.linker_width
_flr_FPS_AV_parameter.probe_radius_1
_flr_FPS_AV_parameter.probe_radius_2
_flr_FPS_AV_parameter.probe_radius_3
1 15 20.000 3.500 10.000 5.000 3.500
#
#
loop_
_flr_FPS_AV_modeling.id
_flr_FPS_AV_modeling.sample_probe_id
_flr_FPS_AV_modeling.FPS_modeling_id
_flr_FPS_AV_modeling.parameter_id
1 1 1 1
2 3 1 1
#
""")

        out = _get_dumper_output(mpp_dumper, system)
        self.assertEqual(out, """#
loop_
_flr_FPS_mean_probe_position.id
_flr_FPS_mean_probe_position.sample_probe_id
_flr_FPS_mean_probe_position.mpp_xcoord
_flr_FPS_mean_probe_position.mpp_ycoord
_flr_FPS_mean_probe_position.mpp_zcoord
1 2 1.000 2.000 3.000
2 4 1.000 2.000 3.000
#
#
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
1 1 1 ALA CA C 1.000 1.000 1.000 1
2 1 2 GLY CA C 2.000 2.000 2.000 1
#
#
loop_
_flr_FPS_MPP_modeling.ordinal_id
_flr_FPS_MPP_modeling.FPS_modeling_id
_flr_FPS_MPP_modeling.mpp_id
_flr_FPS_MPP_modeling.mpp_atom_position_group_id
1 2 1 1
2 2 2 1
#
""")

    def test_variant_base(self):
        """Test Variant base class"""
        v = ihm.dumper.Variant()
        self.assertIsNone(v.get_dumpers())
        self.assertEqual(
            v.get_system_writer('system', 'writer_class', 'writer'), 'writer')

    def test_write_variant(self):
        """Test write() function with Variant object"""
        sys1 = ihm.System(id='system1')
        fh = StringIO()
        ihm.dumper.write(fh, [sys1], variant=ihm.dumper.IHMVariant())

    def test_ignore_writer(self):
        """Test _IgnoreWriter utility class"""
        class BaseWriter(object):
            def flush(self):
                return 'flush called'

            def write_comment(self, comment):
                return 'write comment ' + comment

        s = ihm.dumper._IgnoreWriter(BaseWriter(), [])
        # These methods are not usually called in ordinary operation, but
        # we should provide them for Writer compatibility
        self.assertEqual(s.flush(), 'flush called')
        self.assertEqual(s.write_comment('foo'), 'write comment foo')

    def test_write_ignore_variant(self):
        """Test write() function with IgnoreVariant object"""
        sys1 = ihm.System(id='system1')
        fh = StringIO()
        ihm.dumper.write(fh, [sys1])
        self.assertIn('_ihm_struct_assembly', fh.getvalue())
        # Test exclude of ihm_struct_assembly category
        fh = StringIO()
        ihm.dumper.write(
            fh, [sys1],
            variant=ihm.dumper.IgnoreVariant(['_ihm_struct_assembly']))
        self.assertNotIn('_ihm_struct_assembly', fh.getvalue())
        # Should be case-insensitive and tolerant of missing underscore
        fh = StringIO()
        ihm.dumper.write(
            fh, [sys1],
            variant=ihm.dumper.IgnoreVariant(['IHM_STRUCT_ASSEMBLY',
                                              'AUDIT_CONFORM']))
        self.assertNotIn('_ihm_struct_assembly', fh.getvalue())


if __name__ == '__main__':
    unittest.main()
