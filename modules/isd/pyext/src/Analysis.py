#!/usr/bin/env python

import matplotlib.pyplot
from numpy import *
from IMP.isd.History import History

class Analysis:
    """Class that produces analysis-related output, and is able to parse the
    output of a file produced by the Statistics class.
    """

    def create_entry(self, h, lineno, el):
        """adds a new entry to the hierarchy by parsing a title entry"""
        if lineno == len(self.correspondences):
            self.correspondences.append([])
        entryno, cat, name = el.split(':')
        entryno=int(entryno)-1
        if len(self.correspondences[lineno]) == entryno:
            self.correspondences[lineno].append((cat,name))
        else:
            self.correspondences[lineno][entryno]=(cat,name)
        h.create_entry(cat,name)

    def add_data(self, h, lineno, colno, data):
        """adds data point to hierarchy"""
        cat,name = self.correspondences[lineno][colno]
        if data.isdigit():
            data = int(data)
        else:
            try:
                data = float(data)
            except ValueError:
                pass
        h.add_data(cat,name,data)

    def read_variables(self, statfile):
        """reads a *_stats.txt file and returns variables present in it"""
        h=History(statfile)
        #read title and setup history
        self.correspondences=[]
        for line in open(statfile):
            if line.startswith('*'):
                continue
            tokens=line.split()
            if not tokens[0][1].isdigit():
                continue
            lineno = int(tokens[0][1:])-1
            if line.startswith('#'):
                for el in tokens[1:]:
                    self.create_entry(h, lineno, el)
                continue
            break
        return self.correspondences

    def read_AMBER_variables(self, statfile):
        """reads an AMBER mden file and returns variables present in it"""
        h=History(statfile)
        #read title and setup history
        self.correspondences=[]
        oldnum=-1
        for line in open(statfile):
            tokens=line.split()
            lineno = int(tokens[0][1:])
            if lineno < oldnum:
                break
            oldnum = lineno
            for i,el in enumerate(tokens[1:]):
                self.create_entry(h, lineno, '%d:global:%s' % (i+1,el))
        return self.correspondences

    def read_AMBER_stats(self, statfile):
        """reads an AMBER mden file and returns a History instance"""
        h=History(statfile)
        #read title and setup history
        read_title=True
        oldnum=-1
        self.correspondences=[]
        for line in open(statfile):
            tokens=line.split()
            lineno = int(tokens[0][1:])
            if lineno < oldnum and read_title:
                read_title=False
            oldnum = lineno
            if read_title:
                for i,el in enumerate(tokens[1:]):
                    self.create_entry(h, lineno, '%d:global:%s' % (i+1,el))
                continue
            #from here on, the line contains data
            for i, el in enumerate(tokens[1:]):
                self.add_data(h, lineno, i, el)
        #h.sanity_check()
        return h

    def read_stats(self, statfile):
        """reads a *_stats.txt file and returns a History instance"""
        h=History(statfile)
        #read title and setup history
        read_title=True
        self.correspondences=[]
        for line in open(statfile):
            if line.startswith('*'):
                continue
            tokens=line.split()
            if not tokens[0][1].isdigit():
                continue
            lineno = int(tokens[0][1:])-1
            if line.startswith('#'):
                if read_title:
                    for el in tokens[1:]:
                        self.create_entry(h, lineno, el)
                continue
            elif read_title:
                read_title = False
            #from here on, the line starts with 'L'
            for i, el in enumerate(tokens[1:]):
                self.add_data(h, lineno, i, el)
        #h.sanity_check()
        return h

    def plot(self, h, *datums, **kwargs):
        """plots datum (cat,name) from hierarchy h, optionnally specifying a
        range. To plot multiple data at the same time, add them sequentially.
        Takes x axis from the 'step' entry of the first datum. TODO.
        """
        data=[array(h.get_data(cat,name), dtype=float) \
                for (cat, name) in datums]
        x = h.get_data(datums[0][0],'step')
        toplot = []
        for i in range(len(data)):
            toplot.extend([x,data[i]])
        matplotlib.pyplot.plot(*data, **kwargs)
        matplotlib.pyplot.grid(True)
        matplotlib.pyplot.legend()
        matplotlib.pyplot.show()

    def histogram(self, h, *datums, **kwargs):
        """plots histogram of datum (cat,name) from hierarchy h, optionnally
        specifying a range. To plot multiple data at the same time, add them
        sequentially.
        """
        data=[array(h.get_data(*dat), dtype=float) \
                for dat in datums]
        matplotlib.pyplot.hist(*data, **kwargs)
        matplotlib.pyplot.grid(True)
        matplotlib.pyplot.legend()
        matplotlib.pyplot.show()

    def dump(self, h, dest, *args):
        """"dump float data from history h to file dest.
        args can be either strings corresponding to categories, or tuples
        corresponding to entries of a certain category. Only one counter will be
        output for the whole dump, it corresponds to the counter of the first
        entry's category. You can always specify additional counters if needed.
        """
        #parse args
        cats=[]
        names=[]
        for arg in args:
            if type(arg) == str:
                #get rid of counter
                ent=h.get_entries(arg)[1:]
                names.extend(ent)
                cats.extend([arg]*len(ent))
            else:
                #argument should be (cat, entry)
                names.append(arg[1])
                cats.append(arg[0])
        #write data
        steps=h.get_data(cats[0],'step')
        fl=open(dest,'w')
        fl.write("# %s:step\t" % cats[0])
        fl.write('\t'.join(['%s:%s' % (i,j) for (i,j) in zip(cats,names)]))
        fl.write('\n')
        data=[h.get_data(i,j) for (i,j) in zip(cats,names)]
        for i,st in enumerate(steps):
            fl.write("%10d\t" % st)
            for j in data:
                fl.write('%10f\t' % j[i])
            fl.write('\n')
        fl.close()





if __name__ == '__main__':

    import sys
    a=Analysis()
    h=a.read_stats(sys.argv[1])
    h.toc()
    matplotlib.pyplot.ion() #interactive
