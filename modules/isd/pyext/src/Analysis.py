#!/usr/bin/env ptyhon

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



                
if __name__ == '__main__':

    import sys
    a=Analysis()
    h=a.read_stats(sys.argv[1])
    h.toc()
    
        

