"""@namespace IMP.mmcif.dumper
   @brief Utility classes to dump out information in mmCIF format.
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

class _Dumper(object):
    """Base class for helpers to dump output to mmCIF"""
    def __init__(self):
        pass

class _EntryDumper(_Dumper):
    def dump(self, cifdata, writer):
        entry_id = 'imp_model'
        # Write CIF header (so this dumper should always be first)
        writer.fh.write("data_%s\n" % entry_id)
        with writer.category("_entry") as l:
            l.write(id=entry_id)


class _EntityDumper(_Dumper):
    # todo: we currently only support amino acid sequences here (and
    # then only standard amino acids; need to add support for MSE etc.)
    def dump(self, cifdata, writer):
        with writer.loop("_entity",
                         ["id", "type", "src_method", "pdbx_description",
                          "formula_weight", "pdbx_number_of_molecules",
                          "details"]) as l:
            for entity in cifdata.entities.get_all():
                l.write(id=entity.id, type='polymer', src_method='man',
                        pdbx_description=entity.description,
                        formula_weight=writer.unknown,
                        pdbx_number_of_molecules=1, details=writer.unknown)


class _EntityPolyDumper(_Dumper):
    # todo: we currently only support amino acid sequences here
    def dump(self, cifdata, writer):
        with writer.loop("_entity_poly",
                         ["entity_id", "type", "nstd_linkage",
                          "nstd_monomer", "pdbx_strand_id",
                          "pdbx_seq_one_letter_code",
                          "pdbx_seq_one_letter_code_can"]) as l:
            for entity in cifdata.entities.get_all():
                seq = entity.sequence
                # Split into lines to get tidier CIF output
                seq = "\n".join(seq[i:i+70] for i in range(0, len(seq), 70))
                chain = entity.first_chain
                l.write(entity_id=entity.id, type='polypeptide(L)',
                        nstd_linkage='no', nstd_monomer='no',
                        pdbx_strand_id=chain.get_id(),
                        pdbx_seq_one_letter_code=seq,
                        pdbx_seq_one_letter_code_can=seq)

class _EntityPolySeqDumper(_Dumper):
    def dump(self, cifdata, writer):
        with writer.loop("_entity_poly_seq",
                         ["entity_id", "num", "mon_id", "hetero"]) as l:
            for entity in cifdata.entities.get_all():
                seq = entity.sequence
                for num, one_letter_code in enumerate(seq):
                    restyp = IMP.atom.get_residue_type(one_letter_code)
                    l.write(entity_id=entity.id, num=num + 1,
                            mon_id=restyp.get_string(),
                            hetero=writer.omitted)


class _StructAsymDumper(_Dumper):
    def dump(self, cifdata, writer):
        with writer.loop("_struct_asym",
                         ["id", "entity_id", "details"]) as l:
            for chain in cifdata.chains:
                molecule = get_molecule(chain)
                entity = cifdata.entities[chain]
                l.write(id=chain.get_id(),
                        entity_id=entity.id,
                        details=molecule.get_name() if molecule
                                                    else writer.omitted)
