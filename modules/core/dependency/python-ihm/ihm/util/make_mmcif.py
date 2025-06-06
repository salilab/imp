#!/usr/bin/env python3

"""
Add minimal IHM-related tables to an mmCIF file.

Given any mmCIF file as input, this script will add any missing
IHM-related tables and write out a new file that is minimally compliant
with the IHM dictionary.

This is done by simply reading in the original file with python-ihm and
then writing it out again, so
  a) any data in the input file that is not understood by python-ihm
     will be lost on output; and
  b) input files that aren't compliant with the PDBx dictionary, or that
     contain syntax errors or other problems, may crash or otherwise confuse
     python-ihm.

The --add option can also be used to combine multiple input mmCIF files into
one. This is typically used when the mmCIF files contain models with
differing composition. Only model (coordinate) information is combined, not
other IHM information such as starting models or restraints.
"""


import ihm.reader
import ihm.dumper
import ihm.model
import ihm.protocol
import ihm.util
import ihm.format
import urllib.request
import os
import argparse
import collections
import operator
import warnings


# All canonical atom names for each standard residue type, as per CCD.
# This is generated using the util/get_ccd_atoms.py script.
KNOWN_ATOM_NAMES = {
    'A': {"C4'", "C2'", 'C2', "C1'", 'N7', 'H62', 'OP2', 'N3', 'C5', 'P',
          "H5''", 'H2', "C5'", 'H61', "H3'", 'C4', 'N1', 'H8', "H1'", 'C8',
          'N9', 'HOP3', 'OP1', "O4'", "H2'", "HO2'", 'OP3', "O3'", 'N6',
          'HOP2', "O5'", "O2'", "HO3'", "H5'", "C3'", 'C6', "H4'"},
    'ALA': {'H2', 'HB1', 'HB3', 'HB2', 'N', 'HXT', 'O', 'CB', 'C', 'HA', 'CA',
            'H', 'OXT'},
    'ARG': {'HB2', 'CG', 'NE', 'H', 'H2', 'HH22', 'N', 'HG2', 'CA', 'NH2',
            'HH11', 'HG3', 'HH21', 'CZ', 'HB3', 'HXT', 'O', 'C', 'HD3', 'HH12',
            'CB', 'NH1', 'CD', 'HA', 'HD2', 'HE', 'OXT'},
    'ASN': {'H2', 'HB3', 'HD22', 'HB2', 'N', 'CG', 'O', 'CB', 'ND2', 'HXT',
            'C', 'HA', 'HD21', 'CA', 'OD1', 'H', 'OXT'},
    'ASP': {'H2', 'HB3', 'HB2', 'N', 'CG', 'O', 'CB', 'HXT', 'C', 'HA', 'OD2',
            'CA', 'OD1', 'HD2', 'H', 'OXT'},
    'C': {"C4'", "C2'", 'C2', 'O2', 'H42', 'H5', "C1'", 'OP2', 'N3', 'C5',
          'P', "H5''", 'H41', 'H6', "C5'", "H3'", 'C4', 'N1', 'N4', "H1'",
          'HOP3', 'OP1', "O4'", "H2'", "HO2'", 'OP3', "O3'", 'HOP2', "O5'",
          "O2'", "HO3'", "H5'", "C3'", 'C6', "H4'"},
    'CYS': {'H2', 'HB3', 'HB2', 'N', 'SG', 'O', 'CB', 'HXT', 'C', 'HA', 'HG',
            'CA', 'H', 'OXT'},
    'DA': {"C4'", "C2'", 'C2', "C1'", 'N7', 'H62', 'OP2', 'N3', 'C5', 'P',
           "H5''", 'H2', "C5'", 'H61', "H3'", 'C4', 'N1', 'H8', "H1'", 'C8',
           'N9', 'HOP3', 'OP1', "O4'", "H2'", 'OP3', "O3'", 'N6', 'HOP2',
           "O5'", "H2''", "HO3'", "H5'", "C3'", 'C6', "H4'"},
    'DC': {"C4'", "C2'", 'C2', 'O2', 'H42', 'H5', "C1'", 'OP2', 'N3', 'C5',
           'P', "H5''", 'H41', 'H6', "C5'", "H3'", 'C4', 'N1', 'N4', "H1'",
           'HOP3', 'OP1', "O4'", "H2'", 'OP3', "O3'", 'HOP2', "O5'", "H2''",
           "HO3'", "H5'", "C3'", 'C6', "H4'"},
    'DG': {"C4'", "C2'", 'C2', "C1'", 'N7', 'OP2', 'N3', 'C5', 'P', "H5''",
           "C5'", 'O6', 'H1', "H3'", 'C4', 'N1', 'H8', "H1'", 'C8', 'N9',
           'HOP3', 'OP1', "O4'", "H2'", 'OP3', "O3'", 'HOP2', "O5'", "H2''",
           'H21', 'H22', "HO3'", "H5'", "C3'", 'N2', 'C6', "H4'"},
    'DT': {"C4'", "C2'", 'C2', 'O2', 'O4', "C1'", 'OP2', 'N3', 'C5', 'P',
           "H5''", 'H6', "C5'", "H3'", 'C4', 'N1', 'C7', "H1'", 'H73', 'HOP3',
           'H3', 'OP1', "O4'", "H2'", 'OP3', "O3'", 'HOP2', "O5'", "H2''",
           'H71', "HO3'", "H5'", "C3'", 'H72', 'C6', "H4'"},
    'G': {"C4'", "C2'", 'C2', "C1'", 'N7', 'OP2', 'N3', 'C5', 'P', "H5''",
          "C5'", 'O6', 'H1', "H3'", 'C4', 'N1', 'H8', "H1'", 'C8', 'N9',
          'HOP3', 'OP1', "O4'", "H2'", "HO2'", 'OP3', "O3'", 'HOP2', "O5'",
          "O2'", 'H21', 'H22', "HO3'", "H5'", "C3'", 'N2', 'C6', "H4'"},
    'GLN': {'HB2', 'CG', 'H', 'H2', 'N', 'HG2', 'HE22', 'CA', 'HG3', 'HE21',
            'HB3', 'HXT', 'O', 'NE2', 'C', 'OE1', 'CB', 'CD', 'HA', 'OXT'},
    'GLU': {'HB2', 'CG', 'H', 'H2', 'N', 'HG2', 'CA', 'HG3', 'HB3', 'HXT',
            'O', 'HE2', 'C', 'OE2', 'OE1', 'CB', 'CD', 'HA', 'OXT'},
    'GLY': {'HA3', 'HXT', 'CA', 'O', 'HA2', 'H', 'N', 'C', 'H2', 'OXT'},
    'HIS': {'HB2', 'CG', 'CE1', 'HE1', 'H', 'ND1', 'H2', 'N', 'CA', 'HD1',
            'HB3', 'HXT', 'O', 'HE2', 'NE2', 'C', 'CD2', 'CB', 'HA', 'HD2',
            'OXT'},
    'ILE': {'HD11', 'CG1', 'H', 'HD12', 'H2', 'N', 'CA', 'HD13', 'HG13',
            'HXT', 'O', 'HB', 'C', 'CD1', 'HG23', 'HG22', 'HG21', 'HG12',
            'CB', 'CG2', 'HA', 'OXT'},
    'LEU': {'HD11', 'HB2', 'HD22', 'CG', 'HD21', 'H', 'HD12', 'H2', 'N',
            'HD23', 'CA', 'HD13', 'HB3', 'HXT', 'O', 'C', 'CD2', 'CD1', 'CB',
            'HA', 'HG', 'OXT'},
    'LYS': {'HB2', 'CG', 'CE', 'H', 'H2', 'N', 'HG2', 'HE3', 'CA', 'HG3',
            'HB3', 'HXT', 'O', 'HE2', 'HZ1', 'HZ3', 'C', 'HD3', 'CB', 'CD',
            'HA', 'HZ2', 'HD2', 'NZ', 'OXT'},
    'MET': {'HB2', 'CG', 'HE1', 'CE', 'H', 'H2', 'N', 'HG2', 'HE3', 'CA',
            'HG3', 'SD', 'HB3', 'HXT', 'O', 'HE2', 'C', 'CB', 'HA', 'OXT'},
    'PHE': {'HB2', 'CG', 'CE1', 'HE1', 'H', 'H2', 'N', 'HZ', 'CA', 'HD1',
            'CZ', 'HB3', 'HXT', 'O', 'HE2', 'C', 'CD2', 'CD1', 'CB', 'CE2',
            'HA', 'HD2', 'OXT'},
    'PRO': {'HB3', 'HB2', 'N', 'CG', 'O', 'CB', 'HG2', 'HXT', 'CD', 'C', 'HA',
            'CA', 'HD2', 'H', 'HG3', 'HD3', 'OXT'},
    'SER': {'H2', 'HB3', 'HB2', 'N', 'HXT', 'O', 'CB', 'C', 'HA', 'HG', 'CA',
            'H', 'OG', 'OXT'},
    'THR': {'H2', 'HXT', 'N', 'HG23', 'O', 'CB', 'CG2', 'OG1', 'HB', 'C',
            'HA', 'CA', 'HG22', 'H', 'HG1', 'HG21', 'OXT'},
    'TRP': {'HB2', 'CG', 'CE3', 'CZ3', 'HE1', 'H', 'H2', 'N', 'HE3', 'CA',
            'CZ2', 'HD1', 'HB3', 'HXT', 'O', 'HZ3', 'C', 'CD2', 'CD1', 'NE1',
            'CB', 'HH2', 'CE2', 'HA', 'CH2', 'HZ2', 'OXT'},
    'U': {"C4'", "C2'", 'C2', 'O2', 'H5', 'O4', "C1'", 'OP2', 'N3', 'C5', 'P',
          "H5''", 'H6', "C5'", "H3'", 'C4', 'N1', "H1'", 'HOP3', 'H3', 'OP1',
          "O4'", "H2'", "HO2'", 'OP3', "O3'", 'HOP2', "O5'", "O2'", "HO3'",
          "H5'", "C3'", 'C6', "H4'"},
    'VAL': {'CG1', 'H', 'H2', 'N', 'CA', 'HG13', 'HXT', 'O', 'HB', 'C',
            'HG23', 'HG22', 'HG21', 'HG12', 'CB', 'CG2', 'HA', 'OXT', 'HG11'}
}


