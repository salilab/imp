#!/usr/bin/env ptyhon

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

    def remove_NAs(self):
        for cat in self.data:
            for name in self.data[cat]:
                self.data[cat][name] = [i for i in self.data[cat][name] \
                        if i != 'N/A']

    def toc(self):
        """print the "table of contents" of this History"""
        for cat in self.data:
            print cat
            for name in self.data[cat]:
                ent=self.data[cat][name]
                print "\t%20s\t%12d\t%20s" % (name, len(ent),type(ent[-1]))

