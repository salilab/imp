#!/usr/bin/env python

#imp general
import IMP
import IMP.core

#our project
from IMP.isd import Nuisance

#unit testing framework
import IMP.test

class TestNuisanceParam(IMP.test.TestCase):
    """tests nuisance parameters"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)

    def test_String(self):
        "a parameter cannot take other things than numbers as input"
        self.assertRaises(TypeError, self.sigma.set_nuisance,"a")
        self.assertRaises(TypeError, self.sigma.set_nuisance,(1,2))
        self.assertRaises(TypeError, self.sigma.set_nuisance,[1,2])

    def test_Default(self):
        "default value should be 1"
        print self.sigma.get_nuisance()
        type(self.sigma.get_nuisance())
        self.assertEqual(float(self.sigma.get_nuisance()),1.0)

    def test_Set(self):
        "set returns nothing"
        self.assertEqual(self.sigma.set_nuisance(2),None)

    def test_GetSet(self):
        "tests get and set (sanity checks)"
        for si in range(1,100):
            self.sigma.set_nuisance(si)
            self.assertEqual(self.sigma.get_nuisance(),si)

if __name__ == '__main__':
    IMP.test.main()




        
        
        




        

