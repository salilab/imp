from __future__ import print_function
import IMP.test
import IMP.pmi.metadata
import IMP.pmi.representation
import IMP.pmi.mmcif
import IMP.pmi.macros
import sys
import os
import io
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

class EmptyObject(object):
    pass

class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
    def flush(self):
        pass

def make_dataset_dumper():
    """Make an empty DatasetDumper object."""
    class MockExtRef(IMP.pmi.mmcif._ExternalReferenceDumper):
        def finalize_after_datasets(self):
            pass
    simo = EmptyObject()
    simo.extref_dump = MockExtRef(simo)
    return IMP.pmi.mmcif._DatasetDumper(simo), simo

class Tests(IMP.test.TestCase):

    def test_software(self):
        """Test SoftwareDumper"""
        s = IMP.pmi.metadata.Software(name='test', classification='test code',
                                      description='Some test program',
                                      version=1, url='http://salilab.org')
        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)
        r.add_metadata(s)
        r.add_metadata(IMP.pmi.metadata.Repository(doi="foo", root='.'))
        d = IMP.pmi.mmcif._SoftwareDumper(r)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue().split('\n')
        self.assertEqual(out[-3],
                         "3 test 'test code' 1 program http://salilab.org")

    def test_software_modeller(self):
        """Test SoftwareDumper.set_modeller_used"""
        d = IMP.pmi.mmcif._SoftwareDumper(EmptyObject())
        self.assertEqual(d.modeller_used, False)
        d.set_modeller_used('9.18', '2018-01-01')
        self.assertEqual(d.modeller_used, True)
        self.assertEqual(len(d.software), 3)
        self.assertEqual(d.software[-1].version, '9.18')
        # Further calls should have no effect
        d.set_modeller_used('9.0', 'xxx')
        self.assertEqual(len(d.software), 3)
        self.assertEqual(d.software[-1].version, '9.18')

    def test_software_phyre2(self):
        """Test SoftwareDumper.set_phyre2_used"""
        d = IMP.pmi.mmcif._SoftwareDumper(EmptyObject())
        self.assertEqual(d.phyre2_used, False)
        d.set_phyre2_used()
        self.assertEqual(d.phyre2_used, True)
        self.assertEqual(len(d.software), 3)
        self.assertEqual(d.software[-1].version, '2.0')
        # Further calls should have no effect
        d.set_phyre2_used()
        self.assertEqual(len(d.software), 3)

    def test_workflow(self):
        """Test output of workflow files"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        root = os.path.dirname(sys.argv[0]) or '.'
        simo.add_metadata(IMP.pmi.metadata.Repository(doi="foo", root=root))
        po = DummyPO(None)
        # Usually main_script is populated from sys.argv[0], which is usually
        # the name of the modeling script. But if we run the tests with nose,
        # it will be 'nosetests' instead. So make sure it's reliably the name
        # of the test script:
        po._main_script = __file__
        simo.add_protocol_output(po)

        r = IMP.pmi.metadata.Repository(doi="bar")
        l = IMP.pmi.metadata.FileLocation(repo=r,
                                          path=os.path.join('bar', 'baz'),
                                          details='foo')
        s = IMP.pmi.metadata.PythonScript(location=l)
        simo.add_metadata(s)

        d = IMP.pmi.mmcif._ExternalReferenceDumper(po)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.finalize_metadata()
        d.finalize_after_datasets()
        d.dump(w)
        self.assertEqual(fh.getvalue(), """#
loop_
_ihm_external_reference_info.reference_id
_ihm_external_reference_info.reference_provider
_ihm_external_reference_info.reference_type
_ihm_external_reference_info.reference
_ihm_external_reference_info.refers_to
_ihm_external_reference_info.associated_url
1 . DOI foo Other .
2 . DOI bar Other .
#
#
loop_
_ihm_external_files.id
_ihm_external_files.reference_id
_ihm_external_files.file_path
_ihm_external_files.content_type
_ihm_external_files.details
1 1 test_mmcif.py 'Modeling workflow or script'
'The main integrative modeling script'
2 2 bar/baz 'Modeling workflow or script' foo
#
""")

    def test_file_dataset(self):
        """Test get/set_file_dataset methods"""
        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='baz')
        d = IMP.pmi.metadata.EM2DClassDataset(l)
        r.set_file_dataset('foo', d)
        self.assertEqual(r.get_file_dataset('foo'), d)
        self.assertEqual(r._file_dataset, {os.path.abspath('foo'): d})
        self.assertEqual(r.get_file_dataset('foobar'), None)

    def test_assembly_dumper_get_subassembly(self):
        """Test AssemblyDumper.get_subassembly()"""
        po = DummyPO(EmptyObject())
        d = IMP.pmi.mmcif._AssemblyDumper(po)
        complete = IMP.pmi.mmcif._Assembly(['a', 'b', 'c'])
        d.add(complete)
        self.assertEqual(complete.id, 1)
        x = d.get_subassembly({'a':None, 'b':None})
        self.assertEqual(x.id, 2)
        self.assertEqual(x, ['a', 'b'])
        x = d.get_subassembly({'a':None, 'b':None, 'c':None})
        self.assertEqual(x.id, 1)

    def test_assembly_all_modeled(self):
        """Test AssemblyDumper, all components modeled"""
        po = DummyPO(EmptyObject())
        d = IMP.pmi.mmcif._AssemblyDumper(po)
        for c, seq in (("foo", "AAA"), ("bar", "AAA"), ("baz", "AA")):
            po.create_component(c, True)
            po.add_component_sequence(c, seq)
        d.add(IMP.pmi.mmcif._Assembly(["foo", "bar"]))
        d.add(IMP.pmi.mmcif._Assembly(["bar", "baz"]))

        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_struct_assembly.ordinal_id
_ihm_struct_assembly.assembly_id
_ihm_struct_assembly.entity_description
_ihm_struct_assembly.entity_id
_ihm_struct_assembly.asym_id
_ihm_struct_assembly.seq_id_begin
_ihm_struct_assembly.seq_id_end
1 1 foo 1 A 1 3
2 1 foo 1 B 1 3
3 2 foo 1 B 1 3
4 2 baz 2 C 1 2
#
""")

    def test_assembly_subset_modeled(self):
        """Test AssemblyDumper, subset of components modeled"""
        po = DummyPO(EmptyObject())
        for c, seq, modeled in (("foo", "AAA", True), ("bar", "AA", False)):
            po.create_component(c, modeled)
            po.add_component_sequence(c, seq)

        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        po.assembly_dump.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_struct_assembly.ordinal_id
