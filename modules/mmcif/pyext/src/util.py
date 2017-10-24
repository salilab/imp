import IMP.mmcif.dumper
import IMP.mmcif.data
import IMP.mmcif.format
import string

class Writer(object):
    def __init__(self):
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

    def add_hierarchy(self, h):
        chains = [IMP.atom.Chain(c)
                  for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)]
        self._remove_duplicate_chain_ids(chains)
        # todo: handle same chain in multiple states
        for c in chains:
            entity = self.entities.add(c)
            component = self.components.add(c, entity)
            if component not in self.complete_assembly:
                self.complete_assembly.append(component)

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
