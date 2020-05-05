"""@namespace IMP.pmi.alphabets
   Mapping between FASTA one-letter codes and residue types.
"""

from __future__ import print_function
import IMP.atom


class ResidueAlphabet(object):
    """Map between FASTA codes and residue types.
       Typically one would use the `amino_acid`, `rna`, or `dna` objects."""
    def __init__(self, charmm_to_one):
        self._one_to_charmm = {}
        for k, v in charmm_to_one.items():
            self._one_to_charmm[v] = k
        self.charmm_to_one = charmm_to_one

    def get_residue_type_from_one_letter_code(self, code):
        """Given a one-letter code, return an IMP.atom.ResidueType"""
        return IMP.atom.ResidueType(self._one_to_charmm[code])

    def get_one_letter_code_from_residue_type(self, rt):
        """Given a residue type, return a one-letter code"""
        # Technically this is the CHARMM type, not PDB (but they are the
        # same for amino acids)
        return self.charmm_to_one.get(rt, 'X')


"""Mapping between FASTA one-letter codes and residue types for amino acids"""
amino_acid = ResidueAlphabet(
    {'ALA': 'A', 'ARG': 'R', 'ASN': 'N', 'ASP': 'D',
     'CYS': 'C', 'GLU': 'E', 'GLN': 'Q', 'GLY': 'G',
     'HIS': 'H', 'ILE': 'I', 'LEU': 'L', 'LYS': 'K',
     'MET': 'M', 'PHE': 'F', 'PRO': 'P', 'SER': 'S',
     'THR': 'T', 'TRP': 'W', 'TYR': 'Y', 'VAL': 'V',
     'UNK': 'X'})


"""Mapping between FASTA one-letter codes and residue types for DNA"""
dna = ResidueAlphabet(
    {'DADE': 'A', 'DURA': 'U', 'DCYT': 'C', 'DGUA': 'G',
     'DTHY': 'T', 'UNK': 'X'})


"""Mapping between FASTA one-letter codes and residue types for RNA"""
rna = ResidueAlphabet(
    {'ADE': 'A', 'URA': 'U', 'CYT': 'C', 'GUA': 'G',
     'THY': 'T', 'UNK': 'X'})
