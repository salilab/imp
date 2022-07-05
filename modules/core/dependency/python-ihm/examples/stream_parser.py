# This example demonstrates the use of the Python IHM library at a low
# level, to parse an mmCIF file and extract a subset of its data.
# This particular example just extracts the atomic coordinates.

import ihm.format


# Make an object to handle a given mmCIF category in the file; it will
# be called for each line in the loop construct.
class AtomSiteHandler(object):
    # If a given keyword is not in the file, or has the special
    # mmCIF omitted (.) or unknown (?) value, the corresponding argument
    # to __call__ will be given these values:
    not_in_file = omitted = None
    unknown = ihm.unknown

    # Extract the group_PDB, Cartn_x, Cartn_y, Cartn_z keywords from
    # the mmCIF category (mmCIF keywords are case-insensitive, but the
    # Python arguments here should be lowercase).
    def __call__(self, group_pdb, cartn_x, cartn_y, cartn_z):
        if group_pdb == 'ATOM':
            print("Atom at %s, %s, %s" % (cartn_x, cartn_y, cartn_z))


ash = AtomSiteHandler()
with open('mini.cif') as fh:
    # Extract keywords from the _atom_site mmCIF category using the
    # AtomSiteHandler defined above
    c = ihm.format.CifReader(fh, category_handler={'_atom_site': ash})

    # Read the first data block in mini.cif
    # (This will return True as long as there are more blocks, so it can
    # be put in a while loop instead if you want to read all data blocks.)
    c.read_file()
