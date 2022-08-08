"""Classes for handling restraints on the system.
"""

import ihm


class PseudoSite(object):
    """Selection of a pseudo position in the system.
       Pseudo positions are typically used to reference a point or sphere
       that is not explcitly represented, in a :class:`PseudoSiteFeature`
       or :class:`CrossLinkPseudoSite`.

       :param float x: Cartesian X coordinate of this site.
       :param float y: Cartesian Y coordinate of this site.
       :param float z: Cartesian Z coordinate of this site.
       :param float radius: Radius of the site, if applicable.
       :param str description: Additional text describing this feature.
    """

    def __init__(self, x, y, z, radius=None, description=None):
        self.x, self.y, self.z = x, y, z
        self.radius = radius
        self.description = description


class Restraint(object):
    """Base class for all restraints.
       See :attr:`ihm.System.restraints`.
    """
    pass


class RestraintGroup(list):
    """A set of related :class:`Restraint` objects.
       This is implemented as a simple list.

       Note that due to limitations of the underlying dictionary, only
       certain combinations of restraints can be placed in groups.
       In particular, all objects in a group must be of the same type, and
       only certain types (currently only :class:`DerivedDistanceRestraint`
       and :class:`PredictedContactRestraint`) can be grouped.

       Empty groups can be created, but will be ignored on output as the
       dictionary does not support them.

       Restraint groups should be stored in the system by adding them to
       :attr:`ihm.System.restraint_groups`.
    """
    pass


class EM3DRestraint(Restraint):
    """Restrain part of the system to match an electron microscopy density map.

       :param dataset: Reference to the density map data (usually
              an :class:`~ihm.dataset.EMDensityDataset`).
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param assembly: The part of the system that is fit into the map.
       :type assembly: :class:`~ihm.Assembly`
       :param bool segment: True iff the map has been segmented.
       :param str fitting_method: The method used to fit the model
              into the map.
       :param fitting_method_citation: The publication describing the fitting
              method.
       :type fitting_method_citation: :class:`~ihm.Citation`
       :param int number_of_gaussians: Number of Gaussians used to represent
              the map as a Gaussian Mixture Model (GMM), if applicable.
       :param str details: Additional details regarding the fitting.
    """

    def __init__(self, dataset, assembly, segment=None, fitting_method=None,
                 fitting_method_citation=None, number_of_gaussians=None,
                 details=None):
        self.dataset, self.assembly = dataset, assembly
        self.segment, self.fitting_method = segment, fitting_method
        self.fitting_method_citation = fitting_method_citation
        self.number_of_gaussians = number_of_gaussians
        self.details = details

        #: Information about the fit of each model to this restraint's data.
        #: This is a Python dict where keys are :class:`~ihm.model.Model`
        #: objects and values are :class:`EM3DRestraintFit` objects.
        self.fits = {}


class EM3DRestraintFit(object):
    """Information on the fit of a model to an :class:`EM3DRestraint`.
       See :attr:`EM3DRestaint.fits`.

       :param float cross_correlation_coefficient: The fit between the model
              and the map.
    """
    __slots__ = ["cross_correlation_coefficient"]  # Reduce memory usage

    def __init__(self, cross_correlation_coefficient=None):
        self.cross_correlation_coefficient = cross_correlation_coefficient


class SASRestraint(Restraint):
    """Restrain part of the system to match small angle scattering (SAS) data.

       :param dataset: Reference to the SAS data (usually
              an :class:`~ihm.dataset.SASDataset`).
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param assembly: The part of the system that is fit against SAS data.
       :type assembly: :class:`~ihm.Assembly`
       :param bool segment: True iff the SAS profile has been segmented.
       :param str fitting_method: The method used to fit the model against the
              SAS data (e.g. FoXS, DAMMIF).
       :param str fitting_atom_type: The set of atoms fit against the data
              (e.g. "Heavy atoms", "All atoms").
       :param bool multi_state: Whether multiple state fitting was done.
       :param float radius_of_gyration: Radius of gyration obtained from the
              SAS profile, if used as part of the restraint.
       :param str details: Additional details regarding the fitting.
    """

    def __init__(self, dataset, assembly, segment=None, fitting_method=None,
                 fitting_atom_type=None, multi_state=None,
                 radius_of_gyration=None, details=None):
        self.dataset, self.assembly = dataset, assembly
        self.segment, self.fitting_method = segment, fitting_method
        self.fitting_atom_type = fitting_atom_type
        self.multi_state = multi_state
        self.radius_of_gyration = radius_of_gyration
        self.details = details

        #: Information about the fit of each model to this restraint's data.
        #: This is a Python dict where keys are :class:`~ihm.model.Model`
        #: objects and values are :class:`SASRestraintFit` objects.
        self.fits = {}