def add_ihm_info(s, fix_histidines, check_atom_names):
    # Non-standard histidine names (protonation states)
    histidines = frozenset(('HIP', 'HID', 'HIE'))

    if not s.title:
        s.title = 'Auto-generated system'

    # Simple default assembly containing all chains
    default_assembly = ihm.Assembly(s.asym_units, name='Modeled assembly')

    # Simple default atomic representation for everything
    default_representation = ihm.representation.Representation(
        [ihm.representation.AtomicSegment(asym, rigid=False)
         for asym in s.asym_units])

    # Simple default modeling protocol
    default_protocol = ihm.protocol.Protocol(name='modeling')

    for state_group in s.state_groups:
        for state in state_group:
            for model_group in state:
                for model in model_group:
                    if not model.assembly:
                        model.assembly = default_assembly
                    if not model.representation:
                        model.representation = default_representation
                    if not model.protocol:
                        model.protocol = default_protocol
                    model.not_modeled_residue_ranges.extend(
                        _get_not_modeled_residues(model))
                    if fix_histidines:
                        _fix_histidine_het_atoms(model, histidines)
                    if check_atom_names != 'no':
                        _check_atom_names(model, check_atom_names == 'all')
    if fix_histidines:
        _fix_histidine_chem_comps(s, histidines)
    return s


