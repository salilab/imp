import IMP.mmcif.dumper
import IMP.mmcif.data
import IMP.mmcif.format
import IMP.rmf
import IMP.atom
import RMF
import string
import weakref

class _NonModeledChain(object):
    """Represent a chain that was experimentally characterized but not modeled.
       Such a chain resembles an IMP.atom.Chain, but has no associated
       structure, and belongs to no state."""
    def __init__(self, name, sequence, chain_type):
        self.name = name
        self.sequence = sequence
        self.chain_type = chain_type

    get_sequence = lambda self: self.sequence


class Writer(object):
    def __init__(self):
        self._states = {}
        self.assembly_dump = IMP.mmcif.dumper._AssemblyDumper()
        # The assembly of all known components.
        self.complete_assembly = IMP.mmcif.data._Assembly()
        self.assembly_dump.add(self.complete_assembly)

        self._dumpers = [IMP.mmcif.dumper._EntryDumper(), # must be first
                         IMP.mmcif.dumper._ChemCompDumper(),
                         IMP.mmcif.dumper._EntityDumper(),
                         IMP.mmcif.dumper._EntityPolyDumper(),
                         IMP.mmcif.dumper._EntityPolySeqDumper(),
                         IMP.mmcif.dumper._StructAsymDumper(),
                         self.assembly_dump]
        self.entities = IMP.mmcif.data._EntityMapper()
        self.components = IMP.mmcif.data._ComponentMapper()

    def _add_state(self, state):
        self._states[state] = None
        state.id = len(self._states)

    def _add_hierarchy(self, h, state):
        chains = [IMP.atom.Chain(c)
                  for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)]
        self._remove_duplicate_chain_ids(chains)
        # todo: handle same chain in multiple states
        for c in chains:
            component = self._add_chain(c)
            state._all_modeled_components.append(component)
            state.modeled_assembly.append(component)

    def add_non_modeled_chain(self, name, sequence,
                              chain_type=IMP.atom.UnknownChainType):
        """Add a chain that wasn't modeled by IMP."""
        c = _NonModeledChain(name, sequence, chain_type)
        self._add_chain(c)

    def _add_chain(self, c):
        entity = self.entities.add(c)
        component = self.components.add(c, entity)
        if component not in self.complete_assembly:
            self.complete_assembly.append(component)
        return component

    def _remove_duplicate_chain_ids(self, chains):
        chain_ids = [c.get_id() for c in chains]
        if len(set(chain_ids)) < len(chain_ids):
            print("Duplicate chain IDs detected - reassigning as A-Z, a-z, 0-9")
            # todo: handle > 62 chains
            for chain, cid in zip(chains, string.uppercase
                                          + string.lowercase + string.digits):
                chain.set_id(cid)

    def write(self, fname):
        with open(fname, 'w') as fh:
            writer = IMP.mmcif.format._CifWriter(fh)
            for dumper in self._dumpers:
                dumper.finalize_metadata()
            for dumper in self._dumpers:
                dumper.finalize()
            for dumper in self._dumpers:
                dumper.dump(self, writer)

class State(object):
    """Represent a single IMP state."""
    def __init__(self, writer):
        self.writer = weakref.proxy(writer)
        writer._add_state(self)
        self.model = IMP.Model()
        self.hiers = None
        # The assembly of all components modeled by IMP in this state.
        # This may be smaller than the complete assembly.
        self.modeled_assembly = IMP.mmcif.data._Assembly()
        writer.assembly_dump.add(self.modeled_assembly)

        self._all_modeled_components = []

    def add_rmf_files(self, rmfs):
        for fname in rmfs:
            rmf = RMF.open_rmf_file_read_only(fname)
            if self.hiers is None:
                self.hiers = IMP.rmf.create_hierarchies(rmf, self.model)
            else:
                IMP.rmf.link_hierarchies(rmf, self.hiers)
                # todo: allow reading other frames
                IMP.rmf.load_frame(rmf, RMF.FrameID(0))
            for h in self.hiers:
                self.add_hierarchy(h)

    def add_hierarchy(self, h):
        self.writer._add_hierarchy(h, self)