_ihm_struct_assembly.assembly_id
_ihm_struct_assembly.entity_description
_ihm_struct_assembly.entity_id
_ihm_struct_assembly.asym_id
_ihm_struct_assembly.seq_id_begin
_ihm_struct_assembly.seq_id_end
1 1 foo 1 A 1 3
2 1 bar 2 . 1 2
3 2 foo 1 A 1 3
#
""")

    def test_struct_asym(self):
        """Test StructAsymDumper"""
        po = DummyPO(EmptyObject())
        d = IMP.pmi.mmcif._StructAsymDumper(po)
        for c, seq in (("foo", "AAA"), ("bar", "AAA"), ("baz", "AA")):
            po.create_component(c, True)
            po.add_component_sequence(c, seq)

        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
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

    def test_entry(self):
        """Test EntryDumper"""
        po = DummyPO(EmptyObject())
        d = IMP.pmi.mmcif._EntryDumper(po)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, "data_imp_model\n_entry.id imp_model\n")

    def test_audit_author(self):
        """Test AuditAuthorDumper"""
        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)

        s = IMP.pmi.metadata.Citation(pmid='25161197', title='foo',
              journal="Mol Cell Proteomics", volume=13, page_range=(2927,2943),
              year=2014, authors=['auth1', 'auth2', 'auth3'], doi='doi1')
        r.add_metadata(s)
        s = IMP.pmi.metadata.Citation(pmid='45161197', title='bar',
              journal="Mol Cell Proteomics", volume=13, page_range=(2927,2943),
              year=2014, authors=['auth2', 'auth4'], doi='doi2')
        r.add_metadata(s)

        d = IMP.pmi.mmcif._AuditAuthorDumper(r)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        # auth2 is repeated in the input; we should see it only once in the
        # output
        self.assertEqual(out,"""#
loop_
_audit_author.name
_audit_author.pdbx_ordinal
auth1 1
auth2 2
auth3 3
auth4 4
#
""")

    def test_entity_dumper(self):
        """Test EntityDumper"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        po.add_component_sequence('foo', 'ACGT')
        po.add_component_sequence('bar', 'ACGT')
        po.add_component_sequence('baz', 'ACC')
        d = IMP.pmi.mmcif._EntityDumper(po)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_entity.id
_entity.type
_entity.src_method
_entity.pdbx_description
_entity.formula_weight
_entity.pdbx_number_of_molecules
_entity.details
1 polymer man foo ? 1 ?
2 polymer man baz ? 1 ?
#
""")

    def test_entity_poly_dumper(self):
        """Test EntityPolyDumper"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        po.add_component_sequence('foo', 'ACGT')
        po.add_component_sequence('bar', 'ACGT')
        po.add_component_sequence('baz', 'ACC')
        d = IMP.pmi.mmcif._EntityPolyDumper(po)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_entity_poly.entity_id