def _fix_histidine_het_atoms(model, histidines):
    """Fix any non-standard histidine atoms in atom_site that are marked
       HETATM to instead use ATOM"""
    for atom in model._atoms:
        if atom.seq_id is None or not atom.het:
            continue
        comp = atom.asym_unit.sequence[atom.seq_id - 1]
        if comp.id in histidines:
            atom.het = False


class _ChemCompAtomHandler:
    not_in_file = omitted = unknown = None

    def __init__(self):
        super().__init__()
        self.atoms = collections.defaultdict(set)

    def __call__(self, comp_id, atom_id):
        self.atoms[comp_id].add(atom_id)


def _get_non_std_restyp(restyp):
    """Return CCD info for the given residue type"""
    url_top = 'https://files.rcsb.org'
    url_pattern = url_top + '/pub/pdb/refdata/chem_comp/%s/%s/%s.cif'
    url = url_pattern % (restyp[-1], restyp, restyp)
    cca = _ChemCompAtomHandler()
    try:
        with urllib.request.urlopen(url) as fh:
            c = ihm.format.CifReader(fh,
                                     category_handler={'_chem_comp_atom': cca})
            c.read_file()
    except urllib.error.URLError as exc:
        warnings.warn(
            "Component %s could not be found in CCD: %s" % (restyp, exc))
    return cca.atoms


def _get_non_canon(seen_atom_names, check_all):
    """Get all non-canonical atom names for each residue type"""
    for restyp, atoms in seen_atom_names.items():
        if check_all and restyp not in KNOWN_ATOM_NAMES:
            KNOWN_ATOM_NAMES.update(_get_non_std_restyp(restyp))
        if restyp in KNOWN_ATOM_NAMES:
            non_canon_atoms = atoms - KNOWN_ATOM_NAMES[restyp]
            if non_canon_atoms:
                yield restyp, non_canon_atoms


