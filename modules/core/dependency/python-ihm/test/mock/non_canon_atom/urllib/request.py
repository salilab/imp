from io import BytesIO
import urllib.error

zinc_atoms = b'_chem_comp_atom.comp_id ZN\n_chem_comp_atom.atom_id ZN\n'

mg = b"""
_chem_comp.id                                   MG
_chem_comp.name                                 "MAGNESIUM ION"
_chem_comp.type                                 NON-POLYMER
_chem_comp.pdbx_type                            HETAI
_chem_comp.formula                              Mg
"""

uniprot_fasta = b""">tr|Q90VU7|Q90VU7_HV1 Protein Nef
MGGKWSKSSVIGWPAVRERMRRAEPAADGVGAVSRDLEKHGAITSSNTAANNAACAWLEA
QEEEEVGFPVTPQVPLRPMTYKAAVDLSHFLKEKGGLEGLIHSQRRQDILDLWIYHTQGY
FPDWQNYTPGPGVRYPLTFGWCYKLVPVEPDKVEEANKGENTSLLHPVSLHGMDDPEREV
LEWRFDSRLAFHHVARELHPEYFKNC
"""


def urlopen(url):
    if 'uniprot.org' in url:
        return BytesIO(uniprot_fasta)
    elif 'invalid' in url or 'INVALID' in url:
        raise urllib.error.HTTPError("404")
    return BytesIO(zinc_atoms if url.endswith('ZN.cif') else mg)
