#!/usr/bin/env python

import IMP
import IMP.isd

class Entry:
    """The entry class represents a column in the statistics file. 
    Its components are a title, a format and an additional object.
    It's what gets written to the statistics file in a column.
    - title: the title of the column
    - format: a wisely chosen format string (see below)
    - something: either something that can be formattable, a string, a number
      etc. This is a static entry. In that case all remaining arguments are
      discarded and get_value returns the formatted string : format % something.
      If something is a function, this is a dynamic entry, and the format 
      string is used on the result of the function call 
      something(*args,**kwargs).
    """
    def __init__(self, title, fmt, something, *args, **kwargs):
        self.title = title
        self.format = fmt
        self.is_function = callable(something)
        if self.is_function:
            self.function = something
            self.args = args
            self.kwargs = kwargs
        else:
            self.value = something
        self.was_updated_since_last_get = False

    def __repr__(self):
        if self.is_function:
            return "Entry('%s', '%s', f(...))" % (self.title, self.format)
        else:
            return "Entry('%s', '%s', %s)" % (self.title, self.format,
                    self.value)


    def get_title(self):
        return self.title

    def get_raw_value(self):
        if self.is_function:
            return self.function(*self.args, **self.kwargs)
        else:
            self.was_updated_since_last_get = False
            return self.value

    def get_value(self):
        try:
            return self.format % self.get_raw_value()
        except TypeError:
            return "N/A"

    def set_value(self, val):
        if self.is_function:
            raise RuntimeError, \
                    "Can only set_value on static entries."
        self.value = val
        self.was_updated_since_last_get = True

    def get_was_updated(self):
        return self.is_function or self.was_updated_since_last_get

