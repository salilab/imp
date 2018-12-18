#!/usr/bin/env python

from __future__ import print_function
import IMP
import IMP.multifit
from IMP import ArgumentParser

__doc__ = "Write assembly transformation file in other formats."

class Formatter(object):

    def __init__(self, fh):
        self.fh = fh

    def write_header(self, settings):
        pass


class ChimeraFormatter(Formatter):
    __doc__ = \
"""Each line in 'chimera' format lists the transformation index, the
cross correlation score, and then the transformation for each component,
as a rotation matrix (row-major order) and a translation in angstroms."""

    def write_line(self, ind, score, transforms):
        self.fh.write(str(ind) + "\t" + score + "\t")
        for t in transforms:
            for k in range(3):
                v = t.get_rotation().get_rotation_matrix_row(k)
                self.fh.write(
                    str(v[0]) + " " + str(v[1]) + " " + str(v[2]) + " ")
            v = t.get_translation()
            self.fh.write(
                str(v[0]) + " " + str(v[1]) + " " + str(v[2]) + " | ")
        self.fh.write("\n")


class DockRefFormatter(Formatter):
    __doc__ = \
"""Each line in 'dockref' format lists the transformation for each component,
as a set of three Euler angles (in radians about the fixed x, y and z axes)
and a translation in angstroms."""

    def write_header(self, sd):
        # write the name of the proteins
        for i in range(sd.get_number_of_component_headers()):
            self.fh.write(sd.get_component_header(i).get_name() + "|")
        self.fh.write("\n")

    def write_line(self, ind, score, transforms):
        for t in transforms:
            r = t.get_rotation()
            tr = t.get_translation()
            eulers = IMP.algebra.get_fixed_xyz_from_rotation(r)
            self.fh.write("%.6g %.6g %.6g %.6g %.6g %.6g|"
                          % (eulers.get_x(), eulers.get_y(), eulers.get_z(),
                             tr[0], tr[1], tr[2]))
        self.fh.write("\n")


formatters = {'chimera': ChimeraFormatter,
              'dockref': DockRefFormatter}


def parse_args():
    desc = """
Write assembly transformation file in other formats.

""" + "\n\n".join(x.__doc__ for x in formatters.values())

    p = ArgumentParser(description=desc)
    p.add_argument("-f", "--format", default='chimera',
                      choices=list(formatters.keys()),
                      help="type of output to generate ("
                           + ", ".join(formatters.keys())
                           + "; default: chimera)")
    p.add_argument("assembly_file", help="assembly file name")
    p.add_argument("combinations_file", help="combinations file name")
    p.add_argument("output_file", help="output file name")
    return p.parse_args()


def run(asmb_fn, combs_fn, fmt):
    sd = IMP.multifit.read_settings(asmb_fn)
    sd.set_was_used(True)
    fmt.write_header(sd)
    # read the fitting files
    fits = []
    for i in range(sd.get_number_of_component_headers()):
        fn = sd.get_component_header(i).get_transformations_fn()
        fits.append(IMP.multifit.read_fitting_solutions(fn))
    for ind, line in enumerate(open(combs_fn)):
        s = line.split("|")
        score = s[2]
        comb = s[1]
        fmt.write_line(ind, score,
                       [fits[i][int(c)].get_fit_transformation()
                        for i, c in enumerate(comb.split())])


def main():
    args = parse_args()
    fmt = formatters[args.format](open(args.output_file, 'w'))
    run(args.assembly_file, args.combinations_file, fmt)

if __name__ == "__main__":
    main()