def _check_atom_names(model, check_all):
    """Check that only standard atom names are used for known
       residue types"""
    seen_atom_names = collections.defaultdict(set)
    for atom in model._atoms:
        seq_id = 1 if atom.seq_id is None else atom.seq_id
        comp = atom.asym_unit.sequence[seq_id - 1]
        seen_atom_names[comp.id].add(atom.atom_id)
    non_canon = sorted(_get_non_canon(seen_atom_names, check_all),
                       key=operator.itemgetter(0))
    if non_canon:
        raise ValueError(
            "Non-canonical atom names found in the following residues: "
            + "; ".join("%s: %r" % (restyp, sorted(atoms))
                        for (restyp, atoms) in non_canon))


def _fix_histidine_chem_comps(s, histidines):
    """Change any non-standard histidine chemical components to normal HIS"""
    his = ihm.LPeptideAlphabet()['H']
    for e in s.entities:
        for c in e.sequence:
            if c.id in histidines:
                # Change the ChemComp to HIS in place, as there may be
                # references to this ChemComp elsewhere. Duplicate HIS
                # components will be combined into one at output time.
                c.id = his.id
                c.code = his.code
                c.code_canonical = his.code_canonical
                c.name = his.name
                c.formula = his.formula
                c.__class__ = his.__class__


def _get_not_modeled_residues(model):
    """Yield NotModeledResidueRange objects for all residue ranges in the
       Model that are not referenced by Atom, Sphere, or pre-existing
       NotModeledResidueRange objects"""
    for assem in model.assembly:
        asym = assem.asym if hasattr(assem, 'asym') else assem
        if not asym.entity.is_polymeric():
            continue
        # Make a set of all residue indices of this asym "handled" either
        # by being modeled (with Atom or Sphere objects) or by being
        # explicitly marked as not-modeled
        handled_residues = set()
        for rr in model.not_modeled_residue_ranges:
            if rr.asym_unit is asym:
                for seq_id in range(rr.seq_id_begin, rr.seq_id_end + 1):
                    handled_residues.add(seq_id)
        for atom in model._atoms:
            if atom.asym_unit is asym:
                handled_residues.add(atom.seq_id)
        for sphere in model._spheres:
            if sphere.asym_unit is asym:
                for seq_id in range(sphere.seq_id_range[0],
                                    sphere.seq_id_range[1] + 1):
                    handled_residues.add(seq_id)
        # Convert set to a list of residue ranges
        handled_residues = ihm.util._make_range_from_list(
            sorted(handled_residues))
        # Return not-modeled for each non-handled range
        for r in ihm.util._invert_ranges(handled_residues,
                                         end=assem.seq_id_range[1],
                                         start=assem.seq_id_range[0]):
            yield ihm.model.NotModeledResidueRange(asym, r[0], r[1])


def add_ihm_info_one_system(fname, fix_histidines, check_atom_names):
    """Read mmCIF file `fname`, which must contain a single System, and
       return it with any missing IHM data added."""
    with open(fname) as fh:
        systems = ihm.reader.read(fh)
    if len(systems) != 1:
        raise ValueError("mmCIF file %s must contain exactly 1 data block "
                         "(%d found)" % (fname, len(systems)))
    return add_ihm_info(systems[0], fix_histidines, check_atom_names)


def combine(s, other_s):
    """Add models from the System `other_s` into the System `s`.
       After running this function, `s` will contain all Models from both
       systems. The models are added to new StateGroup(s) in `s`.
       Note that this function also modifies `other_s` in place, so that
       System should no longer be used after calling this function."""
    # First map all Entity and AsymUnit objects in `other_s` to equivalent
    # objects in `s`
    entity_map = combine_entities(s, other_s)
    asym_map = combine_asyms(s, other_s, entity_map)
    # Now handle the Models themselves
    combine_atoms(s, other_s, asym_map)


def combine_entities(s, other_s):
    """Add `other_s` entities into `s`. Returns a dict that maps Entities
       in `other_s` to equivalent objects in `s`."""
    entity_map = {}
    sequences = dict((e.sequence, e) for e in s.entities)
    for e in other_s.entities:
        if e.sequence in sequences:
            # If the `other_s` Entity already exists in `s`, map to it
            entity_map[e] = sequences[e.sequence]
        else:
            # Otherwise, add the `other_s` Entity to `s`
            s.entities.append(e)
            entity_map[e] = e
    return entity_map


