# This example demonstrates the use of the Python IHM library at a very
# low level, to perform housekeeping tasks on an mmCIF file without
# making large changes to its structure, and preserving whitespace,
# case, and comments.

# Note that unlike higher-level interfaces, the tokenizer can generate
# invalid mmCIF if used incorrectly. It is recommended that the resulting
# mmCIF files are run through a validator, as in the `validate_pdb_dev.py`
# example.

import ihm.format

filters = [
    # Change chain ID 'B' to 'Z' by altering the _struct_asym table
    ihm.format.ChangeValueFilter('_struct_asym.id', old='B', new='Z'),
    # Note that the tokenizer does not parse parent-child relationships
    # or understand the underlying dictionary. So we must also change other
    # tables that reference chain IDs. Here we change the label_asym_id keyword
    # in *any* table (typically in _atom_site).
    ihm.format.ChangeValueFilter('.label_asym_id', old='B', new='Z'),
    # Remove the non-standard _modeller.version data item from the file
    ihm.format.RemoveItemFilter('_modeller.version')]

# Read the input file as a set of tokens, modify them using the filters
# above, and write a new file:
with open('mini.cif') as fh_in:
    r = ihm.format.CifTokenReader(fh_in)
    with open('output.cif', 'w') as fh_out:
        for token in r.read_file(filters):
            fh_out.write(token.as_mmcif())
