import IMP.test
import IMP.mmcif
import ihm.location
import ihm.dataset
import ihm.format
import ihm.dumper
import os
from io import StringIO

class MockGaussianEMRestraint(IMP.Restraint):

    def __init__(self, m, em_filename, particles):
        self.particles = particles
        self.em_filename = em_filename
        IMP.Restraint.__init__(self, m, "MockRestraint %1%")
    def unprotected_evaluate(self, accum):
        return 0.
    def get_version_info(self):
        return IMP.VersionInfo("IMP authors", "0.1")
    def do_show(self, fh):
        fh.write('MockRestraint')
    def do_get_inputs(self):
        return self.particles

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
    def make_model(self, m, chains=None):
        if chains is None:
            chains = (('foo', 'ACGT', 'A'), ('bar', 'ACGT', 'B'),
                      ('baz', 'ACC', 'C'))
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
        return top

    def make_model_with_protocol(self, m, chains=None, sampcon=False):
        top = self.make_model(m, chains)
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
        if sampcon:
            prov.set_name("cluster.0")
            prov.set_precision(42.0)
            prov.set_density(self.get_input_file_name("sampcon.json"))
        IMP.core.add_provenance(m, top, prov)
        return top

    def add_structured_residue(self, m, top, ind):
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, ind)
        IMP.core.XYZR.setup_particle(
            residue, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
        IMP.atom.Mass.setup_particle(residue, 1.0)
        top.add_child(residue)
        return residue

    def test_software_dumper(self):
        """Test SoftwareDumper"""
        m = IMP.Model()
        h = self.make_model(m)
        IMP.core.add_imp_provenance(h)
        c = IMP.mmcif.Writer()
        c.add_model([h], [], name="model1")

        # Assign ID to IMP citation
        dumper = ihm.dumper._CitationDumper()
        dumper.finalize(c.system)

        dumper = ihm.dumper._SoftwareDumper()
        dumper.finalize(c.system)
        out = _get_dumper_output(dumper, c.system)
        # Ignore line breaks, since these will be placed differently depending
        # on how long the version number is
        self.assertEqual(out.replace('\n', ' '), ("""#
loop_
_software.pdbx_ordinal
_software.name
_software.classification
_software.description
_software.version
_software.type
_software.location
_software.citation_id
1 'Integrative Modeling Platform (IMP)' 'integrative model building' .
%s program https://integrativemodeling.org 1
#
""" % IMP.get_module_version()).replace('\n', ' '))

    def _assign_entity_ids(self, c):
        ihm.dumper._EntityDumper().finalize(c.system)

    def _assign_range_ids(self, c):
        ihm.dumper._EntityPolySegmentDumper().finalize(c.system)

    def _assign_asym_ids(self, c):
        ihm.dumper._StructAsymDumper().finalize(c.system)

    def _assign_dataset_ids(self, c):
        ihm.dumper._DatasetDumper().finalize(c.system)

    def _assign_location_ids(self, c):
        ihm.dumper._ExternalReferenceDumper().finalize(c.system)

    def test_assembly_all_modeled(self):
        """Test AssemblyDumper, all components modeled"""
        m = IMP.Model()
        h = self.make_model(m, (("foo", "AAA", 'A'),
                                ("bar", "AAA", 'B'),
                                ("baz", "AA", 'C')))
        chain1, chain2, chain3 = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)
        self.add_structured_residue(m, chain1, 1)
        self.add_structured_residue(m, chain1, 2)
        self.add_structured_residue(m, chain1, 3)
        self.add_structured_residue(m, chain2, 1)
        self.add_structured_residue(m, chain2, 2)
        self.add_structured_residue(m, chain2, 3)
        self.add_structured_residue(m, chain3, 1)
        self.add_structured_residue(m, chain3, 2)
        c = IMP.mmcif.Writer()
        c.add_model([h], [])

        foo, bar, baz = c.system.asym_units

        c.system.orphan_assemblies.append(ihm.Assembly((foo, bar)))
        c.system.orphan_assemblies.append(ihm.Assembly((bar, baz)))

        c.system._make_complete_assembly()
        self._assign_entity_ids(c)
        self._assign_asym_ids(c)
        d = ihm.dumper._EntityPolySegmentDumper()
        d.finalize(c.system)
        out = _get_dumper_output(d, c.system)
        self.assertEqual(out, """#
loop_
_ihm_entity_poly_segment.id
_ihm_entity_poly_segment.entity_id
_ihm_entity_poly_segment.seq_id_begin
_ihm_entity_poly_segment.seq_id_end
_ihm_entity_poly_segment.comp_id_begin
_ihm_entity_poly_segment.comp_id_end
1 1 1 3 ALA ALA
2 2 1 2 ALA ALA
#
""")

        d = ihm.dumper._AssemblyDumper()
        d.finalize(c.system)
        out = _get_dumper_output(d, c.system)
        self.assertEqual(out, """#
loop_
_ihm_struct_assembly.id
_ihm_struct_assembly.name
_ihm_struct_assembly.description
1 'Complete assembly' 'All known components & All components modeled by IMP'
2 . .
3 . .
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
5 2 2 foo 1 B 1
6 3 3 foo 1 B 1
7 3 3 baz 2 C 2
#
""")

    def test_model_representation_dumper(self):
        """Test ModelRepresentationDumper"""
        m = IMP.Model()
        h = self.make_model(m, (("foo", "AAAA", 'A'),))
        chain = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)[0]
        # Add starting model information for residues 1-2
        ress = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        sp = IMP.core.StructureProvenance.setup_particle(IMP.Particle(m),
                self.get_input_file_name("test.nup84.pdb"), "A")
        IMP.core.Provenanced.setup_particle(ress, sp)
        chain.add_child(ress)
        res1 = self.add_structured_residue(m, ress, 1)
        res2 = self.add_structured_residue(m, ress, 2)
        frag1 = IMP.atom.Fragment.setup_particle(IMP.Particle(m), [3, 4])
        self.add_structure(frag1)
        chain.add_child(frag1)
        c = IMP.mmcif.Writer()
        c.add_model([h], [])

        self._assign_entity_ids(c)
        self._assign_asym_ids(c)
        self._assign_range_ids(c)
        r = c.system.ensembles[0].model_group[0].representation
        self.assertEqual([s.asym_unit.seq_id_range for s in r], [(1,2), (3,4)])
        # Assign starting model IDs
        d = ihm.dumper._StartingModelDumper()
        d.finalize(c.system)
        d = ihm.dumper._ModelRepresentationDumper()
        d.finalize(c.system)
        out = _get_dumper_output(d, c.system)
        self.assertEqual(out, """#
loop_
_ihm_model_representation.id
_ihm_model_representation.name
_ihm_model_representation.details
1 . .
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
1 1 1 foo A 1 sphere 1 flexible by-residue . .
2 1 1 foo A 2 sphere . flexible by-feature 1 .
#
""")

    def add_state(self, m, top, state_index, name):
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state = IMP.atom.State.setup_particle(h, state_index)
        state.set_name(name)
        top.add_child(state)
        return state

    def _make_residue_chain(self, name, chain_id, model, cif=False):
        if name == 'Nup84':
            if cif:
                fname = 'test.nup84.cif'
            else:
                fname = 'test.nup84.pdb'
            seq = 'ME'
        else:
            fname = 'test.nup85.pdb'
            seq = 'GE'
        h = IMP.atom.read_pdb_or_mmcif(self.get_input_file_name(fname), model)
        for hchain in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
            chain = IMP.atom.Chain(hchain)
            chain.set_sequence(seq)
            chain.set_id(chain_id)
            chain.set_name(name)
        for hres in IMP.atom.get_by_type(h, IMP.atom.RESIDUE_TYPE):
            # Remove all atoms, replace with a dummy sphere
            res = IMP.atom.Residue(hres)
            while res.get_number_of_children() > 0:
                res.remove_child(0)
            self.add_structure(res)
        return h

    def add_structure(self, p):
        IMP.core.XYZR.setup_particle(
            p, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
        IMP.atom.Mass.setup_particle(p, 1.0)

    def test_starting_model_dumper_pdb(self):
        """Test StartingModelDumper with PDB starting models"""
        self._internal_test_starting_model_dumper(cif=False)

    def test_starting_model_dumper_cif(self):
        """Test StartingModelDumper with mmCIF starting models"""
        self._internal_test_starting_model_dumper(cif=True)

    def _internal_test_starting_model_dumper(self, cif):
        m = IMP.Model()

        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state1h = self.add_state(m, top, 0, "State1")

        h1 = self._make_residue_chain('Nup84', 'A', m, cif=cif)
        state1h.add_child(h1)

        # Test multiple states: components that are the same in both states
        # (Nup84) should not be duplicated in the mmCIF output
        state2h = self.add_state(m, top, 0, "State2")
        h1 = self._make_residue_chain('Nup84', 'A', m, cif=cif)
        state2h.add_child(h1)
        h2 = self._make_residue_chain('Nup85', 'B', m)
        state2h.add_child(h2)

        c = IMP.mmcif.Writer()
        c.add_model([top], [])

        self._assign_entity_ids(c)
        self._assign_asym_ids(c)
        self._assign_range_ids(c)
        # Assign dataset IDs (templates=1,2, comparative models=3,4)
        self._assign_dataset_ids(c)
        # assign file IDs (alignment file = 1)
        self._assign_location_ids(c)
        d = ihm.dumper._StartingModelDumper()
        d.finalize(c.system)
        out = _get_dumper_output(d, c.system)
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
1 1 Nup84 A 1 'comparative model' A 0 3 .
2 2 Nup85 B 2 'comparative model' A 0 4 .
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
1 1 A 33 424 C 33 424 100.000 1 1 1
2 1 A 429 488 G 482 551 10.000 1 2 1
3 2 A 33 424 C 33 424 100.000 1 1 1
4 2 A 429 488 G 482 551 10.000 1 2 1
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
1 ATOM 1 C CA MET 1 A 1 -8.986 11.688 -5.817 91.820 1
1 ATOM 2 C CA GLU 1 A 2 -8.986 11.688 -5.817 91.820 2
2 ATOM 1 C CA GLY 2 B 1 -8.986 11.688 -5.817 91.820 3
2 ATOM 2 C CA GLU 2 B 2 -8.986 11.688 -5.817 91.820 4
#
""")

    def test_workflow(self):
        """Test output of workflow files"""
        m = IMP.Model()
        h = self.make_model(m)
        prov = IMP.core.ScriptProvenance.setup_particle(m, IMP.Particle(m),
                                                        __file__)
        IMP.core.add_provenance(m, h, prov)
        c = IMP.mmcif.Writer()
        c.add_model([h], [], name="model1")

        root = os.path.dirname(__file__)
        repo = ihm.location.Repository(doi='foo', root=root)
        c.system.update_locations_in_repositories([repo])

        dump = ihm.dumper._ExternalReferenceDumper()
        dump.finalize(c.system) # assign IDs
        out = _get_dumper_output(dump, c.system)
        self.assertEqual(out, """#
loop_
_ihm_external_reference_info.reference_id
_ihm_external_reference_info.reference_provider
_ihm_external_reference_info.reference_type
_ihm_external_reference_info.reference
_ihm_external_reference_info.refers_to
_ihm_external_reference_info.associated_url
_ihm_external_reference_info.details
1 . DOI foo Other . .
#
#
loop_
_ihm_external_files.id
_ihm_external_files.reference_id
_ihm_external_files.file_path
_ihm_external_files.content_type
_ihm_external_files.file_format
_ihm_external_files.file_size_bytes
_ihm_external_files.details
1 1 %s 'Modeling workflow or script' . %d
'Integrative modeling Python script'
#
""" % (os.path.basename(__file__), os.stat(__file__).st_size))

    def test_modeling_protocol(self):
        """Test ProtocolDumper"""
        m = IMP.Model()
        h = self.make_model_with_protocol(m)
        c = IMP.mmcif.Writer()
        c.add_model([h], [], name="model1")

        self._assign_entity_ids(c)
        self._assign_asym_ids(c)
        dumper = ihm.dumper._AssemblyDumper()
        dumper.finalize(c.system) # Assign assembly IDs

        dumper = ihm.dumper._ProtocolDumper()
        dumper.finalize(c.system)
        out = _get_dumper_output(dumper, c.system)
        self.assertEqual(out, """#
loop_
_ihm_modeling_protocol.id
_ihm_modeling_protocol.protocol_name
_ihm_modeling_protocol.num_steps
_ihm_modeling_protocol.details
1 . 1 .
2 . 1 .
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
1 1 1 2 . Sampling 'Monte Carlo' 0 500 YES NO NO YES . . .
2 2 1 2 . Sampling 'Replica exchange Molecular Dynamics' 400 2000 YES NO NO YES
. . .
#
""")

    def test_post_process(self):
        """Test PostProcessDumper"""
        m = IMP.Model()
        h = self.make_model_with_protocol(m)
        c = IMP.mmcif.Writer()
        c.add_model([h], [], name="model1")

        self._assign_entity_ids(c)
        self._assign_asym_ids(c)
        dumper = ihm.dumper._AssemblyDumper()
        dumper.finalize(c.system)
        dumper = ihm.dumper._ProtocolDumper()
        dumper.finalize(c.system)
        dumper = ihm.dumper._PostProcessDumper()
        dumper.finalize(c.system)
        out = _get_dumper_output(dumper, c.system)
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
1 1 1 1 filter energy/score 500 400 2 . . . .
2 2 1 1 cluster RMSD 2000 2000 2 . . . .
#
""")

    def test_ensemble_info(self):
        """Test EnsembleDumper"""
        m = IMP.Model()
        h = self.make_model(m)
        c = IMP.mmcif.Writer()
        ens = c.add_model([h], [], name="model1")
        ens[None].name = 'cluster 1'

        dumper = ihm.dumper._ModelDumper()
        dumper.finalize(c.system)
        dumper = ihm.dumper._EnsembleDumper()
        dumper.finalize(c.system)
        out = _get_dumper_output(dumper, c.system)
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
_ihm_ensemble_info.model_group_superimposed_flag
_ihm_ensemble_info.sub_sample_flag
_ihm_ensemble_info.sub_sampling_type
1 'cluster 1' . 1 . . 1 1 . . . . NO .
#
""")

    def test_model_list(self):
        """Test ModelListDumper"""
        m = IMP.Model()
        h = self.make_model(m)
        c = IMP.mmcif.Writer()
        ens = c.add_model([h], [], name="model1")
        ens[None].model_group.name = 'cluster 1'
        c.add_model([h], [], name="model2", ensembles=ens)
        self._assign_entity_ids(c)
        self._assign_asym_ids(c)
        dumper = ihm.dumper._ModelRepresentationDumper()
        dumper.finalize(c.system)
        dumper = ihm.dumper._AssemblyDumper()
        dumper.finalize(c.system)
        dumper = ihm.dumper._ModelDumper()
        dumper.finalize(c.system)
        out = _get_dumper_output(dumper, c.system)
        self.assertEqual(out, """#
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 model1 2 . 1
2 model2 2 . 1
#
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 'cluster 1' .
#
#
loop_
_ihm_model_group_link.group_id
_ihm_model_group_link.model_id
1 1
1 2
#
""")

    def test_em3d_dumper(self):
        """Test EM3DDumper"""
        m = IMP.Model()
        h = self.make_model(m)
        chain1 = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)[0]
        res1 = self.add_structured_residue(m, chain1, 1)
        em_filename = self.get_input_file_name('test.gmm.txt')
        r = MockGaussianEMRestraint(m, em_filename, [res1.get_particle()])
        r.set_was_used(True)
        c = IMP.mmcif.Writer()
        c.add_model([h], [r], name="model1")

        # Assign dataset ID (=2 since the gmm is derived from an MRC)
        self._assign_dataset_ids(c)
        self._assign_entity_ids(c)
        self._assign_asym_ids(c)
        dumper = ihm.dumper._ModelDumper()
        dumper.finalize(c.system)
        dumper = ihm.dumper._AssemblyDumper()
        dumper.finalize(c.system)
        dumper = ihm.dumper._EM3DDumper()
        dumper.finalize(c.system)
        out = _get_dumper_output(dumper, c.system)
        self.assertEqual(out, """#