def combine_asyms(s, other_s, entity_map):
    """Add `other_s` asyms into `s`. Returns a dict that maps AsymUnits
       in `other_s` to equivalent objects in `s`."""
    asym_map = {}
    # Collect author-provided information for existing asyms. For polymers,
    # we use the author-provided chain ID; for non-polymers, we also use
    # the author-provided residue number of the first (only) residue
    poly_asyms = dict(((a.entity, a.strand_id), a)
                      for a in s.asym_units if a.entity.is_polymeric())
    nonpoly_asyms = dict(((a.entity, a.strand_id, a.auth_seq_id_map[1]), a)
                         for a in s.asym_units
                         if a.entity.type == 'non-polymer')

    def map_asym(asym, orig_asym):
        if orig_asym:
            # If an equivalent asym already exists, use it (and its asym_id)
            asym_map[asym] = orig_asym
        else:
            # Otherwise, add a new asym
            asym_map[asym] = asym
            asym.id = None  # Assign new ID
            s.asym_units.append(asym)

    for asym in other_s.asym_units:
        # Point to Entity in `s`, not `other_s`
        asym.entity = entity_map[asym.entity]
        # For polymers and non-polymers, if an asym in `other_s` has the
        # same author-provided information and entity_id as an asym in `s`,
        # reuse the asym_id
        if asym.entity.is_polymeric():
            map_asym(asym, poly_asyms.get((asym.entity, asym.strand_id)))
        elif asym.entity.type == 'non-polymer':
            map_asym(asym, nonpoly_asyms.get((asym.entity, asym.strand_id,
                                              asym.auth_seq_id_map[1])))
        else:
            # For waters and branched entities, always assign a new asym_id
            asym_map[asym] = asym
            asym.id = None  # Assign new ID
            s.asym_units.append(asym)
    return asym_map


def combine_atoms(s, other_s, asym_map):
    """Add `other_s` atoms into `s`"""
    seen_asmb = set()
    seen_rep = set()
    for state_group in other_s.state_groups:
        for state in state_group:
            for model_group in state:
                for model in model_group:
                    # Assembly, Representation and Atom and Sphere objects
                    # all reference `other_s` asyms. We must map these to
                    # asyms in `s`.
                    asmb = model.assembly
                    if id(asmb) not in seen_asmb:
                        seen_asmb.add(id(asmb))
                        # todo: also handle AsymUnitRange
                        asmb[:] = [asym_map[asym] for asym in asmb]
                    rep = model.representation
                    if id(rep) not in seen_rep:
                        seen_rep.add(id(rep))
                        for seg in rep:
                            seg.asym_unit = asym_map[seg.asym_unit]
                    for atom in model._atoms:
                        atom.asym_unit = asym_map[atom.asym_unit]
                    for sphere in model._spheres:
                        sphere.asym_unit = asym_map[sphere.asym_unit]

    # Add all models as new state groups
    s.state_groups.extend(other_s.state_groups)


def get_args():
    p = argparse.ArgumentParser(
        description="Add minimal IHM-related tables to an mmCIF file.")
    p.add_argument("input", metavar="input.cif", help="input mmCIF file name")
    p.add_argument("output", metavar="output.cif",
                   help="output mmCIF file name",
                   default="output.cif", nargs="?")
    p.add_argument("--add", "-a", action='append', metavar="add.cif",
                   help="also add model information from the named mmCIF "
                        "file to the output file")
    p.add_argument("--histidines", action='store_true', dest="fix_histidines",
                   help="Convert any non-standard histidine names (HIP, HID, "
                        "HIE, for different protonation states) to HIS")
    p.add_argument('--check_atom_names', choices=['no', 'standard', 'all'],
                   dest="check_atom_names", default='no',
                   help="If 'standard', check for non-canonical atom names "
                        "in standard amino acid and nucleic acid chemical "
                        "components; if 'all', also check non-standard "
                        "residue types by querying CCD (needs network access)")
    return p.parse_args()


def main():
    args = get_args()

    if (os.path.exists(args.input) and os.path.exists(args.output)
            and os.path.samefile(args.input, args.output)):
        raise ValueError("Input and output are the same file")

    if args.add:
        s = add_ihm_info_one_system(args.input, args.fix_histidines,
                                    args.check_atom_names)
        for other in args.add:
            other_s = add_ihm_info_one_system(other, args.fix_histidines,
                                              args.check_atom_names)
            combine(s, other_s)
        with open(args.output, 'w') as fhout:
            ihm.dumper.write(
                fhout, [s],
                variant=ihm.dumper.IgnoreVariant(['_audit_conform']))
    else:
        with open(args.input) as fh:
            with open(args.output, 'w') as fhout:
                ihm.dumper.write(
                    fhout, [add_ihm_info(s, args.fix_histidines,
                                         args.check_atom_names)
                            for s in ihm.reader.read(fh)],
                    variant=ihm.dumper.IgnoreVariant(['_audit_conform']))


if __name__ == '__main__':
    main()
