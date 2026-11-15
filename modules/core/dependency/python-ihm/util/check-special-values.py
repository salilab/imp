#!/usr/bin/python3

"""Check that python-ihm handles the mmCIF special values '?' and '.'.

   In mmCIF, any non-mandatory data item can be given as '?' (unknown) or
   '.' (omitted). This script reads the IHMCIF dictionary and, for every
   non-mandatory data item it defines, builds a small mmCIF file containing
   the item's category with that item set to '?' (and then '.'), plus
   dummy values for the category's mandatory items. It checks that
   ihm.reader can read each file, and that the resulting ihm.System can be
   written out again as mmCIF by ihm.dumper.

   Any failures (exceptions) are reported, and the script exits with a
   nonzero status if there were any.

   This should be periodically rerun in case the dictionary is updated.
"""

import argparse
import io
import sys
import traceback
import urllib.request
import warnings

import ihm.dictionary
import ihm.dumper
import ihm.reader

IHM_DIC = 'http://mmcif.wwpdb.org/dictionaries/ascii/mmcif_ihm.dic'

# Some categories in the IHM dictionary only extend categories from the core
# PDBx dictionary (e.g. atom_site.ihm_model_id), so their mandatory items
# are not listed. Provide some reasonable values for those items here.
EXTRA_MANDATORY = {
    'atom_site': {
        'group_PDB': 'ATOM', 'type_symbol': 'C', 'label_atom_id': 'CA',
        'label_alt_id': '.', 'label_comp_id': 'ALA', 'label_seq_id': '1',
        'auth_seq_id': '1', 'pdbx_PDB_ins_code': '?', 'label_asym_id': 'A',
        'Cartn_x': '1.0', 'Cartn_y': '2.0', 'Cartn_z': '3.0',
        'occupancy': '1.0', 'label_entity_id': '1', 'auth_asym_id': 'A',
        'auth_comp_id': 'ALA', 'B_iso_or_equiv': '1.0', 'id': '1'},
    'pdbx_poly_seq_scheme': {
        'asym_id': 'A', 'entity_id': '1', 'seq_id': '1', 'mon_id': 'ALA',
        'pdb_seq_num': '1', 'auth_seq_num': '1', 'pdb_mon_id': 'ALA',
        'auth_mon_id': 'ALA', 'pdb_strand_id': 'A', 'pdb_ins_code': '.'},
}


# Basic data added to every test file, so that dummy IDs (all '1', or 'A'
# for asym IDs) reference something that exists (a single-residue polymer)
BASE_DATA = {
    'entity': {'id': '1', 'type': 'polymer'},
    'entity_poly': {'entity_id': '1', 'type': 'polypeptide(L)'},
    'entity_poly_seq': {'entity_id': '1', 'num': '1', 'mon_id': 'ALA',
                        'hetero': '.'},
    'struct_asym': {'id': 'A', 'entity_id': '1'},
}


def read_dictionary(fname):
    """Read the dictionary from a local file or URL"""
    if '://' in fname:
        with urllib.request.urlopen(fname) as fh:
            return ihm.dictionary.read(io.TextIOWrapper(fh, 'utf-8'))
    else:
        with open(fname, encoding='utf-8') as fh:
            return ihm.dictionary.read(fh)


def get_dummy_value(keyword):
    """Get a value for a mandatory keyword that should pass validation"""
    if keyword.enumeration:
        # Use a predictable (sorted) choice from the enumeration
        return sorted(keyword.enumeration)[0]
    if keyword.name.endswith('asym_id'):
        return 'A'
    if keyword.name.endswith('comp_id'):
        return 'ALA'
    # A single digit is a valid int, float, code, line, text, or ucode
    return '1'


def quote(value):
    """Quote a value for mmCIF output, if needed"""
    if value in ('?', '.'):
        return value
    if ' ' in value or value[0] in '_#$\'"[];':
        return "'%s'" % value
    return value


def make_cif(category, keyword, value):
    """Make an mmCIF file containing the given category, with the given
       keyword set to the given value (e.g. '?' or '.')"""
    values = dict(BASE_DATA.get(category.name, {}))
    for kw in category.keywords.values():
        if kw.mandatory:
            values[kw.name] = get_dummy_value(kw)
    for name, value in EXTRA_MANDATORY.get(category.name, {}).items():
        values.setdefault(name, value)
    values[keyword.name] = value
    lines = ['data_model']
    for catname, catvalues in BASE_DATA.items():
        if catname != category.name:
            lines.extend('_%s.%s %s' % (catname, name, quote(value))
                         for name, value in catvalues.items())
    lines.extend('_%s.%s %s' % (category.name, name, quote(value))
                 for name, value in values.items())
    return '\n'.join(lines) + '\n'


def check_item(category, keyword, value, verbose):
    """Check that a single item set to the given value can be read and
       written. Return an error message on failure, or None on success."""
    cif = make_cif(category, keyword, value)
    stage = 'reading'
    try:
        with warnings.catch_warnings():
            warnings.simplefilter('ignore')
            systems = ihm.reader.read(io.StringIO(cif))
            stage = 'writing'
            # Don't check the output for sanity, since our dummy systems
            # are not complete
            ihm.dumper.write(io.StringIO(), systems, check=False)
    except Exception:
        msg = "%s failed for _%s.%s = %s" % (stage, category.name,
                                             keyword.name, value)
        if verbose:
            msg += "\nInput file:\n" + cif + traceback.format_exc()
        else:
            msg += ": " + traceback.format_exc().rstrip().split('\n')[-1]
        return msg


def main():
    parser = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    parser.add_argument('--dictionary', default=IHM_DIC,
                        help="IHMCIF dictionary file or URL (default: %s)"
                        % IHM_DIC)
    parser.add_argument('-v', '--verbose', action='store_true',
                        help="Show input file and full traceback for "
                             "each failure, and list untestable items")
    args = parser.parse_args()

    d = read_dictionary(args.dictionary)
    errors = []
    untestable = []
    nchecked = 0
    for catname in sorted(d.categories):
        category = d.categories[catname]
        for kwname in sorted(category.keywords):
            keyword = category.keywords[kwname]
            if keyword.mandatory:
                continue
            # First check that the file can be read and written with the
            # item set to a regular value. If not, our dummy data is
            # inadequate for this category, so a failure with a special
            # value would not tell us anything.
            err = check_item(category, keyword, get_dummy_value(keyword),
                             args.verbose)
            if err:
                untestable.append(err)
                if args.verbose:
                    print("Cannot test: " + err)
                continue
            for special in ('?', '.'):
                nchecked += 1
                err = check_item(category, keyword, special, args.verbose)
                if err:
                    errors.append(err)
                    print(err)
    print("%d checks of %s, %d failed; %d items could not be tested since "
          "the dummy data was inadequate%s"
          % (nchecked, args.dictionary, len(errors), len(untestable),
             '' if args.verbose else ' (use -v to list them)'))
    return 1 if errors else 0


if __name__ == '__main__':
    sys.exit(main())
