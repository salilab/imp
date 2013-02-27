#!/usr/bin/env python

import IMP
import IMP.saxs
import os
import re
import sys
import math
import shutil
import tempfile
import subprocess
import optparse
import inspect

class TempDir(object):
    """Make a temporary directory, and delete it when the object is destroyed"""
    def __init__(self):
        self.tmpdir = tempfile.mkdtemp()
    def __del__(self):
        shutil.rmtree(self.tmpdir, ignore_errors=True)


def _count_lines(filename):
    """Count the number of lines in the file"""
    wc = 0
    for line in open(filename):
        wc += 1
    return wc

def _count_fiber_dock_out(filename):
    """Count the number of transformations in a FiberDock output file"""
    wc = 0
    header_found = False
    for line in open(filename):
        if '|' in line:
            if header_found:
                wc += 1
            elif 'glob' in line:
                header_found = True
    return wc

def _run_binary(path, binary, args, out_file=None):
    if path:
        binary = os.path.join(path, binary)
    cmd = ' '.join([binary] + args)
    if out_file:
        stdout = open(out_file, 'w')
        cmd += ' > ' + out_file
    else:
        stdout = sys.stdout
    print cmd
    p = subprocess.Popen([binary] + args, stdout=stdout, stderr=sys.stderr)
    ret = p.wait()
    if ret != 0:
        raise OSError("subprocess failed with exit code %d" % ret)


class Scorer(object):
    """Score transformations using a type of experimental data.
       To add support for a new type of data, simply create a new Scorer
       subclass and override its methods and variables."""
    transformations_file = 'trans_pd'
    # Number of transforms to be passed to FiberDock; more accurate scoring
    # methods can get away with fewer transforms
    transforms_needed = 5000
    # If set to True, higher scores are considered better when calculating
    # zscores
    reverse_zscores = False
    # A short string (< 8 characters) identifying the method
    short_name = None

    @classmethod
    def add_parser_options(cls, parser):
        """Add command line options to the given parser"""
        pass

    @classmethod
    def check_options(cls, idock, parser):
        """Check command line options for consistency, and create and return
           a new instance if selected by these options"""
        pass

    def __init__(self, idock, output_type):
        self.receptor = idock.receptor
        self.ligand = idock.ligand
        self.output_file = idock.get_filename("%s.res" % output_type)
        self.zscore_output_file = idock.get_filename("%sf.res" % output_type)

    def score(self, num_transforms):
        if os.path.exists(self.output_file) \
           and _count_lines(self.output_file) >= num_transforms:
            print "Skipping %s for %s" % (str(self), self.receptor)
        else:
            self._run_score_binary()

    def recompute_zscore(self, transforms_file):
        """Recompute z-scores for the new set of transformations
           in transforms_file"""
        d = TempDir()
        tmp_tf = os.path.join(d.tmpdir, 'tmp')
        solutions = []
        num_re = re.compile('\d')
        # Read all solutions
        for line in open(self.output_file):
            spl = line.split('|')
            if len(spl) > 1 and num_re.search(spl[0]):
                solutions.append(line)
        # Extract only the solutions that are mentioned in transforms_file
        fh = open(tmp_tf, 'w')
        for line in open(transforms_file):
            spl = line.split()
            if len(spl) > 1:
                fh.write(solutions[int(spl[0])-1])
        fh.close()
        # Recompute z-scores for the new set
        args = [tmp_tf]
        if self.reverse_zscores:
            args.append('1')
        _run_binary(None, 'recompute_zscore', args,
                    out_file=self.zscore_output_file)


