#!/usr/bin/env python

import sys
import os
import re
import shutil


class LogStep:

    def __init__(self, stepno, statline, header):
        self.stepno = stepno
        self.stats = statline
        self.header = header
        self.dumps = {}
        self.trajs = {}

    def add(self, ftype, category, *data):
        if ftype == 'dump':
            self.dumps[category] = data[0]
        elif ftype == 'traj':
            self.trajs[category] = {'ftype':data[0],
                                    'fullpath':data[1],
                                    'stepno':data[2],
                                    'tail':data[3]}
        else:
            raise ValueError, "unknown file type"

    def get_stats_header(self):
        return self.header

    def get_stats(self):
        return self.stats

    def get_dumps(self):
        return self.dumps

    def get_trajs(self):
        return self.trajs


class LogHolder:

    """Manages information on a given simulation.
    Assumes the existence of a _stats.txt file, and handles more files if
    available.
    folder : the folder which contains _stats.txt
    prefix : the stats file is supposed to be prefix+'_stats.txt'
    """

    def __init__(self, folder, prefix):
        self.folder = folder
        self.prefix = prefix
        # verify that stats file exists
        self.stats_file = os.path.join(folder, prefix + 'stats.txt')
        if not os.path.isfile(self.stats_file):
            raise ValueError, 'cannot find stats file %s' % self.stats_file
        # scan for other files
        files = {}
        for fl in os.listdir(folder):
            # get variable part of file, exclude stats file
            match = re.match(prefix + r'(.*)', fl)
            if match is None:
                continue
            tail = match.group(1)
            if tail == 'stats.txt':
                continue
            # get file category and add to files dict
            category = tail.split('_')[0]
            if not category in files:
                files[category] = []
            files[category].append(tail)
        # see if there are multiple files in the same category, and store them
        self.dumpfiles = {}
        self.trajfiles = {}
        for cat, fnames in files.iteritems():
            if len(fnames) > 1:
                # there are multiple files, no need to understand their content
                if not cat in self.dumpfiles:
                    self.dumpfiles[cat] = []
                for fname in fnames:
                    # parse tail and find index number
                    indexno = int(os.path.splitext(fname.split('_')[1])[0])
                    self.dumpfiles[cat].append((indexno, fname))
                self.dumpfiles[cat] = dict(self.dumpfiles[cat])
                # make sure there are no duplicate index numbers
                if len(self.dumpfiles[cat]) != \
                        len(set(self.dumpfiles[cat].keys())):
                            raise ValueError, \
                                "found duplicates in %s %s %s" \
                                % (folder, prefix, fname)
            else:
                # this is a trajectory, need to be able to parse it
                fname = fnames[0]
                ext = os.path.splitext(fname)[1]
                if ext.startswith('.rmf'):
                    self.trajfiles[cat] = (ext[1:], fname)
                else:
                    raise ValueError, "Unknown extension:", ext

    def get_stats_header(self):
        if not hasattr(self, 'stats_handle'):
            self.stats_handle = open(self.stats_file)
            # read the file and guess the number of lines
            # for now, be compatible with only one line
            self.stats_handle.readline()
            self.stats_first_line = self.stats_handle.readline()
            if self.stats_first_line.startswith('#'):
                raise ValueError, 'stats file must be 1-line only'
            self.stats_handle = open(self.stats_file)
            self.stats_header = self.stats_handle.readline()
        return self.stats_header

    def get_first_stats_line(self):
        # make sure file is open, skip header
        self.get_stats_header()
        return self.stats_first_line

    def _get_next_stats(self):
        # make sure file is open, skip header
        self.get_stats_header()
        for line in self.stats_handle:
            yield line

    def items(self):
        """iterate over all time steps"""
        # open stats file, store header and loop over stats file
        for stat in self._get_next_stats():
            # extract step number and create LogStep
            stepno = int(stat.split()[1])
            step = LogStep(stepno, stat, self.get_stats_header())
            # get other files' entries at this step if available
            for cat, df in self.dumpfiles.iteritems():
                if stepno in df:
                    fullpath = os.path.join(self.folder,
                                            self.prefix + df[stepno])
                    step.add('dump', cat, fullpath)
            for cat, tf in self.trajfiles.iteritems():
                fullpath = os.path.join(self.folder,
                                        self.prefix + tf[1])
                step.add('traj', cat, tf[0], fullpath, stepno, tf[1])
            # yield a LogStep containing these entries
            yield step


