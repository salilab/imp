# coding=utf-8

"""Classes to handle fluorescence data.
   The classes roughly correspond to categories in the
   `FLR dictionary <https://github.com/ihmwg/FLR-dictionary/>`_.
   See the top level :class:`FLRData` class for more information.
"""


class Probe(object):
    """Defines a fluorescent probe.

       This class is not in the FLR dictionary, but it collects all the
       information connected by the probe_ids.

       :param probe_list_entry: A probe list object.
       :type probe_list_entry: :class:`ProbeList`
       :param probe_descriptor: A probe descriptor.
       :type probe_descriptor: :class:`ProbeDescriptor`
    """

    def __init__(self, probe_list_entry=None, probe_descriptor=None):
        self.probe_list_entry = probe_list_entry
        self.probe_descriptor = probe_descriptor

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class ProbeDescriptor(object):
    """Collects the chemical descriptors for a fluorescent probe.

       This includes the chemical descriptor of the reactive probe and
       the chromophore.

       :param reactive_probe_chem_descriptor: The chemical descriptor for
              the reactive probe.
       :type reactive_probe_chem_descriptor: :class:`ihm.ChemDescriptor`
       :param chromophore_chem_descriptor: The chemical descriptor of the
              chromophore.
       :type chromophore_chem_descriptor: :class:`ihm.ChemDescriptor`
       :param chromophore_center_atom: The atom describing the center
              of the chromophore.
    """

    def __init__(self, reactive_probe_chem_descriptor,
                 chromophore_chem_descriptor, chromophore_center_atom=None):
        self.reactive_probe_chem_descriptor = reactive_probe_chem_descriptor
        self.chromophore_chem_descriptor = chromophore_chem_descriptor
        self.chromophore_center_atom = chromophore_center_atom

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class ProbeList(object):
    """Store the chromophore name, whether there is a reactive probe
       available, the origin of the probe and the type of linkage of the probe.

       :param str chromophore_name: The name of the chromophore.
       :param bool reactive_probe_flag: Flag to indicate whether a reactive
              probe is given.
       :param str reactive_probe_name: The name of the reactive probe.
       :param str probe_origin: The origin of the probe (intrinsic
              or extrinsic).
       :param str probe_link_type: The type of linkage for the probe (covalent
              or ligand).
    """

    def __init__(self, chromophore_name, reactive_probe_flag=False,
                 reactive_probe_name=None, probe_origin=None,
                 probe_link_type=None):
        self.chromophore_name = chromophore_name
        self.reactive_probe_flag = reactive_probe_flag
        self.reactive_probe_name = reactive_probe_name
        self.probe_origin = probe_origin
        self.probe_link_type = probe_link_type

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class SampleProbeDetails(object):
    """Connects a probe to a sample.

       :param sample: The sample.
       :type sample: :class:`Sample`
       :param probe: A probe that is attached to the sample.
       :type probe: :class:`Probe`
       :param str fluorophore_type: The type of the fluorophore (donor,
              acceptor, or unspecified).
       :param poly_probe_position: The position on the polymer where
              the dye is attached to.
       :type poly_probe_position: :class:`PolyProbePosition`
       :param str description: A description of the sample-probe-connection.
    """

    def __init__(self, sample, probe, fluorophore_type='unspecified',
                 poly_probe_position=None, description=None):
        self.sample = sample
        self.probe = probe
        self.fluorophore_type = fluorophore_type
        self.description = description
        self.poly_probe_position = poly_probe_position

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class PolyProbeConjugate(object):
    """Describes the conjugate of polymer residue and probe (including
       possible linker)

       :param sample_probe: The :class:`SampleProbeDetails` object to
              which the conjugate is related.
       :type sample_probe: :class:`SampleProbeDetails`
       :param chem_descriptor: The chemical descriptor of the conjugate
              of polymer residue and probe.
       :type chem_descriptor: :class:`ihm.ChemDescriptor`
       :param bool ambiguous_stoichiometry: Flag whether the labeling
              is ambiguous.
       :param float probe_stoichiometry: The stoichiometry of the
              ambiguous labeling.
    """

    def __init__(self, sample_probe, chem_descriptor,
                 ambiguous_stoichiometry=False, probe_stoichiometry=None):
        self.sample_probe = sample_probe
        self.chem_descriptor = chem_descriptor
        self.ambiguous_stoichiometry = ambiguous_stoichiometry
        self.probe_stoichiometry = probe_stoichiometry

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class PolyProbePosition(object):
    """Describes a position on the polymer used for attaching the probe.

       This class combines Poly_probe_position, Poly_probe_position_modified,
       and Poly_probe_position_mutated from the FLR dictionary.

       :param resatom: The residue or atom that the probe is attached to.
       :type resatom: :class:`ihm.Residue` or :class:`ihm.Atom`
       :param bool mutation_flag: Flag whether the residue was mutated
              (e.g. a Cys mutation).
       :param bool modification_flag: Flag whether the residue was modified
              (e.g. replacement of a residue with a labeled residue in
              case of nucleic acids).
       :param str auth_name: An author-given name for the position.
       :param mutated_chem_comp_id: The chemical component ID of the
              mutated residue.
       :type modified_chem_descriptor: :class:`ihm.ChemComp`
       :param modified_chem_descriptor: The chemical descriptor of the
              modified residue.
       :type modified_chem_descriptor: :class:`ihm.ChemDescriptor`
    """

    def __init__(self, resatom, mutation_flag=False, modification_flag=False,
                 auth_name=None, mutated_chem_comp_id=None,
                 modified_chem_descriptor=None):
        self.resatom = resatom
        self.mutation_flag = mutation_flag
        self.modification_flag = modification_flag
        self.auth_name = auth_name
        if self.mutation_flag:
            self.mutated_chem_comp_id = mutated_chem_comp_id
        if self.modification_flag:
            self.modified_chem_descriptor = modified_chem_descriptor

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class Sample(object):
    """Sample corresponds to a measurement.

       :param entity_assembly: The assembly of the entities that was measured.
       :type entity_assembly: :class:`EntityAssembly`
       :param int num_of_probes: The number of probes in the sample.
       :param condition: The sample conditions for the Sample.
       :type condition: :class:`SampleCondition`
       :param str description: A description of the sample.
       :param str details: Details about the sample.
       :param solvent_phase: The solvent phase of the sample (liquid,
              vitrified, or other).
    """

    def __init__(self, entity_assembly, num_of_probes, condition,
                 description=None, details=None, solvent_phase=None):
        self.entity_assembly = entity_assembly
        self.num_of_probes = num_of_probes
        self.condition = condition
        self.description = description
        self.details = details
        self.solvent_phase = solvent_phase

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class EntityAssembly(object):
    """The assembly of the entities that are in the system.

       :param entity: The entity to add.
       :type entity: :class:`Entity`
       :param num_copies: The number of copies for the entity in the assembly.
    """

    def __init__(self, entity=None, num_copies=0):
        self.entity_list = []
        self.num_copies_list = []
        if entity is not None and num_copies != 0:
            self.add_entity(entity, num_copies)

    def add_entity(self, entity, num_copies):
        if num_copies < 0:
            raise ValueError("Number of copies for Entity must be "
                             "larger than zero.")
        self.entity_list.append(entity)
        self.num_copies_list.append(num_copies)

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class SampleCondition(object):
    """Description of the sample conditions.

       *Currently this is only text, but will be extended in the future.*

       :param str details: Description of the sample conditions.
    """

    def __init__(self, details=None):
        self.details = details

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class Experiment(object):
    """The Experiment collects combinations of instrument, experimental
       settings and sample.

       :param instrument: The instrument.
       :type instrument: :class:`Instrument`
       :param inst_setting: The instrument setting.
       :type inst_setting: :class:`InstSetting`
       :param exp_condition: The experimental conditions.
       :type exp_condition: :class:`ExpCondition`
       :param sample: The sample.
       :type sample: :class:`Sample`
       :param details: Details on the experiment.
    """

    def __init__(self, instrument=None, inst_setting=None, exp_condition=None,
                 sample=None, details=None):
        """The Experiment object can either be initiated with empty lists,
           or with an entry for each of them. In this way, an experiment
           object is created and filled with one entry.
        """
        self.instrument_list = []
        self.inst_setting_list = []
        self.exp_condition_list = []
        self.sample_list = []
        self.details_list = []
        if (instrument is not None and inst_setting is not None
                and exp_condition is not None and sample is not None):
            self.add_entry(instrument=instrument, inst_setting=inst_setting,
                           exp_condition=exp_condition,
                           sample=sample, details=details)

    def add_entry(self, instrument, inst_setting, exp_condition, sample,
                  details=None):
        """Entries to the experiment object can also be added one by one.
        """
        self.instrument_list.append(instrument)
        self.inst_setting_list.append(inst_setting)
        self.exp_condition_list.append(exp_condition)
        self.sample_list.append(sample)
        self.details_list.append(details)

    def get_entry_by_index(self, index):
        """Returns the combination of :class:`Instrument`,
           :class:`InstSetting`, :class:`ExpCondition`, :class:`Sample`,
           and details for a given index.
        """
        return (self.instrument_list[index],
                self.inst_setting_list[index],
                self.exp_condition_list[index],
                self.sample_list[index],
                self.details_list[index])

    def __eq__(self, other):
        return ((self.instrument_list == other.instrument_list)
                and (self.inst_setting_list == other.inst_setting_list)
                and (self.exp_condition_list == other.exp_condition_list)
                and (self.sample_list == other.sample_list)
                and (self.details_list == other.details_list))

    def contains(self, instrument, inst_setting, exp_condition, sample):
        """Checks whether a combination of :class:`Instrument`,
           :class:`InstSetting`, :class:`ExpCondition`,
           :class:`Sample` is already included in the experiment object.
        """
        # TODO: possibly extend this by the details_list?
        for i in range(len(self.instrument_list)):
            if ((instrument == self.instrument_list[i])
                    and (inst_setting == self.inst_setting_list[i])
                    and (exp_condition == self.exp_condition_list[i])
                    and (sample == self.sample_list[i])):
                return True
        return False