class NMRScorer(Scorer):
    """Score transformations using NMR residue type content"""
    short_name = 'nmr_rtc'

    @classmethod
    def add_parser_options(cls, parser):
        g = optparse.OptionGroup(parser, "NMR-RTC")
        g.add_option('--receptor_rtc', metavar='FILE',
                     help="File name of the receptor NMR residue type content")
        g.add_option('--ligand_rtc', metavar='FILE',
                     help="File name of the ligand NMR residue type content")
        parser.add_option_group(g)

    @classmethod
    def check_options(cls, idock, parser):
        if idock.opts.receptor_rtc or idock.opts.ligand_rtc:
            return cls(idock)

    def __init__(self, idock):
        Scorer.__init__(self, idock, "nmr_rtc_score")
        self.receptor_rtc = idock.opts.receptor_rtc
        self.ligand_rtc = idock.opts.ligand_rtc
        if idock.opts.type == 'AA':
            self.receptor_rtc, self.ligand_rtc = self.ligand_rtc, '-'

    def __str__(self):
        return "NMR score"

    def _run_score_binary(self):
        _run_binary(None, "nmr_rtc_score",
                    [self.receptor, self.ligand, self.transformations_file,
                     self.receptor_rtc, self.ligand_rtc,
                     '-o', self.output_file])


class SAXSScorer(Scorer):
    """Score transformations using SAXS (rg + chi)"""
    short_name = 'saxs'

    @classmethod
    def add_parser_options(cls, parser):
        g = optparse.OptionGroup(parser, "SAXS")
        g.add_option('--saxs', metavar='FILE', dest='saxs_file',
                     help="File name of the complex SAXS profile")
        g.add_option('--saxs_receptor_pdb', metavar='FILE',
                     help='Additional receptor structure for SAXS scoring '
                           'with modeled missing atoms/residues. '
                           'This structure should be aligned to the '
                           'input receptor!')
        g.add_option('--saxs_ligand_pdb', metavar='FILE',
                      help='Additional ligand structure for SAXS scoring '
                           'with modeled missing atoms/residues. '
                           'This structure should be aligned to the '
                           'input ligand!')
        parser.add_option_group(g)

    @classmethod
    def check_options(cls, idock, parser):
        if idock.opts.saxs_file:
            return cls(idock)

    def __init__(self, idock):
        Scorer.__init__(self, idock, "saxs_score")
        self.saxs_file = idock.opts.saxs_file
        self.saxs_receptor = idock.opts.saxs_receptor_pdb or idock.receptor
        self.saxs_ligand = idock.opts.saxs_ligand_pdb or idock.ligand

    def __str__(self):
        return "SAXS score"

    def _run_score_binary(self):
        _run_binary(None, "saxs_score",
                    [self.saxs_receptor, self.saxs_ligand,
                     self.transformations_file, self.saxs_file,
                     '-o', self.output_file])


class EM2DScorer(Scorer):
    """Score transformations using EM2D"""
    short_name = 'em2d'

    @classmethod
    def add_parser_options(cls, parser):
        g = optparse.OptionGroup(parser, "EM2D")
        g.add_option('--em2d', metavar='FILE', action='append', default=[],
                     dest='class_averages',
                     help="File name of a complex 2D class average in PGM "
                          "format. This option can be repeated to use "
                          "multiple class averages (up to 5 in total)")
        g.add_option('--pixel_size', type='float', default=0.,
                     help='Pixel size for EM2D images')
        parser.add_option_group(g)

    @classmethod
    def check_options(cls, idock, parser):
        if idock.opts.class_averages:
            if idock.opts.pixel_size <= 0.:
                parser.error("please specify pixel size for 2D images with "
                             "--pixel_size option")
            else:
                return cls(idock)

    def __init__(self, idock):
        Scorer.__init__(self, idock, "em2d_score")
        self.class_averages = idock.opts.class_averages
        self.pixel_size = idock.opts.pixel_size

    def __str__(self):
        return "EM2D score"

    def _run_score_binary(self):
        _run_binary(None, "em2d_score",
                    [self.receptor, self.ligand, self.transformations_file] \
                    + self.class_averages \
                    + ['-o', self.output_file, '-n', '200', '-s',
                       str(self.pixel_size)])