class Statistics:
    """Statistics gathering and printing class for ISD gibbs sampling.
    Also manages the restart file (TODO).
    - prefix: all outputted files will have this prefix
    - rate: print statistics every so many gibbs sampling steps
    - statfile: suffix of the statistics file
    - append: whether to append to a trajectory or to write multiple files.
    For the statistics class, a trajectory is just a string, you can stuff
    whatever you want in it. If append is False, files will be numbered
    according to the counter of their category.
    TODO: check if everything was updated nicely
    """

    def __init__(self, prefix='r01', rate=1, statfile='_stats.txt', 
            append=True):
        self.prefix = prefix
        self.rate=rate
        self.statfile=prefix+statfile
        self.append=append
        #list of the things that will be printed to the stats file, in order.
        self.entries=[]
        #list of coordinate entries
        self.coordinates=[]
        #internal
        self.__counter_pos = 0
        #the same entries but sorted by category
        self.categories={}
        #create the global category along with the global counter
        self.add_category('global')
        #that makes it the first entry of the table.
        self.add_entry('global',name='counter')
        #output-specific flags
        self.write_title=True
        self.num_entries_per_line = 5
        self.add_numbers_to_titles = True
        self.separator=' '

    def _get_unique_category_name(self, name):
        if name:
            if name in self.categories.keys():
                i=0
                while True:
                    i += 1
                    ncat=''.join([name,'%d' % i])
                    if ncat not in self.categories.keys():
                        break
            else:
                ncat = name
        else:
            i=0
            while True:
                i+=1
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
        self.categories[ncat]={'counter':Entry('step', '%10d', 0)}
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
            raise ValueError, "Should specify at least one of name or entry"
        if entry:
            if not name:
                name = entry.get_title()
            self._append_to_stats(name, entry)
            self.categories[key][name]=entry
        else:
            if not name in self.categories[key]:
                raise ValueError, "entry %s:%s does not exist!" % (key,name)
            self._append_to_stats(name, self.categories[key][name])

    def update(self, key, name, value):
        """updates an entry and change its value to value"""
        if not key in self.categories:
            raise ValueError, "unknown category: %s" % key
        if not name in self.categories[key]:
            raise ValueError, "unknown entry %s:%s" % (key,name)
        self.categories[key][name].set_value(value)

    def add_coordinates(self, key, name):
        """adds a placeholder for coordinates"""
        if not key in self.categories:
            raise ValueError, "unknown category: %s" % key
        self.categories[key][name]=None
        self.coordinates.append((key,name))

    def update_coordinates(self, key, name, value):
        """updates the coordinates of key:name entry. Format should match with
        the format specified at init time (pdb or cdf)
        """
        if not key in self.categories:
            raise ValueError, "unknown category: %s" % key
        if not name in self.categories[key]:
            raise ValueError, "unknown coordinates %s:%s" % (key,name)
        self.categories[key][name]=value

    def increment_counter(self, key, value):
        """increments the counter of category 'key' by 'value' steps."""
        if not key in self.categories:
            raise ValueError, "unknown category: %s" % key
        cnt=self.categories[key]['counter']
        cnt.set_value(cnt.get_raw_value() + value)

    def add_mc_category(self, name='mc', coord='particle'):
        """shortcut for a frequent series of operations on MC simulations'
        statistics. Creates an entry for acceptance, stepsize and one
        coordinate set printed in the statistics file.
        """
        #create category
        mc_key = self.add_category(name=name)
        #giving None as argument is a way to create a static entry.
        self.add_entry(mc_key, entry=Entry('acceptance', '%10f', None))
        self.add_entry(mc_key, entry=Entry('stepsize', '%10f', None))
        #special call to add coordinates to be dumped
        self.add_entry(mc_key, entry=Entry(coord, '%10f', None))
        #add the counter to the output
        self.add_entry(mc_key, name='counter')
        return mc_key

    def add_md_category(self, name='md', coord='protein'):
        """shortcut for a frequent series of operations on MD simulations'
        statistics. Creates an entry for target temp, instantaneous temp,
        kinetic energy, and one set of coordinates called 'protein' by 
        default.
        """
        #create category
        md_key = self.add_category(name=name)
        #giving None as argument is a way to create a static entry.
        self.add_entry(md_key, entry=Entry('target_temp', '%10f', None))
        self.add_entry(md_key, entry=Entry('instant_temp', '%10f', None))
        self.add_entry(md_key, entry=Entry('E_kinetic', '%10f', None))
        #special call to add coordinates to be dumped
        self.add_coordinates(md_key, coord)
        #add the counter to the output
        self.add_entry(md_key, name='counter')
        return md_key

    def add_hmc_category(self, name='hmc', coord='protein'):
        """shortcut for a frequent series of operations on HMC simulations'
        statistics. Adds acceptance, number of MD steps and a trajectory for
        a protein.
        """
        #create category
        hmc_key = self.add_category(name=name)
        #giving None as argument is a way to create a static entry.
        self.add_entry(hmc_key, entry=Entry('acceptance', '%10f', None))
        self.add_entry(hmc_key, entry=Entry('n_md_steps', '%10d', None))
        #special call to add coordinates to be dumped
        self.add_coordinates(hmc_key, coord)
        #add the counter to the output
        self.add_entry(hmc_key, name='counter')
        return hmc_key

    def get_entry_category(self, entry):
        #ugly, find something better
        for cat in self.categories:
            if entry in self.categories[cat].values():
                return cat

    def format_titles(self):
        titles = []
        for (i,entry) in enumerate(self.entries):
            if self.add_numbers_to_titles:
                title='%d:' % ( (i % self.num_entries_per_line) + 1 )
            else:
                title=''
            cat = self.get_entry_category(entry)
            ti = entry.get_title()
            title += '%s:%s' % (cat,ti)
            titles.append(title)
        return titles

    def get_formatted_entries(self):
        return [ent.get_value() for ent in self.entries]

    def prepare_line(self, line):
        out = 'L1'
        out += self.separator
        for i,tok in enumerate(line):
            out += tok
            ln = 2 + (i / self.num_entries_per_line)
            if i % self.num_entries_per_line == self.num_entries_per_line - 1:
                out += '\nL%d' % ln
            out += self.separator
        #don't add a newline if we just did
        if i % self.num_entries_per_line != self.num_entries_per_line - 1:
            out += '\n'
        return out

    def write_stats(self):
        """Writes statistics to the stats file and writes/appends
        trajectories. Only does that if the global step matches 
        the output rate.
        Returns: True if data was written, False if not.
        """
        if self.categories['global']['counter'].get_raw_value() % self.rate != 0:
            return False
        #stats file
        fl=open(self.statfile, 'a')
        #do title if necessary
        if self.write_title:
            self.write_title = False
            titles = self.format_titles()
            fl.write(self.prepare_line(titles))
        #write stats
        entries = self.get_formatted_entries()
        fl.write(self.prepare_line(entries))
        fl.close()
        #write trajs
        for key,name in self.coordinates:
            if self.append:
                fl=open(self.prefix+'_traj.pdb', 'a')
            else:
                num=self.categories[key]['counter'].get_raw_value()
                fl=open(self.prefix + ('_%s_%10d.pdb' % (name,num)), 'w')
            fl.write(self.categories[key][name])
            fl.close()
        return True

