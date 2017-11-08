from __future__ import print_function
import IMP.test
import IMP.mmcif
import IMP.mmcif.dumper
import IMP.mmcif.format
import IMP.mmcif.dataset
import io
import sys
import os
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

def _get_dumper_output(dumper, system):
    fh = StringIO()
    writer = IMP.mmcif.format._CifWriter(fh)
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
1 test 'test code' 1 program http://salilab.org
#
""")

    def test_entry_dumper(self):
        """Test EntryDumper"""
        system = IMP.mmcif.System()
        dumper = IMP.mmcif.dumper._EntryDumper()
        out = _get_dumper_output(dumper, system)
        self.assertEqual(out, "data_imp_model\n_entry.id imp_model\n")

    def test_chem_comp_dumper(self):
        """Test ChemCompDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        state.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._ChemCompDumper()
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

    def test_entity_dumper(self):
        """Test EntityDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        state.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._EntityDumper()
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
1 polymer man foo ? 1 ?
2 polymer man baz ? 1 ?
#
""")

    def test_entity_poly_dumper(self):
        """Test EntityPolyDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        state.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._EntityPolyDumper()
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
2 polypeptide(L) no no C ACC ACC
#
""")

    def test_entity_poly_seq_dumper(self):
        """Test EntityPolySeqDumper"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        state.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._EntityPolySeqDumper()
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
        system = IMP.mmcif.System()
        h, state = self.make_model(system)
        state.add_hierarchy(h)
        dumper = IMP.mmcif.dumper._StructAsymDumper()
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

    def test_assembly_dumper_get_subassembly(self):
        """Test AssemblyDumper.get_subassembly()"""
        d = IMP.mmcif.dumper._AssemblyDumper()
        complete = IMP.mmcif.data._Assembly(['a', 'b', 'c'])
        d.add(complete)
        x1 = d.get_subassembly({'a':None, 'b':None})
        x2 = d.get_subassembly({'a':None, 'b':None, 'c':None})
        d.finalize(None) # assign IDs to all assemblies
        self.assertEqual(complete.id, 1)
        self.assertEqual(x1.id, 2)
        self.assertEqual(x1, ['a', 'b'])
        self.assertEqual(x2.id, 1)

    def test_assembly_all_modeled(self):
        """Test AssemblyDumper, all components modeled"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, (("foo", "AAA", 'A'),
                                            ("bar", "AAA", 'B'),
                                            ("baz", "AA", 'C')))
        state.add_hierarchy(h)
        foo, bar, baz = state._all_modeled_components
        d = IMP.mmcif.dumper._AssemblyDumper()

        d.add(IMP.mmcif.data._Assembly((foo, bar)))
        d.add(IMP.mmcif.data._Assembly((bar, baz)))

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
3 2 2 foo 1 B 1 3
4 2 2 baz 2 C 1 2
#
""")

    def test_assembly_subset_modeled(self):
        """Test AssemblyDumper, subset of components modeled"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, (("foo", "AAA", 'A'),))
        state.add_hierarchy(h)
        system.add_non_modeled_chain(name="bar", sequence="AA")
        d = system.assembly_dump
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
        state.add_hierarchy(h)
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
        state1.add_hierarchy(state1h)
        state2 = IMP.mmcif.State(system)
        state2.add_hierarchy(state2h)

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

        l = IMP.mmcif.dataset.FileLocation(repo="foo", path="bar")
        l.id = 97
        d = IMP.mmcif.dataset.CXMSDataset(l)
        pds = system.datasets.add(d)

        l = IMP.mmcif.dataset.PDBLocation("1abc", "1.0", "test details")
        d = IMP.mmcif.dataset.PDBDataset(l)
        system.datasets.add(d)

        l = IMP.mmcif.dataset.FileLocation(repo="foo2", path="bar2")
        l.id = 98
        d = IMP.mmcif.dataset.PDBDataset(l)
        d.add_parent(pds)
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
        writer = IMP.mmcif.format._CifWriter(fh)
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

        repo1 = IMP.mmcif.dataset.Repository(doi="foo")
        repo2 = IMP.mmcif.dataset.Repository(doi="10.5281/zenodo.46266",
                                     url='nup84-v1.0.zip',
                                     top_directory=os.path.join('foo', 'bar'))
        repo3 = IMP.mmcif.dataset.Repository(doi="10.5281/zenodo.58025",
                                             url='foo.spd')
        l = IMP.mmcif.dataset.FileLocation(repo=repo1, path='bar')
        exfil.add_input(l)
        # Duplicates should be ignored
        l = IMP.mmcif.dataset.FileLocation(repo=repo1, path='bar')
        exfil.add_input(l)
        # Different file, same repository
        l = IMP.mmcif.dataset.FileLocation(repo=repo1, path='baz')
        exfil.add_input(l)
        # Different repository
        l = IMP.mmcif.dataset.FileLocation(repo=repo2, path='baz')
        exfil.add_output(l)
        # Repository containing a single file (not an archive)
        l = IMP.mmcif.dataset.FileLocation(repo=repo3, path='foo.spd',
                                           details='EM micrographs')
        exfil.add_input(l)
        bar = 'test_mmcif_extref.tmp'
        with open(bar, 'w') as f:
            f.write("abcd")
        # Local file
        l = IMP.mmcif.dataset.FileLocation(bar)
        exfil.add_workflow(l)
        # DatabaseLocations should be ignored
        l = IMP.mmcif.dataset.PDBLocation('1abc', '1.0', 'test details')
        exfil.add_workflow(l)

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
        root = os.path.dirname(sys.argv[0]) or '.'
        system.add_repository(doi="foo", root=root)
        system.add_modeling_script(path=__file__, details='Main script')
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
1 1 test_dumper.py 'Modeling workflow or script' %d 'Main script'
#
""" % os.stat(__file__).st_size)


if __name__ == '__main__':
    IMP.test.main()
