from __future__ import print_function
import IMP.test
import IMP.pmi.metadata
import IMP.pmi.representation
import IMP.pmi.mmcif
import IMP.pmi.macros
import sys
import io
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

class EmptyObject(object):
    pass

class Tests(IMP.test.TestCase):

    def test_software(self):
        """Test SoftwareDumper"""
        s = IMP.pmi.metadata.Software(name='test', classification='test code',
                                      description='Some test program',
                                      version=1, url='http://salilab.org')
        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)
        r.add_metadata(s)
        d = IMP.pmi.mmcif.SoftwareDumper(r)
        fh = StringIO()
        w = IMP.pmi.mmcif.CifWriter(fh)
        d.dump(w)
        out = fh.getvalue().split('\n')
        self.assertEqual(out[-3],
                         "3 test 'test code' 1 program http://salilab.org")

    def test_assembly_dumper_get_subassembly(self):
        """Test AssemblyDumper.get_subassembly()"""
        class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
            def flush(self):
                pass
        po = DummyPO(EmptyObject())
        d = IMP.pmi.mmcif.AssemblyDumper(po)
        complete = IMP.pmi.mmcif.Assembly(['a', 'b', 'c'])
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
        d = IMP.pmi.mmcif.AssemblyDumper(po)
        for c, seq in (("foo", "AAA"), ("bar", "AAA"), ("baz", "AA")):
            po.create_component(c, True)
            po.add_component_sequence(c, seq)
        d.add(IMP.pmi.mmcif.Assembly(["foo", "bar"]))
        d.add(IMP.pmi.mmcif.Assembly(["bar", "baz"]))

        fh = StringIO()
        w = IMP.pmi.mmcif.CifWriter(fh)
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
        w = IMP.pmi.mmcif.CifWriter(fh)
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
        d = IMP.pmi.mmcif.StructAsymDumper(po)
        for c, seq in (("foo", "AAA"), ("bar", "AAA"), ("baz", "AA")):
            po.create_component(c, True)
            po.add_component_sequence(c, seq)

        fh = StringIO()
        w = IMP.pmi.mmcif.CifWriter(fh)
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
              title="Structural characterization by cross-linking reveals the "
                    "detailed architecture of a coatomer-related heptameric "
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
        d = IMP.pmi.mmcif.CitationDumper(r)
        fh = StringIO()
        w = IMP.pmi.mmcif.CifWriter(fh)
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
;Structural characterization by cross-linking reveals the detailed arch
itecture of a coatomer-related heptameric module from the nuclear pore
 complex.
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
        d = IMP.pmi.mmcif.CitationDumper(r)
        fh = StringIO()
        w = IMP.pmi.mmcif.CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertTrue("'Mol Cell Proteomics' 13 e1637 . 2014 " in out)

    def test_pdb_helix(self):
        """Test PDBHelix class"""
        p = IMP.pmi.mmcif.PDBHelix("HELIX   10  10 ASP A  607  GLU A  624  1                                  18   ")
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
        mapper = IMP.pmi.mmcif.AsymIDMapper(simo.prot)
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
        mapper = IMP.pmi.mmcif.ComponentMapper(simo.prot)
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
        dump = IMP.pmi.mmcif.DatasetDumper(EmptyObject())
        ds1 = IMP.pmi.mmcif.EM2DClassDataset()
        ds2 = IMP.pmi.mmcif.CXMSDataset()
        ds3 = IMP.pmi.mmcif.PDBDataset('1abc', '1.0', 'test details')

        g = dump.get_all_group()
        self.assertEqual(g.id, 1)
        self.assertEqual(g.datasets, [])

        dump.add(ds1)
        dump.add(ds2)
        g = dump.get_all_group()
        self.assertEqual(g.id, 2)
        self.assertEqual(g.datasets, [ds1, ds2])
        g = dump.get_all_group()
        self.assertEqual(g.id, 2)

        dump.add(ds3)
        g = dump.get_all_group()
        self.assertEqual(g.id, 3)
        self.assertEqual(g.datasets, [ds1, ds2, ds3])

    def test_dataset_dumper_duplicates(self):
        """Check that DatasetDumper ignores duplicate datasets"""
        dump = IMP.pmi.mmcif.DatasetDumper(EmptyObject())
        ds1 = dump.add(IMP.pmi.mmcif.PDBDataset('1abc', '1.0', 'test details'))
        self.assertEqual(ds1.id, 1)
        # A duplicate dataset should be ignored even if details differ
        ds2 = dump.add(IMP.pmi.mmcif.PDBDataset('1abc', '1.0', 'other details'))
        self.assertEqual(ds2.id, 1)
        self.assertEqual(id(ds1), id(ds2))

        loc1 = IMP.pmi.mmcif.DBDatasetLocation("mydb", "abc", "1.0", "")
        loc2 = IMP.pmi.mmcif.DBDatasetLocation("mydb", "xyz", "1.0", "")

        # Identical datasets in the same location aren't duplicated
        cx1 = IMP.pmi.mmcif.CXMSDataset()
        cx1.location = loc1
        cx2 = IMP.pmi.mmcif.CXMSDataset()
        cx2.location = loc1
        ds3 = dump.add(cx1)
        ds4 = dump.add(cx1)
        self.assertEqual(ds3.id, 2)
        self.assertEqual(ds4.id, 2)

        # Datasets in different locations are OK
        cx3 = IMP.pmi.mmcif.CXMSDataset()
        cx3.location = loc2
        ds5 = dump.add(cx3)
        self.assertEqual(ds5.id, 3)

        # Different datasets in same location are OK (but odd)
        em2d = IMP.pmi.mmcif.EM2DClassDataset()
        em2d.location = loc2
        ds6 = dump.add(em2d)
        self.assertEqual(ds6.id, 4)

        # EM3D datasets allow duplication (since they identify the restraint)
        em3d_1 = IMP.pmi.mmcif.EMDBDataset('EMD-123', allow_duplicates=True)
        em3d_2 = IMP.pmi.mmcif.EMDBDataset('EMD-123', allow_duplicates=True)
        ds7 = dump.add(em3d_1)
        self.assertEqual(ds7.id, 5)
        ds8 = dump.add(em3d_2)
        self.assertEqual(ds8.id, 6)

    def test_dataset_dumper_dump(self):
        """Test DatasetDumper.dump()"""
        dump = IMP.pmi.mmcif.DatasetDumper(EmptyObject())
        pds = dump.add(IMP.pmi.mmcif.CXMSDataset())
        pds.set_location(IMP.pmi.metadata.RepositoryFile(doi='foo', path='bar'))
        ds = dump.add(IMP.pmi.mmcif.PDBDataset('1abc', '1.0', 'test details'))
        ds.add_primary(pds)
        self.assertEqual(ds.location.access_code, '1abc')

        fh = StringIO()
        w = IMP.pmi.mmcif.CifWriter(fh)
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
_ihm_dataset_other.id
_ihm_dataset_other.dataset_list_id
_ihm_dataset_other.data_type
_ihm_dataset_other.doi
_ihm_dataset_other.content_filename
1 1 'CX-MS data' foo bar
#
#
loop_
_ihm_dataset_related_db_reference.id
_ihm_dataset_related_db_reference.dataset_list_id
_ihm_dataset_related_db_reference.db_name
_ihm_dataset_related_db_reference.access_code
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

        d = IMP.pmi.mmcif.ModelDumper(po)
        assembly = IMP.pmi.mmcif.Assembly()
        assembly.id = 42
        protocol = IMP.pmi.mmcif.Protocol()
        protocol.id = 93
        group = IMP.pmi.mmcif.ModelGroup("all models")
        group.id = 7
        model = d.add(simo.prot, protocol, assembly, group)
        self.assertEqual(model.id, 1)
        self.assertEqual(model.get_rmsf('Nup84', (1,)), '.')
        fh = StringIO()
        w = IMP.pmi.mmcif.CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
