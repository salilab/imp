#!/usr/bin/env python

"""@namespace IMP.isd.TBLReader
   Classes to handle TBL files.
"""

import sys
import os
import IMP.isd
from IMP.isd.utils import Load, read_sequence_file
#from Isd.io.nomenclature import IUPAC_CONVENTION
IUPAC_CONVENTION = 'iupac'
TYPE_AMINO_ACID = 'AMINO_ACID'
pseudoatoms_dict = IMP.isd.get_data_path('CHARMM_pseudoatoms.dict')


def del_comment(x):

    n = x.find('!')

    if n >= 0:
        x = x[:n]

    return x


def check_assigns(x):
    return 'resid' in x and 'name' in x


class TBLReader:

    atom_dict = {'segid': '',
                 'resid': -1,
                 'name': ''}

    pseudoatom_char = '*', '%', '#'

    def __init__(self, sequence, ignore_warnings=False, sequence_match=(1, 1)):

        self.sequence = sequence
        # sequence_match = (a,b) a: NOE numbering, b: sequence numbering
        self.offset = sequence_match[1] - sequence_match[0]
        self.ignore = ignore_warnings
        self.pseudo_dict = Load(pseudoatoms_dict)

    def extract_contributions(self, contribs):

        new_contribs = []

        for c in contribs:

            if not c:
                continue

            c = c[c.find('('):]
            c = c[:c.rfind(')') + 1]

            new_contribs.append(c)

        return new_contribs

    def split_contribution(self, contrib):

        words = contrib.split('(')
        atoms = [word.split(')')[0] for word in words if word]

        return atoms

    def resolve_pseudoatom(self, residue_type, atom_name):

        if '*' in atom_name:
            char = '*'
        elif '#' in atom_name:
            char = '#'

        atom_name = atom_name.replace(char, '%')

        # TODO: Assumes that pseudo-atom names are compatible with
        # IUPAC name, since pseudo_dict can handle IUPAC
        # names only.

        try:
            group = self.pseudo_dict[residue_type][atom_name]

        except:

            key = atom_name, residue_type

            if not key in self.missing_atoms:

                msg = 'Could not resolve pseudoatom %s.%s.' % (
                    residue_type, atom_name)

                if self.ignore:
                    print msg
                else:
                    raise KeyError(msg)

                self.missing_atoms.append(key)

            return atom_name

        return group

    def to_iupac(self, residue_type, atom_name):

        raise NotImplementedError

        iupac_name = self.thesaurus.convert_atom(residue_type,
                                                 atom_name,
                                                 self.naming_system,
                                                 IUPAC_CONVENTION,
                                                 TYPE_AMINO_ACID)
        try:
            iupac_name = self.thesaurus.convert_atom(residue_type,
                                                     atom_name,
                                                     self.naming_system,
                                                     IUPAC_CONVENTION,
                                                     TYPE_AMINO_ACID)

        except:

            key = atom_name, residue_type

            if not key in self.missing_atoms:

                if '*' in atom_name or '#' in atom_name:

                    msg = 'Pseudoatoms not upported: %s' % atom_name

                    if self.ignore:
                        print msg

                    else:
                        raise KeyError(msg)

                elif self.ignore:
                    msg = 'Warning: atom %s not found in residue %s.' % key
                    print msg
                else:
                    raise KeyError(msg % key)

                self.missing_atoms.append(key)

            return atom_name

        return iupac_name

    def resolve_dihedral_name(self, atoms):

        raise NotImplementedError

        names = [a['name'] for a in atoms]

        try:
            res_type = self.sequence[atoms[1]['resid']]

        except IndexError:
            print 'Residue number overflow in atoms', atoms
            return ''

        for dihedral in self.connectivity[res_type].dihedrals.values():

            keys = sorted([k for k in dihedral.keys() if 'atom' in k])

            atom_names = []

            for k in keys:
                name = dihedral[k]
                if name[-1] in ('-', '+'):
                    name = name[:-1]

                atom_names.append(name)

            if atom_names == names:
                return dihedral['name']

        msg = 'Could not determine name of dihedral angles defined by atoms %s.' % str(
            names)

        if self.ignore:
            print msg
            return ''

        raise KeyError(msg)

    def extract_atom(self, a):

        atom = dict(self.atom_dict)

        words = a.split()

        skip_next = False

        # correct for segid statements

        words = [x for x in words if x != '"']

        for i in range(len(words)):

            if skip_next:
                skip_next = False
                continue

            word = words[i]

            if word == 'and':
                continue

            for key in atom.keys():

                if key in word:

                    if key == 'segid':
                        atom[key] = words[i + 1][:-1]
                    else:
                        atom[key] = words[i + 1]

                    skip_next = True
                    break

            else:
                raise KeyError(
                    'Value or keyword "%s" unknown. Source: "%s", decomposed into "%s"' %
                    (word, str(a), str(words)))

        atom['resid'] = int(atom['resid']) + self.offset
        atom['name'] = atom['name'].upper()

        return atom

    def build_contributions(self, atoms):

        groups = []

        for a in atoms:

            try:
                res_type = self.sequence[a['resid']]

            except IndexError:
                print 'Residue number overflow in atoms', atoms
                return []

            atom_name = a['name']

            if atom_name[-1] in self.pseudoatom_char:
                group = self.resolve_pseudoatom(res_type, atom_name)

            else:
                #group = [self.to_iupac(res_type, atom_name)]
                group = [atom_name]

            groups.append(group)

        group1, group2 = groups

        contribs = []

        res_1 = atoms[0]['resid']
        res_2 = atoms[1]['resid']

        for i in range(len(group1)):

            name_1 = group1[i]

            for j in range(len(group2)):

                if (res_1, name_1) != (res_2, group2[j]):
                    contribs.append(((res_1, name_1), (res_2, group2[j])))

        return contribs

    def extract_target_values(self, line):

        end = line.rfind(')')

        values = line[end + 1:].split()

        try:
            distances = [float(x) for x in values[:3]]
        except:
            distances = None

        # read volume from ARIA 1.x restraint files

        val = line.split('volume=')

        if len(val) > 1:
            volume = float(val[1].split()[0].split(',')[0])
        else:

            volume = None

        return distances, volume

    def read_contents(self, filename):

        keywords = 'class',

        filename = os.path.expanduser(filename)

        f = open(filename)
        lines = f.readlines()
        f.close()

        all = ''

        for x in lines:

            x = x.strip()

            if not x or x[0] == '!':
                continue

            not_valid = [kw for kw in keywords if kw in x]

            if not_valid:
                continue

            all += x.lower() + ' '

        return [x.strip() for x in all.split('assi')]

    def find_contributions(self, line):

        contribs = [del_comment(x).strip() for x in line.split('or')]

        # use alternative parser for implicitly listed atom pairs

        if 1 in [x.count('resid') for x in contribs]:

            atoms = []

            while line:

                start = line.find('(')

                if start < 0:
                    atoms[-1][-1] += line
                    break

                stop = line.find(')')

                selection = [x.strip()
                             for x in line[start:stop + 1].split('or')]

                for i in range(len(selection)):

                    val = selection[i]

                    if not '(' in val:
                        val = '(' + val

                    if not ')' in val:
                        val += ')'

                    selection[i] = val

                atoms.append(selection)

                line = line[stop + 1:]

            if len(atoms) != 2:
                raise

            # find and isolate target distances

            l = []

            for i in range(len(atoms)):

                g = []

                for atom in atoms[i]:

                    n = atom.rfind(')')

                    if n >= 0 and len(atom[n + 1:].strip()) > 3:
                        distances = atom[n + 1:].strip()
                        atom = atom[:n + 1]

                    g.append(atom)

                l.append(g)

            a, b = l

            if len(a) > len(b):
                a, b = b, a

            contribs = []

            for i in a:
                for j in b:
                    contribs.append('%s %s' % (i, j))

            contribs[0] += ' ' + distances

        return contribs

    def create_distance_restraint(self, distances, volume, contributions):
        if distances is None and volume is None:
            raise ValueError("could not find either volume or "
                             "distance: %s %s %s" % (distances, volume, contributions))
        if distances is None:
            distances = [volume ** (-1. / 6), 0, 0]
        dist = distances[0]
        if volume is None:
            volume = dist ** (-6)
        lower = dist - distances[1]
        upper = dist + distances[2]
        return (tuple(contributions), dist, lower, upper, volume)

    def read_distances(self, filename, key, naming_system=IUPAC_CONVENTION,
                       decompose=False):
        """reads a tbl file and parses distance restraints.
        """

        self.naming_system = naming_system

        assigns = self.read_contents(filename)

        restraints = []
        self.missing_atoms = []
        seq_number = 0

        for line in assigns:

            contribs = self.find_contributions(line)

            if False in [check_assigns(x) for x in contribs]:
                continue

            distances, volume = self.extract_target_values(contribs[0])

            if (distances is None and volume is None):
                distances, volume = self.extract_target_values(contribs[-1])

            new_contribs = self.extract_contributions(contribs)

            contributions = []

            for contrib in new_contribs:

                atoms = self.split_contribution(contrib)
                atoms = [self.extract_atom(x) for x in atoms]

                contributions += self.build_contributions(atoms)

            if contributions:
                r = self.create_distance_restraint(distances, volume,
                                                   contributions)

                restraints.append(r)
                seq_number += 1

        if restraints:

            if decompose:

                d = decompose_restraints(restraints)

                for _type in d.keys():
                    if not d[_type]:
                        del d[_type]

                if len(d) > 1:
                    for _type, val in d.items():

                        if val:
                            new_key = key + '_%s' % _type
                            d[new_key] = val

                        del d[_type]

                else:
                    d = {key: d.values()[0]}
            else:
                d = {key: restraints}

            return d

    def read_dihedrals(self, filename, key, naming_system=IUPAC_CONVENTION):

        self.naming_system = naming_system

        assigns = self.read_contents(filename)

        restraints = []
        self.missing_atoms = []
        seq_number = 0

        for line in assigns:

            contribs = [del_comment(x).strip() for x in line.split('or')]

            values, volume = self.extract_target_values(contribs[0])
            new_contribs = self.extract_contributions(contribs)

            if not new_contribs:
                continue

            if len(new_contribs) > 1:
                raise ValueError(
                    'Inconsistency in data file, multiple contributions detected.')

            atoms = self.split_contribution(new_contribs[0])
            atoms = [self.extract_atom(x) for x in atoms]

            name = self.resolve_dihedral_name(atoms)

            r = create_dihedral_restraint(seq_number, name, values, atoms)

            restraints.append(r)
            seq_number += 1

        if restraints:
            return restraints

    def read_rdcs(self, filename, key, naming_system=IUPAC_CONVENTION):

        self.naming_system = naming_system

        assigns = self.read_contents(filename)

        restraints = []
        self.missing_atoms = []
        seq_number = 0

        fake_atom_names = ('OO', 'X', 'Y', 'Z')

        for line in assigns:

            contribs = [del_comment(x).strip() for x in line.split('or')]
            distances, volume = self.extract_target_values(contribs[0])
            new_contribs = self.extract_contributions(contribs)

            contributions = []

            for contrib in new_contribs:

                atoms = self.split_contribution(contrib)
                atoms = [self.extract_atom(x) for x in atoms]

                atoms = [a for a in atoms if not a['name'] in fake_atom_names]

                contributions += self.build_contributions(atoms)

            if contributions:
                r = create_rdc_restraint(
                    seq_number,
                    distances[0],
                    contributions)

                restraints.append(r)
                seq_number += 1

        if restraints:
            return restraints

if __name__ == '__main__':

    noe = 'noe.tbl'
    sequence = read_sequence_file('seq.dat', first_residue_number=1)
    reader = TBLReader(sequence, ignore_warnings=True)
    reader.read_distances(noe, key='test')
