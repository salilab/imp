import utils
import os
import unittest
import sys

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.protocol

class Tests(unittest.TestCase):

    def test_step(self):
        """Test protocol Step class"""
        s = ihm.protocol.Step(assembly='foo', dataset_group='bar', method='baz')
        self.assertEqual(s.name, None)
        self.assertEqual(s.method, 'baz')

    def test_protocol(self):
        """Test Protocol class"""
        s = ihm.protocol.Protocol(name='foo')
        self.assertEqual(s.name, 'foo')
        self.assertEqual(s.steps, [])
        self.assertEqual(s.analyses, [])


if __name__ == '__main__':
    unittest.main()