class SASRestraintFit(object):
    """Information on the fit of a model to a :class:`SASRestraint`.
       See :attr:`SASRestaint.fits`.

       :param float chi_value: The fit between the model and the SAS data.
    """
    __slots__ = ["chi_value"]  # Reduce memory usage

    def __init__(self, chi_value=None):
        self.chi_value = chi_value


class EM2DRestraint(Restraint):
    """Restrain part of the system to match an electron microscopy class
       average.

       :param dataset: Reference to the class average data (usually
              an :class:`~ihm.dataset.EM2DClassDataset`).
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param assembly: The part of the system that is fit against the class.
       :type assembly: :class:`~ihm.Assembly`
       :param bool segment: True iff the image has been segmented.
       :param int number_raw_micrographs: The number of particles picked from
              the original raw micrographs that were used to create the
              class average.
       :param float pixel_size_width: Width of each pixel in the image, in
              angstroms.
       :param float pixel_size_height: Height of each pixel in the image, in
              angstroms.
       :param float image_resolution: Resolution of the image, in angstroms.
       :param int number_of_projections: Number of projections of the assembly
              used to fit against the image, if applicable.
       :param str details: Additional details regarding the fitting.
    """

    def __init__(self, dataset, assembly, segment=None,
                 number_raw_micrographs=None, pixel_size_width=None,
                 pixel_size_height=None, image_resolution=None,
                 number_of_projections=None, details=None):
        self.dataset, self.assembly = dataset, assembly
        self.segment = segment
        self.number_raw_micrographs = number_raw_micrographs
        self.pixel_size_width = pixel_size_width
        self.pixel_size_height = pixel_size_height
        self.image_resolution = image_resolution
        self.number_of_projections = number_of_projections
        self.details = details

        #: Information about the fit of each model to this restraint's data.
        #: This is a Python dict where keys are :class:`~ihm.model.Model`
        #: objects and values are :class:`EM2DRestraintFit` objects.
        self.fits = {}


class EM2DRestraintFit(object):
    """Information on the fit of a model to an :class:`EM2DRestraint`.
       See :attr:`EM2DRestaint.fits`.

       :param float cross_correlation_coefficient: The fit between the model
              and the class average.
       :param rot_matrix: Rotation matrix (as a 3x3 array of floats) that
              places the model on the image.
       :param tr_vector: Translation vector (as a 3-element float list) that
              places the model on the image.
    """
    __slots__ = ["cross_correlation_coefficient",
                 "rot_matrix", "tr_vector"]  # Reduce memory usage

    def __init__(self, cross_correlation_coefficient=None,
                 rot_matrix=None, tr_vector=None):
        self.cross_correlation_coefficient = cross_correlation_coefficient
        self.rot_matrix, self.tr_vector = rot_matrix, tr_vector


class CrossLinkRestraint(Restraint):
    """Restrain part of the system to match a set of cross-links.

       :param dataset: Reference to the cross-link data (usually
              a :class:`~ihm.dataset.CXMSDataset`).
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param linker: The type of chemical linker used.
       :type linker: :class:`ihm.ChemDescriptor`
    """

    assembly = None  # no struct_assembly_id for XL restraints

    def __init__(self, dataset, linker):
        self.dataset, self.linker = dataset, linker

        #: All cross-links identified in the experiment, as a simple list
        #: of lists of :class:`ExperimentalCrossLink` objects. All cross-links
        #: in the same sublist are treated as experimentally ambiguous. For
        #: example, xl2 and xl3 here are considered ambiguous::
        #:
        #:     restraint.experimental_cross_links.append([xl1])
        #:     restraint.experimental_cross_links.append([xl2, xl3])
        self.experimental_cross_links = []

        #: All cross-links used in the modeling, as a list of
        #: :class:`CrossLink` objects.
        self.cross_links = []


