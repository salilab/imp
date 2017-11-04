"""@namespace IMP.mmcif.data
   @brief Classes to represent data structures used in mmCIF.
"""

from __future__ import print_function
import IMP.atom
import IMP.mmcif.dataset
import IMP.mmcif.metadata

def get_molecule(h):
    """Given a Hierarchy, walk up and find the parent Molecule"""
    while h:
        if IMP.atom.Molecule.get_is_setup(h):
            return IMP.atom.Molecule(h)
        h = h.get_parent()
    return None

class _Entity(object):
    """Represent a CIF entity (a component with a unique sequence)"""
    def __init__(self, seq):
        self.sequence = seq
        self.first_component = None
    # Use the name of the first component as the description of the entity
    description = property(lambda self: self.first_component.name)


class _EntityMapper(dict):
    """Handle mapping from IMP chains to CIF entities.
       Multiple components may map to the same entity if they share sequence."""
    def __init__(self):
        super(_EntityMapper, self).__init__()
        self._sequence_dict = {}
        self._entities = []

    def add(self, chain):
        # todo: handle non-protein sequences
        sequence = chain.get_sequence()
        if sequence not in self._sequence_dict:
            entity = _Entity(sequence)
            self._entities.append(entity)
            entity.id = len(self._entities)
            self._sequence_dict[sequence] = entity
        self[chain] = self._sequence_dict[sequence]
        return self[chain]

    def get_all(self):
        """Yield all entities"""
        return self._entities


def _assign_id(obj, seen_objs, obj_by_id):
    """Assign a unique ID to obj, and track all ids in obj_by_id."""
    if obj not in seen_objs:
        if not hasattr(obj, 'id'):
            obj_by_id.append(obj)
            obj.id = len(obj_by_id)
        seen_objs[obj] = obj.id
    else:
        obj.id = seen_objs[obj]


class _Component(object):
    """An mmCIF component. This is an instance of an _Entity. Multiple
       _Components may map to the same _Entity but must have unique
       asym_ids. A _Component is similar to an IMP Chain but multiple
       Chains may map to the same _Component (the Chains represent the
       same structure, just in different states, and potentially in
       different IMP Models). A _Component may also represent something
       that is described by an experiment but was not modeled by IMP, and
       so no Chains map to it but a string name does."""
    def __init__(self, entity, asym_id, name):
        self.entity, self.asym_id, self.name = entity, asym_id, name


class _ComponentMapper(dict):
    """Handle mapping from chains to CIF components."""
    def __init__(self):
        super(_ComponentMapper, self).__init__()
        self._all_components = []
        self._all_modeled_components = []
        self._map = {}

    def add(self, chain, entity):
        """Add a chain (either an IMP Chain object for a modeled component,
           or a NonModeledChain object for a non-modeled component)"""
        if isinstance(chain, IMP.atom.Chain):
            modeled = True
            mol = get_molecule(chain)
            asym_id = map_key = chain.get_id()
            name = mol.get_name() if mol else None
        else:
            modeled = False
            asym_id = None
            name = map_key = chain.name
        if map_key not in self._map:
            component = _Component(entity, asym_id, name)
            if entity.first_component is None:
                entity.first_component = component
            self._all_components.append(component)
            if modeled:
                self._all_modeled_components.append(component)
            self._map[map_key] = component
        else:
            component = self._map[map_key]
            if component.entity != entity:
                raise ValueError("Two chains have the same ID (%s) but "
                                 "different sequences - rename one of the "
                                 "chains" % map_key)
        return component

    def get_all(self):
        """Get all components"""
        return self._all_components

    def get_all_modeled(self):
        """Get all modeled components"""
        return self._all_modeled_components


class _Assembly(list):
    """A collection of components. Currently simply implemented as a list of
       the _Component objects. These must be in creation order."""
    def __hash__(self):
        # allow putting assemblies in a dict. 'list' isn't hashable
        # but 'tuple' is
        return hash(tuple(self))

class _Representation(object):
    """Group a set of contiguous particles with the same representation"""
    def __init__(self):
        self.particles = []
        self.residue_range = () # inclusive range
        self.starting_model = None

    def add(self, particle, starting_model):
        """Potentially add a new particle to this representation.
           Iff the particle could be added, return True."""
        resrange, rigid_body, primitive = self._get_particle_info(particle)
        if not self.particles:
            self.particles.append(particle)
            self.residue_range = resrange
            self.rigid_body = rigid_body
            self.primitive = primitive
            self.starting_model = starting_model
            return True
        elif type(particle) == type(self.particles[0]) \
             and resrange[0] == self.residue_range[1] + 1 \
             and starting_model == self.starting_model \
             and self._same_rigid_body(rigid_body):
            self.particles.append(particle)
            self.residue_range = (self.residue_range[0], resrange[1])
            return True

    def _same_rigid_body(self, rigid_body):
        # Note: can't just use self.rigid_body == rigid_body as IMP may
        # crash when comparing a RigidBody object against None
        if self.rigid_body is None and rigid_body is None:
            return True
        elif self.rigid_body is None or rigid_body is None:
            return False
        else:
            return self.rigid_body == rigid_body

    def _get_particle_info(self, p):
        # Note that we consider nonrigid members to not be rigid here
        if IMP.core.RigidMember.get_is_setup(p):
            rigid_body = IMP.core.RigidMember(p).get_rigid_body()
        else:
            rigid_body = None
        if isinstance(p, IMP.atom.Residue):
            return (p.get_index(), p.get_index()), rigid_body, 'sphere'
        elif isinstance(p, IMP.atom.Fragment):
            resinds = p.get_residue_indexes()
            return (resinds[0], resinds[-1]), rigid_body, 'sphere'
        raise TypeError("Unknown particle ", p)

    def __bool__(self):
        return len(self.particles) > 0

    __nonzero__ = __bool__ # Python 2 compatibility

