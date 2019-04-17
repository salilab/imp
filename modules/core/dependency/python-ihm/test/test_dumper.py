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
import ihm.geometry
import ihm.source

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
        self.assertEqual(lines[14:16],
                         ["data_system23", "_entry.id 'system 2+3'"])

    def test_write_custom_dumper(self):
        """Test write() function with custom dumper"""
        class MyDumper(ihm.dumper.Dumper):
            def dump(self, system, writer):
                with writer.category("_custom_category") as l:
                    l.write(myfield="foo", field2="bar")
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
        system = ihm.System(title='test model')
        dumper = ihm.dumper._StructDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """_struct.entry_id model
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
        self.assertEqual(out[b'dataBlocks'], [])

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
        # Duplicate should be removed
        system.software.append(ihm.Software(
                          name='foo', classification='x',
                          description='y', location='z'))
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
1 test 'test code' 'Some test program' 1 program http://test.org
2 foo 'test code' 'Other test program' . program http://test2.org
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
              journal="Mol Cell Proteomics", volume=13, page_range=(2927,2943),
              year=2014,
              authors=['Shi Y', 'Fernandez-Martinez J', 'Tjioe E', 'Pellarin R',
                       'Kim SJ', 'Williams R', 'Schneidman-Duhovny D', 'Sali A',
                       'Rout MP', 'Chait BT'],
              doi='10.1074/mcp.M114.041673')
        system.citations.extend((c1, c1)) # duplicates should be removed
        dumper = ihm.dumper._CitationDumper()
        dumper.finalize(system) # Assign IDs
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
        out = _get_dumper_output(dumper, system)
        self.assertTrue("'Mol Cell Proteomics' 13 e1637 . 2014 " in out)

    def test_audit_author_empty(self):
        """Test AuditAuthorDumper with empty list"""
        system = ihm.System()

        c1 = ihm.Citation(pmid='25161197', title='foo',
              journal="Mol Cell Proteomics", volume=13, page_range=(2927,2943),
              year=2014, authors=['auth1', 'auth2', 'auth3'], doi='doi1')
        c2 = ihm.Citation(pmid='45161197', title='bar',
              journal="Mol Cell Proteomics", volume=13, page_range=(2927,2943),
              year=2014, authors=['auth2', 'auth4'], doi='doi2')
        system.citations.extend((c1, c2))

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
        dumper.finalize(system) # Assign IDs
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
        dumper.finalize(system) # Assign IDs
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
        dumper.finalize(system) # Assign IDs
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
2 1 1234 'Test latin name' 'Test common name' 'test strain' 5678
'Other latin name' 'Other common name' 'other strain'
#
""")

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

    def test_chem_descriptor_dumper(self):
        """Test ChemDescriptorDumper"""
        system = ihm.System()
        d1 = ihm.ChemDescriptor('EDC', smiles='CCN=C=NCCCN(C)C',
                                inchi_key='LMDZBCPBFSXMTL-UHFFFAOYSA-N')
        system.orphan_chem_descriptors.append(d1)
        dumper = ihm.dumper._ChemDescriptorDumper()
        dumper.finalize(system) # Assign descriptor IDs
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_chemical_descriptor.id
_ihm_chemical_descriptor.auth_name
_ihm_chemical_descriptor.chem_comp_id
_ihm_chemical_descriptor.chemical_name
_ihm_chemical_descriptor.common_name
_ihm_chemical_descriptor.smiles
_ihm_chemical_descriptor.smiles_canonical
_ihm_chemical_descriptor.inchi
_ihm_chemical_descriptor.inchi_key
1 EDC . . . CCN=C=NCCCN(C)C . . LMDZBCPBFSXMTL-UHFFFAOYSA-N
#
""")

    def test_entity_poly_dumper(self):
        """Test EntityPolyDumper"""
        system = ihm.System()
        e1 = ihm.Entity('ACGT') # sequence containing glycine
        e2 = ihm.Entity(('A', 'C', 'C', 'MSE'))  # no glycine
        # All D-peptides (with glycine)
        e3 = ihm.Entity(('DAL', 'DCY', 'G'), alphabet=ihm.DPeptideAlphabet)
        # All D-peptides (without glycine)
        e4 = ihm.Entity(('DAL', 'DCY'), alphabet=ihm.DPeptideAlphabet)
        # Mix of L- and D-peptides
        dpep_al = ihm.DPeptideAlphabet()
        e5 = ihm.Entity(('A', dpep_al['DCY'], 'G'))
        # Non-polymeric entity
        e6 = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        system.entities.extend((e1, e2, e3, e4, e5, e6))
        # One protein entity is modeled (with an asym unit) the other not;
        # this should be reflected in pdbx_strand_id
        system.asym_units.append(ihm.AsymUnit(e1, 'foo'))
        system.asym_units.append(ihm.AsymUnit(e1, 'bar'))

        rna = ihm.Entity('AC', alphabet=ihm.RNAAlphabet)
        dna = ihm.Entity(('DA', 'DC'), alphabet=ihm.DNAAlphabet)
        hybrid = ihm.Entity(rna.sequence + dna.sequence)
        system.entities.extend((rna, dna, hybrid))

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
2 polypeptide(L) no no . ACC(MSE) ACCM
3 polypeptide(D) no no . (DAL)(DCY)G ACG
4 polypeptide(D) no no . (DAL)(DCY) AC
5 polypeptide(L) no no . A(DCY)G ACG
7 polyribonucleotide no no . AC AC
8 polydeoxyribonucleotide no no . (DA)(DC) AC
9 'polydeoxyribonucleotide/polyribonucleotide hybrid' no no . AC(DA)(DC) ACAC
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
        ed.finalize(system) # Assign entity IDs
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
        system.asym_units.append(ihm.AsymUnit(e4, 'test'))
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
A 1 1 ALA 1 1 ALA ALA A
A 1 2 CYS 2 2 CYS CYS A
A 1 3 GLY 3 3 GLY GLY A
A 1 4 THR 4 4 THR THR A
B 2 1 ALA 6 6 ALA ALA B
B 2 2 CYS 7 7 CYS CYS B
B 2 3 CYS 8 8 CYS CYS B
C 3 1 A 1 1 A A C
C 3 2 C 2 2 C C C
D 4 1 DA 1 1 DA DA D
D 4 2 DC 2 2 DC DC D
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
        system.asym_units.append(ihm.AsymUnit(e2, 'baz'))
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
_pdbx_nonpoly_scheme.pdb_seq_num
_pdbx_nonpoly_scheme.auth_seq_num
_pdbx_nonpoly_scheme.pdb_mon_id
_pdbx_nonpoly_scheme.auth_mon_id
_pdbx_nonpoly_scheme.pdb_strand_id
B 2 HEM 1 1 HEM HEM B
C 3 ZN 6 6 ZN ZN C
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
        dumper.finalize(system) # assign IDs
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
        e1 = ihm.Entity('AAA', description='foo')
        e2 = ihm.Entity('AA', description='baz')
        a1 = ihm.AsymUnit(e1)
        a2 = ihm.AsymUnit(e1)
        a3 = ihm.AsymUnit(e2)
        system.entities.extend((e1, e2))
        system.asym_units.extend((a1, a2, a3))

        system.orphan_assemblies.append(ihm.Assembly((a1, a2(2,3)), name='foo'))
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

        # Assign entity and asym IDs
        ihm.dumper._EntityDumper().finalize(system)
        ihm.dumper._StructAsymDumper().finalize(system)

        d = ihm.dumper._AssemblyDumper()
        d.finalize(system)
        self.assertEqual(system.complete_assembly._id, 1)
        self.assertEqual([a._id for a in system.orphan_assemblies], [2,3,3,3,3])
        out = _get_dumper_output(d, system)
        self.assertEqual(out, """#
