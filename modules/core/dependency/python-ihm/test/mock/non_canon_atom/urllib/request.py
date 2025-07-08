from io import BytesIO
import urllib.error

zinc_atoms = b'_chem_comp_atom.comp_id ZN\n_chem_comp_atom.atom_id ZN\n'


def urlopen(url):
    if 'invalid' in url:
        raise urllib.error.HTTPError("404")
    return BytesIO(zinc_atoms)