def _get_all_structure_provenance(p):
    """Yield all StructureProvenance decorators for the given particle."""
    if IMP.core.Provenanced.get_is_setup(p):
        prov = IMP.core.Provenanced(p).get_provenance()
        while prov:
            if IMP.core.StructureProvenance.get_is_setup(prov):
                yield IMP.core.StructureProvenance(prov)
            prov = prov.get_previous()

class _StartingModel(object):
    _eq_keys = ['filename', 'chain_id', 'offset']

    def __init__(self, struc_prov):
        self.filename = struc_prov[0].get_filename()
        self.chain_id = struc_prov[0].get_chain_id()
        self.offset = struc_prov[0].get_residue_offset()

    # Two starting models with same filename, chain ID, and offset
    # compare identical
    # note: this results in separate starting models if only the offset differs;
    # maybe consolidate into one?
    def _eq_vals(self):
        return tuple([self.__class__]
                     + [getattr(self, x) for x in self._eq_keys])
    def __eq__(self, other):
        return other is not None and self._eq_vals() == other._eq_vals()
    def __hash__(self):
        return hash(self._eq_vals())

    def _set_sources_datasets(self, system):
        # Attempt to identify PDB file vs. comparative model
        p = IMP.mmcif.metadata._PDBMetadataParser()
        p.parse_file(self.filename, self.chain_id, system)
        self.dataset = p.dataset
        self.sources = p.sources
        self.alignment_file = p.alignment_file


class _StartingModelFinder(object):
    """Map IMP particles to starting model objects"""
    def __init__(self, existing_starting_models):
        self._seen_particles = {}
        self._seen_starting_models = dict.fromkeys(existing_starting_models)

    def find(self, particle, system):
        """Return a StartingModel object, or None, for this particle"""
        def _get_starting_model(sp):
            s = _StartingModel(sp)
            if s not in self._seen_starting_models:
                self._seen_starting_models[s] = s
                s._set_sources_datasets(system)
            return self._seen_starting_models[s]
        sp = list(_get_all_structure_provenance(particle))
        if sp:
            return _get_starting_model(sp)
        elif IMP.atom.Hierarchy.get_is_setup(particle):
            h = IMP.atom.Hierarchy(particle).get_parent()
            # Remember all nodes we inspect
            seen_parents = []
            while h:
                pi = h.get_particle_index()
                seen_parents.append(pi)
                # If we inspected this node before, return the cached result
                if pi in self._seen_particles:
                    sp = self._seen_particles[pi]
                    return sp[0] if sp else None
                else:
                    sp = list(_get_all_structure_provenance(h))
                    self._seen_particles[pi] = []
                    if sp:
                        s = _get_starting_model(sp)
                        # Set cache for this node and all the children we
                        # inspected on the way up
                        for spi in seen_parents:
                            self._seen_particles[spi].append(s)
                        return s
                h = h.get_parent()


class _Datasets(object):
    """Store all datasets used."""
    def __init__(self):
        super(_Datasets, self).__init__()
        self._datasets = {}

    def add(self, d):
        """Add and return a new dataset."""
        if d not in self._datasets:
            self._datasets[d] = d
            d.id = len(self._datasets)
        return self._datasets[d]

    def get_all(self):
        """Yield all datasets"""
        return self._datasets.keys()


class _Citation(object):
    """A publication that describes the modeling."""
    def __init__(self, pmid, title, journal, volume, page_range, year, authors,
                 doi):
        self.title, self.journal, self.volume = title, journal, volume
        self.page_range, self.year = page_range, year
        self.pmid, self.authors, self.doi = pmid, authors, doi


class _Software(object):
    """Software (other than IMP) used as part of the modeling protocol."""
    def __init__(self, name, classification, description, url,
                 type='program', version=None):
        self.name = name
        self.classification = classification
        self.description = description
        self.url = url
        self.type = type
        self.version = version


class _AllSoftware(list):
    """Keep track of all _Software objects."""
    def __init__(self):
        super(_AllSoftware, self).__init__()
        self.modeller_used = self.phyre2_used = False

    def set_modeller_used(self, version, date):
        if self.modeller_used:
            return
        self.modeller_used = True
        self.append(_Software(
                name='MODELLER', classification='comparative modeling',
                description='Comparative modeling by satisfaction '
                            'of spatial restraints, build ' + date,
                url='https://salilab.org/modeller/',
                version=version))

    def set_phyre2_used(self):
        if self.phyre2_used:
            return
        self.phyre2_used = True
        self.append(_Software(
               name='Phyre2', classification='protein homology modeling',
               description='Protein Homology/analogY Recognition '
                           'Engine V 2.0',
               version='2.0', url='http://www.sbg.bio.ic.ac.uk/~phyre2/'))
