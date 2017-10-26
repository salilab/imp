"""@namespace IMP.mmcif.dumper
   @brief Utility classes to dump out information in mmCIF format.
"""

from __future__ import print_function
import IMP.atom
import IMP.mmcif.data

class _Dumper(object):
    """Base class for helpers to dump output to mmCIF"""
    def __init__(self):
        pass
    def finalize_metadata(self):
        pass
    def finalize(self):
        pass

class _EntryDumper(_Dumper):
    def dump(self, system, writer):
        entry_id = 'imp_model'
        # Write CIF header (so this dumper should always be first)
        writer.fh.write("data_%s\n" % entry_id)
        with writer.category("_entry") as l:
            l.write(id=entry_id)

class _ChemCompDumper(_Dumper):
    def dump(self, system, writer):
        seen = {}
        std = dict.fromkeys(('ALA', 'CYS', 'ASP', 'GLU', 'PHE', 'GLY', 'HIS',
               'ILE', 'LYS', 'LEU', 'MET', 'ASN', 'PRO', 'GLN', 'ARG', 'SER',
               'THR', 'VAL', 'TRP', 'TYR'))
        with writer.loop("_chem_comp", ["id", "type"]) as l:
            for entity in system.entities.get_all():
                seq = entity.sequence
                for num, one_letter_code in enumerate(seq):
                    restyp = IMP.atom.get_residue_type(one_letter_code)
                    resid = restyp.get_string()
                    if resid not in seen:
                        seen[resid] = None
                        l.write(id=resid,
                                type='L-peptide linking' if resid in std \
                                                         else 'other')


class _EntityDumper(_Dumper):
    # todo: we currently only support amino acid sequences here (and
    # then only standard amino acids; need to add support for MSE etc.)
    def dump(self, system, writer):
        with writer.loop("_entity",
                         ["id", "type", "src_method", "pdbx_description",
                          "formula_weight", "pdbx_number_of_molecules",
                          "details"]) as l:
            for entity in system.entities.get_all():
                l.write(id=entity.id, type='polymer', src_method='man',
                        pdbx_description=entity.description,
                        formula_weight=writer.unknown,
                        pdbx_number_of_molecules=1, details=writer.unknown)


class _EntityPolyDumper(_Dumper):
    # todo: we currently only support amino acid sequences here
    def dump(self, system, writer):
        with writer.loop("_entity_poly",
                         ["entity_id", "type", "nstd_linkage",
                          "nstd_monomer", "pdbx_strand_id",
                          "pdbx_seq_one_letter_code",
                          "pdbx_seq_one_letter_code_can"]) as l:
            for entity in system.entities.get_all():
                seq = entity.sequence
                # Split into lines to get tidier CIF output
                seq = "\n".join(seq[i:i+70] for i in range(0, len(seq), 70))
                comp = entity.first_component
                l.write(entity_id=entity.id, type='polypeptide(L)',
                        nstd_linkage='no', nstd_monomer='no',
                        pdbx_strand_id=comp.asym_id,
                        pdbx_seq_one_letter_code=seq,
                        pdbx_seq_one_letter_code_can=seq)

class _EntityPolySeqDumper(_Dumper):
    def dump(self, system, writer):
        with writer.loop("_entity_poly_seq",
                         ["entity_id", "num", "mon_id", "hetero"]) as l:
            for entity in system.entities.get_all():
                seq = entity.sequence
                for num, one_letter_code in enumerate(seq):
                    restyp = IMP.atom.get_residue_type(one_letter_code)
                    l.write(entity_id=entity.id, num=num + 1,
                            mon_id=restyp.get_string(),
                            hetero=writer.omitted)


class _StructAsymDumper(_Dumper):
    def dump(self, system, writer):
        with writer.loop("_struct_asym",
                         ["id", "entity_id", "details"]) as l:
            for comp in system.components.get_all_modeled():
                l.write(id=comp.asym_id,
                        entity_id=comp.entity.id,
                        details=comp.name if comp.name else writer.omitted)


class _AssemblyDumper(_Dumper):
    def __init__(self):
        super(_AssemblyDumper, self).__init__()
        self.assemblies = []

    def add(self, a):
        """Add a new assembly. The first such assembly is assumed to contain
           all components. Duplicate assemblies will be pruned at the end."""
        self.assemblies.append(a)
        return a

    def get_subassembly(self, compdict):
        """Get an _Assembly consisting of the given components."""
        # Put components in creation order
        newa = IMP.mmcif.data._Assembly(c for c in self.assemblies[0]
                                        if c in compdict)
        return self.add(newa)

    def finalize(self):
        seen_assemblies = {}
        # Assign IDs to all assemblies
        self._assembly_by_id = []
        for a in self.assemblies:
            IMP.mmcif.data._assign_id(a, seen_assemblies, self._assembly_by_id)

    def dump(self, system, writer):
        ordinal = 1
        with writer.loop("_ihm_struct_assembly",
                         ["ordinal_id", "assembly_id", "parent_assembly_id",
                          "entity_description",
                          "entity_id", "asym_id", "seq_id_begin",
                          "seq_id_end"]) as l:
            for a in self._assembly_by_id:
                for comp in a:
                    entity = comp.entity
                    l.write(ordinal_id=ordinal, assembly_id=a.id,
                            # Currently all assemblies are not hierarchical,
                            # so each assembly is a self-parent
                            parent_assembly_id=a.id,
                            entity_description=entity.description,
                            entity_id=entity.id,
                            asym_id=comp.asym_id if comp.asym_id
                                                 else writer.omitted,
                            seq_id_begin=1,
                            seq_id_end=len(entity.sequence))
                    ordinal += 1
