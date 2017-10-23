import IMP.mmcif.dumper
import IMP.mmcif.format

class Writer(object):
    def __init__(self):
        self._dumpers = [IMP.mmcif.dumper._EntryDumper(), # must be first
                         IMP.mmcif.dumper._EntityDumper(),
                         IMP.mmcif.dumper._EntityPolyDumper(),
                         IMP.mmcif.dumper._EntityPolySeqDumper(),
                         IMP.mmcif.dumper._StructAsymDumper()]
        self.entities = _EntityMapper()
        self.chains = []

    def add_hierarchy(self, h):
        chains = [IMP.atom.Chain(c)
                  for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)]
        # todo: handle chains with duplicated IDs, same chain in multiple states
        for c in chains:
            self.entities.add(c)
            self.chains.append(c)

    def write(self, fname):
        with open(fname, 'w') as fh:
            writer = IMP.mmcif.format._CifWriter(fh)
            for dumper in self._dumpers:
                dumper.dump(self, writer)

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
            mol = IMP.mmcif.dumper.get_molecule(chain)
            entity = _Entity(sequence, chain, mol.get_name() if mol else None)
            self._entities.append(entity)
            entity.id = len(self._entities)
            self._sequence_dict[sequence] = entity
        self[chain] = self._sequence_dict[sequence]

    def get_all(self):
        """Yield all entities"""
        return self._entities