class Demuxer:

    """uses column to demux a replica trajectory. Assumes column points to a
    float or integer type, which is allowed to change over time. Attribution is
    based on order of float params. State 0 will be lowest param etc. Use
    reverse=True to start with highest.
    """

    def __init__(self, logs, outfolder, column, reverse=False):
        self.logs = logs
        self.reverse = reverse
        self.column = column
        self.outfolder = outfolder
        self.stat_handles = {}
        self.traj_handles_in = {}
        self.traj_handles_out = {}
        self.folders = {}
        # create needed folders
        if not os.path.isdir(outfolder):
            os.mkdir(outfolder)
        for l in xrange(len(self.logs)):
            fname = os.path.join(outfolder, 'p%d' % l)
            if not os.path.isdir(fname):
                os.mkdir(fname)
            self.folders[l] = fname
        # make sure every log has the same header
        h0 = self.logs[0].get_stats_header()
        for log in self.logs[1:]:
            if h0 != log.get_stats_header():
                raise "headers must be identical!"
        # get column number from header
        tokens = [idx for idx, i in enumerate(h0.split()) if self.column in i]
        if len(tokens) == 0:
            raise ValueError, "column %d not found in this header\n%s" \
                % (column, h0)
        elif len(tokens) > 1:
            raise ValueError, "column %d found multiple times!\n%s" \
                % (column, h0)
        self.colno = tokens[0]

    def get_param(self, statline):
        return float(statline.split()[self.colno])

    def _write_step_stats(self, stateno, lstep):
        # check if stats file is open
        if stateno not in self.stat_handles:
            self.stat_handles[stateno] = open(
                os.path.join(self.folders[stateno],
                             str(stateno) + '_stats.txt'), 'w')
            self.stat_handles[stateno].write(lstep.get_stats_header())
        # write stats
        self.stat_handles[stateno].write(lstep.get_stats())

    def _write_step_dump(self, stateno, lstep):
        for cat, fname in lstep.get_dumps().iteritems():
            shutil.copyfile(fname, os.path.join(self.folders[stateno],
                        str(stateno) + '_' + cat + fname.split(cat)[1]))

    def _write_traj_rmf(self, infile, instep, outfile, stateno, cat):
        import RMF
        #make sure infile is open
        if infile not in self.traj_handles_in:
            src = RMF.open_rmf_file_read_only(infile)
            self.traj_handles_in[infile] = src
        src = self.traj_handles_in[infile]
        #make sure outfile is open
        if outfile not in self.traj_handles_out:
            dest = RMF.create_rmf_file(outfile)
            self.traj_handles_out[outfile] = dest
            RMF.clone_file_info(src, dest)
            RMF.clone_hierarchy(src, dest)
            RMF.clone_static_frame(src, dest)
        dest = self.traj_handles_out[outfile]
        #clone frame
        frameid = src.get_frames()[instep-1]
        src.set_current_frame(frameid)
        dest.add_frame(src.get_name(frameid), src.get_type(frameid))
        RMF.clone_loaded_frame(src, dest)

    def _write_step_traj(self, stateno, lstep):
        #loop over categories
        for cat, data in lstep.get_trajs().iteritems():
            destfile = os.path.join(self.outfolder, 'p'+str(stateno),
                        str(stateno) + '_' + data['tail'])
            if data['ftype'].startswith('rmf'):
                self._write_traj_rmf(data['fullpath'], data['stepno'],
                                     destfile, stateno, cat)
            else:
                raise ValueError, "unknown trajectory file type"

    def _write_step(self, stateno, lstep):
        self._write_step_stats(stateno, lstep)
        self._write_step_dump(stateno, lstep)
        self._write_step_traj(stateno, lstep)

    def write(self):
        # loop over time steps
        log_iterators = [l.items() for l in self.logs]
        print "Demuxing",len(log_iterators),"replicas"
        for idx,steps in enumerate(zip(*log_iterators)):
            if idx % 10 == 0 and idx>0:
                print "step",idx,'\r',
                sys.stdout.flush()
            # assign state numbers to these logs
            params = [(self.get_param(i.get_stats()), i) for i in steps]
            params.sort(reverse=self.reverse)
            # write them
            for i in xrange(len(params)):
                self._write_step(i, params[i][1])
        print "Done"


def get_prefix(folder):
    rval = [re.match(r'(.*_)stats.txt', f) for f in os.listdir(folder)]
    rval = [i for i in rval if i]
    if len(rval) != 1:
        raise ValueError, "stats file not unique, found", len(rval)
    return rval[0].group(1)

if __name__ == '__main__':
    if len(sys.argv) == 1 or len(sys.argv) > 4:
        sys.exit("""demux_trajs.py column [infolder [outfolder]]
        expects r?? folders in infolder and will write p?? folders in outfolder.
        infolder must contain a _stats.txt file which will contain a header.
        column must be a substring matching to one of the columns in the
        _stats.txt files. It will typically be a temperature, or a state number.
        That column will be used for demuxing. Folders are optional and will be
        taken as ./ if not indicated.
        """)
    column = sys.argv[1]
    if len(sys.argv) == 3:
        infolder = sys.argv[2]
        outfolder = './'
    elif len(sys.argv) == 4:
        infolder = sys.argv[2]
        outfolder = sys.argv[3]
    else:
        infolder = './'
        outfolder = './'
    # loop over infolder and read stats files
    folders = [os.path.join(infolder,f)
               for f in os.listdir(infolder) if re.match(r'r\d+', f)]
    replica_logs = [LogHolder(f, prefix)
                    for f,prefix in zip(folders,map(get_prefix,folders))]
    demux = Demuxer(replica_logs, outfolder, column, reverse=True)
    demux.write()
