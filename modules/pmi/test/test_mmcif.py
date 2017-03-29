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
        d = IMP.pmi.mmcif._SoftwareDumper(r)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
        out = fh.getvalue().split('\n')
        self.assertEqual(out[-3],
                         "3 test 'test code' 1 program http://salilab.org")

    def test_workflow(self):
        """Test output of workflow files"""
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        root = os.path.dirname(sys.argv[0]) or '.'
        simo.add_metadata(IMP.pmi.metadata.Repository(doi="foo", root=root))
        po = DummyPO(None)
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
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass
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
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass
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
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass
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
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass
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
_ihm_dataset_list.ordinal_id
_ihm_dataset_list.id
_ihm_dataset_list.group_id
_ihm_dataset_list.data_type
_ihm_dataset_list.database_hosted
1 1 1 'CX-MS data' NO
2 2 1 'Experimental model' YES
#
#
loop_
_ihm_dataset_external_reference.id
_ihm_dataset_external_reference.dataset_list_id
_ihm_dataset_external_reference.data_type
_ihm_dataset_external_reference.file_id
1 1 'CX-MS data' 97
#
#
loop_
_ihm_dataset_related_db_reference.id
_ihm_dataset_related_db_reference.dataset_list_id
_ihm_dataset_related_db_reference.db_name
_ihm_dataset_related_db_reference.accession_code
_ihm_dataset_related_db_reference.version
_ihm_dataset_related_db_reference.data_type
_ihm_dataset_related_db_reference.details
1 2 PDB 1abc 1.0 'Experimental model' 'test details'
#
#
loop_
_ihm_related_datasets.ordinal_id
_ihm_related_datasets.dataset_list_id_derived
_ihm_related_datasets.data_type_derived
_ihm_related_datasets.dataset_list_id_primary
_ihm_related_datasets.data_type_primary
1 2 'Experimental model' 1 'CX-MS data'
#
""")

    def test_external_reference_dumper_dump(self):
        """Test ExternalReferenceDumper.dump()"""
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass

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
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass

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
1 1 1 1 A 0.000 0.000 0.000 3.068 . 1
2 1 2 2 A 0.000 0.000 0.000 2.997 . 1
3 1 3 4 A 0.000 0.000 0.000 3.504 . 1
#
""")

    def test_model_dumper_sphere_rmsf(self):
        """Test ModelDumper sphere_obj output with RMSF"""
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass

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
1 1 1 1 A 0.000 0.000 0.000 3.068 4.500 1
2 1 2 2 A 0.000 0.000 0.000 2.997 3.500 1
3 1 3 4 A 0.000 0.000 0.000 3.504 5.500 1
#
""")

    def test_chem_comp_dumper(self):
        """Test ChemCompDumper"""
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass

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
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass

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

    def test_density_dumper(self):
        """Test DensityDumper"""
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass

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
        r = IMP.pmi.mmcif._EM2DRestraint(rd, resolution=10.0, pixel_size=4.2,
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
        r = IMP.pmi.mmcif._EM2DRestraint(rd, resolution=10.0, pixel_size=4.2,
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
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass
        class DummyRestraint(object):
            pass
        pr = DummyRestraint()
        rd = IMP.pmi.mmcif._RestraintDataset(pr, num=None,
                                             allow_duplicates=False)
        r = IMP.pmi.mmcif._EM2DRestraint(rd, resolution=10.0, pixel_size=4.2,
                                         image_resolution=1.0,
                                         projection_number=200)
        lp = IMP.pmi.metadata.FileLocation(repo='foo', path='baz')
        dp = IMP.pmi.metadata.EMMicrographsDataset(lp, number=50)
        l = IMP.pmi.metadata.FileLocation(repo='foo', path='bar')
        d = IMP.pmi.metadata.EM2DClassDataset(l)
        d.id = 4
        d.add_primary(dp)
        pr.dataset = d
        po = DummyPO(EmptyObject())
        d = IMP.pmi.mmcif._EM2DDumper(po)
        d.add(r)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump(w)
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
""")

    def test_update_location(self):
        """Test update_location() method"""
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass
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
            id = 39
            chain_id = 'X'

        po = DummyPO()
        d = IMP.pmi.mmcif._StartingModelDumper(po)
        fh = StringIO()
        w = IMP.pmi.mmcif._CifWriter(fh)
        d.dump_seq_dif(w, [IMP.pmi.mmcif._MSESeqDif(DummyRes(), 'nup84',
                                                    DummySource(), 2)])
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_starting_model_seq_dif.ordinal_id
_ihm_starting_model_seq_dif.entity_id
_ihm_starting_model_seq_dif.asym_id
_ihm_starting_model_seq_dif.seq_id
_ihm_starting_model_seq_dif.comp_id
_ihm_starting_model_seq_dif.starting_model_ordinal_id
_ihm_starting_model_seq_dif.db_asym_id
_ihm_starting_model_seq_dif.db_seq_id
_ihm_starting_model_seq_dif.db_comp_id
_ihm_starting_model_seq_dif.details
1 4 H 42 MET 39 X 40 MSE 'Conversion of modified residue MSE to MET'
#
""")

if __name__ == '__main__':
    IMP.test.main()
