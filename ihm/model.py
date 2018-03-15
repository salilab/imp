"""Classes for handling models (sets of coordinates) as well as
   groups of models.
"""

class Sphere(object):
    """Coordinates of part of the model represented by a sphere.

       :param asym_unit: The asymmetric unit that this sphere represents
       :type asym_unit: :class:`ihm.AsymUnit`
       :param tuple seq_id_range: The range of residues represented by this
              sphere (as a two-element tuple)
       :param float x: x coordinate of the center of the sphere
       :param float y: y coordinate of the center of the sphere
       :param float z: z coordinate of the center of the sphere
       :param float radius: radius of the sphere
       :param float rmsf: root-mean-square fluctuation of the coordinates
    """

    # Reduce memory usage
    __slots__ = ['asym_unit', 'seq_id_range', 'x', 'y', 'z', 'radius', 'rmsf']

    def __init__(self, asym_unit, seq_id_range, x, y, z, radius, rmsf=None):
        self.asym_unit = asym_unit
        self.seq_id_range = seq_id_range
        self.x, self.y, self.z = x, y, z
        self.radius, self.rmsf = radius, rmsf


class Atom(object):
    """Coordinates of part of the model represented by an atom.

       :param asym_unit: The asymmetric unit that this sphere represents
       :type asym_unit: :class:`ihm.AsymUnit`
       :param int seq_id: The residue index represented by this atom
       :param str atom_id: The name of the atom in the residue
       :param float x: x coordinate of the atom
       :param float y: y coordinate of the atom
       :param float z: z coordinate of the atom
    """

    # Reduce memory usage
    __slots__ = ['asym_unit', 'seq_id', 'atom_id', 'x', 'y', 'z']

    def __init__(self, asym_unit, seq_id, atom_id, x, y, z):
        self.asym_unit = asym_unit
        self.seq_id, self.atom_id = seq_id, atom_id
        self.x, self.y, self.z = x, y, z


class Model(object):
    """A single set of coordinates (conformation).

       See :class:`ModelGroup`.

       :param assembly: The parts of the system that were modeled.
       :type assembly: :class:`~ihm.Assembly`
       :param protocol: Description of how the modeling was done.
       :type protocol: :class:`~ihm.protocol.Protocol`
       :param representation: Level of detail at which the system
              was represented.
       :type representation: :class:`~ihm.representation.Representation`
       :param str name: Descriptive name for this model.
    """
    def __init__(self, assembly, protocol, representation, name=None):
        self.assembly, self.protocol = assembly, protocol
        self.representation, self.name = representation, name
        self._atoms = []
        self._spheres = []

    def get_spheres(self):
        """Yield :class:`Sphere` objects that represent this model.

           The default implementation simply iterates over an internal
           list of spheres, but this is not very memory-efficient, particularly
           if the spheres are already stored somewhere else, e.g. in the
           software's own data structures. It is recommended to subclass
           and provide a more efficient implementation.

           Note that the set of spheres should match the model's
           :class:`~ihm.representation.Representation`. This is not currently
           enforced.
        """
        for s in self._spheres:
            yield s

    def set_spheres(self, spheregen):
        """Populate the model's set of :class:`Sphere` objects from the
           given Python generator.

           See :meth:`get_spheres` for more details.
        """
        self._spheres = [s for s in spheregen]

    def get_atoms(self):
        """Yield :class:`Atom` objects that represent this model.

           See :meth:`get_spheres` for more details.
        """
        for a in self._atoms:
            yield a

    def set_atoms(self, atomgen):
        """Populate the model's set of :class:`Atom` objects from the
           given Python generator.

           See :meth:`get_spheres` for more details.
        """
        self._atoms = [a for a in atomgen]


class ModelGroup(list):
    """A set of related models. See :class:`Model` and
       :attr:`State.model_groups`. It is implemented as a simple
       list of the models.

       :param elements: Initial set of models in the group.
       :param str name: Descriptive name for the group.
    """
    def __init__(self, elements=(), name=None):
        self.name = name
        super(ModelGroup, self).__init__(elements)


class State(list):
    """A set of model groups that constitute a single state of the system.
       It is implemented as a simple list of the model groups.
       See :class:`StateGroup`.

       :param elements: The initial set of :class:`ModelGroup` objects in
              this state.
    """
    def __init__(self, elements=(), type=None, name=None, details=None,
                 experiment_type=None, population_fraction=None):
        self.type, self.name, self.details = type, name, details
        self.experiment_type = experiment_type
        self.population_fraction = population_fraction
        super(State, self).__init__(elements)


class StateGroup(list):
    """A set of related states. See :class:`State` and
       :attr:`ihm.System.state_groups`. It is implemented as a simple
       list of the states.

       :param elements: Initial set of states in the group.
    """
    def __init__(self, elements=()):
        super(StateGroup, self).__init__(elements)


class Ensemble(object):
    """Details about a model cluster or ensemble.
       See :attr:`ihm.System.ensembles`.

       :param model_group: The set of models in this ensemble.
       :type model_group: :class:`ModelGroup`
       :param int num_models: The total number of models in this ensemble. This
              may be more than the number of models in `model_group`, for
              example if only representative or top-scoring models
              are deposited.
       :param post_process: The final analysis step that generated this
              ensemble.
       :type post_process: :class:`ihm.analysis.Step`
       :param str clustering_method: The method used to obtain the ensemble,
              if applicable.
       :param str clustering_feature: The feature used for clustering
              the models, if applicable.
       :param str name: A descriptive name for this ensemble.
       :param float precision: The precision of the entire ensemble.
       :param file: A reference to an external file containing coordinates
              for the entire ensemble, for example as a DCD file.
       :type file: :class:`ihm.location.OutputFileLocation`
    """
    def __init__(self, model_group, num_models, post_process=None,
                 clustering_method=None, clustering_feature=None, name=None,
                 precision=None, file=None):
        self.model_group, self.num_models = model_group, num_models
        self.post_process = post_process
        self.clustering_method = clustering_method
        self.clustering_feature = clustering_feature
        self.name, self.precision, self.file = name, precision, file

        #: All localization densities for this ensemble, as
        #: :class:`LocalizationDensity` objects
        self.densities = []

    num_models_deposited = property(lambda self: len(self.model_group),
                                    doc="Number of models in this ensemble "
                                        "that are in the mmCIF file")


class LocalizationDensity(object):
    """Localization density of part of the system, over all models
       in an ensemble.

       See :attr:`Ensemble.densities`.

       :param file: A reference to an external file containing the density,
              for example as an MRC file.
       :type file: :class:`ihm.location.OutputFileLocation`
       :param asym_unit: The asymmetric unit (or part of one) that
              this density represents.
       :type asym_unit: :class:`~ihm.AsymUnit` or :class:`~ihm.AsymUnitRange`
    """
    def __init__(self, file, asym_unit):
        self.file, self.asym_unit = file, asym_unit