loop_
_ihm_struct_assembly_details.assembly_id
_ihm_struct_assembly_details.assembly_name
_ihm_struct_assembly_details.assembly_description
1 'Complete assembly' 'All known components'
2 foo .
3 bar 'desc1 & desc2'
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
            self.assertEqual(len(d._ref_by_id), 5)
            self.assertEqual(len(d._repo_by_id), 4)
            # Repeated calls to finalize() should yield identical results
            d.finalize(system)
            self.assertEqual(len(d._ref_by_id), 5)
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
        system.orphan_datasets.append(ds1)
        # A duplicate dataset should be ignored even if details differ
        l = ihm.location.PDBLocation('1abc', '1.0', 'other details')
        ds2 = ihm.dataset.PDBDataset(l)
        system.orphan_datasets.append(ds2)
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
        system.orphan_datasets.extend((cx1, cx2))
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
        system.orphan_datasets.extend((cx1, cx2))
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
        system.orphan_datasets.extend((cx2, em3d))
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
        system.orphan_datasets.extend((em3d_1, em3d_2))
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

        system.orphan_datasets.append(ds1)
        system.orphan_dataset_groups.extend((group1, group2))

        d = ihm.dumper._DatasetDumper()
        d.finalize(system) # Assign IDs
        self.assertEqual(len(d._dataset_by_id), 1)
        self.assertEqual(len(d._dataset_group_by_id), 1)

        # Repeated calls to finalize should yield identical results
        d.finalize(system)
        self.assertEqual(len(d._dataset_by_id), 1)
        self.assertEqual(len(d._dataset_group_by_id), 1)

    def test_dataset_dumper_dump(self):
        """Test DatasetDumper.dump()"""
        system = ihm.System()
        l = ihm.location.InputFileLocation(repo='foo', path='bar')
        l._id = 97
        ds1 = ihm.dataset.CXMSDataset(l)
        system.orphan_datasets.append(ds1)

        # group1 contains just the first dataset (but duplicated)
        group1 = ihm.dataset.DatasetGroup([ds1, ds1])
        system.orphan_dataset_groups.append(group1)

        l = ihm.location.InputFileLocation(repo='foo2', path='bar2')
        l._id = 98
        ds2 = ihm.dataset.CXMSDataset(l)

        # group2 contains all datasets so far (ds1 & ds2)
        group2 = ihm.dataset.DatasetGroup([ds1, ds2])
        system.orphan_dataset_groups.append(group2)

        l = ihm.location.PDBLocation('1abc', '1.0', 'test details')
        ds3 = ihm.dataset.PDBDataset(l)
        system.orphan_datasets.append(ds3)
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
        system.orphan_representations.extend((r1, r2))

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
        l = ihm.location.PDBLocation('1abc', '1.0', 'test details')
        dstemplate = ihm.dataset.PDBDataset(l)
        l = ihm.location.PDBLocation('2xyz', '1.0', 'test details')
        dstarget = ihm.dataset.PDBDataset(l)
        ali = ihm.location.InputFileLocation(repo='foo', path='test.ali')
        script = ihm.location.WorkflowFileLocation(repo='foo', path='test.py')
        software = ihm.Software(name='test', classification='test code',
                                description='Some test program',
                                version=1, location='http://test.org')

        s1 = ihm.startmodel.Template(dataset=dstemplate, asym_id='C',
                             seq_id_range=(-9,0), # 1,10 in IHM numbering
                             template_seq_id_range=(101,110),
                             sequence_identity=30.)
        s2 = ihm.startmodel.Template(dataset=dstemplate, asym_id='D',
                             seq_id_range=(-5,2), # 5,12 in IHM numbering
                             template_seq_id_range=(201,210),
                             sequence_identity=40.,
                             alignment_file=ali)

        sm = TestStartingModel(asym(1,15), dstarget, 'A', [s1, s2], offset=10,
                               script_file=script, software=software)
        system.orphan_starting_models.append(sm)

        sm = TestStartingModel(asym(1,15), dstarget, 'A', [])
        system.orphan_starting_models.append(sm)

        e1._id = 42
        asym._id = 99
        dstemplate._id = 101
        dstarget._id = 102
        ali._id = 5
        script._id = 8
        software._id = 99
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
1 42 foo 99 1 12 'experimental model' A 10 102
2 42 foo 99 1 15 'experimental model' A 0 102
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
1 1 A 1 10 C 101 110 30.000 1 101 .
2 1 A 5 12 D 201 210 40.000 1 101 5
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
        p1.steps.append(ihm.protocol.Step(assembly=assembly, dataset_group=dsg,
                               method='Monte Carlo', num_models_begin=0,
                               num_models_end=500, multi_scale=True, name='s1'))
        p1.steps.append(ihm.protocol.Step(assembly=assembly, dataset_group=dsg,
                               method='Replica exchange', num_models_begin=500,
                               num_models_end=2000, multi_scale=True))
        system.orphan_protocols.append(p1)

        p2 = ihm.protocol.Protocol('sampling')
        p2.steps.append(ihm.protocol.Step(assembly=assembly, dataset_group=dsg2,
                               method='Replica exchange', num_models_begin=2000,
                               num_models_end=1000, multi_scale=True,
                               software=software, script_file=script))
        system.orphan_protocols.append(p2)

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
_ihm_modeling_protocol.ordered_flag
_ihm_modeling_protocol.software_id
_ihm_modeling_protocol.script_file_id
1 1 1 42 99 foo equilibration s1 'Monte Carlo' 0 500 YES NO NO . .
2 1 2 42 99 foo equilibration . 'Replica exchange' 500 2000 YES NO NO . .
3 2 1 42 101 foo sampling . 'Replica exchange' 2000 1000 YES NO NO 80 90
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
                             feature='RMSD', num_models_begin=200,
                             num_models_end=42))
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
                             num_models_end=42,
                             assembly=asmb1, dataset_group=dg1,
                             software=software, script_file=script))
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
_ihm_modeling_post_process.struct_assembly_id
_ihm_modeling_post_process.dataset_group_id
_ihm_modeling_post_process.software_id
_ihm_modeling_post_process.script_file_id
1 1 1 1 none none . . . . . .
2 1 2 1 filter energy/score 1000 200 . . . .
3 1 2 2 cluster RMSD 200 42 . . . .
4 1 2 3 validation energy/score 42 42 101 301 401 501
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
        model.assembly.append(asym(1,2))
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
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        # Atom is not OK (bad asym)
        atom = ihm.model.Atom(asym_unit=asym2, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Sphere is not OK (bad asym)
        sphere = ihm.model.Sphere(asym_unit=asym2, seq_id_range=(1,2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

    def test_range_checker_asmb_seq_id(self):
        """Test RangeChecker class checking assembly seq_id range"""
        system, model, asym = self._make_test_model()
        # Only part of asym is in the assembly
        asmb = ihm.Assembly([asym(1,2)])
        model.assembly = asmb

        # Everything is represented
        s = ihm.representation.AtomicSegment(asym, rigid=True)
        model.representation.append(s)
        s = ihm.representation.FeatureSegment(asym, rigid=False,
                                              primitive='sphere', count=2)
        model.representation.append(s)

        rngcheck = ihm.dumper._RangeChecker(model)
        self.assertEqual(rngcheck._last_asmb_range_matched, None)
        self.assertEqual(rngcheck._last_asmb_asym_matched, None)
        # Atom is OK (good range)
        atom = ihm.model.Atom(asym_unit=asym, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)
        # Cache should now be set
        self.assertEqual(
                rngcheck._last_asmb_range_matched, (1,2))
        self.assertEqual(
                rngcheck._last_asmb_asym_matched, 'X')
        # 2nd check should use the cache
        rngcheck(atom)
        # Sphere is OK (good range)
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        # Atom is not OK (bad range)
        atom = ihm.model.Atom(asym_unit=asym, seq_id=10, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Sphere is not OK (bad range)
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,10),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

    def test_range_checker_repr_asym(self):
        """Test RangeChecker class checking representation asym ID match"""
        system, model, asym = self._make_test_model()
        asym2 = ihm.AsymUnit(asym.entity, 'bar')
        asym2._id = 'Y'
        system.asym_units.append(asym2)
        model.assembly.append(asym2)

        # Add multiple representation segments for asym
        s = ihm.representation.AtomicSegment(asym(1,2), rigid=True)
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
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        # Atom is not OK (bad asym)
        atom = ihm.model.Atom(asym_unit=asym2, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Sphere is not OK (bad asym)
        sphere = ihm.model.Sphere(asym_unit=asym2, seq_id_range=(1,2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

    def test_range_checker_repr_seq_id(self):
        """Test RangeChecker class checking representation seq_id range"""
        system, model, asym = self._make_test_model()
        asym2 = ihm.AsymUnit(asym.entity, 'bar')
        asym2._id = 'Y'
        system.asym_units.append(asym2)
        model.assembly.append(asym2)

        # Add multiple representation segments for asym2
        s = ihm.representation.AtomicSegment(asym2(1,2), rigid=True)
        model.representation.append(s)
        s = ihm.representation.FeatureSegment(asym2(1,2), rigid=False,
                                              primitive='sphere', count=2)
        model.representation.append(s)

        rngcheck = ihm.dumper._RangeChecker(model)
        self.assertEqual(rngcheck._last_repr_segment_matched, None)
        # Atom is OK (good range)
        atom = ihm.model.Atom(asym_unit=asym2, seq_id=1, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        rngcheck(atom)
        # Cache should now be set
        self.assertEqual(
                rngcheck._last_repr_segment_matched.asym_unit.seq_id_range,
                (1,2))
        # 2nd check should use the cache
        rngcheck(atom)
        # Sphere is OK (good range)
        sphere = ihm.model.Sphere(asym_unit=asym2, seq_id_range=(1,2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        # Atom is not OK (bad range)
        atom = ihm.model.Atom(asym_unit=asym2, seq_id=4, atom_id='C',
                              type_symbol='C', x=1.0, y=2.0, z=3.0)
        self.assertRaises(ValueError, rngcheck, atom)

        # Sphere is not OK (bad range)
        sphere = ihm.model.Sphere(asym_unit=asym2, seq_id_range=(1,4),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

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
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,1),
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
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        self.assertRaises(ValueError, rngcheck, sphere)

        # Single-residue Sphere is OK
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,1),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

    def test_range_checker_repr_type_multi_residue(self):
        """Test RangeChecker class type checking against MultiResidueSegments"""
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
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,4),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,2),
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
        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,2),
                                  x=1.0, y=2.0, z=3.0, radius=4.0)
        rngcheck(sphere)

        sphere = ihm.model.Sphere(asym_unit=asym, seq_id_range=(1,20),
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
                                           seq_id_range=(1,3), x=1.0,
                                           y=2.0, z=3.0, radius=4.0),
                          ihm.model.Sphere(asym_unit=asym,
                                           seq_id_range=(4,4), x=4.0,
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
                                       biso=42.0)]

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
ATOM 1 C C . ALA 1 X 1.000 2.000 3.000 9 X . 1 1
HETATM 2 C CA . ALA 1 X 10.000 20.000 30.000 9 X . 1 1
ATOM 3 N N . CYS 2 X 4.000 5.000 6.000 9 X 42.000 1 1
#
#
loop_
_atom_type.symbol
C
N
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
        e1 = ihm.Entity('AHCD')
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
        r = ihm.restraint.EM3DRestraint(dataset=dataset, assembly=assembly,
                       segment=False, fitting_method='Gaussian mixture model',
                       number_of_gaussians=40, details='GMM fitting')
        r2 = ihm.restraint.EM3DRestraint(dataset=dataset2, assembly=assembly,
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
        dumper.finalize(system) # assign IDs

        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_ihm_3dem_restraint.ordinal_id
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
        xxl1 = ihm.restraint.ExperimentalCrossLink(e1.residue(2), e1.residue(3))
        # inter, ambiguous
        xxl2 = ihm.restraint.ExperimentalCrossLink(e1.residue(2), e2.residue(3))
        xxl3 = ihm.restraint.ExperimentalCrossLink(e1.residue(2), e2.residue(2))
        # duplicate crosslink, should be combined with the original (xxl2)
        xxl4 = ihm.restraint.ExperimentalCrossLink(e1.residue(2), e2.residue(3))
        # should end up in own group, not with xxl4 (since xxl4==xxl2)
        xxl5 = ihm.restraint.ExperimentalCrossLink(e1.residue(1), e2.residue(1))
        r.experimental_cross_links.extend(([xxl1], [xxl2, xxl3], [xxl4, xxl5]))
        system.restraints.extend((r, MockObject()))

        d = ihm.restraint.UpperBoundDistanceRestraint(25.0)
        xl1 = ihm.restraint.ResidueCrossLink(xxl1, asym1, asym1, d,
                                psi=0.5, sigma1=1.0, sigma2=2.0,
                                restrain_all=True)
        d = ihm.restraint.LowerBoundDistanceRestraint(34.0)
        xl2 = ihm.restraint.AtomCrossLink(xxl3, asym1, asym2, 'C', 'N', d,
                                restrain_all=False)
        # Duplicates should be ignored
        xl3 = ihm.restraint.AtomCrossLink(xxl3, asym1, asym2, 'C', 'N', d,
                                restrain_all=False)
        r.cross_links.extend((xl1, xl2, xl3))

        model = MockObject()
        model._id = 201
        xl1.fits[model] = ihm.restraint.CrossLinkFit(psi=0.1, sigma1=4.2,
                                                     sigma2=2.1)

        ihm.dumper._EntityDumper().finalize(system) # assign entity IDs
        ihm.dumper._StructAsymDumper().finalize(system) # assign asym IDs
        ihm.dumper._ChemDescriptorDumper().finalize(system) # descriptor IDs
        dumper = ihm.dumper._CrossLinkDumper()
        dumper.finalize(system) # assign IDs

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
_ihm_cross_link_list.linker_descriptor_id
_ihm_cross_link_list.linker_type
_ihm_cross_link_list.dataset_list_id
1 1 foo 1 2 THR foo 1 3 CYS 1 DSS 97
2 2 foo 1 2 THR bar 2 3 PHE 1 DSS 97
3 2 foo 1 2 THR bar 2 2 GLU 1 DSS 97
4 3 foo 1 1 ALA bar 2 1 ASP 1 DSS 97
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
1 1 1 A 2 THR 1 A 3 CYS . . 'upper bound' ALL by-residue 25.000 0.500 1.000
2.000
2 3 1 A 2 THR 2 B 2 GLU C N 'lower bound' ANY by-atom 34.000 . . .
#
#
loop_
_ihm_cross_link_result_parameters.ordinal_id
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
        center = ihm.geometry.Center(1.,2.,3.)
        trans = ihm.geometry.Transformation([[1,0,0],[0,1,0],[0,0,1]],
                                            [1.,2.,3.])

        sphere = ihm.geometry.Sphere(center=center, transformation=trans,
                                     radius=2.2, name='my sphere',
                                     description='a test sphere',
                                     details='some details')
        torus = ihm.geometry.Torus(center=center, transformation=trans,
                                   major_radius=5.6, minor_radius=1.2)
        half_torus = ihm.geometry.HalfTorus(center=center, transformation=trans,
                                            major_radius=5.6, minor_radius=1.2,
                                            thickness=0.1, inner=True)
        axis = ihm.geometry.XAxis()
        plane = ihm.geometry.XYPlane()

        system.orphan_geometric_objects.extend((sphere, torus, half_torus,
                                                axis, plane))

        dumper = ihm.dumper._GeometricObjectDumper()
        dumper.finalize(system) # assign IDs
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
_ihm_geometric_object_list.other_details
1 sphere 'my sphere' 'a test sphere' 'some details'
2 torus . . .
3 half-torus . . .
4 axis . . .
5 plane . . .
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

        f = ihm.restraint.ResidueFeature([a1, a2(2,3)])
        system.orphan_features.append(f)
        # Cannot make a ResidueFeature that includes a non-polymer 'residue'
        self.assertRaises(ValueError, ihm.restraint.ResidueFeature, [a1, a3])

        # Polymeric atom feature
        f = ihm.restraint.AtomFeature([a1.residue(1).atom('CA'),
                                       a2.residue(2).atom('N')])
        system.orphan_features.append(f)
        # Nonpolymeric atom feature
        f = ihm.restraint.AtomFeature([a3.residue(1).atom('FE')])
        system.orphan_features.append(f)
        # Cannot make one feature that selects both polymer and nonpolymer
        self.assertRaises(ValueError, ihm.restraint.AtomFeature,
                                      [a1.residue(1).atom('CA'),
                                       a2.residue(2).atom('N'),
                                       a3.residue(1).atom('FE')])
        # Nonpolymeric feature
        f = ihm.restraint.NonPolyFeature([a3])
        system.orphan_features.append(f)
        # Cannot make a NonPolyFeature that includes a polymer 'residue'
        self.assertRaises(ValueError, ihm.restraint.NonPolyFeature, [a1, a3])

        # Pseudo site feature
        f = ihm.restraint.PseudoSiteFeature(x=10., y=20., z=30.)
        system.orphan_features.append(f)

        ihm.dumper._EntityDumper().finalize(system) # assign entity IDs
        ihm.dumper._StructAsymDumper().finalize(system) # assign asym IDs

        dumper = ihm.dumper._FeatureDumper()
        dumper.finalize(system) # assign IDs
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
1 'residue range' polymer
2 atom polymer
3 atom non-polymer
4 ligand non-polymer
5 'pseudo site' other
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
2 4 2 C HEM .
#
#
loop_
_ihm_pseudo_site_feature.feature_id
_ihm_pseudo_site_feature.Cartn_x
_ihm_pseudo_site_feature.Cartn_y
_ihm_pseudo_site_feature.Cartn_z
_ihm_pseudo_site_feature.radius
_ihm_pseudo_site_feature.description
5 10.000 20.000 30.000 . .
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
        r = ihm.restraint.CenterGeometricRestraint(dataset=dataset,
                geometric_object=geom, feature=feat, distance=dist,
                harmonic_force_constant=2.0, restrain_all=False)
        system.restraints.append(r)

        dumper = ihm.dumper._GeometricRestraintDumper()
        dumper.finalize(system) # assign IDs

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
        r1 = ihm.restraint.DerivedDistanceRestraint(dataset=dataset,
                 feature1=feat1, feature2=feat2, distance=dist,
                 probability=0.8)
        r2 = ihm.restraint.DerivedDistanceRestraint(dataset=dataset,
                 feature1=feat1, feature2=feat2, distance=dist,
                 probability=0.4)
        r3 = ihm.restraint.DerivedDistanceRestraint(dataset=dataset,
                 feature1=feat1, feature2=feat2, distance=dist,
                 probability=0.6)
        rg = ihm.restraint.RestraintGroup((r2, r3))
        system.restraints.extend((r1, r2)) # r2 is in restraints and groups
        system.restraint_groups.append(rg)

        dumper = ihm.dumper._DerivedDistanceRestraintDumper()
        dumper.finalize(system) # assign IDs

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
_ihm_derived_distance_restraint.group_conditionality
_ihm_derived_distance_restraint.dataset_list_id
1 . 44 84 'lower bound' 25.000 . 0.800 . 97
2 1 44 84 'lower bound' 25.000 . 0.400 . 97
3 1 44 84 'lower bound' 25.000 . 0.600 . 97
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
        r1 = ihm.restraint.DerivedDistanceRestraint(dataset=dataset,
                 feature1=feat1, feature2=feat2, distance=dist,
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
        assembly = MockObject()

        # Empty restraint groups are OK (even though they don't get IDs)
        rg = ihm.restraint.RestraintGroup([])
        s.restraint_groups.append(rg)
        fh = StringIO()
        ihm.dumper.write(fh, [s])

        r = ihm.restraint.SASRestraint(dataset=dataset, assembly=assembly,
                       segment=False, fitting_method='FoXS',
                       fitting_atom_type='Heavy atoms',
                       multi_state=False,
                       radius_of_gyration=21.07, details='FoXS fitting')

        rg = ihm.restraint.RestraintGroup([r])
        s.restraint_groups.append(rg)

        fh = StringIO()
        # SASRestraint is an unsupported type in RestraintGroup
        self.assertRaises(TypeError, ihm.dumper.write, fh, [s])


if __name__ == '__main__':
    unittest.main()
