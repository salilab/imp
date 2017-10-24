import IMP.mmcif.dumper
import IMP.mmcif.data
import IMP.mmcif.format

class Writer(object):
    def __init__(self):
        self.assembly_dump = IMP.mmcif.dumper._AssemblyDumper()
        # The assembly of all known components.
        self.complete_assembly = IMP.mmcif.data._Assembly()
        self.assembly_dump.add(self.complete_assembly)

        self._dumpers = [IMP.mmcif.dumper._EntryDumper(), # must be first
                         IMP.mmcif.dumper._EntityDumper(),
                         IMP.mmcif.dumper._EntityPolyDumper(),
                         IMP.mmcif.dumper._EntityPolySeqDumper(),
                         IMP.mmcif.dumper._StructAsymDumper(),
                         self.assembly_dump]
        self.entities = IMP.mmcif.data._EntityMapper()
        self.chains = []

    def add_hierarchy(self, h):
        chains = [IMP.atom.Chain(c)
                  for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)]
        # todo: handle chains with duplicated IDs, same chain in multiple states
        for c in chains:
            self.entities.add(c)
            self.chains.append(c)
            self.complete_assembly.append(c)

    def write(self, fname):
        with open(fname, 'w') as fh:
            writer = IMP.mmcif.format._CifWriter(fh)
            for dumper in self._dumpers:
                dumper.finalize_metadata()
            for dumper in self._dumpers:
                dumper.finalize()
            for dumper in self._dumpers:
                dumper.dump(self, writer)
