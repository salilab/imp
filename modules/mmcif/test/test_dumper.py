from __future__ import print_function
import IMP.test
import IMP.mmcif
import IMP.mmcif.dumper
import ihm.location
import ihm.dataset
import ihm.format
import io
import sys
import os
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

class MockGaussianEMRestraint(IMP.Restraint):

    def __init__(self, m, em_filename):
        self.em_filename = em_filename
        IMP.Restraint.__init__(self, m, "MockRestraint %1%")
    def unprotected_evaluate(self, accum):
        return 0.
    def get_version_info(self):
        return IMP.VersionInfo("IMP authors", "0.1")
    def do_show(self, fh):
        fh.write('MockRestraint')
    def do_get_inputs(self):
        return []

    def get_static_info(self):
        i = IMP.RestraintInfo()
        i.add_string("type", "IMP.isd.GaussianEMRestraint")
        i.add_filename("filename", self.em_filename)
        return i

    def get_dynamic_info(self):
        i = IMP.RestraintInfo()
        i.add_float("cross correlation", 0.4)
        return i


def _get_dumper_output(dumper, system):
    fh = StringIO()
    writer = ihm.format.CifWriter(fh)
    dumper.dump(system, writer)
    return fh.getvalue()

class Tests(IMP.test.TestCase):
    def make_model(self, system, chains=None):
        if chains is None:
            chains = (('foo', 'ACGT', 'A'), ('bar', 'ACGT', 'B'),
                      ('baz', 'ACC', 'C'))
        s = IMP.mmcif.State(system)
        m = s.model
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        for name, seq, cid in chains:
            h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            mol = IMP.atom.Molecule.setup_particle(h)
            mol.set_name(name)
            top.add_child(mol)

            h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            chain = IMP.atom.Chain.setup_particle(h, cid)
            chain.set_sequence(seq)
            mol.add_child(chain)
        return top, s

    def make_model_with_protocol(self, system, chains=None):
        top, s = self.make_model(system, chains)
        m = s.model
        prov = IMP.core.SampleProvenance.setup_particle(m, IMP.Particle(m),
                                "Monte Carlo", 100, 5)
        IMP.core.add_provenance(m, top, prov)

        prov = IMP.core.CombineProvenance.setup_particle(m, IMP.Particle(m),
                                                         5, 500)
        IMP.core.add_provenance(m, top, prov)

        prov = IMP.core.FilterProvenance.setup_particle(m, IMP.Particle(m),
                                                  "Total score", 100.5, 400)
        IMP.core.add_provenance(m, top, prov)

        prov = IMP.core.SampleProvenance.setup_particle(m, IMP.Particle(m),
                                "Molecular Dynamics", 2000, 5, 16)
        IMP.core.add_provenance(m, top, prov)

        prov = IMP.core.ClusterProvenance.setup_particle(m, IMP.Particle(m), 10)
        IMP.core.add_provenance(m, top, prov)
        return top, s

    def test_citation_dumper(self):
        """Test CitationDumper"""
        system = IMP.mmcif.System()
        system.add_citation(
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
        dumper = IMP.mmcif.dumper._CitationDumper()
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
        system._citations[0].page_range = 'e1637'
        out = _get_dumper_output(dumper, system)
        self.assertTrue("'Mol Cell Proteomics' 13 e1637 . 2014 " in out)

    def test_software_dumper(self):
        """Test SoftwareDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        IMP.core.add_imp_provenance(h)
        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [], "model1")

        system.add_software(name='test', classification='test code',
                            description='Some test program',
                            version=1, url='http://salilab.org')
        dumper = IMP.mmcif.dumper._SoftwareDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
loop_
_software.pdbx_ordinal
_software.name
_software.classification
_software.version
_software.type
_software.location
1 'Integrative Modeling Platform (IMP)' 'integrative model building'
%s program https://integrativemodeling.org
2 test 'test code' 1 program http://salilab.org
#
""" % IMP.get_module_version())

    def _assign_entity_ids(self, system):
        ihm.dumper._EntityDumper().finalize(system.system)

    def test_assembly_dumper_get_subassembly(self):
        """Test AssemblyDumper.get_subassembly()"""
        system = IMP.mmcif.System()
        a = system._assemblies
        system.complete_assembly.extend(['a', 'b', 'c'])
        x1 = a.get_subassembly({'a':None, 'b':None})
        x2 = a.get_subassembly({'a':None, 'b':None, 'c':None})

        d = IMP.mmcif.dumper._AssemblyDumper()
        d.finalize(system) # assign IDs to all assemblies
        self.assertEqual(system.complete_assembly.id, 1)
        self.assertEqual(x1.id, 2)
        self.assertEqual(x1, ['a', 'b'])
        self.assertEqual(x2.id, 1)

    def test_assembly_all_modeled(self):
        """Test AssemblyDumper, all components modeled"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, (("foo", "AAA", 'A'),
                                            ("bar", "AAA", 'B'),
                                            ("baz", "AA", 'C')))
        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [], "model1")
        foo, bar, baz = state._all_modeled_components

        system._assemblies.add(IMP.mmcif.data._Assembly((foo, bar)))
        system._assemblies.add(IMP.mmcif.data._Assembly((bar, baz)))

        self._assign_entity_ids(system)
        d = IMP.mmcif.dumper._AssemblyDumper()
        d.finalize(system)
        out = _get_dumper_output(d, system)
        self.assertEqual(out, """#
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
5 2 2 foo 1 B 1 3
6 3 3 foo 1 B 1 3
7 3 3 baz 2 C 1 2
#
""")

    def test_assembly_subset_modeled(self):
        """Test AssemblyDumper, subset of components modeled"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, (("foo", "AAA", 'A'),))
        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [], "model1")
        system.add_non_modeled_chain(name="bar", sequence="AA")
        self._assign_entity_ids(system)
        d = IMP.mmcif.dumper._AssemblyDumper()
        d.finalize(system) # assign IDs
        out = _get_dumper_output(d, system)
        self.assertEqual(out, """#
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
3 2 2 foo 1 A 1 3
#
""")

    def test_model_representation_dumper(self):
        """Test ModelRepresentationDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, (("foo", "AAAA", 'A'),))
        chain = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)[0]
        m = state.model
        # Add starting model information for residues 1-2
        ress = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        sp = IMP.core.StructureProvenance.setup_particle(IMP.Particle(m),
                self.get_input_file_name("test.nup84.pdb"), "A")
        IMP.core.Provenanced.setup_particle(ress, sp)
        chain.add_child(ress)
        res1 = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                               IMP.atom.ALA, 1)
        ress.add_child(res1)
        res2 = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                               IMP.atom.ALA, 2)
        ress.add_child(res2)
        # First matching object (res1 and res2) will be used
        frag1 = IMP.atom.Fragment.setup_particle(IMP.Particle(m),[1,2])
        chain.add_child(frag1)
        frag2 = IMP.atom.Fragment.setup_particle(IMP.Particle(m),[3,4])
        chain.add_child(frag2)
        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [], "model1")
        self._assign_entity_ids(system)
        # Assign starting model IDs
        d = IMP.mmcif.dumper._StartingModelDumper()
        d.finalize(system)
        d = IMP.mmcif.dumper._ModelRepresentationDumper()
        out = _get_dumper_output(d, system)
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
1 1 1 1 foo A 1 2 sphere foo-m1 flexible by-residue 2
2 1 2 1 foo A 3 4 sphere . flexible by-feature 1
#
""")

    def _make_residue_chain(self, name, chain_id, model):
        if name == 'Nup84':
            fname = 'test.nup84.pdb'
            seq = 'ME'
        else:
            fname = 'test.nup85.pdb'
            seq = 'GE'
        h = IMP.atom.read_pdb(self.get_input_file_name(fname), model)
        for hchain in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
            chain = IMP.atom.Chain(hchain)
            chain.set_sequence(seq)
            chain.set_id(chain_id)
            chain.set_name(name)
        for hres in IMP.atom.get_by_type(h, IMP.atom.RESIDUE_TYPE):
            res = IMP.atom.Residue(hres)
            while res.get_number_of_children() > 0:
                res.remove_child(0)
        return h

    def test_starting_model_dumper(self):
        """Test StartingModelDumper"""
        m = IMP.Model()
        system = IMP.mmcif.System()

        state1h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        h1 = self._make_residue_chain('Nup84', 'A', m)
        state1h.add_child(h1)

        # Test multiple states: components that are the same in both states
        # (Nup84) should not be duplicated in the mmCIF output
        state2h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        h1 = self._make_residue_chain('Nup84', 'A', m)
        state2h.add_child(h1)
        h2 = self._make_residue_chain('Nup85', 'B', m)
        state2h.add_child(h2)

        state1 = IMP.mmcif.State(system)
        IMP.mmcif.Ensemble(state1, "cluster 1").add_model([state1h],
                                                          [], "model1")
        state2 = IMP.mmcif.State(system)
        IMP.mmcif.Ensemble(state2, "cluster 1").add_model([state2h],
                                                          [], "model1")

        self._assign_entity_ids(system)
        d = IMP.mmcif.dumper._ExternalReferenceDumper()
        d.finalize(system) # assign file IDs (nup84 pdb = 1, alignment file = 2)
        d = IMP.mmcif.dumper._StartingModelDumper()
        d.finalize(system)
        out = _get_dumper_output(d, system)
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
Nup84-m1 1 Nup84 A 33 2 'comparative model' A 0 1
Nup85-m1 2 Nup85 B 33 2 'comparative model' A 0 4
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
1 Nup84-m1 A 33 2 C 33 424 100.0 1 2 2
2 Nup84-m1 A 429 2 G 482 551 10.0 1 3 2
3 Nup85-m1 A 33 2 C 33 424 100.0 1 2 2
4 Nup85-m1 A 429 2 G 482 551 10.0 1 3 2
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
Nup85-m1 ATOM 1 C CA GLY 2 A 1 -8.986 11.688 -5.817 91.820 3
Nup85-m1 ATOM 2 C CA GLU 2 A 2 -8.986 11.688 -5.817 91.820 4
#
""")

    def test_dataset_dumper(self):
        """Test DatasetDumper"""
        m = IMP.Model()
        system = IMP.mmcif.System()

        l = ihm.location.InputFileLocation(repo="foo", path="bar")
        l.id = 97
        d = ihm.dataset.CXMSDataset(l)
        pds = system.datasets.add(d)

        l = ihm.location.PDBLocation("1abc", "1.0", "test details")
        d = ihm.dataset.PDBDataset(l)
        system.datasets.add(d)

        l = ihm.location.InputFileLocation(repo="foo2", path="bar2")
        l.id = 98
        d = ihm.dataset.PDBDataset(l)
        d.parents.append(pds)
        system.datasets.add(d)

        d = IMP.mmcif.dumper._DatasetDumper()
        out = _get_dumper_output(d, system)
        self.assertEqual(out, """#