class EM3DScorer(Scorer):
    """Score transformations using EM3D"""
    short_name = 'em3d'
    transforms_needed = 1000
    reverse_zscores = True

    @classmethod
    def add_parser_options(cls, parser):
        g = optparse.OptionGroup(parser, "EM3D")
        g.add_option('--em3d', metavar='FILE', dest='map_file',
                     help="File name of the complex density map in mrc format")
        parser.add_option_group(g)

    @classmethod
    def check_options(cls, idock, parser):
        if idock.opts.map_file:
            return cls(idock)

    def __init__(self, idock):
        Scorer.__init__(self, idock, "em3d_score")
        self.map_file = idock.opts.map_file

    def __str__(self):
        return "EM3D score"

    def _run_score_binary(self):
        _run_binary(None, "em3d_score",
                    [self.receptor, self.ligand, self.transformations_file,
                     self.map_file, '-o', self.output_file, '-s'])


class CXMSScorer(Scorer):
    """Score transformations using CXMS"""
    short_name = 'cxms'
    transforms_needed = 2000
    reverse_zscores = True

    @classmethod
    def add_parser_options(cls, parser):
        g = optparse.OptionGroup(parser, "CXMS")
        g.add_option('--cxms', metavar='FILE', dest='cross_links_file',
                     help="File name of the cross links file")
        parser.add_option_group(g)

    @classmethod
    def check_options(cls, idock, parser):
        if idock.opts.cross_links_file:
            return cls(idock)

    def __init__(self, idock):
        Scorer.__init__(self, idock, "cxms_score")
        self.cross_links_file = idock.opts.cross_links_file

    def __str__(self):
        return "CXMS score"

    def _run_score_binary(self):
        _run_binary(None, "cross_links_score",
                    [self.receptor, self.ligand, self.transformations_file,
                     self.cross_links_file, '-o', self.output_file])


