#!/usr/bin/python3

"""Check which IHM data items are not supported by python-ihm.

   Data items that python-ihm does not support are silently dropped on a
   read+write cycle (such as that done by make_mmcif). To find them, this
   script builds an ihm.System containing dummy data, such that every dumper
   that outputs an IHM category or keyword is called and generates output,
   writes it to a file, and then validates that file against the PDBx, IHM
   and FLR dictionaries, warning about every optional category or keyword
   containing `ihm_` that is not present in the file.

   The script also checks that the dummy data really does exercise every
   dumper, so that a gap in the dummy data is not mistaken for an
   unsupported data item.

   This should be periodically rerun in case the dictionaries are updated.
"""

import inspect
import io
import re
import sys
import urllib.request
import warnings

import ihm
import ihm.analysis
import ihm.cross_linkers
import ihm.dataset
import ihm.dictionary
import ihm.dumper
import ihm.flr
import ihm.geometry
import ihm.location
import ihm.model
import ihm.multi_state_scheme
import ihm.protocol
import ihm.representation
import ihm.restraint
import ihm.startmodel

DICTIONARIES = [
    'http://mmcif.wwpdb.org/dictionaries/ascii/mmcif_pdbx_v50.dic',
    'http://mmcif.wwpdb.org/dictionaries/ascii/mmcif_ihm.dic',
    'https://raw.githubusercontent.com/ihmwg/flrCIF/master/dist/'
    'mmcif_ihm_flr.dic']

# String literals in dumper source code that name an IHM category
# (starting with an underscore) or keyword
IHM_LITERAL = re.compile(r"""["'](_?[a-z0-9_]*ihm_[a-z0-9_]*)["']""")


EX_CATEGORIES = frozenset(
    # Don't complain about missing deprecated categories
    ('ihm_ordered_ensemble',))

EX_KEYWORDS = frozenset(
    # Don't complain about entity or assembly descriptions (they are just
    # copies of descriptions in the entity/assembly tables already)
    ('struct_assembly_description', 'entity_description',
     'entity_description_1', 'entity_description_2'))


def is_ihm_category(name):
    """Return True iff the given category is an IHM category that we should
       warn about if it is missing"""
    if name in EX_CATEGORIES:
        return False
    return 'ihm_' in name


def is_ihm_keyword(cat_name, key_name):
    """Return True iff the given keyword is an IHM keyword that we should
       warn about if it is missing"""
    if key_name in EX_KEYWORDS:
        return False
    return is_ihm_category(cat_name) or 'ihm_' in key_name


class DummyStartingModel(ihm.startmodel.StartingModel):
    """A starting model with a single atom and some sequence differences"""
    def get_atoms(self):
        return [ihm.model.Atom(asym_unit=self.asym_unit.asym, seq_id=1,
                               atom_id='CA', type_symbol='C',
                               x=1., y=2., z=3., biso=4.)]

    def get_seq_dif(self):
        return [ihm.startmodel.MSESeqDif(db_seq_id=5, seq_id=5),
                ihm.startmodel.SeqDif(db_seq_id=6, seq_id=6,
                                      db_comp_id='LEU', details='LEU -> GLY')]