1 1 7 'all models' 42 93
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

        d = IMP.pmi.mmcif.ModelDumper(po)
        assembly = IMP.pmi.mmcif.Assembly()
        assembly.id = 42
        protocol = IMP.pmi.mmcif.Protocol()
        protocol.id = 93
        group = IMP.pmi.mmcif.ModelGroup("all models")
        group.id = 7
        model = d.add(simo.prot, protocol, assembly, group)
        self.assertEqual(model.id, 1)
        model.parse_rmsf_file(self.get_input_file_name('test.nup84.rmsf'),
                              'Nup84')
        self.assertAlmostEqual(model.get_rmsf('Nup84', (1,)), 4.5, delta=1e-4)
        self.assertRaises(ValueError, model.get_rmsf, 'Nup84', (1,2))
        fh = StringIO()
        w = IMP.pmi.mmcif.CifWriter(fh)
        d.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
1 1 7 'all models' 42 93
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

        d = IMP.pmi.mmcif.ChemCompDumper(po)

        fh = StringIO()
        w = IMP.pmi.mmcif.CifWriter(fh)
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
        w = IMP.pmi.mmcif.CifWriter(fh)
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
        p = IMP.Particle(m)
        IMP.atom.Mass.setup_particle(p, 3.5)
        rot = IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,0,1),
                                                  0.5)
        tran = IMP.algebra.Vector3D(4,6,8)
        tr = IMP.algebra.Transformation3D(rot, tran)
        rf = IMP.algebra.ReferenceFrame3D(tr)
        g = IMP.algebra.Gaussian3D(rf, IMP.algebra.Vector3D(10,11,12))
        IMP.core.Gaussian.setup_particle(p, g)
        ensemble.localization_density = {'Nup84': [p]}
        po.density_dump.add(ensemble)

        fh = StringIO()
        w = IMP.pmi.mmcif.CifWriter(fh)
        po.density_dump.dump(w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_gaussian_obj_ensemble.ordinal_id
_ihm_gaussian_obj_ensemble.entity_id
_ihm_gaussian_obj_ensemble.seq_id_begin
_ihm_gaussian_obj_ensemble.seq_id_end
_ihm_gaussian_obj_ensemble.asym_id
_ihm_gaussian_obj_ensemble.mean_Cartn_x
_ihm_gaussian_obj_ensemble.mean_Cartn_y
_ihm_gaussian_obj_ensemble.mean_Cartn_z
_ihm_gaussian_obj_ensemble.weight
_ihm_gaussian_obj_ensemble.covariance_matrix[1][1]
_ihm_gaussian_obj_ensemble.covariance_matrix[1][2]
_ihm_gaussian_obj_ensemble.covariance_matrix[1][3]
_ihm_gaussian_obj_ensemble.covariance_matrix[2][1]
_ihm_gaussian_obj_ensemble.covariance_matrix[2][2]
_ihm_gaussian_obj_ensemble.covariance_matrix[2][3]
_ihm_gaussian_obj_ensemble.covariance_matrix[3][1]
_ihm_gaussian_obj_ensemble.covariance_matrix[3][2]
_ihm_gaussian_obj_ensemble.covariance_matrix[3][3]
_ihm_gaussian_obj_ensemble.ensemble_id
1 1 1 4 A 4.000 6.000 8.000 3.500 10.230 -0.421 0.000 -0.421 10.770 0.000 0.000
0.000 12.000 42
#
""")

if __name__ == '__main__':
    IMP.test.main()
