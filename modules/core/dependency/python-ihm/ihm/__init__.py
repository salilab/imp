"""Representation of an IHM mmCIF file as a set of Python classes.

   Generally class names correspond to mmCIF table names and class
   attributes to mmCIF attributes (with prefixes like `pdbx_` stripped).
   For example, the data item _entity.details is found in the
   :class:`Entity` class, as the `details` member.

   Ordinals and IDs are generally not used in this representation (instead,
   pointers to objects are used).
"""

import itertools
from .format import CifWriter

#: A value that isn't known. Note that this is distinct from a value that
#: is deliberately omitted, which is represented by Python None.
unknown = CifWriter.unknown

def _remove_identical(gen):
    """Return only unique objects from `gen`.
       Objects that are identical are only returned once, although multiple
       non-identical objects that compare equal may be returned."""
    seen_objs = {}
    for obj in gen:
        if id(obj) in seen_objs:
            continue
        seen_objs[id(obj)] = None
        yield obj

class System(object):
    """Top-level class representing a complete modeled system"""

    def __init__(self, name='model'):
        self.name = name

        #: List of all software used in the modeling. See :class:`Software`.
        self.software = []

        #: All entities used in the system. See :class:`Entity`.
        self.entities = []

        #: All asymmetric units used in the system. See :class:`AsymUnit`.
        self.asym_units = []

        #: All assemblies used in the system. See :class:`Assembly`.
        self.assemblies = []

        #: The assembly of the entire system. By convention this is always
        #: the first assembly in the mmCIF file (assembly_id=1). Note that
        #: currently this isn't filled in on output until dumper.write()
        #: is called.
        self.complete_assembly = Assembly((), name='Complete assembly',
                                          description='All known components')
        self.assemblies.append(self.complete_assembly)

        #: Locations of all extra resources.
        #: See :class:`~ihm.location.Location`.
        self.locations = []

        #: All datasets used in the modeling.
        #: See :class:`~ihm.dataset.Dataset`.
        self.datasets = []

        #: All orphaned groups of datasets.
        #: This can be used to keep track of all dataset groups that are not
        #: otherwise used - normally a group is assigned to a
        #: :class:`~ihm.protocol.Protocol`.
        #: See :class:`~ihm.dataset.DatasetGroup`.
        self.orphan_dataset_groups = []

        #: All representations of the system.
        #: See :class:`~ihm.representation.Representation`.
        self.representations = []

        #: All starting models for the system.
        #: See :class:`~ihm.startmodel.StartingModel`.
        self.starting_models = []

        #: All restraints on the system.
        #: See :class:`~ihm.restraint.Restraint`.
        self.restraints = []

        #: All orphaned modeling protocols.
        #: This can be used to keep track of all protocols that are not
        #: otherwise used - normally a protocol is assigned to a
        #: :class:`~ihm.model.Model`.
        #: See :class:`~ihm.protocol.Protocol`.
        self.orphan_protocols = []

        #: All ensembles.
        #: See :class:`~ihm.model.Ensemble`.
        self.ensembles = []

        #: All state groups (collections of models).
        #: See :class:`~ihm.model.StateGroup`.
        self.state_groups = []

    def update_locations_in_repositories(self, repos):
        """Update all :class:`Location` objects in the system that lie within
           a checked-out :class:`Repository` to point to that repository.

           This is intended for the use case where the current working directory
           is a checkout of a repository which is archived somewhere with a DOI.
           Locations can then be simply constructed pointing to local files,
           and retroactively updated with this method to point to the DOI if
           appropriate.

           For each Location, if it points to a local file that is below the
           `root` of one of the `repos`, update it to point to that repository.
           If is under multiple roots, pick the one that gives the shortest
           path. For example, if run in a subdirectory `foo` of a repository
           archived as `repo.zip`, the local path `simple.pdb` will
           be updated to be `repo-top/foo/simple.pdb` in `repo.zip`::

               l = ihm.location.InputFileLocation("simple.pdb")
               system.locations.append(l)

               r = ihm.location.Repository(doi='1.2.3.4',
                         url='https://example.com/repo.zip',)
                         top_directory="repo-top", root="..")
               system.update_locations_in_repositories([r])
        """
        for loc in self._all_locations():
            if isinstance(loc, location.FileLocation):
                location.Repository._update_in_repos(loc, repos)

    def _all_model_groups(self):
        """Iterate over all ModelGroups in the system"""
        # todo: raise an error if a modelgroup is present in multiple states
        for state_group in self.state_groups:
            for state in state_group:
                for model_group in state:
                    yield model_group

    def _all_models(self):
        """Iterate over all Models in the system"""
        # todo: raise an error if a model is present in multiple groups
        for group in self._all_model_groups():
            seen_models = {}
            for model in group:
                if model in seen_models:
                    continue
                seen_models[model] = None
                yield group, model

    def _all_protocols(self):
        """Iterate over all Protocols in the system.
           This includes all Protocols referenced from other objects, plus
           any orphaned Protocols. Duplicates are filtered out."""
        return _remove_identical(itertools.chain(
                        self.orphan_protocols,
                        (model.protocol for group, model in self._all_models()
                                        if model.protocol)))

    def _all_dataset_groups(self):
        """Iterate over all DatasetGroups in the system.
           This includes all DatasetGroups referenced from other objects, plus
           any orphaned groups. Duplicates may be present."""
        def all_protocol_steps():
            for protocol in self._all_protocols():
                for step in protocol.steps:
                    yield step
        return itertools.chain(
                  self.orphan_dataset_groups,
                  (step.dataset_group for step in all_protocol_steps()
                                      if step.dataset_group))

    def _all_locations(self):
        """Iterate over all Locations in the system.
           This includes all Locations referenced from other objects, plus
           any referenced from the top-level system.
           Duplicates may be present."""
        def all_densities():
            for ensemble in self.ensembles:
                for density in ensemble.densities:
                    yield density
        def all_templates():
            for startmodel in self.starting_models:
                for template in startmodel.templates:
                    yield template
        return itertools.chain(
                self.locations,
                (dataset.location for dataset in self.datasets
                          if hasattr(dataset, 'location') and dataset.location),
                (ensemble.file for ensemble in self.ensembles if ensemble.file),
                (density.file for density in all_densities() if density.file),
                (template.alignment_file for template in all_templates()
                                         if template.alignment_file))

    def _make_complete_assembly(self):
        """Fill in the complete assembly with all entities/asym units"""
        # Clear out any existing components
        self.complete_assembly[:] = []

        # Include all asym units
        seen_entities = {}
        for asym in self.asym_units:
            self.complete_assembly.append(AssemblyComponent(asym))
            seen_entities[asym.entity] = None
        # Add all entities without structure
        for entity in self.entities:
            if entity not in seen_entities:
                self.complete_assembly.append(AssemblyComponent(entity))


