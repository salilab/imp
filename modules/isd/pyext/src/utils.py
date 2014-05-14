"""@namespace IMP.isd.utils
   Miscellaneous utilities.
"""

#
# The Inferential Structure Determination (ISD) software library
#
# Authors: Michael Habeck and Wolfgang Rieping
#
# Copyright (C) Michael Habeck and Wolfgang Rieping
#
# All rights reserved.
#
# NO WARRANTY. This library is provided 'as is' without warranty of any
# kind, expressed or implied, including, but not limited to the implied
# warranties of merchantability and fitness for a particular purpose or
# a warranty of non-infringement.
#
# Distribution of substantively modified versions of this module is
# prohibited without the explicit permission of the copyright holders.
#

import atexit
import sys
import time
import os
import os.path
import socket


from Queue import Queue
from threading import Thread

debug = False

code = {
    'A': 'ALA',
    'R': 'ARG',
    'N': 'ASN',
    'D': 'ASP',
    'C': 'CYS',
    'E': 'GLU',
    'Q': 'GLN',
    'G': 'GLY',
    'H': 'HIS',
    'I': 'ILE',
    'L': 'LEU',
    'K': 'LYS',
    'M': 'MET',
    'F': 'PHE',
    'P': 'PRO',
    'S': 'SER',
    'T': 'THR',
    'W': 'TRP',
    'Y': 'TYR',
    'V': 'VAL'
}


def average(x):
    return sum(x) / float(len(x))


def atexit_register(*args):

    atexit.register(*args)


def atexit_unregister(func):

    exit_funcs = [x[0] for x in atexit._exithandlers]

    try:
        i = exit_funcs.index(func)
    except:
        return

    atexit._exithandlers.pop(i)


class WatchDog(Thread):

    def __init__(self, timeout, debug=False, logfile=None):
        """
        timeout: in minutes.
        """

        Thread.__init__(self)

        self.timeout = timeout * 60.
        self.debug = debug
        self._last_ping = None
        self._stop = False

        if logfile is not None:
            logfile = os.path.expanduser(logfile)

        self.logfile = logfile

        self.setDaemon(True)

    def stop(self):
        self._stop = True

    def set(self, x):
        "set the _last_ping variable of the WatchDog instance"

        if self.debug:
            print 'Watchdog: set(%s) called.' % str(x)

        self._last_ping = x

    def run(self):
        """run the Watchdog thread, which sits in a loop sleeping for timeout/4. at
        each iteration, and if abs(time() - _last_ping) > timeout, exits.
        """

        while not self._stop:

            if self._last_ping is not None:
                delta = abs(self._last_ping - time.time())
            else:
                delta = None

            if self.debug:

                if delta is None:
                    val = 'N/A s'
                else:
                    val = '%.0f s' % delta

                print 'Watchdog: last life sign %s ago; timeout is %d min(s).' % \
                      (val, self.timeout / 60.)

            if self._last_ping is not None and delta > self.timeout:

                s = 'No life sign for > %d minute(s)' % (self.timeout / 60.)

                print s + ', exiting...'

                if self.logfile is not None:

                    if os.path.exists(self.logfile):
                        mode = 'a'
                    else:
                        mode = 'w'

                    try:
                        f = open(self.logfile, mode)
                        f.write(
                            s + '; host %s, %s\n' %
                            (socket.gethostname(), time.ctime()))
                        f.close()

                    except IOError:
                        pass

                if not self.debug:
                    os._exit(0)
                else:
                    print 'Watchdog: keeping Python interpreter alive.'
                    self.stop()

            time.sleep(self.timeout / 4.)


class SpinWheel:

    symbols = ('-', '/', '|', '\\')

    def __init__(self):
        self.state = 0

    def update(self, s=''):

        import sys

        sys.stdout.write('\r%s%s' % (s, self.symbols[self.state]))
        sys.stdout.flush()

        self.state = (self.state + 1) % len(self.symbols)


class Pipe(object):

    """implements a FIFO pipe that merges lists (see self.put)"""

    def __init__(self, length=-1):

        self.length = length
        self.pipe = []

    def put(self, x):
        """if x is subscriptable, insert its contents at the beginning of the pipe.
        Else insert the element itself.
        If the pipe is full, drop the oldest element.
        """

        try:
            x[0]
            self.pipe = list(x) + self.pipe

        except:
            self.pipe.insert(0, x)

        if self.length > 0 and len(self.pipe) > self.length:
            self.pipe = self.pipe[:-1]

    def append(self, x):
        """ x must be a list and will be appended to the end of the pipe, dropping
        rightmost elements if necessary
        """

        self.pipe = (list(x) + self.pipe)[:self.length]

    def get(self):
        """returns the oldest element, without popping it out of the pipe.
        Popping occurs in the put() method
        """
        return self.pipe[-1]

    def __getitem__(self, index):
        return self.pipe.__getitem__(index)

    def __len__(self):
        return len(self.pipe)

    def __str__(self):
        return str(self.pipe)

    def is_full(self):
        return len(self.pipe) == self.length

    __repr__ = __str__


