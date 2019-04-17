"""Classes for handling models (sets of coordinates) as well as
   groups of models.
"""

import struct
import itertools

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
              (can be None for HETATM sites)
       :param str atom_id: The name of the atom in the residue
       :param str type_symbol: Element name
       :param float x: x coordinate of the atom
       :param float y: y coordinate of the atom
       :param float z: z coordinate of the atom
       :param bool het: True for HETATM sites, False (default) for ATOM
       :param float biso: Temperature factor or equivalent (if applicable)
    """

    # Reduce memory usage
    __slots__ = ['asym_unit', 'seq_id', 'atom_id', 'type_symbol',
                 'x', 'y', 'z', 'het', 'biso']

    def __init__(self, asym_unit, seq_id, atom_id, type_symbol, x, y, z,
                 het=False, biso=None):
        self.asym_unit = asym_unit
        self.seq_id, self.atom_id = seq_id, atom_id
        self.type_symbol = type_symbol
        self.x, self.y, self.z = x, y, z
        self.het, self.biso = het, biso


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

    def add_sphere(self, sphere):
        """Add to the model's set of :class:`Sphere` objects.

           See :meth:`get_spheres` for more details.
        """
        self._spheres.append(sphere)

    def get_atoms(self):
        """Yield :class:`Atom` objects that represent this model.

           See :meth:`get_spheres` for more details.
        """
        for a in self._atoms:
            yield a

    def add_atom(self, atom):
        """Add to the model's set of :class:`Atom` objects.

           See :meth:`get_spheres` for more details.
        """
        self._atoms.append(atom)


class ModelGroup(list):
    """A set of related models. See :class:`Model`. It is implemented as
       a simple list of the models.

       These objects are typically stored in a :class:`State`,
       :class:`Ensemble`, or :class:`OrderedProcess`.

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
              for the entire ensemble, for example as a DCD file
              (see :class:`DCDWriter`).
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


class OrderedProcess(object):
    """Details about a process that orders two or more model groups.

       A process is represented as a directed graph, where the nodes
       are :class:`ModelGroup` objects and the edges represent transitions.

       These objects are generally added to
       :attr:`ihm.System.ordered_processes`.

       :param str ordered_by: Text that explains how the ordering is done,
              such as "time steps".
       :param str description: Text that describes this process.
    """
    def __init__(self, ordered_by, description=None):
        self.ordered_by, self.description = ordered_by, description
        #: All steps in this process, as a simple list of
        #: :class:`ProcessStep` objects
        self.steps = []


class ProcessStep(list):
    """A single step in an :class:`OrderedProcess`.
    
       This is implemented as a simple list of :class:`ProcessEdge` objects,
       each of which orders two :class:`ModelGroup` objects. (To order more
       than two groups, for example to represent a branched reaction step
       that generates two products, simply add multiple edges to the step.)

       :param sequence elements: Initial set of :class:`ProcessEdge` objects.
       :param str description: Text that describes this step.
    """
    def __init__(self, elements=(), description=None):
        self.description = description
        super(ProcessStep, self).__init__(elements)


class ProcessEdge(object):
    """A single directed edge in the graph for a :class:`OrderedProcess`,
       representing the transition from one :class:`ModelGroup` to another.
       These objects are added to :class:`ProcessStep` objects.

       :param group_begin: The set of models at the origin of the edge.
       :type group_begin: :class:`ModelGroup`
       :param group_end: The set of models at the end of the edge.
       :type group_end: :class:`ModelGroup`
       :param str description: Text that describes this edge.
    """
    def __init__(self, group_begin, group_end, description=None):
        self.group_begin, self.group_end = group_begin, group_end
        self.description = description


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


class DCDWriter(object):
    """Utility class to write model coordinates to a binary DCD file.

       See :class:`Ensemble` and :class:`Model`. Since mmCIF is a text-based
       format, it is not efficient to store entire ensembles in this format.
       Instead, representative models should be deposited as mmCIF and
       the :class:`Ensemble` then linked to an external file containing
       only model coordinates. One such format is CHARMM/NAMD's DCD, which
       is written out by this class. The DCD files simply contain the xyz
       coordinates of all :class:`Atom` and :class:`Sphere` objects in each
       :class:`Model`. (Note that no other data is stored, such as sphere
       radii or restraint parameters.)

       :param file fh: The filelike object to write the coordinates to. This
              should be open in binary mode and should be a seekable object.
    """
    def __init__(self, fh):
        self.fh = fh
        self.nframes = 0

    def add_model(self, model):
        """Add the coordinates for the given :class:`Model` to the file as
           a new frame. All models in the file should have the same number of
           atoms and/or spheres, in the same order.

           :param model: Model with coordinates to write to the file.
           :type model: :class:`Model`
        """
        x = []
        y = []
        z = []
        for a in itertools.chain(model.get_atoms(), model.get_spheres()):
            x.append(a.x)
            y.append(a.y)
            z.append(a.z)
        self._write_frame(x, y, z)

    def _write_frame(self, x, y, z):
        self.nframes += 1
        if self.nframes == 1:
            self.ncoord = len(x)
            remarks = [
              b'Produced by python-ihm, https://github.com/ihmwg/python-ihm',
              b'This file is designed to be used in combination with an '
              b'mmCIF file',
              b'See PDB-Dev at https://pdb-dev.wwpdb.org/ for more details']
            self._write_header(self.ncoord, remarks)
        else:
            if len(x) != self.ncoord:
                raise ValueError("Frame size mismatch - frames contain %d "
                        "coordinates but attempting to write a frame "
                        "containing %d coordinates" % (self.ncoord, len(x)))
            # Update number of frames
            self.fh.seek(self._pos_nframes)
            self.fh.write(struct.pack('i', self.nframes))
            self.fh.seek(0, 2) # Move back to end of file

        # Write coordinates
        frame_size = struct.pack('i', struct.calcsize("%df" % self.ncoord))
        for coord in x, y, z:
            self.fh.write(frame_size)
            self.fh.write(struct.pack("%df" % self.ncoord, *coord))
            self.fh.write(frame_size)

    def _write_header(self, natoms, remarks):
        self.fh.write(struct.pack('i', 84) + b'CORD')
        self._pos_nframes = self.fh.tell()
        self.fh.write(struct.pack('i', self.nframes))
        self.fh.write(struct.pack('i', 0)) # istart
        self.fh.write(struct.pack('i', 0)) # nsavc
        self.fh.write(struct.pack('5i', 0, 0, 0, 0, 0))
        self.fh.write(struct.pack('i', 0)) # number of fixed atoms
        self.fh.write(struct.pack('d', 0.)) # delta
        self.fh.write(struct.pack('10i', 0, 0, 0, 0, 0, 0, 0, 0, 0, 84))
        remark_size = struct.calcsize('i') + 80 * len(remarks)
        self.fh.write(struct.pack('i', remark_size))
        self.fh.write(struct.pack('i', len(remarks)))
        for r in remarks:
            self.fh.write(r.ljust(80)[:80])
        self.fh.write(struct.pack('i', remark_size))
        self.fh.write(struct.pack('i', struct.calcsize('i')))
        self.fh.write(struct.pack('i', natoms)) # total number of atoms
        self.fh.write(struct.pack('i', struct.calcsize('i')))