class IDock(object):
    """Handle all stages of the integrative docking protocol"""

    # Get list of all Scorer subclasses
    _all_scorers = [x[1] for x in inspect.getmembers(sys.modules[__name__],
                            lambda x: inspect.isclass(x) \
                                and issubclass(x, Scorer) and x is not Scorer)]

    def parse_args(self):
        """Parse command line options, and return all applicable Scorers"""
        data_types = [x.short_name.upper() for x in self._all_scorers]
        data_types = ", ".join(data_types[:-1]) + " and/or " + data_types[-1]
        epilog = """Note that in order to run this application, you must also
have PatchDock and FiberDock (available from
http://bioinfo3d.cs.tau.ac.il/FiberDock/) and reduce (available from
http://kinemage.biochem.duke.edu/software/reduce.php)
installed. See the --patch-dock and --fiber-dock parameters to tell idock
where the PatchDock and FiberDock programs are installed. idock expects to
find 'reduce' in the system path."""
        parser = IMP.OptionParser(usage="%prog [options] <receptor_pdb> "
                                        "<ligand_pdb>",
                                  description="Integrative pairwise docking "
                                              "using %s data" % data_types,
                                  epilog=epilog,
                                  imp_module=IMP.saxs)
        choices=['EI', 'AA', 'other']
        parser.add_option('--complex_type', metavar='TYPE', type='choice',
                          dest="type", choices=choices, default='other',
                          help='/'.join(choices) + '; use this order for '
                               'receptor-ligand: '
                               'antibody-antigen, enzyme-inhibitor')
        parser.add_option('--patch_dock', metavar='DIR',
                          default=os.environ.get('PATCH_DOCK_HOME', None),
                          help='Directory where PatchDock tools are installed. '
                               'If not specified, the value of the '
                               'PATCH_DOCK_HOME environment variable is used '
                               'if set, otherwise the tools are assumed to be '
                               'in the default path.')
        parser.add_option('--fiber_dock', metavar='DIR',
                          default=os.environ.get('FIBER_DOCK_HOME', None),
                          help='Directory where FiberDock tools are installed. '
                               'If not specified, the value of the '
                               'FIBER_DOCK_HOME environment variable is used '
                               'if set, otherwise the tools are assumed to be '
                               'in the default path.')
        parser.add_option('--prefix', default='',
                          help='Add prefix string (separated by an underscore) '
                               'to filenames generated by the current run')
        parser.add_option('--precision', type='choice', choices=['1', '2', '3'],
                          default='1',
                          help='Sampling precision for rigid docking: '
                               '1-normal, 2-medium, 3-high. The higher the '
                               'precision, the higher are the run times')
        for s in self._all_scorers:
            s.add_parser_options(parser)

        opts, args = parser.parse_args()
        if len(args) != 2:
            parser.error("incorrect number of arguments")
        opts.precision = int(opts.precision)
        if opts.prefix:
            opts.prefix += '_'

        self.opts = opts
        self.receptor, self.ligand = args
        scorers = self.get_scorers(parser)

        if len(scorers) == 0:
            parser.error("please provide %s experimental data" % data_types)
        return scorers

    def get_scorers(self, parser):
        """Get all scorers that are applicable to the command line options"""
        scorers = []
        for s in self._all_scorers:
            si = s.check_options(self, parser)
            if si:
                scorers.append(si)
        return scorers

    def run_patch_dock_binary(self, binary, args):
        """Run a binary that is part of the PatchDock distribution"""
        _run_binary(self.opts.patch_dock, binary, args)

    def run_fiber_dock_binary(self, binary, args):
        """Run a binary that is part of the FiberDock distribution"""
        _run_binary(self.opts.fiber_dock, binary, args)

    def make_patch_dock_surfaces(self):
        """Make molecular surfaces for PatchDock"""
        self.run_patch_dock_binary('buildMS.pl', [self.receptor, self.ligand])

    def make_patch_dock_parameters(self):
        """Make parameter file for PatchDock"""
        if self.opts.precision == 1:
            rmsd = '4.0'
        else:
            rmsd = '2.0'
        if self.opts.precision == 3:
            script = 'buildParamsFine.pl'
        else:
            script = 'buildParams.pl'
        self.run_patch_dock_binary(script, [self.receptor, self.ligand, rmsd,
                                            self.opts.type])

    def get_filename(self, fn):
        """Get a filename, with user-defined prefix if given"""
        return self.opts.prefix + fn

    def get_all_scores_filename(self, scorers, prefix, suffix):
        """Get a filename containing the names of all scores used"""
        return self.get_filename(prefix \
                          + '_'.join([x.short_name for x in scorers]) + suffix)

    def do_patch_dock_docking(self):
        """Do PatchDock docking, using previously generated surfaces
           and parameter files"""
        out_file = self.get_filename('docking.res')
        # Skip if PatchDock output file exists and contains transformations
        # (not just the header containing parameter information)
        if os.path.exists(out_file) and _count_lines(out_file) > 36:
            print "Skipping PatchDock for %s" % self.receptor
        else:
            self.run_patch_dock_binary('patch_dock.Linux',
                                       ['params.txt', out_file])

    def make_transformation_file(self):
        """Extract transformation image from PatchDock output file"""
        out_file = self.get_filename('docking.res')
        num_re = re.compile('\d')
        num_transforms = 0
        fout = open('trans_pd', 'w')
        for line in open(out_file):
            fields = line.split('|')
            if len(fields) > 1 and num_re.search(fields[0]):
                num_transforms += 1
                print >> fout, int(fields[0]), fields[-1].strip(' \r\n')
                if self.opts.precision == 1 and num_transforms >= 5000:
                    break
        return num_transforms

    def run_patch_dock(self):
        """Run PatchDock on the ligand and receptor"""
        self.make_patch_dock_surfaces()
        self.make_patch_dock_parameters()
        self.do_patch_dock_docking()
        num_transforms = self.make_transformation_file()
        # Swap ligand/receptor if we're doing AA
        if self.opts.type == 'AA':
            self.ligand, self.receptor = self.receptor, self.ligand
            self.opts.saxs_receptor_pdb, self.opts.saxs_ligand_pdb = \
                    self.opts.saxs_ligand_pdb, self.opts.saxs_receptor_pdb
        return num_transforms

    def get_filtered_scores(self, scorers):
        """Get scores that were filtered by an experimental method"""
        # If only one score, simply extract from its file (note we extract the
        # z-score, not the raw score, since the combined score is the weighted
        # sum of z-scores, not raw scores)
        if len(scorers) == 1:
            out_fh = open('trans_for_cluster', 'w')
            in_fh = open(scorers[0].output_file)
            for line in in_fh:
                spl = line.split('|')
                if '+' in line and '#' not in line and len(spl) > 1:
                    out_fh.write("%s %s %s" % (spl[0], spl[3], spl[-1]))
        else:
            out_file = self.get_all_scores_filename(scorers, 'combined_',
                                                    '.res')
            args = []
            for s in scorers:
                args.extend([s.output_file, '1.0'])
            _run_binary(None, 'combine_scores', args, out_file=out_file)
            out_fh = open('trans_for_cluster', 'w')
            in_fh = open(out_file)
            for line in in_fh:
                spl = line.split('|')
                if '#' not in line and len(spl) > 1:
                    out_fh.write("%s %s %s" % (spl[0], spl[1], spl[-1]))

    def get_clustered_transforms(self, scorers):
        """Cluster transformations with PatchDock"""
        out_file = self.get_all_scores_filename(scorers, 'clustered_', '.res')
        self.run_patch_dock_binary('interface_cluster.linux',
                                   [self.receptor + '.ms', self.ligand,
                                    'trans_for_cluster', '4.0', out_file])
        return out_file

    def get_transforms_for_fiber_dock(self, scorers, transforms_file):
        """Convert PatchDock-clustered transforms into FiberDock input"""
        trans_for_fd = self.get_all_scores_filename(scorers, 'trans_for_fd_',
                                                    '')
        transforms_needed = min([s.transforms_needed for s in scorers])
        fh_in = open(transforms_file)
        fh_out = open(trans_for_fd, 'w')
        for line in fh_in:
            spl = line.split('|')
            if len(spl) > 1:
                print >> fh_out, spl[0] + " " + spl[4].rstrip('\r\n')
                transforms_needed -= 1
                if transforms_needed <= 0:
                    break
        return trans_for_fd

    def add_hydrogens(self, in_pdb):
        """Run reduce to add hydrogens to in_pdb; return the name of the new
           PDB with hydrogens"""
        out_pdb = in_pdb + '.HB'
        _run_binary(None, 'reduce',
                    ['-OH', '-HIS', '-NOADJ', '-NOROTMET', in_pdb],
                    out_file=out_pdb)
        return out_pdb

    def make_fiber_dock_parameters(self, scorers, receptorH, ligandH,
                                   trans_for_fd):
        params = self.get_all_scores_filename(scorers, 'params_fd_', '.txt')
        fd_out = self.get_all_scores_filename(scorers, 'fd_res_', '')
        self.run_fiber_dock_binary('buildFiberDockParams.pl',
                 [receptorH, ligandH, 'U', 'U', self.opts.type, trans_for_fd,
                  fd_out, '0', '50', '0.8', '0', 'glpk', params])
        return params, fd_out + '.ref'

    def do_fiber_dock_docking(self, trans_for_fd, params, fd_out):
        if os.path.exists(fd_out) \
           and _count_fiber_dock_out(fd_out) == _count_lines(trans_for_fd):
            print "Skipping FiberDock for %s" % self.receptor
            return
        else:
            self.run_fiber_dock_binary('FiberDock', [params])

    def convert_fiber_dock_to_score(self, scorers, fd_out):
        """Convert FiberDock or FireDock output file to the same format
           as the scorers"""
        out = self.get_all_scores_filename(scorers, 'fd_res_', '.res')
        solutions = []
        fire_dock = False
        num_re = re.compile('\d')
        # Extract solutions
        for line in open(fd_out):
            spl = line.split('|')
            if len(spl) > 1 and num_re.search(spl[0]):
                num = int(spl[0])
                if fire_dock:
                    solutions.append((num, float(spl[5]),
                                      spl[-1].rstrip('\r\n')))
                else:
                    solutions.append((num, float(spl[1]), spl[-3]))
            elif len(spl) > 5 and 'glob' in spl[5]:
                fire_dock = True
        # Compute Z-score
        average = 0.
        stddev = 0.
        for solution in solutions:
            score = solution[1]
            average += score
            stddev += (score * score)
        average /= len(solutions)
        stddev = math.sqrt(stddev / len(solutions) - (average * average))

        # Output in new format
        fh = open(out, 'w')
        print >> fh, "    # | Score | filt| ZScore | Transformation"
        for solution in solutions:
            num, score, trans = solution
            zscore = (score - average) / stddev
            print >> fh, " %4d | %5.3f |  +  | %5.3f | %s" \
                         % (num, score, zscore, trans)
        return out

    def run_fiber_dock(self, scorers, transforms_file):
        trans_for_fd = self.get_transforms_for_fiber_dock(scorers,
                                                          transforms_file)
        receptorH = self.add_hydrogens(self.receptor)
        ligandH = self.add_hydrogens(self.ligand)
        params, fd_out = self.make_fiber_dock_parameters(scorers, receptorH,
                                                         ligandH, trans_for_fd)
        self.do_fiber_dock_docking(trans_for_fd, params, fd_out)
        return trans_for_fd, self.convert_fiber_dock_to_score(scorers, fd_out)

    def combine_final_scores(self, scorers, fd_out):
        """Combine scores for the final set with those from FireDock"""
        out_file = self.get_filename('combined_final.res')
        args = []
        for s in scorers:
            args.extend([s.zscore_output_file, '1.0'])
        args.extend([fd_out, '1.0'])
        _run_binary(None, 'combine_scores', args, out_file=out_file)
        return out_file

    def write_results(self, scorers, comb_final):
        """Write final results file"""
        out_fname = self.get_all_scores_filename(scorers, 'results_', '.txt')
        # Get all solutions
        solutions = []
        for line in open(comb_final):
            spl = line.split('|')
            if len(spl) > 1 and 'Score' not in line:
                solutions.append(spl)
        # Sort by z-score
        solutions.sort(key=lambda x: float(x[3]))
        # Write results file
        fh = open(out_fname, 'w')
        print >> fh, "receptorPdb Str " + self.receptor
        print >> fh, "ligandPdb Str " + self.ligand
        print >> fh, "     # |  Score  | filt| ZScore |" \
                 + "|".join(["%-8s| Zscore " % s.short_name for s in scorers]) \
                 + "| Energy | Zscore | Transformation"
        for i, solution in enumerate(solutions):
            fh.write("%6d | " % (i+1) + "|".join(solution[1:]))
        return out_fname

    def main(self):
        """Run the entire protocol"""
        scorers = self.parse_args()
        num_transforms = self.run_patch_dock()
        for scorer in scorers:
            scorer.score(num_transforms)
        self.get_filtered_scores(scorers)
        transforms_file = self.get_clustered_transforms(scorers)
        trans_for_fd, fd_out = self.run_fiber_dock(scorers, transforms_file)
        for scorer in scorers:
            scorer.recompute_zscore(trans_for_fd)
        fh = self.combine_final_scores(scorers, fd_out)
        self.write_results(scorers, fh)


if __name__ == "__main__":
    dock = IDock()
    dock.main()