class Software(object):
    """Software used as part of the modeling protocol.

       See :attr:`System.software`."""
    def __init__(self, name, classification, description, location,
                 type='program', version=None):
        self.name = name
        self.classification = classification
        self.description = description
        self.location = location
        self.type = type
        self.version = version

    # Software compares equal if the names and versions are the same
    def _eq_vals(self):
        return (self.name, self.version)
    def __eq__(self, other):
        return self._eq_vals() == other._eq_vals()
    def __hash__(self):
        return hash(self._eq_vals())


class Entity(object):
    """Represent a CIF entity (with a unique sequence)

       See :attr:`System.entities`.

       Note that currently only standard amino acids are supported.
    """

    type = 'polymer'
    src_method = 'man'
    number_of_molecules = 1
    formula_weight = unknown

    def __init__(self, seq, description=None, details=None):
        self.sequence = seq
        self.description, self.details = description, details

    # Entities are considered identical if they have the same sequence
    def __eq__(self, other):
        return self.sequence == other.sequence
    def __hash__(self):
        return hash(self.sequence)

class AsymUnitRange(object):
    """Part of an asymmetric unit. Usually these objects are created from
       an :class:`AsymUnit`, e.g. to get a range covering residues 4 through
       7 in `asym` use::

           asym = ihm.AsymUnit(entity)
           rng = asym(4,7)
    """
    def __init__(self, asym, seq_id_begin, seq_id_end):
        self.asym = asym
        # todo: check range for validity (at property read time)
        self.seq_id_range = (seq_id_begin, seq_id_end)

    # Use same ID and entity as the original asym unit
    _id = property(lambda self: self.asym._id)
    entity = property(lambda self: self.asym.entity)


class AsymUnit(object):
    """An asymmetric unit, i.e. a unique instance of an Entity that
       was modeled.

       See :attr:`System.asym_units`.
    """

    def __init__(self, entity, details=None):
        self.entity, self.details = entity, details

    def __call__(self, seq_id_begin, seq_id_end):
        return AsymUnitRange(self, seq_id_begin, seq_id_end)

    seq_id_range = property(lambda self: (1, len(self.entity.sequence)),
                            doc="Sequence range")


class AssemblyComponent(object):
    """A single component in an :class:`Assembly`.

       :param component: The part of the system. :class:`Entity` can be used
                         here for a part that is known but has no structure.
       :type component: :class:`AsymUnit` or :class:`Entity`
       :param tuple seq_id_range: The subset of the sequence range to include in
                         the assembly. This should be a two-element tuple.
                         If `None` (the default) the entire range is included.
    """

    def __init__(self, component, seq_id_range=None):
        self._component, self._seqrange = component, seq_id_range

    def __eq__(self, other):
        return (self._component is other._component
                and self.seq_id_range == other.seq_id_range)
    def __hash__(self):
        return hash((id(self._component), self.seq_id_range))

    def __get_seqrange(self):
        if self._seqrange:
            # todo: fail if this is out of entity.sequence range
            return self._seqrange
        else:
            return (1, len(self.entity.sequence))
    seq_id_range = property(__get_seqrange, doc="Sequence range")

    def __get_entity(self):
        if isinstance(self._component, Entity):
            return self._component
        else:
            return self._component.entity
    entity = property(__get_entity,
                      doc="The Entity for this part of the system")

    def __get_asym(self):
        if isinstance(self._component, AsymUnit):
            return self._component
    asym = property(__get_asym,
                    doc="The AsymUnit for this part of the system, or None")


class Assembly(list):
    """A collection of parts of the system that were modeled or probed
       together.

       This is implemented as a simple list of :class:`AssemblyComponent`
       objects. (For convenience, the constructor will also accept
       :class:`Entity` and :class:`AsymUnit` objects in the initial list.)

       See :attr:`System.assemblies`.

       Note that any duplicate assemblies will be pruned on output."""

    #: :class:`Assembly` that is the immediate parent in a hierarchy, or `None`
    parent = None

    def __init__(self, elements=(), name=None, description=None):
        def fix_element(e):
            if isinstance(e, AssemblyComponent):
                return e
            else:
                return AssemblyComponent(e)
        super(Assembly, self).__init__(fix_element(e) for e in elements)
        self.name, self.description = name, description