loop_
_ihm_3dem_restraint.id
_ihm_3dem_restraint.dataset_list_id
_ihm_3dem_restraint.fitting_method
_ihm_3dem_restraint.fitting_method_citation_id
_ihm_3dem_restraint.struct_assembly_id
_ihm_3dem_restraint.map_segment_flag
_ihm_3dem_restraint.number_of_gaussians
_ihm_3dem_restraint.model_id
_ihm_3dem_restraint.cross_correlation_coefficient
_ihm_3dem_restraint.details
1 2 'Gaussian mixture model' . 3 . 20 1 0.400 .
#
""")

    def test_site_dumper_spheres_only(self):
        """Test SiteDumper, spheres only"""
        m = IMP.Model()
        h = self.make_model(m, [("foo", "AA", 'A'),
                                ("bar", "AAA", "B")])

        # Add coordinates
        chains = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)
        pres = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        res = IMP.atom.Residue.setup_particle(pres, IMP.atom.ALA, 1)
        IMP.atom.Mass.setup_particle(pres, 1.0)
        xyzr = IMP.core.XYZR.setup_particle(pres)
        xyzr.set_coordinates(IMP.algebra.Vector3D(1,2,3))
        xyzr.set_radius(4.2)

        chains[0].add_child(pres)

        pfrag = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        frag = IMP.atom.Fragment.setup_particle(pfrag, [1,2])
        IMP.atom.Mass.setup_particle(pfrag, 1.0)
        xyzr = IMP.core.XYZR.setup_particle(pfrag)
        xyzr.set_coordinates(IMP.algebra.Vector3D(4,5,6))
        xyzr.set_radius(9.2)
        chains[1].add_child(pfrag)

        c = IMP.mmcif.Writer()
        ens = c.add_model([h], [], name="model1")
        ens[None].model_group.name = 'cluster 1'

        self._assign_entity_ids(c)
        self._assign_asym_ids(c)
        dumper = ihm.dumper._AssemblyDumper()
        dumper.finalize(c.system)
        dumper = ihm.dumper._ModelRepresentationDumper()
        dumper.finalize(c.system)
        dumper = ihm.dumper._ModelDumper()
        dumper.finalize(c.system)
        out = _get_dumper_output(dumper, c.system)
        self.assertEqual(out, """#