class ExperimentalCrossLink(object):
    """A cross-link identified in the experiment.

       :param residue1: The first residue linked by the cross-link.
       :type residue1: :class:`ihm.Residue`
       :param residue2: The second residue linked by the cross-link.
       :type residue2: :class:`ihm.Residue`
       :param str details: Additional text describing the cross-link.
    """
    def __init__(self, residue1, residue2, details=None):
        self.residue1, self.residue2 = residue1, residue2
        self.details = details


class DistanceRestraint(object):
    """Base class for all distance restraints. These are typically
       used in a :class:`DerivedDistanceRestraint`.

       Do not use this class directly but instead use a derived class
       such as :class:`HarmonicDistanceRestraint`,
       :class:`UpperBoundDistanceRestraint`,
       :class:`LowerBoundDistanceRestraint`,
       or :class:`LowerUpperBoundDistanceRestraint`.
    """
    restraint_type = None

    #: The minimum distance allowed for this restraint,
    #: or None if unconstrained
    distance_lower_limit = None

    #: The maximum distance allowed for this restraint,
    #: or None if unconstrained
    distance_upper_limit = None


class HarmonicDistanceRestraint(DistanceRestraint):
    """Harmonically restrain two objects to be close to a given distance apart.
       These objects are typically used in a :class:`DerivedDistanceRestraint`.

       :param float distance: Equilibrium distance
    """
    restraint_type = 'harmonic'

    def __init__(self, distance):
        self.distance = distance

    #: The equilibrium distance
    distance_lower_limit = property(lambda self: self.distance)

    #: The equilibrium distance
    distance_upper_limit = distance_lower_limit


class UpperBoundDistanceRestraint(DistanceRestraint):
    """Harmonically restrain two objects to be below a given distance apart.
       These objects are typically used in a :class:`DerivedDistanceRestraint`.

       :param float distance: Distance threshold
    """
    restraint_type = 'upper bound'

    def __init__(self, distance):
        self.distance = distance

    #: The maximum distance allowed by this restraint
    distance_upper_limit = property(lambda self: self.distance)

    #: Minimum distance (unconstrained, so always None)
    distance_lower_limit = None


class LowerBoundDistanceRestraint(DistanceRestraint):
    """Harmonically restrain two objects to be above a given distance apart.
       These objects are typically used in a :class:`DerivedDistanceRestraint`.

       :param float distance: Distance threshold
    """
    restraint_type = 'lower bound'

    def __init__(self, distance):
        self.distance = distance

    #: The minimum distance allowed by this restraint
    distance_lower_limit = property(lambda self: self.distance)

    #: Maximum distance (unconstrained, so always None)
    distance_upper_limit = None


class LowerUpperBoundDistanceRestraint(DistanceRestraint):
    """Harmonically restrain two objects to be above a given distance
       and below another distance apart.
       These objects are typically used in a :class:`DerivedDistanceRestraint`.

       :param float distance_lower_limit: Lower bound on the distance.
       :param float distance_upper_limit: Upper bound on the distance.
    """
    restraint_type = 'lower and upper bound'

    def __init__(self, distance_lower_limit, distance_upper_limit):
        #: The minimum distance allowed by this restraint
        self.distance_lower_limit = distance_lower_limit

        #: The maximum distance allowed by this restraint
        self.distance_upper_limit = distance_upper_limit


class CrossLink(object):
    """Base class for all cross-links used in the modeling.
       Do not use this class directly, but instead use a subclass:
       :class:`ResidueCrossLink`, :class:`AtomCrossLink`,
       or :class:`FeatureCrossLink`."""
    pass