class SortedQueue(Queue):

    def sort(self):

        from numpy.oldnumeric import array
        from Isd.misc.mathutils import average

        self.queue.sort(lambda a, b: cmp(average(a.time), average(b.time)))

        self.times = array([average(x.time) for x in self.queue])

    def _put(self, item):

        Queue._put(self, item)
        self.sort()

    def _get(self):

        from numpy.oldnumeric import power
        from Isd.misc.mathutils import draw_dirichlet, rescale_uniform

        # compute "probabilities"

        p = 1. - rescale_uniform(self.times)
        p = power(p, 2.)

        index = draw_dirichlet(p)

        val = self.queue[index]

        self.queue = self.queue[:index] + self.queue[index + 1:]

        if len(self.queue):
            self.sort()

        return val


def load_pdb(filename):

    import os

    from Scientific.IO.PDB import Structure

    return Structure(os.path.expanduser(filename))


def copyfiles(src_path, dest_path, pattern=None, verbose=False):

    from glob import glob
    from shutil import copyfile
    import os

    if pattern is None:
        pattern = '*'

    file_list = glob(os.path.join(src_path, pattern))

    for f in file_list:
        copyfile(f, os.path.join(dest_path, os.path.basename(f)))

        if verbose:
            print f


def touch(filename):

    try:
        f = open(filename, 'w')
        f.close()

    except IOError, error:
        import os
        if os.path.isdir(filename):
            pass
        else:
            raise IOError(error)

# Yannick


def read_sequence_file(filename, first_residue_number=1):
    """read sequence of ONE chain, 1-letter or 3-letter, returns dict of
    no:3-letter code. Fails on unknown amino acids.
    """

    filename = os.path.abspath(filename)
    try:
        f = open(filename)
    except IOError, msg:
        raise IOError('Could not open sequence file "%s".' % filename)
    seq = f.read().upper()

    if seq.startswith('>'):
        print "Detected FASTA 1-letter sequence"
        pos = seq.find('\n')
        # get rid of first line and get sequence in one line
        seq = ''.join(seq[pos + 1:].split())
        names = [code[i] for i in seq]
        numbers = range(first_residue_number, first_residue_number + len(seq))
        return dict(zip(numbers, names))
    else:
        l = seq.split()
        for x in l:
            if not x in code.values():
                print 'Warning: unknown 3-letter code: %s' % x
        numbers = range(first_residue_number, first_residue_number + len(l))
        return dict(zip(numbers, l))

# Yannick


def check_residue(a, b):
    "checks whether residue codes a and b are the same, doing necessary conversions"
    a = a.upper()
    b = b.upper()
    if len(a) == 1:
        if a not in code:
            print 'Warning: unknown 1-letter code: %s' % a
            return False
        a = code[a]
    if len(b) == 1:
        if b not in code:
            print 'Warning: unknown 1-letter code: %s' % b
            return False
        b = code[b]
    if len(a) != 3:
        print 'Unknown residue code %s' % a
        return False
    if len(b) != 3:
        print 'Unknown residue code %s' % b
        return False
    if a != b:
        print 'Residues %s and %s are not the same' % (a, b)
        return False
    else:
        return True


def my_glob(x, do_touch=False):

    from glob import glob

    if do_touch:

        import os

        path, name = os.path.split(x)

        # os.system('touch %s' % path) #this is very inefficient
        touch(path)  # this is better (4x to 6x faster)

    return glob(x)


def Dump(this, filename, gzip=0, mode='w', bin=1):
    """
    Dump(this, filename, gzip = 0)
    Supports also '~' or '~user'.
    """

    import os
    import cPickle

    filename = os.path.expanduser(filename)

    if not mode in ['w', 'a']:
        raise "mode has to be 'w' (write) or 'a' (append)"

    if gzip:
        import gzip
        f = gzip.GzipFile(filename, mode)
    else:
        f = open(filename, mode)

    cPickle.dump(this, f, bin)

    f.close()


def Load(filename, gzip=0, force=0):
    """
    Load(filename, gzip=0, force=0)

    force: returns all objects that could be unpickled. Useful
           when unpickling of sequential objects fails at some point.
    """
    import cPickle
    import os

    filename = os.path.expanduser(filename)

    if gzip:
        import gzip
        try:
            f = gzip.GzipFile(filename)
        except:
            return

    f = open(filename)

    objects = None

    eof = 0
    n = 0

    while not eof:

        try:
            object = cPickle.load(f)

            if objects is None:
                objects = object

            else:
                objects += object

            n += 1

        except EOFError:
            eof = 1

        except Exception:
            print 'Could not load chunk %d. Stopped.' % n

            if force:
                eof = 1
            else:
                object = cPickle.load(f)

    f.close()

    return objects


