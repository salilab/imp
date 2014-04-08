#!/usr/bin/env python

"""@namespace IMP.isd.TALOSReader
   Classes to handle TALOS files or folders.
"""

import sys
import os
from math import pi
from IMP.isd.utils import check_residue, read_sequence_file


class TALOSReader:

    """ reads a TALOS file, or a TALOS folder, and stores the data """

    def __init__(self, sequence, detailed_input, keep_all=False,
                 sequence_match=(1, 1)):
        """start the TALOSReader
        sequence : a dictionnary of sequence number keys and 3-letter code
                   values.
        detailed_input : True if the input will be either predAll.tab or the
                         pred/res???.tab files. False if it's pred.tab
        keep_all : whether to keep outliers or not, when detailed_input==True.
        sequence_match : in the form (talos_no, sequence_no), assigns a
        correspondence between residue numberings.
        """
        self.detailed_input = detailed_input
        self.data = {}
        self.keep_all = keep_all
        self.sequence = sequence
        self.offset = sequence_match[1] - sequence_match[0]

    def add_full_datum(self, resno, phi, psi):
        """in the case of a list of predictions for one residue, add an entry to
        data which is:
            'full' : always True
            'num'  : number of predictions
            'phi'  : the list of predictions for phi
            'psi'  : same for psi

        """
        if resno not in self.data:
            self.data[resno] = {
                'full': True, 'num': len(phi), 'phi': phi, 'psi': psi}
        else:
            raise RuntimeError("would overwrite data for residue %d" % resno)

    def add_mean_datum(self, resno, num, phi, psi):
        """in the case of a single (average) prediction output by talos for a
        given residue, add an entry to data which is:
            'full' : always False
            'num' : the number of matches this average was calculated from
            'phi' : a tuple in the form (mean, error)
            'psi' : same as phi.

        """

        if resno not in self.data:
            self.data[
                resno] = {
                'full': False,
                'num': num,
                'phi': phi,
                'psi': psi}
        else:
            raise RuntimeError("would overwrite data for residue %d" % resno)

    def _read_one_residue(self, fname):
        fl = open(fname)
        resno = int(os.path.basename(fname)[3:6]) + self.offset
        phi = []
        psi = []
        for line in fl:
            tokens = line.split()
            if len(tokens) < 1:
                continue
            if tokens[1] == 'RESNAMES':
                check_residue(self.sequence[resno], tokens[3])
                continue
            if not tokens[0].isdigit():
                continue
            if float(tokens[4]) < 0.999 and not self.keep_all:
                continue
            phi.append(float(tokens[1]) * 2 * pi / 360.)
            psi.append(float(tokens[2]) * 2 * pi / 360.)
        self.add_full_datum(resno, phi, psi)

    def _read_predAll(self, fname):
        fl = open(fname)
        resno = -1
        for line in fl:
            tokens = line.split()
            if len(tokens) == 0 or not tokens[0].isdigit():
                continue
            oldresno = resno
            resno = int(tokens[1]) + self.offset
            if resno != oldresno:
                if oldresno != -1:
                    self.add_full_datum(resno, phi, psi)
                phi = []
                psi = []
            resname = tokens[2][1]
            check_residue(self.sequence[resno], resname)
            if float(tokens[6]) < 0.999 and not self.keep_all:
                continue
            phi.append(float(tokens[3]) * 2 * pi / 360.)
            psi.append(float(tokens[4]) * 2 * pi / 360.)

    def _read_observations(self, fname):
        if fname.endswith('predAll.tab'):
            self._read_predAll(fname)
        else:
            self._read_one_residue(fname)

    def _read_averages(self, fname):
        fl = open(fname)
        for line in fl:
            tokens = line.split()
            if not tokens[0].isdigit():
                continue
            resno = int(tokens[0]) + self.offset
            check_residue(resno, tokens[1])
            phi, psi, dphi, dpsi = map(lambda a: 2 * pi * float(a) / 360.,
                                       tokens[2:6])
            num = int(tokens[8])
            if num == 0:
                continue
            self.add_mean_datum(resno, num, (phi, dphi), (psi, dpsi))

    def read(self, fname):
        "reads a TALOS file and returns data. See add_datum methods."
        if self.detailed_input:
            self._read_observations(fname)
        else:
            self._read_averages(fname)

    def get_data(self):
        return self.data


if __name__ == '__main__':

    talos = 'pred.tab'
    sequence = read_sequence_file('seq.dat', sequence_match=(1, 5))
    reader = TALOSReader(sequence)
    reader.read(talos)
    data = reader.get_data()