class CrossLinkPseudoSite(object):
    """Pseudo site corresponding to one end of a cross-link.

       These objects are used when the end of a cross-link is not represented
       in the model but its position is known (e.g. it may have been
       approximated given the position of nearby residues). They are passed
       as the ``pseudo1`` or ``pseudo2`` arguments to :class:`CrossLink`
       subclasses.

       :param site: The pseudo site coordinates
       :type site: :class:`PseudoSite`
       :param model: The model in whose coordinate system the pseudo site
              is active (if not specified, the coordinates are assumed to
              be valid for all models using this cross-link).
       :type model: :class:`ihm.model.Model`
    """
    def __init__(self, site, model=None):
        self.site, self.model = site, model


class ResidueCrossLink(CrossLink):
    """A cross-link used in the modeling, applied to residue
       alpha carbon atoms.

       :param experimental_cross_link: The corresponding cross-link identified
              by experiment. Multiple cross-links can map to a single
              experimental identification.
       :type experimental_cross_link: :class:`ExperimentalCrossLink`
       :param asym1: The asymmetric unit containing the first linked residue.
       :type asym1: :class:`ihm.AsymUnit`
       :param asym2: The asymmetric unit containing the second linked residue.
       :type asym2: :class:`ihm.AsymUnit`
       :param distance: Restraint on the distance.
       :type distance: :class:`DistanceRestraint`
       :param float psi: Initial uncertainty in the experimental data.
       :param float sigma1: Initial uncertainty in the position of the first
              residue.
       :param float sigma2: Initial uncertainty in the position of the second
              residue.
       :param bool restrain_all: If True, all cross-links are restrained.
       :param pseudo1: List of pseudo sites representing the position of the
              first residue (if applicable).
       :type pseudo1: List of :class:`CrossLinkPseudoSite`
       :param pseudo2: List of pseudo sites representing the position of the
              second residue (if applicable).
       :type pseudo2: List of :class:`CrossLinkPseudoSite`
    """
    granularity = 'by-residue'
    atom1 = atom2 = None

    def __init__(self, experimental_cross_link, asym1, asym2, distance,
                 psi=None, sigma1=None, sigma2=None, restrain_all=None,
                 pseudo1=None, pseudo2=None):
        self.experimental_cross_link = experimental_cross_link
        self.asym1, self.asym2 = asym1, asym2
        self.psi, self.sigma1, self.sigma2 = psi, sigma1, sigma2
        self.distance, self.restrain_all = distance, restrain_all
        self.pseudo1, self.pseudo2 = pseudo1, pseudo2

        #: Information about the fit of each model to this cross-link
        #: This is a Python dict where keys are :class:`~ihm.model.Model`
        #: objects and values are :class:`CrossLinkFit` objects.
        self.fits = {}


class FeatureCrossLink(CrossLink):
    """A cross-link used in the modeling, applied to the closest primitive
       object with the highest resolution.

       :param experimental_cross_link: The corresponding cross-link identified
              by experiment. Multiple cross-links can map to a single
              experimental identification.
       :type experimental_cross_link: :class:`ExperimentalCrossLink`
       :param asym1: The asymmetric unit containing the first linked residue.
       :type asym1: :class:`ihm.AsymUnit`
       :param asym2: The asymmetric unit containing the second linked residue.
       :type asym2: :class:`ihm.AsymUnit`
       :param distance: Restraint on the distance.
       :type distance: :class:`DistanceRestraint`
       :param float psi: Initial uncertainty in the experimental data.
       :param float sigma1: Initial uncertainty in the position of the first
              residue.
       :param float sigma2: Initial uncertainty in the position of the second
              residue.
       :param bool restrain_all: If True, all cross-links are restrained.
       :param pseudo1: List of pseudo sites representing the position of the
              first residue (if applicable).
       :type pseudo1: List of :class:`CrossLinkPseudoSite`
       :param pseudo2: List of pseudo sites representing the position of the
              second residue (if applicable).
       :type pseudo2: List of :class:`CrossLinkPseudoSite`
    """
    granularity = 'by-feature'
    atom1 = atom2 = None

    def __init__(self, experimental_cross_link, asym1, asym2, distance,
                 psi=None, sigma1=None, sigma2=None, restrain_all=None,
                 pseudo1=None, pseudo2=None):
        self.experimental_cross_link = experimental_cross_link
        self.asym1, self.asym2 = asym1, asym2
        self.psi, self.sigma1, self.sigma2 = psi, sigma1, sigma2
        self.distance, self.restrain_all = distance, restrain_all
        self.pseudo1, self.pseudo2 = pseudo1, pseudo2

        #: Information about the fit of each model to this cross-link
        #: This is a Python dict where keys are :class:`~ihm.model.Model`
        #: objects and values are :class:`CrossLinkFit` objects.
        self.fits = {}