class DummyModel(ihm.model.Model):
    """A model with one atom or sphere for each segment in the
       representation"""
    def __init__(self, asym_a, asym_b, asym_h, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._atoms = [ihm.model.Atom(asym_unit=asym_a, seq_id=1,
                                      atom_id='CA', type_symbol='C',
                                      x=1., y=2., z=3.),
                       ihm.model.Atom(asym_unit=asym_h, seq_id=None,
                                      atom_id='FE', type_symbol='FE',
                                      x=1., y=2., z=3., het=True)]
        self._spheres = [
            ihm.model.Sphere(asym_unit=asym_a, seq_id_range=(6, 6),
                             x=4., y=5., z=6., radius=7., rmsf=8.),
            ihm.model.Sphere(asym_unit=asym_a, seq_id_range=(11, 20),
                             x=4., y=5., z=6., radius=7.),
            ihm.model.Sphere(asym_unit=asym_b, seq_id_range=(1, 20),
                             x=4., y=5., z=6., radius=7.)]

    def get_atoms(self):
        return self._atoms

    def get_spheres(self):
        return self._spheres


def make_system():
    """Make an ihm.System containing enough dummy data that every dumper
       that outputs IHM categories or keywords is called and generates
       output."""
    system = ihm.System(title='Dummy system', id='dummy')
    system.collections.append(ihm.Collection('coll1', name='Collection 1'))

    repo = ihm.location.Repository(doi='10.5281/zenodo.1234',
                                   url='https://zenodo.org/dummy.zip')
    software = ihm.Software(name='dummy', classification='modeling',
                            description='Dummy software',
                            location='https://example.org', version='1.0')
    system.software.append(software)
    script = ihm.location.WorkflowFileLocation(repo=repo, path='script.py')
    citation = ihm.Citation(pmid='1234', title='Dummy', journal='J Dummy',
                            volume=1, page_range=(1, 10), year=2026,
                            authors=['Doe J'], doi='10.1/dummy')
    system.citations.append(citation)

    # Entities and asyms
    protein = ihm.Entity('ACDEFGHIKLMNPQRSTVWY', description='Protein')
    heme = ihm.Entity([ihm.NonPolymerChemComp('HEM', name='heme')],
                      description='Heme')
    system.entities.extend((protein, heme))
    asym_a = ihm.AsymUnit(protein, details='Chain A')
    asym_b = ihm.AsymUnit(protein, details='Chain B')
    asym_h = ihm.AsymUnit(heme, details='Heme ligand')
    system.asym_units.extend((asym_a, asym_b, asym_h))

    # Assembly of a subset of the system
    assembly = ihm.Assembly((asym_a, asym_b(1, 10)), name='Subassembly',
                            description='Part of the system')
    system.orphan_assemblies.append(assembly)

    # Datasets, with related datasets, transformations and groups
    pdb_loc = ihm.location.PDBLocation('1abc', version='1.0',
                                       details='PDB entry')
    template_ds = ihm.dataset.PDBDataset(pdb_loc)
    file_loc = ihm.location.InputFileLocation(repo=repo, path='xl.csv',
                                              details='Crosslink file')
    xl_ds = ihm.dataset.CXMSDataset(file_loc)
    em3d_ds = ihm.dataset.EMDensityDataset(
        ihm.location.EMDBLocation('EMD-1234'))
    em2d_ds = ihm.dataset.EM2DClassDataset(
        ihm.location.InputFileLocation(repo=repo, path='class.png'))
    sas_ds = ihm.dataset.SASDataset(ihm.location.SASBDBLocation('SASDA12'))
    epr_ds = ihm.dataset.EPRDataset(
        ihm.location.InputFileLocation(repo=repo, path='epr.dat'))
    hdx_ds = ihm.dataset.HDXDataset(
        ihm.location.InputFileLocation(repo=repo, path='hdx.dat'))
    fp_ds = ihm.dataset.HydroxylRadicalFootprintingDataset(
        ihm.location.InputFileLocation(repo=repo, path='fp.dat'))
    contact_ds = ihm.dataset.PredictedContactsDataset(
        ihm.location.InputFileLocation(repo=repo, path='contacts.dat'))
    fret_ds = ihm.dataset.FRETDataset(
        ihm.location.InputFileLocation(repo=repo, path='fret.dat'))
    # Derived dataset with a transformation relative to its parent
    transform = ihm.geometry.Transformation(
        rot_matrix=[[1., 0., 0.], [0., 1., 0.], [0., 0., 1.]],
        tr_vector=[1., 2., 3.])
    em3d_ds.parents.append(ihm.dataset.TransformedDataset(template_ds,
                                                          transform))
    all_datasets = [template_ds, xl_ds, em3d_ds, em2d_ds, sas_ds, epr_ds,
                    hdx_ds, fp_ds, contact_ds, fret_ds]
    system.orphan_datasets.extend(all_datasets)
    dsg = ihm.dataset.DatasetGroup(all_datasets, name='All datasets',
                                   application='modeling',
                                   details='All input data')
    system.orphan_dataset_groups.append(dsg)

    # Representation using every type of segment
    starting_model = DummyStartingModel(
        asym_a(1, 20), template_ds, 'A',
        templates=[ihm.startmodel.Template(
            dataset=template_ds, asym_id='C', seq_id_range=(1, 20),
            template_seq_id_range=(101, 120),
            sequence_identity=ihm.startmodel.SequenceIdentity(40.),
            alignment_file=ihm.location.InputFileLocation(
                repo=repo, path='align.ali'))],
        software=software, script_file=script, description='Dummy model')
    system.orphan_starting_models.append(starting_model)
    rep = ihm.representation.Representation(
        [ihm.representation.AtomicSegment(asym_a(1, 5), rigid=True,
                                          starting_model=starting_model),
         ihm.representation.ResidueSegment(asym_a(6, 10), rigid=False,
                                           primitive='sphere'),
         ihm.representation.MultiResidueSegment(asym_a(11, 20), rigid=False,
                                                primitive='sphere'),
         ihm.representation.FeatureSegment(asym_b, rigid=False,
                                           primitive='sphere', count=2),
         ihm.representation.AtomicSegment(asym_h, rigid=True)],
        name='Dummy representation', details='Mixed resolution')
    system.orphan_representations.append(rep)

    # Protocol with modeling and analysis steps
    protocol = ihm.protocol.Protocol(name='Modeling', details='Dummy')
    protocol.steps.append(ihm.protocol.Step(
        assembly=system.complete_assembly, dataset_group=dsg,
        method='Monte Carlo', num_models_begin=0, num_models_end=100,
        software=software, script_file=script, multi_scale=True,
        multi_state=True, ordered=True, name='Sampling',
        description='Dummy sampling'))
    analysis = ihm.analysis.Analysis()
    analysis.steps.append(ihm.analysis.ClusterStep(
        feature='RMSD', num_models_begin=100, num_models_end=10,
        assembly=system.complete_assembly, dataset_group=dsg,
        software=software, script_file=script, details='Clustering'))
    protocol.analyses.append(analysis)
    system.orphan_protocols.append(protocol)

    # Models, grouped into states
    model = DummyModel(asym_a, asym_b, asym_h,
                       assembly=system.complete_assembly, protocol=protocol,
                       representation=rep, name='Model 1')
    model2 = DummyModel(asym_a, asym_b, asym_h,
                        assembly=system.complete_assembly, protocol=protocol,
                        representation=rep, name='Model 2')
    mg1 = ihm.model.ModelGroup([model], name='Group 1', details='Cluster 1')
    mg2 = ihm.model.ModelGroup([model2], name='Group 2', details='Cluster 2')
    mg1.representatives = [ihm.model.ModelRepresentative(
        model, selection_criteria='medoid')]
    model.not_modeled_residue_ranges.append(
        ihm.model.NotModeledResidueRange(asym_b, 15, 20, reason='Other'))
    state1 = ihm.model.State([mg1], type='complex', name='State 1',
                             details='First state',
                             experiment_type='Fraction of bulk',
                             population_fraction=0.4)
    state2 = ihm.model.State([mg2], type='complex', name='State 2',
                             details='Second state',
                             experiment_type='Fraction of bulk',
                             population_fraction=0.6)
    system.state_groups.append(ihm.model.StateGroup([state1, state2]))

    # Ensemble with subsamples and localization densities
    density_file = ihm.location.OutputFileLocation(repo=repo,
                                                   path='density.mrc')
    ensemble = ihm.model.Ensemble(
        mg1, num_models=10, post_process=analysis.steps[0],
        clustering_method='Hierarchical', clustering_feature='RMSD',
        name='Cluster 1', precision=4.2,
        file=ihm.location.OutputFileLocation(repo=repo, path='ens.dcd'),
        details='Dummy ensemble', superimposed=True)
    ensemble.subsamples.append(ihm.model.RandomSubsample(
        'sample1', num_models=5, model_group=mg1,
        file=ihm.location.OutputFileLocation(repo=repo, path='sub.dcd')))
    ensemble.densities.append(ihm.model.LocalizationDensity(density_file,
                                                            asym_a))
    system.ensembles.append(ensemble)

    # Ordered process connecting model groups
    proc = ihm.model.OrderedProcess('time steps', description='Dummy order')
    step = ihm.model.ProcessStep(description='Step 1')
    step.append(ihm.model.ProcessEdge(mg1, mg2, description='Edge 1'))
    proc.steps.append(step)
    system.ordered_processes.append(proc)

    # Geometric objects of every type
    center = ihm.geometry.Center(1., 2., 3.)
    sphere = ihm.geometry.Sphere(center=center, radius=5., name='sphere',
                                 description='Dummy sphere',
                                 transformation=transform)
    torus = ihm.geometry.Torus(center=center, major_radius=5.,
                               minor_radius=1., transformation=transform)
    half_torus = ihm.geometry.HalfTorus(center=center, major_radius=5.,
                                        minor_radius=1., thickness=0.1,
                                        inner=True, transformation=transform)
    axis = ihm.geometry.XAxis(transformation=transform)
    plane = ihm.geometry.XYPlane(transformation=transform)
    system.orphan_geometric_objects.extend((sphere, torus, half_torus, axis,
                                            plane))

    # Features of every type
    res_feature = ihm.restraint.ResidueFeature([asym_a(1, 5)],
                                               details='Residue feature',
                                               by_residue=True, rep_atom='CA')
    atom_feature = ihm.restraint.AtomFeature([asym_a.residue(1).atom('CA')],
                                             details='Atom feature')
    nonpoly_feature = ihm.restraint.NonPolyFeature([asym_h],
                                                   details='Ligand feature')
    pseudo_site = ihm.restraint.PseudoSite(x=1., y=2., z=3., radius=4.,
                                           description='Pseudo site')
    pseudo_feature = ihm.restraint.PseudoSiteFeature(pseudo_site)
    iface_feature = ihm.restraint.InterfaceResidueFeature(
        [asym_a(6, 10)], binding_partners=[asym_b], dataset=xl_ds,
        details='Interface')
    system.orphan_features.extend((res_feature, atom_feature,
                                   nonpoly_feature, pseudo_feature,
                                   iface_feature))

    # Restraints of every type
    system.restraints.append(ihm.restraint.CenterGeometricRestraint(
        dataset=em3d_ds, geometric_object=sphere, feature=res_feature,
        distance=ihm.restraint.HarmonicDistanceRestraint(5.),
        harmonic_force_constant=2., restrain_all=True))
    system.restraints.append(ihm.restraint.DerivedDistanceRestraint(
        dataset=xl_ds, feature1=res_feature, feature2=atom_feature,
        distance=ihm.restraint.LowerUpperBoundDistanceRestraint(
            1., 10., 0.1, 0.2),
        probability=0.8, restrain_all=False, mic_value=0.5,
        random_exclusion_fraction=0.1))
    system.restraints.append(ihm.restraint.PredictedContactRestraint(
        dataset=contact_ds, resatom1=asym_a.residue(1),
        resatom2=asym_b.residue(2),
        distance=ihm.restraint.UpperBoundDistanceRestraint(8.),
        by_residue=True, probability=0.9, software=software))
    system.restraints.append(ihm.restraint.HDXRestraint(
        dataset=hdx_ds, feature=res_feature, protection_factor=1.5,
        details='HDX'))
    system.restraints.append(ihm.restraint.HydroxylRadicalFPRestraint(
        dataset=fp_ds, residue=asym_a.residue(3), predicted_sasa=10.,
        rate=1., rate_error=0.1, log_pf=2., log_pf_error=0.2,
        software=software))

    em3d = ihm.restraint.EM3DRestraint(
        dataset=em3d_ds, assembly=system.complete_assembly,
        fitting_method='Gaussian mixture', fitting_method_citation=citation,
        number_of_gaussians=40, details='EM3D')
    em3d.fits[model] = ihm.restraint.EM3DRestraintFit(0.9)
    em2d = ihm.restraint.EM2DRestraint(
        dataset=em2d_ds, assembly=system.complete_assembly,
        number_raw_micrographs=100, pixel_size_width=1.,
        pixel_size_height=1., image_resolution=10.,
        number_of_projections=1000, details='EM2D')
    em2d.fits[model] = ihm.restraint.EM2DRestraintFit(
        cross_correlation_coefficient=0.8,
        rot_matrix=[[1., 0., 0.], [0., 1., 0.], [0., 0., 1.]],
        tr_vector=[1., 2., 3.])
    sas = ihm.restraint.SASRestraint(
        dataset=sas_ds, assembly=system.complete_assembly,
        fitting_method='FoXS', fitting_atom_type='Heavy atoms',
        multi_state=False, radius_of_gyration=20., details='SAS')
    sas.fits[model] = ihm.restraint.SASRestraintFit(chi_value=1.5)
    epr = ihm.restraint.EPRRestraint(
        dataset=epr_ds, fitting_particle_type='Spin label',
        fitting_method='Dummy', fitting_method_citation=citation,
        multi_state=False, software=software, details='EPR')
    epr.fits[model] = ihm.restraint.EPRRestraintFit(chi_value=1.2)
    system.restraints.extend((em3d, em2d, sas, epr))

    # Cross-links of every type, with pseudo sites and fits
    xlr = ihm.restraint.CrossLinkRestraint(dataset=xl_ds,
                                           linker=ihm.cross_linkers.dss)
    ex1 = ihm.restraint.ExperimentalCrossLink(
        protein.residue(1), protein.residue(10), details='XL 1')
    ex2 = ihm.restraint.ExperimentalCrossLink(
        protein.residue(2), protein.residue(11), details='XL 2')
    ex3 = ihm.restraint.ExperimentalCrossLink(
        protein.residue(3), protein.residue(12), details='XL 3')
    xlr.experimental_cross_links.append([ex1, ex2, ex3])
    dist = ihm.restraint.UpperBoundDistanceRestraint(25.)
    xl1 = ihm.restraint.ResidueCrossLink(
        ex1, asym_a, asym_b, dist, psi=0.1, sigma1=1., sigma2=2.,
        restrain_all=True,
        pseudo1=[ihm.restraint.CrossLinkPseudoSite(pseudo_site, model=model)])
    xl1.fits[model] = ihm.restraint.CrossLinkFit(psi=0.1, sigma1=1.,
                                                 sigma2=2.)
    xl2 = ihm.restraint.FeatureCrossLink(ex2, asym_a, asym_b, dist)
    # Fit to a group of models, rather than a single model
    xl2.fits[ensemble] = ihm.restraint.CrossLinkGroupFit(
        median_distance=20., num_models=10, details='Group fit')
    xl3 = ihm.restraint.AtomCrossLink(ex3, asym_a, asym_b, 'CA', 'CA', dist)
    xlr.cross_links.extend((xl1, xl2, xl3))
    system.restraints.append(xlr)

    # Probes (conjugate and ligand)
    probe_type = ihm.restraint.ProbeType(
        name='Dummy probe', intrinsic=False, covalent=True, reactive=True,
        reactive_name='MTSL', reactive_descriptor=ihm.cross_linkers.dss,
        descriptor=ihm.cross_linkers.dss)
    position = ihm.restraint.ProbePosition(
        asym_a.residue(4), mutated=True, modified=True,
        mutated_chem_comp=ihm.LPeptideAlphabet()['C'],
        modified_descriptor=ihm.cross_linkers.dss, description='Position')
    system.probes.append(ihm.restraint.ConjugateProbe(
        probe_type, position, dataset=epr_ds,
        descriptor=ihm.cross_linkers.dss, ambiguous_stoichiometry=False,
        probe_stoichiometry=1., details='Conjugate'))
    system.probes.append(ihm.restraint.LigandProbe(
        probe_type, heme, dataset=epr_ds, details='Ligand probe'))

    # Multi-state scheme with kinetic rates and relaxation times
    rate = ihm.multi_state_scheme.KineticRate(
        transition_rate_constant=1.0,
        equilibrium_constant=(
            ihm.multi_state_scheme.KineticRateEquilibriumConstant(
                2.0, unit='unitless')),
        details='Rate', dataset_group=dsg,
        file=ihm.location.InputFileLocation(repo=repo, path='rate.dat'))
    relax = ihm.multi_state_scheme.RelaxationTime(
        value=3.0, unit='seconds', amplitude=0.5, details='Relaxation',
        dataset_group=dsg,
        file=ihm.location.InputFileLocation(repo=repo, path='relax.dat'))
    conn = ihm.multi_state_scheme.Connectivity(
        begin_state=state1, end_state=state2, details='Transition',
        dataset_group=dsg, kinetic_rate=rate, relaxation_time=relax)
    scheme = ihm.multi_state_scheme.MultiStateScheme(
        name='Scheme', details='Dummy scheme', connectivities=[conn],
        relaxation_times=[ihm.multi_state_scheme.RelaxationTime(
            value=4.0, unit='milliseconds', details='Scheme relaxation')])
    system.multi_state_schemes.append(scheme)

    add_flr(system, protocol, protein, asym_a, fret_ds)
    return system


def add_flr(system, protocol, entity, asym, dataset):
    """Add minimal FLR data, so that FPS modeling (which references the
       modeling protocol) is output"""
    flr = ihm.flr.FLRData()
    entity_assembly = ihm.flr.EntityAssembly()
    entity_assembly.add_entity(entity=entity, num_copies=1)
    sample = ihm.flr.Sample(entity_assembly=entity_assembly, num_of_probes=2,
                            condition=ihm.flr.SampleCondition('Dummy'),
                            description='Sample')
    experiment = ihm.flr.Experiment()
    experiment.add_entry(instrument=ihm.flr.Instrument('Instrument'),
                         inst_setting=ihm.flr.InstSetting('Setting'),
                         exp_condition=ihm.flr.ExpCondition('Condition'),
                         sample=sample)

    def make_sample_probe(name, fluorophore_type, seq_id):
        probe = ihm.flr.Probe(
            probe_list_entry=ihm.flr.ProbeList(name),
            probe_descriptor=ihm.flr.ProbeDescriptor(
                reactive_probe_chem_descriptor=ihm.ChemDescriptor(
                    name + ' reactive', smiles='CC'),
                chromophore_chem_descriptor=ihm.ChemDescriptor(
                    name + ' chromophore', smiles='C')))
        return ihm.flr.SampleProbeDetails(
            sample=sample, probe=probe, fluorophore_type=fluorophore_type,
            poly_probe_position=ihm.flr.PolyProbePosition(
                asym.residue(seq_id)))
    donor = make_sample_probe('Donor', 'donor', 1)
    acceptor = make_sample_probe('Acceptor', 'acceptor', 10)
    analysis = ihm.flr.FRETAnalysis(
        experiment=experiment, sample_probe_1=donor, sample_probe_2=acceptor,
        forster_radius=ihm.flr.FRETForsterRadius(
            donor_probe=donor.probe, acceptor_probe=acceptor.probe,
            forster_radius=52.),
        type='intensity-based',
        calibration_parameters=ihm.flr.FRETCalibrationParameters(
            phi_acceptor=0.35),
        method_name='PDA', dataset=dataset)
    group = ihm.flr.FRETDistanceRestraintGroup()
    group.add_distance_restraint(ihm.flr.FRETDistanceRestraint(
        sample_probe_1=donor, sample_probe_2=acceptor, analysis=analysis,
        distance=50., peak_assignment=ihm.flr.PeakAssignment('Population')))
    flr.distance_restraint_groups.append(group)
    params = ihm.flr.FPSGlobalParameters(
        forster_radius=52., conversion_function_polynom_order=3,
        repetition=1000, av_grid_rel=0.2, av_min_grid_a=0.4,
        av_allowed_sphere=0.5, av_search_nodes=3, av_e_samples_k=200,
        sim_viscosity_adjustment=1, sim_dt_adjustment=1, sim_max_iter_k=200,
        sim_max_force=400, sim_clash_tolerance_a=1,
        sim_reciprocal_kt=10, sim_clash_potential='^2',
        convergence_e=100, convergence_k=0.001, convergence_f=0.001,
        convergence_t=0.002)
    fps_modeling = ihm.flr.FPSModeling(
        protocol=protocol, restraint_group=group, global_parameter=params,
        probe_modeling_method='AV3')
    flr.fps_modeling.append(ihm.flr.FPSAVModeling(
        fps_modeling=fps_modeling, sample_probe=donor,
        parameter=ihm.flr.FPSAVParameter(num_linker_atoms=15,
                                         linker_length=20., linker_width=3.5,
                                         probe_radius_1=4.)))
    system.flr_data.append(flr)


class RecordingCategoryWriter:
    """Pass through to a real loop or category writer, recording what
       was written"""
    def __init__(self, base, category, recorder):
        self._base = base
        self._category = category
        self._recorder = recorder

    def write(self, **kwargs):
        self._recorder.record(self._category, kwargs)
        self._base.write(**kwargs)

    def __enter__(self):
        self._base.__enter__()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        return self._base.__exit__(exc_type, exc_value, traceback)


class RecordingWriter:
    """Pass through to a real writer, recording which categories and
       keywords were written, and by which dumper"""
    def __init__(self, base):
        self._base = base
        self.dumper = None
        # Map from dumper class name to set of categories written
        self.categories_by_dumper = {}
        # Map from category name to set of keywords given a value
        self.keywords = {}

    def record(self, category, kwargs):
        category = category.lower()
        self.categories_by_dumper.setdefault(self.dumper, set()).add(category)
        self.keywords.setdefault(category, set()).update(
            k.lower() for k, v in kwargs.items() if v is not None)

    def category(self, category):
        return RecordingCategoryWriter(self._base.category(category),
                                       category, self)

    def loop(self, category, keys):
        return RecordingCategoryWriter(self._base.loop(category, keys),
                                       category, self)

    def __getattr__(self, name):
        return getattr(self._base, name)


class RecordingVariant(ihm.dumper.IHMVariant):
    """Write standard IHM output, recording what each dumper writes"""
    writer = None

    def get_dumpers(self):
        dumpers = super().get_dumpers()
        for d in dumpers:
            self._wrap_dump(d)
        return dumpers

    def _wrap_dump(self, dumper):
        orig_dump = dumper.dump

        def dump(system, writer):
            writer.dumper = type(dumper).__name__
            orig_dump(system, writer)
        dumper.dump = dump

    def get_system_writer(self, system, writer_class, writer):
        self.writer = RecordingWriter(writer)
        return self.writer


def get_ihm_literals(cls):
    """Get all IHM categories and keywords named in the source code of
       the given dumper class (and its base classes)"""
    src = ''.join(inspect.getsource(c) for c in cls.__mro__
                  if c not in (ihm.dumper.Dumper, object))
    return set(IHM_LITERAL.findall(src))


def check_coverage(variant, recorder):
    """Check that every dumper that outputs IHM categories or keywords
       was called and generated that output. Return a list of problems."""
    problems = []
    written_keys = set()
    for cat, keys in recorder.keywords.items():
        written_keys.update(keys)
    for dumper in variant.get_dumpers():
        cls = type(dumper)
        literals = get_ihm_literals(cls)
        if not literals:
            continue
        name = cls.__name__
        if not recorder.categories_by_dumper.get(name):
            problems.append("%s generated no output" % name)
        for lit in sorted(literals):
            if lit.startswith('_'):
                if lit.lower() not in recorder.keywords:
                    problems.append("%s did not write category %s"
                                    % (name, lit))
            elif lit not in written_keys:
                problems.append("%s did not write a value for keyword %s"
                                % (name, lit))
    return problems


def read_dictionaries():
    d = ihm.dictionary.Dictionary()
    for url in DICTIONARIES:
        with urllib.request.urlopen(url) as fh:
            d += ihm.dictionary.read(fh)
    return d


def main():
    system = make_system()
    variant = RecordingVariant()
    fh = io.StringIO()
    ihm.dumper.write(fh, [system], variant=variant)

    problems = check_coverage(variant, variant.writer)
    if problems:
        print("The dummy data does not exercise every IHM dumper, so the "
              "results below may be incomplete:\n  "
              + "\n  ".join(problems) + "\n")

    d = read_dictionaries()
    fh.seek(0)
    valid = True
    with warnings.catch_warnings(record=True) as w:
        warnings.simplefilter("always", ihm.dictionary.ValidatorWarning)
        try:
            d.validate(fh, warn_missing_category=is_ihm_category,
                       warn_missing_keyword=is_ihm_keyword)
        except ihm.dictionary.ValidatorError as exc:
            valid = False
            print("The dummy output failed to validate:\n%s\n" % exc)
    missing = []
    for x in w:
        if issubclass(x.category, ihm.dictionary.ValidatorWarning):
            missing.append(str(x.message))
        else:
            # Pass through any other warnings
            warnings.showwarning(x.message, x.category, x.filename, x.lineno)
    if missing:
        print("The following IHM data items are not supported by "
              "python-ihm:\n  " + "\n  ".join(missing))
    else:
        print("All IHM data items are supported by python-ihm.")
    return 0 if valid and not problems and not missing else 1


if __name__ == '__main__':
    sys.exit(main())