class Instrument(object):
    """Description of the Instrument used for the measurements.

       *Currently this is only text, but will be extended in the future.*

       :param details: Description of the instrument used for the measurements.
    """
    def __init__(self, details=None):
        self.details = details

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class InstSetting(object):
    """Description of the instrument settings.

       *Currently this is only text, but will be extended in the future.*

       :param str details: Description of the instrument settings used for
              the measurement (e.g. laser power or size of observation
              volume in case of confocal measurements).
    """
    def __init__(self, details=None):
        self.details = details

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class ExpCondition(object):
    """Description of the experimental conditions.

    * Currently this is only text, but will be extended in the future.*

    :param str details: Description of the experimental conditions (e.g.
        the temperature at which the experiment was carried out).
    """
    def __init__(self, details=None):
        self.details = details

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FRETAnalysis(object):
    """An analysis of FRET data that was performed.

       :param experiment: The Experiment object for this FRET analysis.
       :type experiment: :class:`Experiment`
       :param sample_probe_1: The combination of sample and probe for the
              first probe.
       :type sample_probe_1: :class:`SampleProbeDetails`
       :param sample_probe_2: The combination of sample and probe for the
              second probe.
       :type sample_probe_2: :class:`SampleProbeDetails`
       :param forster_radius: The Förster radius object for this FRET analysis.
       :type forster_radius: :class:`FRETForsterRadius`.
       :param str type: The type of the FRET analysis (intensity-based
              or lifetime-based).
       :param calibration_parameters: The calibration parameters used for
              this analysis (only in case of intensity-based analyses).
       :type calibration_parameters: :class:`FRETCalibrationParameters`
       :param lifetime_fit_model: The fit model used in case of
              lifetime-based analyses.
       :type lifetime_fit_model: :class:`LifetimeFitModel`
       :param ref_measurement_group: The group of reference measurements
             in case of lifetime-based analyses.
       :type ref_measurement_group: :class:`LifetimeRefMeasurementGroup`
       :param str method_name: The method used for the analysis.
       :param float chi_square_reduced: The chi-square reduced as a quality
              measure for the fit.
       :param float donor_only_fraction: The donor-only fraction.
       :param dataset: The dataset used.
       :type dataset: :class:`ihm.dataset.Dataset`
       :param external_file: The external file that contains (results of)
              the analysis.
       :param software: The software used for the analysis.
       :type software: :class:`ihm.Software`
    """

    def __init__(self, experiment, sample_probe_1, sample_probe_2,
                 forster_radius, type, calibration_parameters=None,
                 lifetime_fit_model=None,
                 ref_measurement_group=None,
                 method_name=None, details=None,
                 chi_square_reduced=None, donor_only_fraction=None,
                 dataset=None, external_file=None, software=None):
        if type not in ['lifetime-based', 'intensity-based', None]:
            raise ValueError(
                'FRETAnalysis.type can be \'lifetime-based\' or '
                '\'intensity-based\'. The value is %s' % type)
        self.experiment = experiment
        self.sample_probe_1 = sample_probe_1
        self.sample_probe_2 = sample_probe_2
        self.forster_radius = forster_radius
        self.type = type
        self.calibration_parameters = calibration_parameters
        self.lifetime_fit_model = lifetime_fit_model
        self.ref_measurement_group = ref_measurement_group
        self.method_name = method_name
        self.details = details
        self.chi_square_reduced = chi_square_reduced
        self.donor_only_fraction = donor_only_fraction
        self.dataset = dataset
        self.external_file = external_file
        self.software = software

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class LifetimeFitModel(object):
    """A lifetime-fit model used for lifetime-based analysis.

        :param str name: The name of the fit model.
        :param str description: A description of the fit model.
        :param external_file: An external file that contains additional
               information on the fit model.
        :param citation: A citation for the fit model.
        :type citation: :class:`ihm.Citation`
    """
    def __init__(self, name, description, external_file=None, citation=None):
        self.name = name
        self.description = description
        self.external_file = external_file
        self.citation = citation

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class RefMeasurementGroup(object):
    """A Group containing reference measurements for lifetime-based analysis.

        :param str details: Details on the Group of reference measurements.
    """
    def __init__(self, details=None):
        self.details = details
        self.ref_measurement_list = []
        self.num_measurements = len(self.ref_measurement_list)

    def add_ref_measurement(self, ref_measurement):
        """Add a lifetime reference measurement to a ref_measurement_group."""
        self.ref_measurement_list.append(ref_measurement)
        self.num_measurements = len(self.ref_measurement_list)

    def get_info(self):
        return self.ref_measurement_list

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class RefMeasurement(object):
    """A reference measurement for lifetime-based analysis.

        :param ref_sample_probe: The combination of sample and probe used
               for the reference measurement.
        :type ref_sample_probe: :class:`SampleProbeDetails`
        :param str details: Details on the measurement.
        :param list_of_lifetimes: A list of the results from the reference
               measurement.
        :type list_of_lifetimes: List of :class:`RefMeasurementLifetime`
    """
    def __init__(self, ref_sample_probe, details=None, list_of_lifetimes=None):
        self.ref_sample_probe = ref_sample_probe
        self.details = details
        self.list_of_lifetimes = \
            list_of_lifetimes if list_of_lifetimes is not None else []
        self.num_species = len(self.list_of_lifetimes)

    def add_lifetime(self, lifetime):
        """Add a lifetime to the list_of_lifetimes."""
        self.list_of_lifetimes.append(lifetime)
        self.num_species = len(self.list_of_lifetimes)

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class RefMeasurementLifetime(object):
    """Lifetime for a species in a reference measurement.
        :param float species_fraction: The species-fraction for the
               respective lifetime.
        :param float lifetime: The lifetime (in ns).
        :param str species_name: A name for the species.
    """
    def __init__(self, species_fraction, lifetime, species_name=None):
        self.species_fraction = species_fraction
        self.lifetime = lifetime
        self.species_name = species_name

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FRETDistanceRestraintGroup(object):
    """A collection of FRET distance restraints that are used together.
    """
    def __init__(self):
        self.distance_restraint_list = []

    def add_distance_restraint(self, distance_restraint):
        """Add a distance restraint to a distance_restraint_group"""
        self.distance_restraint_list.append(distance_restraint)

    def get_info(self):
        return self.distance_restraint_list

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FRETDistanceRestraint(object):
    """A distance restraint from FRET.

       :param sample_probe_1: The combination of sample and probe for
              the first probe.
       :type sample_probe_1: :class:`SampleProbeDetails`
       :param sample_probe_2: The combination of sample and probe for
              the second probe.
       :type sample_probe_2: :class:`SampleProbeDetails`
       :param analysis: The FRET analysis from which the distance
              restraint originated.
       :type analysis: :class:`FRETAnalysis`
       :param float distance: The distance of the restraint.
       :param float distance_error_plus: The (absolute, e.g. in Angstrom) error
              in the upper direction, such that
              ``upper boundary = distance + distance_error_plus``.
       :param float distance_error_minus: The (absolute, e.g. in Angstrom)
              error in the lower direction, such that
              ``lower boundary = distance + distance_error_minus``.
       :param str distance_type: The type of distance (<R_DA>, <R_DA>_E,
              or R_mp).
       :param state: The state the distance restraints is connected to.
              Important for multi-state models.
       :type state: :class:`ihm.model.State`
       :param float population_fraction: The population fraction of the state
              in case of multi-state models.
       :param peak_assignment: The method how a peak was assigned.
       :type peak_assignment: :class:`PeakAssignment`
    """

    def __init__(self, sample_probe_1, sample_probe_2, analysis, distance,
                 distance_error_plus=0., distance_error_minus=0.,
                 distance_type=None, state=None, population_fraction=0.,
                 peak_assignment=None):
        self.sample_probe_1 = sample_probe_1
        self.sample_probe_2 = sample_probe_2
        self.state = state
        self.analysis = analysis
        self.distance = distance
        self.distance_error_plus = distance_error_plus
        self.distance_error_minus = distance_error_minus
        self.distance_type = distance_type
        self.population_fraction = population_fraction
        self.peak_assignment = peak_assignment

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FRETForsterRadius(object):
    """The FRET Förster radius between two probes.

       :param donor_probe: The donor probe.
       :type donor_probe: :class:`Probe`
       :param acceptor_probe: The acceptor probe.
       :type acceptor_probe: :class:`Probe`
       :param float forster_radius: The Förster radius between the two probes.
       :param float reduced_forster_radius: The reduced Förster radius between
              the two probes.
    """

    def __init__(self, donor_probe, acceptor_probe, forster_radius,
                 reduced_forster_radius=None):
        self.donor_probe = donor_probe
        self.acceptor_probe = acceptor_probe
        self.forster_radius = forster_radius
        self.reduced_forster_radius = reduced_forster_radius

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FRETCalibrationParameters(object):
    """The calibration parameter from the FRET measurements.
        For the definitions of the parameters see
        Hellenkamp et al. Nat. Methods 2018.

        :param float phi_acceptor: The quantum yield of the acceptor.
        :param float alpha: The alpha parameter.
        :param float alpha_sd: The standard deviation of the alpha parameter.
        :param float gg_gr_ratio: The ratio of the green and red detection
               efficiencies.
        :param float beta: The beta parameter.
        :param float gamma: The gamma parameter.
        :param float delta: The delta parameter.
        :param float a_b: The fraction of bright molecules.
    """

    def __init__(self, phi_acceptor=None, alpha=None, alpha_sd=None,
                 gg_gr_ratio=None, beta=None, gamma=None, delta=None,
                 a_b=None):
        self.phi_acceptor = phi_acceptor
        self.alpha = alpha
        self.alpha_sd = alpha_sd
        self.gg_gr_ratio = gg_gr_ratio
        self.beta = beta
        self.gamma = gamma
        self.delta = delta
        self.a_b = a_b

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class PeakAssignment(object):
    """The method of peak assignment in case of multiple peaks,
        e.g. by population.

        :param str method_name: The method used for peak assignment.
        :param str details: The details of the peak assignment procedure.
    """
    def __init__(self, method_name, details=None):
        self.method_name = method_name
        self.details = details

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FRETModelQuality(object):
    """The quality measure for a Model based on FRET data.

       :param model: The model being described.
       :type model: :class:`ihm.model.Model`
       :param chi_square_reduced: The quality of the model in terms of
              chi_square_reduced based on the Distance restraints used
              for the modeling.
       :param dataset_group: The group of datasets that was used for the
              quality estimation.
       :type dataset_group: :class:`ihm.dataset.DatasetGroup`
       :param method: The method used for judging the model quality.
       :param str details: Details on the model quality.
    """

    def __init__(self, model, chi_square_reduced, dataset_group,
                 method, details=None):
        self.model = model
        self.chi_square_reduced = chi_square_reduced
        self.dataset_group = dataset_group
        self.method = method
        self.details = details

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FRETModelDistance(object):
    """The distance in a model for a certain distance restraint.

       :param restraint: The Distance restraint.
       :type restraint: :class:`FRETDistanceRestraint`
       :param model: The model the distance applies to.
       :type model: :class:`ihm.model.Model`
       :param distance: The distance obtained for the distance restraint
              in the current model.
       :param distance_deviation: The deviation of the distance in the
              model compared to the value of the distance restraint.
    """

    def __init__(self, restraint, model, distance,
                 distance_deviation=None):
        self.restraint = restraint
        self.model = model
        self.distance = distance
        self.distance_deviation = distance_deviation
        if self.distance_deviation is None and self.restraint is not None:
            self.calculate_deviation()

    def calculate_deviation(self):
        if self.distance_deviation is None and self.restraint is not None:
            self.distance_deviation = \
                float(self.restraint.distance) - float(self.distance)

    def update_deviation(self):
        if self.restraint is not None:
            self.distance_deviation = \
                float(self.restraint.distance) - float(self.distance)

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FPSModeling(object):
    """Collect the modeling parameters for different steps of FPS,
       e.g. Docking, Refinement, or Error estimation.

       :param protocol: The modeling protocol to which the FPS modeling
              step belongs.
       :type protocol: :class:`ihm.protocol.Protocol`
       :param restraint_group: The restraint group used for the modeling.
       :type restraint_group: :class:`FRETDistanceRestraintGroup`
       :param global_parameter: The global FPS parameters used.
       :type global_parameter: :class:`FPSGlobalParameters`
       :param str probe_modeling_method: either "AV" or "MPP".
       :param str details: Details on the FPS modeling.
    """

    def __init__(self, protocol, restraint_group,
                 global_parameter, probe_modeling_method, details=None):
        self.protocol = protocol
        self.restraint_group = restraint_group
        self.global_parameter = global_parameter
        self.probe_modeling_method = probe_modeling_method
        self.details = details

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FPSGlobalParameters(object):
    """The global parameters in the FPS program.

       *For a description of the parameters, see also the FPS manual.*

       :param float forster_radius: The Förster radius used in the FPS program.
       :param int conversion_function_polynom_order: Order of the polynom for
              the conversion function between Rmp and <RDA>E.
       :param int repetition: The number of repetitions.
       :param float av_grid_rel: The AV grid spacing relative to the smallest
              dye or linker dimension.
       :param float av_min_grid_a: The minimal AV grid spacing in Angstrom.
       :param float av_allowed_sphere: The allowed sphere radius.
       :param int av_search_nodes: Number of neighboring positions to be
              scanned for clashes.
       :param float av_e_samples_k: The number of samples for calculation
              of E (in thousand).
       :param float sim_viscosity_adjustment: Daming rate during docking
              and refinement.
       :param float sim_dt_adjustment: Time step during simulation.
       :param float sim_max_iter_k: Maximal number of iterations (in thousand).
       :param float sim_max_force: Maximal force.
       :param float sim_clash_tolerance_a: Clash tolerance in Angstrom.
       :param float sim_reciprocal_kt: reciprocal kT.
       :param str sim_clash_potential: The clash potential.
       :param float convergence_e: Convergence criterion E.
       :param float convergence_k: Convergence criterion K.
       :param float convergence_f: Convergence criterion F.
       :param float convergence_t: Convergence criterion T.
       :param str optimized_distances: Which distances are optimized?

    """
    def __init__(self, forster_radius, conversion_function_polynom_order,
                 repetition, av_grid_rel, av_min_grid_a, av_allowed_sphere,
                 av_search_nodes, av_e_samples_k, sim_viscosity_adjustment,
                 sim_dt_adjustment, sim_max_iter_k, sim_max_force,
                 sim_clash_tolerance_a, sim_reciprocal_kt, sim_clash_potential,
                 convergence_e, convergence_k, convergence_f, convergence_t,
                 optimized_distances='All'):
        self.forster_radius = forster_radius
        self.conversion_function_polynom_order \
            = conversion_function_polynom_order
        self.repetition = repetition
        self.av_grid_rel = av_grid_rel
        self.av_min_grid_a = av_min_grid_a
        self.av_allowed_sphere = av_allowed_sphere
        self.av_search_nodes = av_search_nodes
        self.av_e_samples_k = av_e_samples_k
        self.sim_viscosity_adjustment = sim_viscosity_adjustment
        self.sim_dt_adjustment = sim_dt_adjustment
        self.sim_max_iter_k = sim_max_iter_k
        self.sim_max_force = sim_max_force
        self.sim_clash_tolerance_a = sim_clash_tolerance_a
        self.sim_reciprocal_kt = sim_reciprocal_kt
        self.sim_clash_potential = sim_clash_potential
        self.convergence_e = convergence_e
        self.convergence_k = convergence_k
        self.convergence_f = convergence_f
        self.convergence_t = convergence_t
        self.optimized_distances = optimized_distances

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FPSAVModeling(object):
    """FPS modeling using AV.
       This object connects the FPS_modeling step, the sample_probe and
       the respective AV parameters.

       :param fps_modeling: The FPS modeling ID.
       :type fps_modeling: :class:`FPSModeling`
       :param sample_probe: The Sample probe ID.
       :type sample_probe: :class:`SampleProbeDetails`
       :param parameter: The FPS AV parameters used.
       :type parameter: :class:`FPSAVParameter`
    """

    def __init__(self, fps_modeling, sample_probe, parameter):
        # fps_modeling is the object containing information on the
        # ihm modeling protocol, the restraint group and the global
        # FPS parameters
        self.fps_modeling = fps_modeling
        self.sample_probe = sample_probe
        self.parameter = parameter

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FPSAVParameter(object):
    """The AV parameters used for the modeling using FPS.

       :param int num_linker_atoms: The number of atoms in the linker.
       :param float linker_length: The length of the linker in Angstrom.
       :param float linker_width: The width of the linker in Angstrom.
       :param float probe_radius_1: The first radius of the probe.
       :param float probe_radius_2: If AV3 is used, the second radius
              of the probe.
       :param float probe_radius_3: If AV3 is used, the third radius
              of the probe.
    """

    def __init__(self, num_linker_atoms, linker_length, linker_width,
                 probe_radius_1, probe_radius_2=None, probe_radius_3=None):
        self.num_linker_atoms = num_linker_atoms
        self.linker_length = linker_length
        self.linker_width = linker_width
        self.probe_radius_1 = probe_radius_1
        self.probe_radius_2 = probe_radius_2
        self.probe_radius_3 = probe_radius_3

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FPSMPPModeling(object):
    """Maps the FPSModeling object to a mean probe position and connects it
       to the reference coordinate system.

       :param fps_modeling: The FPS modeling object.
       :type fps_modeling: :class:`FPSModeling`
       :param mpp: The ID of the mean probe position.
       :type mpp: :class:`FPSMeanProbePosition`
       :param mpp_atom_position_group:
       :type mpp_atom_position_group: :class:`FPSMPPAtomPositionGroup`
    """

    def __init__(self, fps_modeling, mpp, mpp_atom_position_group):
        # fps_modeling is the object containing information on the
        # ihm modeling protocol, the restraint group and the global
        # FPS parameters
        self.fps_modeling = fps_modeling
        self.mpp = mpp
        self.mpp_atom_position_group = mpp_atom_position_group

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FPSMeanProbePosition(object):
    """The mean probe position of an AV, which can be used instead of an AV.

       *It is usually not recommended to use this. Use AVs instead.*
       The coordinates are with respect to a reference coordinate system
       defined by :class:`FPSMPPAtomPositionGroup`.

       :param sample_probe: The Sample probe.
       :type sample_probe: :class:`SampleProbeDetails`
       :param float x: The x-coordinate of the mean probe position.
       :param float y: The y-coordinate of the mean probe position.
       :param float z: The z-coordinate of the mean probe position.
    """

    def __init__(self, sample_probe, x, y, z):
        self.sample_probe = sample_probe
        self.x, self.y, self.z = x, y, z

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FPSMPPAtomPositionGroup(object):
    """A group of atom positions used to define the coordinate system
       of a mean probe position.
       *Not part of the FLR dictionary.*
    """
    def __init__(self):
        self.mpp_atom_position_list = []

    def add_atom_position(self, atom_position):
        self.mpp_atom_position_list.append(atom_position)

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FPSMPPAtomPosition(object):
    """An atom used to describe the coordinate system for a mean probe position

       :param atom: The atom being described.
       :type atom: :class:`ihm.Atom`
       :param float x: The x-coordinate of the atom.
       :param float y: The y-coordinate of the atom.
       :param float z: The z-coordinate of the atom.
    """

    # atoms describing the coordinate system for a mean probe position
    def __init__(self, atom, x, y, z):
        self.atom, self.x, self.y, self.z = atom, x, y, z

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class FLRData(object):
    """A collection of the fluorescence data to be added to the system.

       Instances of this class are generally added to
       :attr:`~ihm.System.flr_data`.
    """
    def __init__(self):
        #: All groups of FRET distance restraints.
        #: See :class:`FRETDistanceRestraintGroup`.
        self.distance_restraint_groups = []

        #: All conjugates of polymer residue and probe.
        #: See :class:`PolyProbeConjugate`.
        self.poly_probe_conjugates = []

        #: All quality measures for models based on FRET data.
        #: See :class:`FRETModelQuality`.
        self.fret_model_qualities = []

        #: All distances in models for distance restraints.
        #: See :class:`FRETModelDistance`.
        self.fret_model_distances = []

        #: All modeling objects.
        #: See :class:`FPSAVModeling` and :class:`FPSMPPModeling`.
        self.fps_modeling = []

        # The following dictionaries are so far only used when reading data
        self._collection_flr_experiment = {}
        self._collection_flr_inst_setting = {}
        self._collection_flr_exp_condition = {}
        self._collection_flr_instrument = {}
        self._collection_flr_entity_assembly = {}
        self._collection_flr_sample_condition = {}
        self._collection_flr_sample = {}
        self._collection_flr_sample_probe_details = {}
        self._collection_flr_probe = {}
        self._collection_flr_poly_probe_position = {}
        self._collection_flr_poly_probe_position_modified = {}
        self._collection_flr_poly_probe_position_mutated = {}
        self._collection_flr_poly_probe_conjugate = {}
        self._collection_flr_fret_forster_radius = {}
        self._collection_flr_fret_calibration_parameters = {}
        self._collection_flr_fret_analysis = {}
        self._collection_flr_lifetime_fit_model = {}
        self._collection_flr_ref_measurement_group = {}
        self._collection_flr_ref_measurement = {}
        self._collection_flr_ref_measurement_lifetime = {}
        self._collection_flr_peak_assignment = {}
        self._collection_flr_fret_distance_restraint = {}
        self._collection_flr_fret_distance_restraint_group = {}
        self._collection_flr_fret_model_quality = {}
        self._collection_flr_fret_model_distance = {}
        self._collection_flr_fps_global_parameters = {}
        self._collection_flr_fps_modeling = {}
        self._collection_flr_fps_av_parameter = {}
        self._collection_flr_fps_av_modeling = {}
        self._collection_flr_fps_mean_probe_position = {}
        self._collection_flr_fps_mpp_atom_position = {}
        self._collection_flr_fps_mpp_modeling = {}

    def _all_distance_restraints(self):
        """Yield all FRETDistanceRestraint objects"""
        for rg in self.distance_restraint_groups:
            for r in rg.distance_restraint_list:
                yield r

    def _all_analyses(self):
        """Yield all FRETAnalysis objects"""
        for r in self._all_distance_restraints():
            yield r.analysis

    def _all_peak_assignments(self):
        """Yield all PeakAssignment objects"""
        for r in self._all_distance_restraints():
            yield r.peak_assignment

    def _all_experiments(self):
        """Yield all Experiment objects"""
        for a in self._all_analyses():
            yield a.experiment

    def _all_forster_radii(self):
        """Yield all FRETForsterRadius objects"""
        for a in self._all_analyses():
            yield a.forster_radius

    def _all_calibration_parameters(self):
        """Yield all FRETCalibrationParameters objects"""
        for a in self._all_analyses():
            if a.type == 'intensity-based':
                yield a.calibration_parameters

    def _all_lifetime_fit_models(self):
        """Yield all LifetimeFitModel objects"""
        for a in self._all_analyses():
            if a.type == 'lifetime-based':
                yield a.lifetime_fit_model

    def _all_ref_measurement_groups(self):
        """Yield all RefMeasurementGroup objects"""
        for a in self._all_analyses():
            if a.type == 'lifetime-based':
                yield a.ref_measurement_group

    def _all_ref_measurements(self):
        """Yield all RefMeasurement objects"""
        for rg in self._all_ref_measurement_groups():
            for x in rg.ref_measurement_list:
                yield x

    def _all_ref_measurement_lifetimes(self):
        """Yield all RefMeasurementLifetime objects"""
        for r in self._all_ref_measurements():
            for x in r.list_of_lifetimes:
                yield x

    def _all_sample_probe_details(self):
        """Yield all SampleProbeDetails objects"""
        for r in self._all_distance_restraints():
            yield r.sample_probe_1
            yield r.sample_probe_2
        for r in self._all_ref_measurements():
            yield r.ref_sample_probe

    def _all_samples(self):
        """Yield all Sample objects"""
        for s in self._all_sample_probe_details():
            yield s.sample

    def _all_probes(self):
        """Yield all Probe objects"""
        for s in self._all_sample_probe_details():
            yield s.probe

    def _all_poly_probe_positions(self):
        """Yield all PolyProbePosition objects"""
        for s in self._all_sample_probe_details():
            yield s.poly_probe_position

    def _all_inst_settings(self):
        """Yield all InstSetting objects"""
        for e in self._all_experiments():
            for s in e.inst_setting_list:
                yield s

    def _all_exp_conditions(self):
        """Yield all ExpCondition objects"""
        for e in self._all_experiments():
            for s in e.exp_condition_list:
                yield s

    def _all_instruments(self):
        """Yield all Instrument objects"""
        for e in self._all_experiments():
            for s in e.instrument_list:
                yield s

    def _all_fps_modeling(self):
        """Yield all FPSModeling objects"""
        for m in self.fps_modeling:
            yield m.fps_modeling

    def _all_fps_global_parameters(self):
        """Yield all FPSGlobalParameters objects"""
        for m in self._all_fps_modeling():
            yield m.global_parameter

    def _all_fps_av_modeling(self):
        """Yield all FPSAVModeling objects"""
        for m in self.fps_modeling:
            if isinstance(m, FPSAVModeling):
                yield m

    def _all_fps_av_parameter(self):
        """Yield all FPSAVParameter objects"""
        for m in self._all_fps_av_modeling():
            yield m.parameter

    def _all_fps_mpp_modeling(self):
        """Yield all FPSMPPModeling objects"""
        for m in self.fps_modeling:
            if isinstance(m, FPSMPPModeling):
                yield m

    def _all_fps_mean_probe_position(self):
        """Yield all FPSMeanProbePosition objects"""
        for m in self._all_fps_mpp_modeling():
            yield m.mpp

    def _all_fps_atom_position_group(self):
        """Yield all FPSMPPAtomPositionGroup objects"""
        for m in self._all_fps_mpp_modeling():
            yield m.mpp_atom_position_group

    def _all_flr_chemical_descriptors(self):
        """Collect the chemical descriptors from the flr part.
           *This might contain duplicates.*
        """
        # collect from all distance_restraint_groups
        for drgroup in self.distance_restraint_groups:
            # collect from all distance restraints
            for dr in drgroup.distance_restraint_list:
                # collect from both sample_probe_1 and sample_probe_2
                for this_sample_probe in (dr.sample_probe_1,
                                          dr.sample_probe_2):
                    # collect from the probe
                    probe = this_sample_probe.probe
                    # reactive probe
                    yield probe.probe_descriptor.reactive_probe_chem_descriptor
                    # chromophore
                    yield probe.probe_descriptor.chromophore_chem_descriptor
                    # collect from the poly_probe_position
                    pos = this_sample_probe.poly_probe_position
                    # modified chem descriptor
                    if pos.modification_flag:
                        yield pos.modified_chem_descriptor
                # collect from all analyses if they are lifetime-based
                a = dr.analysis
                if a.type == 'lifetime-based':
                    # RefMeasurementGroup
                    rg = a.ref_measurement_group
                    # collect from all RefMeasurements
                    for rm in rg.ref_measurement_list:
                        # collect from the ref_sample_probe
                        this_ref_sample_probe = rm.ref_sample_probe
                        probe = this_ref_sample_probe.probe
                        pd = probe.probe_descriptor
                        # reactive probe
                        yield pd.reactive_probe_chem_descriptor
                        # chromophore
                        yield pd.chromophore_chem_descriptor
                        # collect from the poly_probe_position
                        pos = this_ref_sample_probe.poly_probe_position
                        # modified chem descriptor
                        if pos.modification_flag:
                            yield pos.modified_chem_descriptor
        # and collect from all poly_probe_conjugates
        for c in self.poly_probe_conjugates:
            yield c.chem_descriptor