class AtomCrossLink(CrossLink):
    """A cross-link used in the modeling, applied to the specified atoms.

       :param experimental_cross_link: The corresponding cross-link identified
              by experiment. Multiple cross-links can map to a single
              experimental identification.
       :type experimental_cross_link: :class:`ExperimentalCrossLink`
       :param asym1: The asymmetric unit containing the first linked residue.
       :type asym1: :class:`ihm.AsymUnit`
       :param asym2: The asymmetric unit containing the second linked residue.
       :type asym2: :class:`ihm.AsymUnit`
       :param str atom1: The name of the first linked atom.
       :param str atom2: The name of the second linked atom.
       :param distance: Restraint on the distance.
       :type distance: :class:`DistanceRestraint`
       :param float psi: Initial uncertainty in the experimental data.
       :param float sigma1: Initial uncertainty in the position of the first
              residue.
       :param float sigma2: Initial uncertainty in the position of the second
              residue.
       :param bool restrain_all: If True, all cross-links are restrained.
       :param pseudo1: List of pseudo sites representing the position of the
              first residue (if applicable).
       :type pseudo1: List of :class:`CrossLinkPseudoSite`
       :param pseudo2: List of pseudo sites representing the position of the
              second residue (if applicable).
       :type pseudo2: List of :class:`CrossLinkPseudoSite`
    """
    granularity = 'by-atom'

    def __init__(self, experimental_cross_link, asym1, asym2, atom1, atom2,
                 distance, psi=None, sigma1=None, sigma2=None,
                 restrain_all=None, pseudo1=None, pseudo2=None):
        self.experimental_cross_link = experimental_cross_link
        self.asym1, self.asym2 = asym1, asym2
        self.atom1, self.atom2 = atom1, atom2
        self.psi, self.sigma1, self.sigma2 = psi, sigma1, sigma2
        self.distance, self.restrain_all = distance, restrain_all
        self.pseudo1, self.pseudo2 = pseudo1, pseudo2

        #: Information about the fit of each model to this cross-link
        #: This is a Python dict where keys are :class:`~ihm.model.Model`
        #: objects and values are :class:`CrossLinkFit` objects.
        self.fits = {}


class CrossLinkFit(object):
    """Information on the fit of a model to a :class:`CrossLink`.
       See :attr:`ResidueCrossLink.fits`, :attr:`AtomCrossLink.fits`, or
       :attr:`FeatureCrossLink.fits`.

       :param float psi: Uncertainty in the experimental data.
       :param float sigma1: Uncertainty in the position of the first residue.
       :param float sigma2: Uncertainty in the position of the second residue.
    """
    __slots__ = ["psi", "sigma1", "sigma2"]  # Reduce memory usage

    def __init__(self, psi=None, sigma1=None, sigma2=None):
        self.psi, self.sigma1, self.sigma2 = psi, sigma1, sigma2


class Feature(object):
    """Base class for selecting parts of the system that a restraint acts on.
       See :class:`ResidueFeature`, :class:`AtomFeature`,
       :class:`NonPolyFeature`, and :class:`PseudoSiteFeature`.

       Features are typically assigned to one or more
       :class:`~ihm.restraint.GeometricRestraint` or
       :class:`~ihm.restraint.DerivedDistanceRestraint` objects.
    """
    details = None

    def _all_entities_or_asyms(self):
        # Get all Entities or AsymUnits referenced by this object
        return []


