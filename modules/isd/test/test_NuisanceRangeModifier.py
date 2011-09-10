#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import Scale,Nuisance,Switching,NuisanceRangeModifier

#unit testing framework
import IMP.test

class TestJeffreysRestraint(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(0)
        self.m = IMP.Model()

    def test_simple(self):

if __name__ == '__main__':
    IMP.test.main()



