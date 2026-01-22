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


def urlopen(url):
    if 'invalid' in url or 'INVALID' in url:
        raise urllib.error.HTTPError("404")
    return BytesIO(zinc_atoms if url.endswith('ZN.cif') else mg)