class ResidueFeature(Feature):
    """Selection of one or more residues from the system.

       Residues can be selected from both :class:`AsymUnit` and
       :class:`Entity` (the latter implies that it selects residues
       in all instances of that entity). Individual residues can
       also be selected by passing :class:`Residue` objects.

       :param sequence ranges: A list of :class:`AsymUnitRange`,
              :class:`AsymUnit`, :class:`EntityRange`, :class:`Residue`,
              and/or :class:`Entity` objects.
       :param str details: Additional text describing this feature.
    """

    # Type is 'residue' if each range selects a single residue, otherwise
    # it is 'residue range'
    def __get_type(self):
        for r in self.ranges:
            if r.seq_id_range[0] != r.seq_id_range[1]:
                return 'residue range'
        return 'residue'
    type = property(__get_type)

    def __init__(self, ranges, details=None):
        self.ranges, self.details = ranges, details
        _ = self._get_entity_type()

    def _all_entities_or_asyms(self):
        return self.ranges

    def _get_entity_type(self):
        def _get_entity(x):
            if isinstance(x, ihm.Entity):
                return x
            return x.entity if x.entity else x.asym.entity
        if any(not _get_entity(r).is_polymeric() for r in self.ranges):
            raise ValueError("%s cannot select non-polymeric entities" % self)
        else:
            return _get_entity(self.ranges[0]).type if self.ranges else None


class AtomFeature(Feature):
    """Selection of one or more atoms from the system.
       Atoms can be selected from polymers or non-polymers (but not both).
       Atoms can also be selected from both :class:`AsymUnit` and
       :class:`Entity` (the latter implies that it selects atoms
       in all instances of that entity).
       For selecting an entire polymer or residue(s),
       see :class:`ResidueFeature`. For selecting an entire non-polymer,
       see :class:`NonPolyFeature`.

       :param sequence atoms: A list of :class:`ihm.Atom` objects.
       :param str details: Additional text describing this feature.
    """
    type = 'atom'

    def __init__(self, atoms, details=None):
        self.atoms, self.details = atoms, details
        _ = self._get_entity_type()

    def _get_entity_type(self):
        def _get_entity(residue):
            return residue.entity if residue.entity else residue.asym.entity
        types = frozenset(_get_entity(a.residue).type for a in self.atoms)
        if len(types) > 1:
            raise ValueError("%s cannot span both polymeric and "
                             "non-polymeric entities" % self)
        elif types:
            return tuple(types)[0]


class NonPolyFeature(Feature):
    """Selection of one or more non-polymers from the system.
       To select individual atoms from a non-polymer, see :class:`AtomFeature`.

       Features can include both :class:`AsymUnit` and
       :class:`Entity` (the latter implies that it selects non-polymers
       in all instances of that entity).

       :param sequence objs: A list of :class:`AsymUnit` and/or
              :class:`Entity` objects.
       :param str details: Additional text describing this feature.
    """

    type = 'ligand'

    def __init__(self, objs, details=None):
        self.objs, self.details = objs, details
        _ = self._get_entity_type()

    def _all_entities_or_asyms(self):
        return self.objs

    def _get_entity_type(self):
        def _get_entity(x):
            return x if isinstance(x, ihm.Entity) else x.entity
        if any(_get_entity(r).is_polymeric() for r in self.objs):
            raise ValueError(
                "%s can only select non-polymeric entities" % self)
        else:
            return _get_entity(self.objs[0]).type if self.objs else None


class PseudoSiteFeature(Feature):
    """Selection of a pseudo position in the system.

       :param site: The pseudo site to use for the feature.
       :type site: :class:`PseudoSite`
    """

    type = 'pseudo site'

    def __init__(self, site):
        self.site = site

    def _get_entity_type(self):
        return 'other'