loop_
_ihm_model_list.model_id
_ihm_model_list.model_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 model1 2 . 1
#
#
loop_
_ihm_model_group.id
_ihm_model_group.name
_ihm_model_group.details
1 'cluster 1' .
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
1 1 1 1 A 1.000 2.000 3.000 4.200 . 1
2 2 1 2 B 4.000 5.000 6.000 9.200 . 1
#
""")

    def test_no_localization_densities(self):
        """Test DensityDumper with no density information"""
        m = IMP.Model()
        h = self.make_model_with_protocol(m)
        c = IMP.mmcif.Writer()
        c.add_model([h], [], name="model1")

        dumper = ihm.dumper._DensityDumper()
        dumper.finalize(c.system)
        out = _get_dumper_output(dumper, c.system)
        self.assertEqual(out, "")

    def test_sampcon_localization_densities(self):
        """Test DensityDumper with IMP.sampcon output"""
        m = IMP.Model()
        h = self.make_model_with_protocol(m, sampcon=True)
        c = IMP.mmcif.Writer()
        ens = c.add_model([h], [], name="model1")
        e = ens[None]

        # Name and precision should be assigned based on sampcon output
        self.assertEqual(e.name, "cluster.0")
        self.assertAlmostEqual(e.precision, 42.0, delta=1e-4)

        den1, den2 = e.densities
        self.assertEqual(den1.asym_unit.details, 'foo')
        self.assertEqual(den1.asym_unit.seq_id_range, (1, 4))
        self.assertEqual(os.path.basename(den1.file.path), 'test_1.mrc')
        self.assertEqual(den2.asym_unit.details, 'bar')
        self.assertEqual(den2.asym_unit.seq_id_range, (2, 3))
        self.assertEqual(os.path.basename(den2.file.path), 'test_2.mrc')

        # assign IDs
        self._assign_entity_ids(c)
        self._assign_asym_ids(c)
        self._assign_range_ids(c)
        self._assign_location_ids(c)
        dumper = ihm.dumper._EnsembleDumper()
        dumper.finalize(c.system)

        dumper = ihm.dumper._DensityDumper()
        dumper.finalize(c.system)
        out = _get_dumper_output(dumper, c.system)
        self.assertEqual(out, """#
loop_
_ihm_localization_density_files.id
_ihm_localization_density_files.file_id
_ihm_localization_density_files.ensemble_id
_ihm_localization_density_files.entity_id
_ihm_localization_density_files.asym_id
_ihm_localization_density_files.entity_poly_segment_id
1 1 1 1 A 1
2 2 1 1 B 3
#
""")


if __name__ == '__main__':
    IMP.test.main()
