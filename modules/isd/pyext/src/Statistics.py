#!/usr/bin/env python

from IMP.isd.Entry import Entry
import gzip
import os


class Statistics:

    """Statistics gathering and printing class for ISD gibbs sampling.
    Also manages the restart file (TODO).
    - prefix: all outputted files will have this prefix
    - rate: print statistics every so many gibbs sampling steps
    - trajrate: print trajectories (pdb) every multiple of rate (default 1).
                implies that update_coordinates() has been called by that time
                otherwise writing will not occur.
    - statfile: suffix of the statistics file
    - num_entries_per_line: number of entries per line in the output. -1 to
                            disable wrapping.
    - repeat_title: if 0 (default) only print it in the beginning. Else repeat
                    it every 'repeat_title' outputted lines in the statistics file.
    - separate_lines: If False the entries are not separated (default). If True,
                      the lines are separated with stars.
    - compress: If set to a positive number of steps, compress trajectories each
                time so many steps have elapsed, appending the current frame
                number to the filename. Only works in append mode, and when it
                is set to a multiple of rate.

    TODO: check if everything was updated nicely
    """

    def __init__(self, prefix='r01', rate=1, trajrate=1, statfile='_stats.txt',
                 num_entries_per_line=5, repeat_title=0,
                 separate_lines=False, compress=10000):
        self.prefix = prefix
        self.rate = rate
        self.trajrate = trajrate
        self.statfile = prefix + statfile
        self.compress = compress
        # list of the things that will be printed to the stats file, in order.
        self.entries = []
        # list of coordinate entries
        self.coordinates = []
        # internal
        self.__counter_pos = 0
        # the same entries but sorted by category
        self.categories = {}
        # create the global category along with the global counter
        self.add_category('global')
        # that makes it the first entry of the table.
        self.add_entry('global', name='counter')
        # output-specific flags
        self.write_title = True
        if num_entries_per_line == 0 or num_entries_per_line < -1:
            raise ValueError("number of entries per line is >0 or equal to -1")
        if num_entries_per_line == -1:
            self.wrap_stats = False
        else:
            self.wrap_stats = True
        self.num_entries_per_line = num_entries_per_line
        self.add_numbers_to_titles = True
        self.separator = ' '
        self.repeat_title = repeat_title
        self.separate_lines = separate_lines
        self.comment_marker = '#'

    def _get_unique_category_name(self, name):
        if name:
            if name in self.categories.keys():
                i = 0
                while True:
                    i += 1
                    ncat = ''.join([name, '%d' % i])
                    if ncat not in self.categories.keys():
                        break
            else:
                ncat = name
        else:
            i = 0
            while True:
                i += 1
                ncat = '%d' % i
                if ncat not in self.categories.keys():
                    break
        return ncat

    def add_category(self, name=None):
        """creates a logging entry for a simulation substep of the gibbs
        sampler. Each category has its own counter, initialized to zero.
        The global category does not need to be created, it's already created by
        the init method, and its key is 'global'.
        - name: an optional name, must be string.
        Returns: a unique key to refer to this category, which will start with
        the optional name.
        """
        ncat = self._get_unique_category_name(name)
        self.categories[ncat] = {'counter': Entry('step', '%10d', 0)}
        return ncat

    def _append_to_stats(self, name, entry):
        """append to stats, or put in front if entry is a counter"""
        if name == 'counter':
            self.entries.insert(self.__counter_pos, entry)
            self.__counter_pos += 1
        else:
            self.entries.append(entry)

    def add_entry(self, key, name=None, entry=None):
        """add an entry for the statistics file
        - key: which category it belongs to (key returned by add_category)
        You must specify at least one of the two following:
        - name: a name for this entry
        - entry: an instance of the Entry class.
        Arguments:  - name only: must already have an entry by that name.
                    - entry only: name is set to the entry title and added. If
                      it didn't exist before it is stored as well.
                    - name and entry: name is used instead of the title.
                    - nothing: raises an error.
        Currently, not providing entry only makes sense for the counter since
        there is no method to create an entry without adding it to the
        statistics file.
        """
        if not entry and not name:
            raise ValueError("Should specify at least one of name or entry")
        if entry:
            if not name:
                name = entry.get_title()
            self._append_to_stats(name, entry)
            self.categories[key][name] = entry
        else:
            if not name in self.categories[key]:
                raise ValueError("entry %s:%s does not exist!" % (key, name))
            self._append_to_stats(name, self.categories[key][name])

    def update(self, key, name, value):
        """updates an entry and change its value to value"""
        if not key in self.categories:
            raise ValueError("unknown category: %s" % key)
        if not name in self.categories[key]:
            raise ValueError("unknown entry %s:%s" % (key, name))
        self.categories[key][name].set_value(value)

    def add_coordinates(self, key, name, format='raw', append=True,
            extension='pdb', hierarchies=None, restraints=None):
        """adds a placeholder for coordinates
        - format = rmf3:
            will write the whole system as provided, in rmf3 format
            - hierarchies must contain protein hierarchies
            - restraints is a list of restraints
        - format = raw:
            will write provided data as-is
            - append: whether to append to a trajectory or to write multiple
              files. With this format, a trajectory is just a string, you can
              stuff whatever you want in it. If append is False, files will be
              numbered according to the counter of their category.
            - extension: the file extension to use
              """
        if not key in self.categories:
            raise ValueError("unknown category: %s" % key)
        self.categories[key][name] = None
        if format == 'raw':
            self.coordinates.append((key, name, 'raw', (append, extension)))
        elif format == 'rmf3':
            import RMF
            import IMP.rmf
            assert hierarchies is not None
            rh = RMF.create_rmf_file(self.prefix + '_' + name + '_traj.rmf3')
            IMP.rmf.add_hierarchies(rh, hierarchies)
            if restraints:
                IMP.rmf.add_restraints(rh, restraints)
            self.coordinates.append((key, name, 'rmf3', rh))
        else:
            raise ValueError, "format can only be rmf3 or raw"

    def update_coordinates(self, key, name, value=True):
        """updates the coordinates of key:name entry. Format should match with
        the format specified at init time (raw or rmf3)
        note that setting value to None is equivalent to not calling this
        function
        """
        if not key in self.categories:
            raise ValueError("unknown category: %s" % key)
        if not name in self.categories[key]:
            raise ValueError("unknown coordinates %s:%s" % (key, name))
        self.categories[key][name] = value

    def increment_counter(self, key, value):
        """increments the counter of category 'key' by 'value' steps."""
        if not key in self.categories:
            raise ValueError("unknown category: %s" % key)
        cnt = self.categories[key]['counter']
        cnt.set_value(cnt.get_raw_value() + value)

    def get_entry_category(self, entry):
        # ugly, find something better
        for cat in self.categories:
            if entry in self.categories[cat].values():
                return cat

    def format_titles(self):
        titles = []
        for (i, entry) in enumerate(self.entries):
            if self.add_numbers_to_titles:
                if self.num_entries_per_line>0:
                    title = '%d:' % ((i % self.num_entries_per_line) + 1)
                else:
                    title = '%d:' % (i + 1)
            else:
                title = ''
            cat = self.get_entry_category(entry)
            ti = entry.get_title()
            title += '%s:%s' % (cat, ti)
            titles.append(title)
        return titles

    def get_formatted_entries(self):
        return [ent.get_value() for ent in self.entries]

    def should_wrap_line(self, pos, line):
        if self.wrap_stats:
            num = self.num_entries_per_line
            if pos % num == num - 1 and pos != len(line) - 1:
                return True
        return False

    def prepare_line(self, line, marker='L'):
        out = marker + '1'
        out += self.separator
        for i, tok in enumerate(line):
            out += tok
            ln = 2 + (i / self.num_entries_per_line)
            if self.should_wrap_line(i, line):
                out += '\n%s%d' % (marker, ln)
            out += self.separator
        # don't add a newline if we just did
        if not self.should_wrap_line(i, line):
            out += '\n'
        return out

    def compress_file(self, fname):
        gz = gzip.open(fname + '.gz', 'wb')
        fl = open(fname, 'rb')
        gz.writelines(fl)
        gz.close()
        fl.close()
        os.system('rm %s' % fname)

    def new_stage(self, name):
        fl = open(self.statfile, 'a')
        fl.write("### STAGE %s\n" % name)
        fl.close()

    def write_stats(self):
        """Writes statistics to the stats file and writes/appends
        trajectories. Only does that if the global step matches
        the output rate. Trajectories are written more sparsely, see trajrate.
        Returns: True if data was written, False if not.
        """
        stepno = self.categories['global']['counter'].get_raw_value()
        if stepno % self.rate != 0:
            return False
        # stats file
        fl = open(self.statfile, 'a')
        # do title if necessary
        if self.write_title:
            self.write_title = False
            titles = self.format_titles()
            fl.write(self.prepare_line(titles, marker=self.comment_marker))
        elif self.repeat_title > 0:
            if (stepno / self.rate) % self.repeat_title == 0:
                self.write_title = True
        # write stats
        entries = self.get_formatted_entries()
        fl.write(self.prepare_line(entries))
        if self.separate_lines:
            fl.write('*' * 80 + '\n')
        fl.close()
        # write trajs
        if stepno % (self.rate * self.trajrate) != 0:
            return True
        for key, name, format, args in self.coordinates:
            if self.categories[key][name] is None:
                continue
            if format == 'raw':
                do_append, extension = args
                if do_append:
                    pdbname = self.prefix + '_traj.' + extension
                    if self.compress > 0 and stepno % self.compress == 0:
                        newname = "%s_traj_%d.%s" % (self.prefix, stepno, extension)
                        os.system('mv %s %s' % (pdbname, newname))
                        self.compress_file(newname)
                    fl = open(pdbname, 'a')
                else:
                    num = self.categories[key]['counter'].get_raw_value()
                    fl = open(self.prefix + ('_%s_%010d.%s' % (name, num, extension)), 'w')
                fl.write(self.categories[key][name])
                fl.close()
            elif format == 'rmf3':
                import IMP.rmf
                IMP.rmf.save_frame(args)
                args.flush()
            else:
                raise RuntimeError
            self.categories[key][name] = None
        return True