_entity_poly.type
_entity_poly.nstd_linkage
_entity_poly.nstd_monomer
_entity_poly.pdbx_strand_id
_entity_poly.pdbx_seq_one_letter_code
_entity_poly.pdbx_seq_one_letter_code_can
1 polypeptide(L) no no . ACGT ACGT
2 polypeptide(L) no no . ACC ACC
#
""")

    def test_entity_poly_seq_dumper(self):
        """Test EntityPolySeqDumper"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        po.add_component_sequence('foo', 'ACGT')
        po.add_component_sequence('bar', 'ACGT')
        po.add_component_sequence('baz', 'ACC')
        d = IMP.pmi.mmcif._EntityPolySeqDumper(po)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
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

    def test_citation(self):
        """Test CitationDumper"""
        s = IMP.pmi.metadata.Citation(
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

        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)
        r.add_metadata(s)
        d = IMP.pmi.mmcif._CitationDumper(r)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        expected = """#
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
"""
        self.assertEqual(out, expected)

        # Handle no last page
        s.page_range = 'e1637'
        d = IMP.pmi.mmcif._CitationDumper(r)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertTrue("'Mol Cell Proteomics' 13 e1637 . 2014 " in out)

    def test_pdb_helix(self):
        """Test PDBHelix class"""
        p = IMP.pmi.mmcif._PDBHelix("HELIX   10  10 ASP A  607  GLU A  624  1                                  18   ")
        self.assertEqual(p.helix_id, '10')
        self.assertEqual(p.start_asym, 'A')
        self.assertEqual(p.start_resnum, 607)
        self.assertEqual(p.end_asym, 'A')
        self.assertEqual(p.end_resnum, 624)
        self.assertEqual(p.helix_class, 1)
        self.assertEqual(p.length, 18)

    def test_asym_id_mapper(self):
        """Test AsymIDMapper class"""
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
        mapper = IMP.pmi.mmcif._AsymIDMapper(simo.prot)
        self.assertEqual(mapper[h1[0]], 'A')
        self.assertEqual(mapper[h1[1]], 'A')
        self.assertEqual(mapper[h2[0]], 'B')
        self.assertEqual(mapper[h2[1]], 'B')

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
        c = IMP.pmi.mmcif._EntityMapper()
        c.add('foo', 'MELS')
        c.add('bar', 'SELM')
        c.add('foo_2', 'MELS')
        self.assertEqual(c['foo'].id, 1)
        self.assertEqual(c['foo_2'].id, 1)
        self.assertEqual(c['bar'].id, 2)
        a = c.get_all()
        self.assertEqual(len(a), 2)
        self.assertEqual(a[0].id, 1)
        self.assertEqual(a[0].first_component, 'foo')
        self.assertEqual(a[0].description, 'foo')
        self.assertEqual(a[0].sequence, 'MELS')
        self.assertEqual(a[1].id, 2)
        self.assertEqual(a[1].first_component, 'bar')
        self.assertEqual(a[1].description, 'bar')
        self.assertEqual(a[1].sequence, 'SELM')

    def test_dataset_dumper_all_group(self):
        """Test DatasetDumper.get_all_group()"""
        dump, simo = make_dataset_dumper()
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='baz')
        ds1 = IMP.pmi.metadata.EM2DClassDataset(l)
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        ds2 = IMP.pmi.metadata.CXMSDataset(l)
        l = IMP.pmi.metadata.PDBLocation('1abc', '1.0', 'test details')
        ds3 = IMP.pmi.metadata.PDBDataset(l)

        g1 = dump.get_all_group()

        dump.add(ds1)
        dump.add(ds2)
        g2 = dump.get_all_group()
        g3 = dump.get_all_group()

        dump.add(ds3)
        g4 = dump.get_all_group()
        dump.finalize() # Assign IDs

        self.assertEqual(g1.id, 1)
        self.assertEqual(list(g1._datasets), [])

        self.assertEqual(g2.id, 2)
        self.assertEqual(list(g2._datasets), [ds1, ds2])
        self.assertEqual(g3.id, 2)

        self.assertEqual(g4.id, 3)
        self.assertEqual(list(g4._datasets), [ds1, ds2, ds3])

    def test_dataset_dumper_duplicates_details(self):
        """DatasetDumper ignores duplicate datasets with differing details"""
        dump, simo = make_dataset_dumper()
        l = IMP.pmi.metadata.PDBLocation('1abc', '1.0', 'test details')
        ds1 = dump.add(IMP.pmi.metadata.PDBDataset(l))
        # A duplicate dataset should be ignored even if details differ
        l = IMP.pmi.metadata.PDBLocation('1abc', '1.0', 'other details')
        ds2 = dump.add(IMP.pmi.metadata.PDBDataset(l))
        dump.finalize() # Assign IDs
        self.assertEqual(ds1.id, 1)
        self.assertEqual(ds2.id, 1)
        self.assertEqual(len(dump._dataset_by_id), 1)

    def test_dataset_dumper_duplicates_location(self):
        """DatasetDumper ignores duplicate dataset locations"""
        loc1 = IMP.pmi.metadata.DatabaseLocation("mydb", "abc", "1.0", "")
        loc2 = IMP.pmi.metadata.DatabaseLocation("mydb", "xyz", "1.0", "")

        # Identical datasets in the same location aren't duplicated
        cx1 = IMP.pmi.metadata.CXMSDataset(loc1)
        cx2 = IMP.pmi.metadata.CXMSDataset(loc1)

        dump, simo = make_dataset_dumper()
        dump.add(cx1)
        dump.add(cx2)
        dump.finalize() # Assign IDs
        self.assertEqual(cx1.id, 1)
        self.assertEqual(cx2.id, 1)
        self.assertEqual(len(dump._dataset_by_id), 1)

        # Datasets in different locations are OK
        cx1 = IMP.pmi.metadata.CXMSDataset(loc1)
        cx2 = IMP.pmi.metadata.CXMSDataset(loc2)
        dump, simo = make_dataset_dumper()
        dump.add(cx1)
        dump.add(cx2)
        dump.finalize() # Assign IDs
        self.assertEqual(cx1.id, 1)
        self.assertEqual(cx2.id, 2)
        self.assertEqual(len(dump._dataset_by_id), 2)

        # Different datasets in same location are OK (but odd)
        cx2 = IMP.pmi.metadata.CXMSDataset(loc2)
        em2d = IMP.pmi.metadata.EM2DClassDataset(loc2)
        dump, simo = make_dataset_dumper()
        dump.add(cx2)
        dump.add(em2d)
        dump.finalize() # Assign IDs
        self.assertEqual(cx2.id, 1)
        self.assertEqual(em2d.id, 2)
        self.assertEqual(len(dump._dataset_by_id), 2)

        # Datasets can be duplicated if allow_duplicates=True
        emloc1 = IMP.pmi.metadata.EMDBLocation("abc")
        emloc2 = IMP.pmi.metadata.EMDBLocation("abc")
        emloc1._allow_duplicates = True
        em3d_1 = IMP.pmi.metadata.EMDensityDataset(emloc1)
        em3d_2 = IMP.pmi.metadata.EMDensityDataset(emloc2)
        dump, simo = make_dataset_dumper()
        dump.add(em3d_1)
        dump.add(em3d_2)
        dump.finalize() # Assign IDs
        self.assertEqual(em3d_1.id, 1)
        self.assertEqual(em3d_2.id, 2)
        self.assertEqual(len(dump._dataset_by_id), 2)

    def test_dataset_dumper_dump(self):
        """Test DatasetDumper.dump()"""
        dump, simo = make_dataset_dumper()
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        l.id = 97
        pds = dump.add(IMP.pmi.metadata.CXMSDataset(l))
        # group1 contains just the first dataset
        group1 = dump.get_all_group()
        l = IMP.pmi.metadata.FileLocation(repo='foo2', path='bar2')
        l.id = 98
        pds = dump.add(IMP.pmi.metadata.CXMSDataset(l))
        # group2 contains the first two datasets
        group2 = dump.get_all_group()
        # last dataset is in no group
        l = IMP.pmi.metadata.PDBLocation('1abc', '1.0', 'test details')
        ds = dump.add(IMP.pmi.metadata.PDBDataset(l))
        ds.add_primary(pds)
        self.assertEqual(ds.location.access_code, '1abc')

        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        dump.finalize()
        dump.dump(w)
        out = fh.getvalue()
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

    def test_external_reference_dumper_dump(self):
        """Test ExternalReferenceDumper.dump()"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        dump = IMP.pmi.mmcif._ExternalReferenceDumper(po)
        repo1 = IMP.pmi.metadata.Repository(doi="foo")
        repo2 = IMP.pmi.metadata.Repository(doi="10.5281/zenodo.46266",
                                     url='nup84-v1.0.zip',
                                     top_directory=os.path.join('foo', 'bar'))
        repo3 = IMP.pmi.metadata.Repository(doi="10.5281/zenodo.58025",
                                            url='foo.spd')
        l = IMP.pmi.metadata.FileLocation(repo=repo1, path='bar')
        dump.add(l, IMP.pmi.mmcif._ExternalReferenceDumper.INPUT_DATA)
        # Duplicates should be ignored
        l = IMP.pmi.metadata.FileLocation(repo=repo1, path='bar')
        dump.add(l, IMP.pmi.mmcif._ExternalReferenceDumper.INPUT_DATA)
        # Different file, same repository
        l = IMP.pmi.metadata.FileLocation(repo=repo1, path='baz')
        dump.add(l, IMP.pmi.mmcif._ExternalReferenceDumper.INPUT_DATA)
        # Different repository
        l = IMP.pmi.metadata.FileLocation(repo=repo2, path='baz')
        dump.add(l, IMP.pmi.mmcif._ExternalReferenceDumper.MODELING_OUTPUT)
        # Repository containing a single file (not an archive)
        l = IMP.pmi.metadata.FileLocation(repo=repo3, path='foo.spd',
                                          details='EM micrographs')
        dump.add(l, IMP.pmi.mmcif._ExternalReferenceDumper.INPUT_DATA)
        bar = 'test_mmcif_extref.tmp'
        with open(bar, 'w') as f:
            f.write("")
        # Local file
        l = IMP.pmi.metadata.FileLocation(bar)
        dump.add(l, IMP.pmi.mmcif._ExternalReferenceDumper.WORKFLOW)
        # DatabaseLocations should be ignored
        l = IMP.pmi.metadata.PDBLocation('1abc', '1.0', 'test details')
        dump.add(l, IMP.pmi.mmcif._ExternalReferenceDumper.WORKFLOW)
        dump.finalize_after_datasets()
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        dump.dump(w)
        out = fh.getvalue()
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
_ihm_external_files.details
1 1 bar 'Input data or restraints' .
2 1 baz 'Input data or restraints' .
3 2 foo/bar/baz 'Modeling or post-processing output' .
4 3 foo.spd 'Input data or restraints' 'EM micrographs'
5 4 %s 'Modeling workflow or script' .
#
""" % bar)
        os.unlink(bar)

    def test_model_dumper_sphere(self):
        """Test ModelDumper sphere_obj output"""
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

        d = IMP.pmi.mmcif._ModelDumper(po)
        assembly = IMP.pmi.mmcif._Assembly()
        assembly.id = 42
        protocol = IMP.pmi.mmcif._Protocol()
        protocol.id = 93
        group = IMP.pmi.mmcif._ModelGroup("all models")
        group.id = 7
        model = d.add(simo.prot, protocol, assembly, group)
        self.assertEqual(model.id, 1)
        self.assertEqual(model.get_rmsf('Nup84', (1,)), '.')
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
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
1 1 7 . 'all models' 42 93
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
3 1 3 4 A -8.986 11.688 -5.817 3.504 . 1
#
""")

    def test_model_dumper_atom(self):
        """Test ModelDumper atom_site output"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A", resolutions=[0])

        d = IMP.pmi.mmcif._ModelDumper(po)
        assembly = IMP.pmi.mmcif._Assembly()
        assembly.id = 42
        protocol = IMP.pmi.mmcif._Protocol()
        protocol.id = 93
        group = IMP.pmi.mmcif._ModelGroup("all models")
        group.id = 7
        model = d.add(simo.prot, protocol, assembly, group)
        self.assertEqual(model.id, 1)
        self.assertEqual(model.get_rmsf('Nup84', (1,)), '.')
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
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
1 1 7 . 'all models' 42 93
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
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")

        d = IMP.pmi.mmcif._ModelDumper(po)
        assembly = IMP.pmi.mmcif._Assembly()
        assembly.id = 42
        protocol = IMP.pmi.mmcif._Protocol()
        protocol.id = 93
        group = IMP.pmi.mmcif._ModelGroup("all models")
        group.id = 7
        model = d.add(simo.prot, protocol, assembly, group)
        self.assertEqual(model.id, 1)
        model.name = 'foo'
        model.parse_rmsf_file(self.get_input_file_name('test.nup84.rmsf'),
                              'Nup84')
        self.assertAlmostEqual(model.get_rmsf('Nup84', (1,)), 4.5, delta=1e-4)
        self.assertRaises(ValueError, model.get_rmsf, 'Nup84', (1,2))
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
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
1 1 7 foo 'all models' 42 93
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

    def test_starting_model_dumper(self):
        """Test StartingModelDumper"""
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
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        po.dataset_dump.finalize()
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
Nup84-m1 1 Nup84 A 1 2 'comparative model' A 0 1
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
#
""")

    def get_dumper_sources(self, pdbname):
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        chain = 'A'
        fragment = IMP.pmi.mmcif._PDBFragment(m, "mypdb", 1, 10, 0, pdbname,
                                              chain, None)
        model = IMP.pmi.mmcif._StartingModel(fragment)
        sources = po.starting_model_dump.get_sources(model, pdbname, chain)
        return m, model, sources

    def test_get_sources_official_pdb(self):
        """Test get_sources() when given an official PDB"""
        pdbname = self.get_input_file_name('official.pdb')
        m, model, sources = self.get_dumper_sources(pdbname)
        (s, ) = sources
        self.assertEqual(s.db_code, '2HBJ')
        self.assertEqual(s.chain_id, 'A')
        self.assertEqual(model.dataset._data_type, 'Experimental model')
        self.assertEqual(model.dataset.location.db_name, 'PDB')
        self.assertEqual(model.dataset.location.access_code, '2HBJ')
        self.assertEqual(model.dataset.location.version, '14-JUN-06')
        self.assertEqual(model.dataset.location.details,
                         'STRUCTURE OF THE YEAST NUCLEAR EXOSOME COMPONENT, '
                         'RRP6P, REVEALS AN INTERPLAY BETWEEN THE ACTIVE '
                         'SITE AND THE HRDC DOMAIN')

    def test_get_sources_derived_pdb(self):
        """Test get_sources() when given a file derived from a PDB"""
        pdbname = self.get_input_file_name('derived_pdb.pdb')
        m, model, sources = self.get_dumper_sources(pdbname)
        (s, ) = sources
        self.assertEqual(s.db_code, '?')
        self.assertEqual(s.chain_id, 'A')
        self.assertEqual(model.dataset._data_type, 'Experimental model')
        self.assertEqual(model.dataset.location.path, pdbname)
        self.assertEqual(model.dataset.location.repo, None)
        self.assertEqual(model.dataset.location.details,
                         'MED7C AND MED21 STRUCTURES FROM PDB ENTRY 1YKH, '
                         'ROTATED AND TRANSLATED TO ALIGN WITH THE '
                         'MED4-MED9 MODEL')
        (parent,) = model.dataset._parents
        self.assertEqual(parent._data_type, 'Experimental model')
        self.assertEqual(parent.location.db_name, 'PDB')
        self.assertEqual(parent.location.access_code, '1YKH')
        self.assertEqual(parent.location.version, None)
        self.assertEqual(parent.location.details, None)

    def test_get_sources_derived_model(self):
        """Test get_sources() when given a file derived from a model"""
        pdbname = self.get_input_file_name('derived_model.pdb')
        m, model, sources = self.get_dumper_sources(pdbname)
        (s, ) = sources
        self.assertEqual(s.db_code, '?')
        self.assertEqual(s.chain_id, 'A')
        self.assertEqual(model.dataset._data_type, 'Comparative model')
        self.assertEqual(model.dataset.location.path, pdbname)
        self.assertEqual(model.dataset.location.repo, None)
        self.assertEqual(model.dataset.location.details,
                         'MED4 AND MED9 STRUCTURE TAKEN FROM LARIVIERE '
                         'ET AL, NUCLEIC ACIDS RESEARCH. 2013;41:9266-9273. '
                         'DOI: 10.1093/nar/gkt704. THE MED10 STRUCTURE ALSO '
                         'PROPOSED IN THAT WORK IS NOT USED IN THIS STUDY.')
        (parent,) = model.dataset._parents
        self.assertEqual(parent._data_type, 'Comparative model')
        self.assertEqual(parent.location.path, '.')
        self.assertEqual(parent.location.repo.doi, '10.1093/nar/gkt704')
        self.assertEqual(parent.location.details,
                         'Starting comparative model structure')

    def test_get_sources_modeller(self):
        """Test get_sources() when given a Modeller model with alignment"""
        pdbname = self.get_input_file_name('modeller_model.pdb')
        m, model, sources = self.check_modeller_model(pdbname)
        self.assertEqual(model.alignment_file.path,
                         self.get_input_file_name('modeller_model.ali'))

    def test_get_sources_modeller_no_aln(self):
        """Test get_sources() when given a Modeller model with no alignment"""
        pdbname = self.get_input_file_name('modeller_model_no_aln.pdb')
        m, model, sources = self.check_modeller_model(pdbname)

    def check_modeller_model(self, pdbname):
        m, model, sources = self.get_dumper_sources(pdbname)
        s1, s2 = sources
        self.assertEqual(s1.db_code, '.')
        self.assertEqual(s1.chain_id, 'A')
        self.assertEqual(s1.tm_db_code, '3JRO')
        self.assertEqual(s1.tm_chain_id, 'C')
        self.assertEqual(s2.db_code, '.')
        self.assertEqual(s2.chain_id, 'A')
        self.assertEqual(s2.tm_db_code, '3F3F')
        self.assertEqual(s2.tm_chain_id, 'G')
        self.assertEqual(model.dataset._data_type, 'Comparative model')
        self.assertEqual(model.dataset.location.path, pdbname)
        self.assertEqual(model.dataset.location.repo, None)
        self.assertEqual(model.dataset.location.details,
                         'Starting model structure')
        p1, p2 = model.dataset._parents
        self.assertEqual(p1._data_type, 'Experimental model')
        self.assertEqual(p1.location.db_name, 'PDB')
        self.assertEqual(p1.location.access_code, '3JRO')
        self.assertEqual(p1.location.version, None)
        self.assertEqual(p1.location.details, None)
        self.assertEqual(p2.location.access_code, '3F3F')
        return m, model, sources

    def test_get_sources_modeller_local(self):
        """Test get_sources() when given a Modeller model with local template"""
        pdbname = self.get_input_file_name('modeller_model_local.pdb')
        m, model, sources = self.get_dumper_sources(pdbname)
        s1, = sources
        self.assertEqual(s1.db_code, '.')
        self.assertEqual(s1.chain_id, 'A')
        self.assertEqual(s1.tm_db_code, '.')
        self.assertEqual(s1.tm_chain_id, 'C')
        p1, = model.dataset._parents
        self.assertEqual(p1._data_type, 'Experimental model')
        self.assertEqual(p1.location.details,
                         'Template for comparative modeling')
        self.assertEqual(p1.location.path,
                         self.get_input_file_name('15133C.pdb'))

    def test_get_sources_phyre2(self):
        """Test get_sources() when given a Phyre2 model"""
        pdbname = self.get_input_file_name('phyre2_model.pdb')
        m, model, sources = self.get_dumper_sources(pdbname)
        (s,) = sources
        self.assertEqual(s.db_code, '.')
        self.assertEqual(s.chain_id, 'A')
        self.assertEqual(s.tm_db_code, '4BZK')
        self.assertEqual(s.tm_chain_id, 'A')
        self.assertEqual(model.dataset._data_type, 'Comparative model')
        self.assertEqual(model.dataset.location.path, pdbname)
        self.assertEqual(model.dataset.location.repo, None)
        self.assertEqual(model.dataset.location.details,
                         'Starting model structure')
        (p,) = model.dataset._parents
        self.assertEqual(p._data_type, 'Experimental model')
        self.assertEqual(p.location.db_name, 'PDB')
        self.assertEqual(p.location.access_code, '4BZK')
        self.assertEqual(p.location.version, None)
        self.assertEqual(p.location.details, None)

    def test_chem_comp_dumper(self):
        """Test ChemCompDumper"""
        po = DummyPO(None)
        po.create_component("Nup84", True)
        po.add_component_sequence("Nup84", "MELS")
        po.create_component("Nup85", True)
        po.add_component_sequence("Nup85", "MC")

        d = IMP.pmi.mmcif._ChemCompDumper(po)

        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_chem_comp.id
_chem_comp.type
MET 'L-peptide linking'
GLU 'L-peptide linking'
LEU 'L-peptide linking'
SER 'L-peptide linking'
CYS 'L-peptide linking'
#
""")

    def test_protocol_dumper(self):
        """Test ModelProtocolDumper output"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)
        # Need Repository in order to handle PDB file datasets
        simo.add_metadata(IMP.pmi.metadata.Repository(doi='foo', root='.'))
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
        w = IMP.pmi.mmcif._CifWriter(fh)
        po.dataset_dump.finalize() # Assign IDs to datasets
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
_ihm_modeling_protocol.time_ordered_flag
1 1 1 1 1 . . Sampling 'Replica exchange monte carlo' 0 1000 YES NO NO
2 1 2 1 1 . . Sampling 'Replica exchange monte carlo' 1000 1000 YES NO NO
#
""")

    def test_simple_postprocessing(self):
        """Test add_simple_postprocessing"""
        po = DummyPO(None)
        pp = po._add_simple_postprocessing(10, 90)
        self.assertEqual(pp.type, 'cluster')
        self.assertEqual(pp.feature, 'RMSD')
        self.assertEqual(pp.num_models_begin, 10)
        self.assertEqual(pp.num_models_end, 90)

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
        self.assertEqual(e.num_deposit, 1)

    def test_ensemble_dumper(self):
        """Test EnsembleDumper"""
        class DummyPostProcess(object):
            pass
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)

        pp = DummyPostProcess()
        pp.id = 99
        e = po._add_simple_ensemble(pp, 'Ensemble 1', 5, 0.1, 1, {}, None)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        po.ensemble_dump.dump(w)
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
        ensemble.id = 42
        loc = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        loc.id = 97
        ensemble.localization_density = {'Nup84': loc}
        po.density_dump.add(ensemble)

        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        po.density_dump.dump(w)
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
        simo.create_component("Nup84", True)
        simo.add_component_sequence("Nup84",
                                    self.get_input_file_name("test.fasta"))
        nup84 = simo.autobuild_model("Nup84",
                                     self.get_input_file_name("test.nup84.pdb"),
                                     "A")
        r = DummyRestraint()
        r.dataset = DummyDataset()
        r.dataset.id = 42
        xl_group = po.get_cross_link_group(r)
        ex_xl = po.add_experimental_cross_link(1, 'Nup84',
                                               2, 'Nup84', 42.0, xl_group)
        # Non-modeled component should be ignored
        nm_ex_xl = po.add_experimental_cross_link(1, 'Nup85',
                                                  2, 'Nup84', 42.0, xl_group)
        self.assertEqual(nm_ex_xl, None)
        rs = nup84[0].get_children()
        sigma1 = IMP.isd.Scale.setup_particle(IMP.Particle(m), 1.0)
        sigma2 = IMP.isd.Scale.setup_particle(IMP.Particle(m), 0.5)
        psi = IMP.isd.Scale.setup_particle(IMP.Particle(m), 0.8)
        po.add_cross_link(ex_xl, rs[0], rs[1], sigma1, sigma2, psi)

        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
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
_ihm_cross_link_list.type
_ihm_cross_link_list.dataset_list_id
1 1 Nup84 1 1 MET Nup84 1 2 GLU foo 42
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
_ihm_cross_link_restraint.type
_ihm_cross_link_restraint.conditional_crosslink_flag
_ihm_cross_link_restraint.model_granularity
_ihm_cross_link_restraint.distance_threshold
_ihm_cross_link_restraint.psi
_ihm_cross_link_restraint.sigma_1
_ihm_cross_link_restraint.sigma_2
1 1 1 A 1 MET 1 A 2 GLU foo ALL by-residue 42.000 0.800 1.000 0.500
#
""")

    def test_restraint_dataset(self):
        """Test RestraintDataset class"""
        class DummyRestraint(object):
            pass
        r = DummyRestraint()
        rd = IMP.pmi.mmcif._RestraintDataset(r, num=None,
                                             allow_duplicates=False)
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        d = IMP.pmi.metadata.CXMSDataset(l)
        r.dataset = d
        # Get current dataset from restraint
        d2 = rd.dataset
        self.assertEqual(d2._data_type, 'CX-MS data')
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
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        d1 = IMP.pmi.metadata.CXMSDataset(l)
        l = IMP.pmi.metadata.FileLocation(repo='bar', path='baz')
        d2 = IMP.pmi.metadata.CXMSDataset(l)
        r.datasets = [d1, d2]
        # Get current dataset from restraint
        d2 = rd.dataset
        self.assertEqual(d2._data_type, 'CX-MS data')
        self.assertEqual(d2.location.repo, 'bar')

    def test_restraint_dataset_duplicate(self):
        """Test RestraintDataset with allow_duplicates=True"""
        class DummyRestraint(object):
            pass
        r = DummyRestraint()
        rd = IMP.pmi.mmcif._RestraintDataset(r, num=None, allow_duplicates=True)
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        d = IMP.pmi.metadata.CXMSDataset(l)
        r.dataset = d
        # Get current dataset from restraint
        d2 = rd.dataset
        self.assertEqual(d2._data_type, 'CX-MS data')
        self.assertEqual(d2.location.repo, 'foo')
        # Should be a copy, but should not compare equal
        # since allow_duplicates=True
        self.assertNotEqual(d, d2)

    def test_em2d_restraint_no_raw(self):
        """Test EM2DRestraint class, no raw micrographs"""
        class DummyRestraint(object):
            pass
        pr = DummyRestraint()
        rd = IMP.pmi.mmcif._RestraintDataset(pr, num=None,
                                             allow_duplicates=False)
        r = IMP.pmi.mmcif._EM2DRestraint(rd, pr, 0,
                                         resolution=10.0, pixel_size=4.2,
                                         image_resolution=1.0,
                                         projection_number=200)
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        d = IMP.pmi.metadata.EM2DClassDataset(l)
        pr.dataset = d
        self.assertEqual(r.get_num_raw_micrographs(), None)
        self.assertEqual(r.rdataset.dataset.location.repo, 'foo')

    def test_em2d_restraint_with_raw(self):
        """Test EM2DRestraint class, with raw micrographs"""
        class DummyRestraint(object):
            pass
        pr = DummyRestraint()
        rd = IMP.pmi.mmcif._RestraintDataset(pr, num=None,
                                             allow_duplicates=False)
        r = IMP.pmi.mmcif._EM2DRestraint(rd, pr, 0,
                                         resolution=10.0, pixel_size=4.2,
                                         image_resolution=1.0,
                                         projection_number=200)
        lp = IMP.pmi.metadata.FileLocation(repo='foo', path='baz')
        dp = IMP.pmi.metadata.EMMicrographsDataset(lp, number=50)
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        d = IMP.pmi.metadata.EM2DClassDataset(l)
        d.add_primary(dp)
        pr.dataset = d
        self.assertEqual(r.get_num_raw_micrographs(), 50)

    def test_em2d_dumper(self):
        """Test EM2DDumper class"""
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

        class DummyRestraint(object):
            label = 'foo'
        class DummyProtocol(object):
            pass
        pr = DummyRestraint()
        rd = IMP.pmi.mmcif._RestraintDataset(pr, num=None,
                                             allow_duplicates=False)
        r = IMP.pmi.mmcif._EM2DRestraint(rd, pr, 0,
                                         resolution=10.0, pixel_size=4.2,
                                         image_resolution=1.0,
                                         projection_number=200)
        lp = IMP.pmi.metadata.FileLocation(repo='foo', path='baz')
        dp = IMP.pmi.metadata.EMMicrographsDataset(lp, number=50)
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        d = IMP.pmi.metadata.EM2DClassDataset(l)
        d.id = 4
        d.add_primary(dp)
        pr.dataset = d
        po.model_prot_dump.add(DummyProtocol())
        m = po.add_model()
        prefix = 'ElectronMicroscopy2D_foo_Image1_'
        m.stats = {prefix + 'CCC': '0.872880665234',
                   prefix + 'Translation0': '304.187464117',
                   prefix + 'Translation1': '219.585852373',
                   prefix + 'Translation2': '0.0',
                   prefix + 'Rotation0': '0.443696289233',
                   prefix + 'Rotation1': '0.316041672423',
                   prefix + 'Rotation2': '-0.419293315413',
                   prefix + 'Rotation3': '-0.726253660826'}
        po.em2d_dump.add(r)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        po.em2d_dump.dump(w)
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

    def test_em3d_dumper(self):
        """Test EM3DDumper class"""
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
        class DummyRestraint(object):
            pass
        class DummyProtocol(object):
            pass
        pr = DummyRestraint()
        rd = IMP.pmi.mmcif._RestraintDataset(pr, num=None,
                                             allow_duplicates=True)
        r = IMP.pmi.mmcif._EM3DRestraint(po, rd, target_ps=[None, None],
                                         densities=[])

        l = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        d = IMP.pmi.metadata.EM2DClassDataset(l)
        d.id = 4
        pr.dataset = d

        po.model_prot_dump.add(DummyProtocol())
        po.add_model()
        po.add_model()
        po.em3d_dump.add(r)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        po.em3d_dump.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_3dem_restraint.ordinal_id
_ihm_3dem_restraint.dataset_list_id
_ihm_3dem_restraint.fitting_method
_ihm_3dem_restraint.struct_assembly_id
_ihm_3dem_restraint.number_of_gaussians
_ihm_3dem_restraint.model_id
_ihm_3dem_restraint.cross_correlation_coefficient
1 4 'Gaussian mixture models' 2 2 1 .
2 4 'Gaussian mixture models' 2 2 2 .
#
""")

    def test_update_location(self):
        """Test update_location() method"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        po = DummyPO(None)
        simo.add_protocol_output(po)

        with IMP.test.temporary_directory() as tmpdir:
            bar = os.path.join(tmpdir, 'bar')
            with open(bar, 'w') as f:
                f.write("")
            local = IMP.pmi.metadata.FileLocation(bar)
            # No Repository set, so cannot map local to repository location
            po._update_location(local)
            self.assertEqual(local.repo, None)

            simo.add_metadata(IMP.pmi.metadata.Software(
                                  name='test', classification='test code',
                                  description='Some test program',
                                  version=1, url='http://salilab.org'))
            simo.add_metadata(IMP.pmi.metadata.Repository(doi='foo',
                                                          root=tmpdir))
            loc = IMP.pmi.metadata.FileLocation(bar)
            po._update_location(loc)
            self.assertEqual(loc.repo.doi, 'foo')
            self.assertEqual(loc.path, 'bar')
            # Further calls shouldn't change things
            po._update_location(loc)
            self.assertEqual(loc.repo.doi, 'foo')
            self.assertEqual(loc.path, 'bar')

    def test_seq_dif(self):
        """Test StartingModelDumper.dump_seq_dif"""
        class DummyEntity(object):
            id = 4
        class DummyPO(object):
            def _get_chain_for_component(self, comp, output):
                return 'H'
            entities = {'nup84': DummyEntity()}
        class DummyRes(object):
            def get_index(self):
                return 42
        class DummySource(object):
            chain_id = 'X'
        class DummyModel(object):
            name = 'dummy-m1'

        po = DummyPO()
        d = IMP.pmi.mmcif._StartingModelDumper(po)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump_seq_dif(w, [IMP.pmi.mmcif._MSESeqDif(DummyRes(), 'nup84',
                                                    DummySource(),
                                                    DummyModel(), 2)])
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
        d = IMP.pmi.mmcif._ModelRepresentationDumper(EmptyObject())
        b = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=0,
                                         end=10, num=2, hier=None)
        d.add_fragment(b)
        self.assertEqual(len(d.fragments['comp1']), 1)
        frag = d.fragments['comp1'][0]
        self.assertEqual(frag.start, 0)
        self.assertEqual(frag.end, 10)

        b = IMP.pmi.mmcif._BeadsFragment(m, 'comp1', start=11,
                                         end=30, num=3, hier=None)
        d.add_fragment(b)
        self.assertEqual(len(d.fragments['comp1']), 1)
        frag = d.fragments['comp1'][0]
        self.assertEqual(frag.start, 0)
        self.assertEqual(frag.end, 30)

    def test_model_repr_dump(self):
        """Test ModelRepresentationDumper"""
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
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
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
2 1 2 1 Nup84 A 3 4 sphere . flexible by-feature 1
#
""")

    def test_pdb_source(self):
        """Test PDBSource class"""
        class DummyModel(object):
            seq_id_begin = 1
            seq_id_end = 100
        m = DummyModel()
        p = IMP.pmi.mmcif._PDBSource(m, '1abc', 'A', metadata=[])
        self.assertEqual(p.source, 'experimental model')
        self.assertEqual(p.get_seq_id_range(m), (1, 100))

    def test_template_source_pdb(self):
        """Test TemplateSource class, where template is from PDB"""
        class DummyModel(object):
            seq_id_begin = 10
            seq_id_end = 100
        m = DummyModel()
        p = IMP.pmi.mmcif._TemplateSource(tm_code='1abcA', tm_seq_id_begin=30,
                    tm_seq_id_end=90, seq_id_begin=1, chain_id='G',
                    seq_id_end=90, seq_id=42., model=m)
        self.assertEqual(p.source, 'comparative model')
        self.assertEqual(p.tm_db_code, '1ABC')
        self.assertEqual(p.tm_chain_id, 'A')
        self.assertEqual(p.get_seq_id_range(m), (10, 90))

    def test_template_source_unknown(self):
        """Test TemplateSource class, where template is not in PDB"""
        class DummyModel(object):
            seq_id_begin = 10
            seq_id_end = 100
        m = DummyModel()
        p = IMP.pmi.mmcif._TemplateSource(tm_code='fooA', tm_seq_id_begin=30,
                    tm_seq_id_end=90, seq_id_begin=1, chain_id='G',
                    seq_id_end=90, seq_id=42., model=m)
        self.assertEqual(p.source, 'comparative model')
        self.assertEqual(p.tm_db_code, '.')
        self.assertEqual(p.tm_chain_id, 'A')
        self.assertEqual(p.get_seq_id_range(m), (10, 90))

    def test_unknown_source(self):
        """Test UnknownSource class"""
        class DummyDataset(object):
            _data_type = 'Comparative model'
        class DummyModel(object):
            dataset = DummyDataset()
            seq_id_begin = 10
            seq_id_end = 100
        m = DummyModel()
        p = IMP.pmi.mmcif._UnknownSource(m, 'A')
        self.assertEqual(p.source, 'comparative model')
        self.assertEqual(p.get_seq_id_range(m), (10, 100))

if __name__ == '__main__':
    IMP.test.main()
