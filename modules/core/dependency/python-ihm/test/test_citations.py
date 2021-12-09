import utils
import os
import unittest

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.citations


class Tests(unittest.TestCase):

    def test_citations(self):
        """Test citations module"""
        pmi = ihm.citations.pmi
        self.assertEqual(pmi.pmid, '31396911')
        self.assertEqual(pmi.doi, '10.1007/978-1-4939-9608-7_15')


if __name__ == '__main__':
    unittest.main()