def get_pdb(pdb_entry, dest='.', verbose_level=0):

    import ftplib
    from tempfile import mktemp
    import os

    url = 'ftp.ebi.ac.uk'
    path = 'pub/databases/rcsb/pdb-remediated/data/structures/all/pdb'
    filename_template = 'pdb%s.ent.gz'

    dest = os.path.expanduser(dest)

    ftp = ftplib.FTP(url)
    ftp.login()
    ftp.set_debuglevel(verbose_level)

    ftp.cwd(path)

    filename = os.path.join(dest, '%s.pdb.gz' % pdb_entry)

    f = open(filename, 'wb')

    try:
        ftp.retrbinary('RETR %s' % filename_template % pdb_entry.lower(),
                       f.write)

        f.close()

        ftp.quit()

    except ftplib.error_perm:
        raise IOError('File %s not found on server' % filename)

    os.system('gunzip -f %s' % filename)


def compile_index_list(chain, atom_names, residue_index_list=None):

    if residue_index_list is None:
        residue_index_list = range(len(chain))

    index_list = []

    names = atom_names

    index_map = {}

    i = 0

    for res_index in residue_index_list:

        if atom_names is None:
            names = sorted(chain[res_index].keys())

        for n in names:

            if n in chain[res_index]:
                index = chain[res_index][n].index
                index_list.append(index)
                index_map[index] = i
                i += 1

    return index_list, index_map


def get_coordinates(universe, E, indices=None, atom_names=('CA',),
                    residue_index_list=None, atom_index_list=None):

    from numpy.oldnumeric import array, take

    if indices is None:
        indices = range(len(E))

    chain = universe.get_polymer()

    if atom_index_list is None:
        atom_index_list, index_map = compile_index_list(chain, atom_names,
                                                        residue_index_list)

    l = []

    for i in indices:

        chain.set_torsions(E.torsion_angles[i], 1)

        X = array(take(universe.X, atom_index_list))

        l.append(X)

    return array(l)


def map_angles(angles, period=None):
    """
    maps angles into interval [-pi,pi]
    """

    from numpy.oldnumeric import fmod, greater, logical_not

    if period is None:
        from numpy.oldnumeric import pi as period

    mask = greater(angles, 0.)

    return mask * (fmod(angles + period, 2 * period) - period) + \
        logical_not(mask) * (fmod(angles - period, 2 * period) + period)


def remove_from_dict(d, items):

    for item in items:
        if item in d:
            del d[item]


def myrange(a, b, n):

    from numpy.oldnumeric import arange

    step = (b - a) / (n - 1)

    x = arange(a, b + step, step)

    return x[:n]


def indent(lines, prefix):

    tag = ' ' * len(str(prefix))

    lines[0] = prefix + lines[0]
    lines = [lines[0]] + map(lambda s, t=tag: t + s, lines[1:])

    return '\n'.join(lines)


def make_block(s, length=80, tol=10):
    blocks = s.split('\n')
    l = []
    for block in blocks:
        l += _make_block(block, length, tol)

    return l


def _make_block(s, length, tol):

    l = s.split(' ')
    l = [(w, ' ') for w in l]

    words = []
    for ll in l:
        g = ll[0].split('/')
        g = [w + '/' for w in g]
        g[-1] = g[-1][:-1] + ' '

        words += g

    l = []
    line = ''

    for i in range(len(words)):
        word = words[i]

        if len(line + word) <= length:
            line += word

        else:
            if length - len(line) > tol:
                m = length - len(line)
                line += word[:m]
                word = word[m:]

            if len(line) > 1 and line[0] == ' ' and \
                    line[1] != ' ':
                line = line[1:]

            l.append(line)
            line = word

    line = line[:-1]
    if len(line) > 1 and line[0] == ' ' and \
       line[1] != ' ':
        line = line[1:]

    l.append(line)

    return l


def _save_dump(x, filename, err_msg=None, delay=10, show_io_err=True,
               gzip=False, bin=True):

    try:
        Dump(x, filename, gzip=gzip, bin=bin)

    except IOError, msg:

        import time

        if err_msg is None:
            print 'IOError: %s' % str(msg)

        else:
            if show_io_err:
                print '%s. %s' % (str(msg), err_msg)
            else:
                print err_msg

        while True:

            # wait for 10 minutes

            time.sleep(60. * delay)

            try:
                Dump(x, filename, gzip=gzip, bin=bin)
                break

            except IOError:
                continue


def save_dump(x, filename, err_msg=None, delay=10, show_io_err=True,
              gzip=False, mode='w', bin=True):

    import os
    import tempfile

    path, _filename = os.path.split(filename)

    temp_path, temp_filename = os.path.split(tempfile.mktemp())
    temp_filename = os.path.join(path, temp_filename)

    _save_dump(x, temp_filename, err_msg, delay, show_io_err,
               gzip, bin)

    # if that worked, dump properly

    if mode == 'w':
        os.rename(temp_filename, filename)

    elif mode == 'a':
        os.unlink(temp_filename)
        Dump(x, filename, mode='a', gzip=gzip, bin=bin)

    else:
        raise Exception('Mode "%s" invalid.' % mode)
