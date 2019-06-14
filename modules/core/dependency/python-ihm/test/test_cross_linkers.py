import utils
import os
import unittest
import sys

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.cross_linkers

class Tests(unittest.TestCase):

    def test_cross_linkers(self):
        """Test cross_linkers module"""
        d = ihm.cross_linkers.dss
        self.assertEqual(d.auth_name, 'DSS')
        self.assertEqual(d.smiles,
                         'C1CC(=O)N(C1=O)OC(=O)CCCCCCC(=O)ON2C(=O)CCC2=O')


if __name__ == '__main__':
    unittest.main()