class GeometricRestraint(object):
    """A restraint between part of the system and some part of a
       geometric object. See :class:`CenterGeometricRestraint`,
       :class:`InnerSurfaceGeometricRestraint`,
       :class:`OuterSurfaceGeometricRestraint`.

       :param dataset: Reference to the data from which the restraint is
              derived.
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param geometric_object: The geometric object to restrain against.
       :type geometric_object: :class:`ihm.geometry.GeometricObject`
       :param feature: The part of the system to restrain.
       :type feature: :class:`Feature`
       :param distance: Restraint on the distance.
       :type distance: :class:`DistanceRestraint`
       :param float harmonic_force_constant: Force constant, if applicable.
       :param bool restrain_all: If True, all distances are restrained.
    """
    object_characteristic = 'other'
    assembly = None  # no struct_assembly_id for geometric restraints

    def __init__(self, dataset, geometric_object, feature, distance,
                 harmonic_force_constant=None, restrain_all=None,
                 pseudo1=None, pseudo2=None):
        self.dataset = dataset
        self.geometric_object, self.feature = geometric_object, feature
        self.distance, self.restrain_all = distance, restrain_all
        self.harmonic_force_constant = harmonic_force_constant
    _all_features = property(lambda self: (self.feature,))


class CenterGeometricRestraint(GeometricRestraint):
    """A restraint between part of the system and the center of a
       geometric object. See :class:`GeometricRestraint` for a description
       of the parameters.
    """
    object_characteristic = 'center'


class InnerSurfaceGeometricRestraint(GeometricRestraint):
    """A restraint between part of the system and the inner surface of a
       geometric object. See :class:`GeometricRestraint` for a description
       of the parameters.
    """
    object_characteristic = 'inner surface'


class OuterSurfaceGeometricRestraint(GeometricRestraint):
    """A restraint between part of the system and the outer surface of a
       geometric object. See :class:`GeometricRestraint` for a description
       of the parameters.
    """
    object_characteristic = 'outer surface'


class DerivedDistanceRestraint(object):
    """A restraint between two parts of the system, derived from experimental
       data.

       :param dataset: Reference to the data from which the restraint is
              derived.
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param feature1: The first part of the system to restrain.
       :type feature1: :class:`Feature`
       :param feature2: The second part of the system to restrain.
       :type feature2: :class:`Feature`
       :param distance: Restraint on the distance.
       :type distance: :class:`DistanceRestraint`
       :param float probability: Likelihood that restraint is correct (0. - 1.)
       :param bool restrain_all: If True, all distances are restrained.
       :param float mic_value: Value of the Maximal Information Coefficient
              (MIC) for this interaction, if applicable.
    """
    assembly = None  # no struct_assembly_id for derived distance restraints

    def __init__(self, dataset, feature1, feature2, distance,
                 probability=None, restrain_all=None, mic_value=None):
        self.dataset = dataset
        self.feature1, self.feature2 = feature1, feature2
        self.distance, self.restrain_all = distance, restrain_all
        self.probability = probability
        self.mic_value = mic_value
    _all_features = property(lambda self: (self.feature1, self.feature2))


class PredictedContactRestraint(object):
    """A predicted contact between two parts of the system, derived from
       various computational tools.

       :param dataset: Reference to the data from which the restraint is
              derived.
       :type dataset: :class:`~ihm.dataset.Dataset`
       :param resatom1: The first residue or atom to restrain.
       :type resatom1: :class:`ihm.Residue` or :class:`ihm.Atom`
       :param resatom2: The second residue or atom to restrain.
       :type resatom2: :class:`ihm.Residue` or :class:`ihm.Atom`
       :param distance: Restraint on the distance.
       :type distance: :class:`DistanceRestraint`
       :param bool by_residue: If True, the restraint is applied to specific
              residues; otherwise, it is applied to the closest primitive
              object with the highest resolution.
       :param float probability: Likelihood that restraint is correct (0. - 1.)
       :param software: The software used to generate the contact.
       :type software: :class:`~ihm.Software`

    """
    assembly = None  # no struct_assembly_id for predicted contact restraints

    def __init__(self, dataset, resatom1, resatom2, distance,
                 by_residue, probability=None, software=None):
        self.dataset = dataset
        self.resatom1, self.resatom2 = resatom1, resatom2
        self.distance, self.by_residue = distance, by_residue
        self.probability, self.software = probability, software
