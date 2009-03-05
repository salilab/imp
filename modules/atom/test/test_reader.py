import unittest
import IMP
import IMP.test
import IMP.atom
import IMP.core
import os
import time

class PDBReadTest(IMP.test.TestCase):

    def test_saxs(self):
        """Check reading a pdb with one protein"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                                        m, IMP.atom.NonWaterSelector())
        print "number of particles"
        print m.get_number_of_particles()
        #IMP.core.show_molecular_hierarchy(mp)

        m2 = IMP.Model()
        mp= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                                             m2, IMP.atom.CAlphaSelector())
        print "number of particles"
        print m2.get_number_of_particles()

if __name__ == '__main__':
    unittest.main()
