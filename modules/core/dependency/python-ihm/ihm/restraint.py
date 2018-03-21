"""Classes for handling restraints on the system.
"""

class Restraint(object):
    """Base class for all restraints.
       See :attr:`ihm.System.restraints`.
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
       :param str fitting_method: The method used to fit the model into the map.
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
    __slots__ = ["cross_correlation_coefficient"] # Reduce memory usage

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
    __slots__ = ["chi_value"] # Reduce memory usage

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
                 "rot_matrix", "tr_vector"] # Reduce memory usage

    def __init__(self, cross_correlation_coefficient=None,
                 rot_matrix=None, tr_vector=None):
        self.cross_correlation_coefficient = cross_correlation_coefficient
        self.rot_matrix, self.tr_vector = rot_matrix, tr_vector
