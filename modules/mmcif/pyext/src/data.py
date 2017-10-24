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
    """Represent a CIF entity (a chain with a unique sequence)"""
    def __init__(self, seq, first_chain, description):
        self.sequence = seq
        self.first_chain = first_chain
        self.description = description


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
            mol = get_molecule(chain)
            entity = _Entity(sequence, chain, mol.get_name() if mol else None)
            self._entities.append(entity)
            entity.id = len(self._entities)
            self._sequence_dict[sequence] = entity
        self[chain] = self._sequence_dict[sequence]

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


class _Assembly(list):
    """A collection of components. Currently simply implemented as a list of
       the chain objects. These must be in creation order."""
    def __hash__(self):
        # allow putting assemblies in a dict. 'list' isn't hashable
        # but 'tuple' is
        return hash(tuple(self))
