'''
Handles combination manipulations for building the list of annotation to enumerate over
Each annotation maps a subset of component ids to the features
'''
import sys, probstat
class CombinationIterator:
    def __init__(self, l):
        l_=[]
        for i in l:
            l_.append(range(i))
        self.combs= probstat.Cartesian(l_)
        self.id=0
    def __iter__(self):
        self.id=0
        return self                 # simplest iterator creation
    def next(self):
        if self.id == len(self.combs):         # threshhold terminator
            raise StopIteration     # end of iteration
        else:
            self.id = self.id+1
            return self.combs[self.id-1]