loop_
_ihm_dataset_list.id
_ihm_dataset_list.data_type
_ihm_dataset_list.database_hosted
1 'CX-MS data' NO
2 'Experimental model' YES
3 'Experimental model' NO
#
#
loop_
_ihm_dataset_external_reference.id
_ihm_dataset_external_reference.dataset_list_id
_ihm_dataset_external_reference.file_id
1 1 97
2 3 98
#
#
loop_
_ihm_dataset_related_db_reference.id
_ihm_dataset_related_db_reference.dataset_list_id
_ihm_dataset_related_db_reference.db_name
_ihm_dataset_related_db_reference.accession_code
_ihm_dataset_related_db_reference.version
_ihm_dataset_related_db_reference.details
1 2 PDB 1abc 1.0 'test details'
#
#
loop_
_ihm_related_datasets.ordinal_id
_ihm_related_datasets.dataset_list_id_derived
_ihm_related_datasets.dataset_list_id_primary
1 3 1
#
""")

    def test_seq_dif(self):
        """Test StartingModelDumper.dump_seq_dif"""
        class MockEntity(object):
            id = 4
        class MockRes(object):
            def get_index(self):
                return 42
        class MockComponent(object):
            entity = MockEntity()
        class MockSource(object):
            chain_id = 'X'
        class MockModel(object):
            name = 'dummy-m1'
            chain_id = 'H'
            offset = 2
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        dumper = IMP.mmcif.dumper._StartingModelDumper()
        sd = IMP.mmcif.dumper._MSESeqDif(MockRes(), MockComponent(),
                                         MockSource(), MockModel())
        dumper.dump_seq_dif(writer, [sd])
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

    def test_external_reference_dumper_dump(self):
        """Test ExternalReferenceDumper.dump()"""
        system = IMP.mmcif.System()
        exfil = system._external_files

        repo1 = ihm.location.Repository(doi="foo")
        repo2 = ihm.location.Repository(doi="10.5281/zenodo.46266",
                                     url='nup84-v1.0.zip',
                                     top_directory=os.path.join('foo', 'bar'))
        repo3 = ihm.location.Repository(doi="10.5281/zenodo.58025",
                                        url='foo.spd')
        l = ihm.location.InputFileLocation(repo=repo1, path='bar')
        exfil.add(l)
        # Duplicates should be ignored
        l = ihm.location.InputFileLocation(repo=repo1, path='bar')
        exfil.add(l)
        # Different file, same repository
        l = ihm.location.InputFileLocation(repo=repo1, path='baz')
        exfil.add(l)
        # Different repository
        l = ihm.location.OutputFileLocation(repo=repo2, path='baz')
        exfil.add(l)
        # Repository containing a single file (not an archive)
        l = ihm.location.InputFileLocation(repo=repo3, path='foo.spd',
                                           details='EM micrographs')
        exfil.add(l)
        bar = 'test_mmcif_extref.tmp'
        with open(bar, 'w') as f:
            f.write("abcd")
        # Local file
        l = ihm.location.WorkflowFileLocation(bar)
        exfil.add(l)
        # DatabaseLocations should be ignored
        l = ihm.location.PDBLocation('1abc', '1.0', 'test details')
        exfil.add(l)

        dump = IMP.mmcif.dumper._ExternalReferenceDumper()
        dump.finalize(system) # assign IDs
        out = _get_dumper_output(dump, system)
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
""" % bar)
        os.unlink(bar)

    def test_workflow(self):
        """Test output of workflow files"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        m = state.model
        prov = IMP.core.ScriptProvenance.setup_particle(m, IMP.Particle(m),
                                                        __file__)
        IMP.core.add_provenance(m, h, prov)
        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [], "model1")

        root = os.path.dirname(__file__)
        system.add_repository(doi="foo", root=root)
        dump = IMP.mmcif.dumper._ExternalReferenceDumper()
        dump.finalize(system) # assign IDs
        out = _get_dumper_output(dump, system)
        self.assertEqual(out, """#
