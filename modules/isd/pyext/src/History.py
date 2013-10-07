#!/usr/bin/env python

"""@namespace IMP.isd.History
   Classes to store output from replicas.
"""

import numpy
import sys

class History:
    """Class that contains the output of one replica, used by the
    Analysis class.
    """
    def __init__(self, filename):
        self.filename = filename
        self.data={}

    def create_category(self, name):
        self.data[name]={}

    def create_entry(self, cat, name):
        if not cat in self.data:
            self.create_category(cat)
        self.data[cat][name]=[]

    def add_data(self, cat, name, data):
        self.data[cat][name].append(data)

    def sanity_check(self):
        """checks if all entries have same length and are of constant type"""
        goodlen=None
        had_warnings = False
        for cat in self.data:
            for name in self.data[cat]:
                if goodlen is None:
                    goodlen = len(self.data[cat][name])
                if len(self.data[cat][name]) != goodlen:
                    print "category %s entry %s : length %s expected, got %s"\
                            % (cat, name, goodlen, len(self.data[cat][name]))
                    break
                goodtype = type(self.data[cat][name][0])
                for ent in self.data[cat][name]:
                    if type(ent) != goodtype:
                        print "category %s entry %s : %s expected, got %s"\
                                % (cat, name, goodtype, type(ent))
                        break

    def get_data(self, cat, name):
        return self.data[cat][name]

    def get_categories(self):
        i=sorted(self.data.keys())
        if 'step' in i:
            i.remove('step')
            i=['step']+i
        return i

    def get_entries(self,cat):
        i=sorted(self.data[cat].keys())
        if 'step' in i:
            i.remove('step')
            i=['step']+i
        return i

    def remove_NAs(self):
        for cat in self.data:
            for name in self.data[cat]:
                self.data[cat][name] = [i for i in self.data[cat][name] \
                        if i != 'N/A']

    def toc(self, out=sys.stdout):
        """print the "table of contents" of this History
        tendency is a comparison of the last 200 frames, and whether it goes up
        or down.
        mean100 is the average over the last 100 frames
        """
        if not type(out) == file:
            out=open(out,'w')
        out.write("category\tkey_name\tn_frames\ttype\taverage\tstd\tmean100\ttendency\n")
        for cat in self.data:
            out.write(cat+'\n')
            for name in self.data[cat]:
                ent=self.data[cat][name]
                tp=type(ent[-1])
                if tp==float:
                    tp='float'
                elif tp==int:
                    tp='int'
                if tp=='float':
                    try:
                        ent=numpy.array(ent)
                        avg=numpy.mean(ent)
                        avg100=numpy.mean(ent[-100:])
                        avg200=numpy.mean(ent[-200:-100])
                        st=numpy.std(ent)
                        st200=numpy.std(ent[-200])
                        if st200 ==0 or abs(avg200 - avg100)/st200 > 3:
                            if avg200>avg100:
                                tend='\\'
                            else:
                                tend='/'
                        else:
                            tend='--'
                        out.write("\t%20s\t%12d\t%20s\t%10f\t%10f\t%10f\t%5s\n" % \
                                (name, len(ent),tp,avg,st,avg100,tend))
                        continue
                    except:
                        pass
                out.write("\t%20s\t%12d\t%20s\n" % (name, len(ent),tp))
