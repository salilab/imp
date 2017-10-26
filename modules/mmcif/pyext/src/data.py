"""@namespace IMP.mmcif.data
   @brief Classes to represent data structures used in mmCIF.
"""

from __future__ import print_function
import IMP.atom

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

    def add(self, particle):
        """Potentially add a new particle to this representation.
           Iff the particle could be added, return True."""
        resrange, rigid_body, primitive = self._get_particle_info(particle)
        if not self.particles:
            self.particles.append(particle)
            self.residue_range = resrange
            self.rigid_body = rigid_body
            self.primitive = primitive
            return True
        elif type(particle) == type(self.particles[0]) \
             and resrange[0] == self.residue_range[1] + 1 \
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