loop_
_ihm_external_reference_info.reference_id
_ihm_external_reference_info.reference_provider
_ihm_external_reference_info.reference_type
_ihm_external_reference_info.reference
_ihm_external_reference_info.refers_to
_ihm_external_reference_info.associated_url
1 . DOI foo Other .
#
#
loop_
_ihm_external_files.id
_ihm_external_files.reference_id
_ihm_external_files.file_path
_ihm_external_files.content_type
_ihm_external_files.file_size_bytes
_ihm_external_files.details
1 1 %s 'Modeling workflow or script' %d
'Integrative modeling Python script'
#
""" % (os.path.basename(__file__), os.stat(__file__).st_size))

    def test_modeling_protocol(self):
        """Test ProtocolDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model_with_protocol(system)
        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [], "model1")

        dumper = IMP.mmcif.dumper._AssemblyDumper()
        dumper.finalize(system) # Assign assembly IDs

        dumper = IMP.mmcif.dumper._ProtocolDumper()
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
1 1 1 1 . . . Sampling 'Monte Carlo' 0 500 YES NO NO
2 2 1 1 . . . Sampling 'Replica exchange Molecular Dynamics' 400 2000 YES NO NO
#
""")

    def test_post_process(self):
        """Test PostProcessDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model_with_protocol(system)
        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [], "model1")
        dumper = IMP.mmcif.dumper._PostProcessDumper()
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
1 1 1 1 filter energy/score 500 400
2 2 1 1 cluster RMSD 2000 2000
#
""")

    def test_ensemble_info(self):
        """Test EnsembleDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        e = IMP.mmcif.Ensemble(state, "cluster 1")
        e.add_model([h], [], "model1")
        dumper = IMP.mmcif.dumper._EnsembleDumper()
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
1 'cluster 1' . 1 . . 1 . . .
#
""")

    def test_model_list(self):
        """Test ModelListDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        e = IMP.mmcif.Ensemble(state, "cluster 1")
        e.add_model([h], [], "model1")
        e.add_model([h], [], "model2")
        dumper = IMP.mmcif.dumper._ModelListDumper()
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
1 1 1 model1 'cluster 1' . . 1
2 2 1 model2 'cluster 1' . . 1
#
""")

    def test_em3d_dumper(self):
        """Test EM3DDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        em_filename = self.get_input_file_name('test.gmm.txt')
        r = MockGaussianEMRestraint(state.model, em_filename)
        r.set_was_used(True)
        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [r], "model1")
        dumper = IMP.mmcif.dumper._EM3DDumper()
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
1 1 'Gaussian mixture model' . 20 1 0.400
#
""")

    def test_site_dumper_spheres_only(self):
        """Test SiteDumper, spheres only"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, [("foo", "A", 'A'),
                                            ("bar", "AA", "B")])
        m = state.model

        # Add coordinates
        chains = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)
        pres = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        res = IMP.atom.Residue.setup_particle(pres, IMP.atom.ALA, 1)
        xyzr = IMP.core.XYZR.setup_particle(pres)
        xyzr.set_coordinates(IMP.algebra.Vector3D(1,2,3))
        xyzr.set_radius(4.2)

        chains[0].add_child(pres)

        pfrag = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        frag = IMP.atom.Fragment.setup_particle(pfrag, [1,2])
        xyzr = IMP.core.XYZR.setup_particle(pfrag)
        xyzr.set_coordinates(IMP.algebra.Vector3D(4,5,6))
        xyzr.set_radius(9.2)
        chains[1].add_child(pfrag)

        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [], "model1")
        self._assign_entity_ids(system)
        dumper = IMP.mmcif.dumper._SiteDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, """#
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
1 1 1 1 A 1.000 2.000 3.000 4.200 . 1
2 2 1 2 B 4.000 5.000 6.000 9.200 . 1
#
""")


if __name__ == '__main__':
    IMP.test.main()
