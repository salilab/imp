import utils
import os
import unittest
import sys

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.analysis

class Tests(unittest.TestCase):

    def test_filter_step(self):
        """Test analysis FilterStep class"""
        s = ihm.analysis.FilterStep(feature='RMSD', num_models_begin=42,
                                    num_models_end=5)
        self.assertEqual(s.type, 'filter')
        self.assertEqual(s.feature, 'RMSD')
        self.assertEqual(s.num_models_begin, 42)
        self.assertEqual(s.num_models_end, 5)

    def test_cluster_step(self):
        """Test analysis ClusterStep class"""
        s = ihm.analysis.ClusterStep(feature='RMSD', num_models_begin=42,
                                     num_models_end=5)
        self.assertEqual(s.type, 'cluster')
        self.assertEqual(s.feature, 'RMSD')
        self.assertEqual(s.num_models_begin, 42)
        self.assertEqual(s.num_models_end, 5)

    def test_rescore_step(self):
        """Test analysis RescoreStep class"""
        s = ihm.analysis.RescoreStep(feature='RMSD', num_models_begin=42,
                                     num_models_end=5)
        self.assertEqual(s.type, 'rescore')
        self.assertEqual(s.feature, 'RMSD')
        self.assertEqual(s.num_models_begin, 42)
        self.assertEqual(s.num_models_end, 5)

    def test_step(self):
        """Test analysis Step class"""
        s = ihm.analysis.Step(feature='RMSD', num_models_begin=42,
                              num_models_end=5)
        self.assertEqual(s.type, 'other')
        self.assertEqual(s.feature, 'RMSD')
        self.assertEqual(s.num_models_begin, 42)
        self.assertEqual(s.num_models_end, 5)

    def test_empty_step(self):
        """Test analysis EmptyStep class"""
        s = ihm.analysis.EmptyStep()
        self.assertEqual(s.type, 'none')
        self.assertEqual(s.feature, 'none')
        self.assertEqual(s.num_models_begin, None)
        self.assertEqual(s.num_models_end, None)

    def test_analysis(self):
        """Test Analysis class"""
        a = ihm.analysis.Analysis()
        self.assertEqual(a.steps, [])


if __name__ == '__main__':
    unittest.main()
