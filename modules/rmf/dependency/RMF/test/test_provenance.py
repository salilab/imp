from __future__ import print_function
import sys
import RMF
import unittest
import os

class Tests(unittest.TestCase):

    def test_provenance(self):
        """Test the Provenance decorator"""
        for suffix in RMF.suffixes:
            fname = RMF._get_temporary_file_path("provenance." + suffix)
            self._create(fname)
            self._read(fname)

    def _add_provenance_nodes(self, rmf, rt):
        """Add *Provenance nodes under rt. Return the root."""
        strucpf = RMF.StructureProvenanceFactory(rmf)
        samppf = RMF.SampleProvenanceFactory(rmf)
        filtpf = RMF.FilterProvenanceFactory(rmf)
        clustpf = RMF.ClusterProvenanceFactory(rmf)

        clust_node = rt.add_child("clustering", RMF.PROVENANCE)
        clust = clustpf.get(clust_node)
        clust.set_members(10)

        filt_node = clust_node.add_child("filtering", RMF.PROVENANCE)
        filt = filtpf.get(filt_node)
        filt.set_method("Total score")
        filt.set_threshold(250.)
        filt.set_frames(4)

        samp_node = filt_node.add_child("sampling", RMF.PROVENANCE)
        samp = samppf.get(samp_node)
        samp.set_method("Monte Carlo")
        samp.set_frames(100)
        samp.set_iterations(10)
        samp.set_replicas(8)

        struc_node = samp_node.add_child("structure", RMF.PROVENANCE)
        struc = strucpf.get(struc_node)
        struc.set_filename('foo.pdb')
        struc.set_chain('X')

        return clust_node

    def _create(self, fname):
        rmf = RMF.create_rmf_file(fname)
        rmf.add_frame('zero', RMF.FRAME)
        rt = rmf.get_root_node()

        c1 = rt.add_child("c1", RMF.REPRESENTATION)
        c0 = self._add_provenance_nodes(rmf, c1)
        self._check_provenance_nodes(rmf, c0)

    def _read(self, fname):
        rmf = RMF.open_rmf_file_read_only(fname)
        rt = rmf.get_root_node()
        c1, = rt.get_children()
        c0, = c1.get_children()
        self._check_provenance_nodes(rmf, c0)

    def _check_provenance_nodes(self, rmf, prov_root):
        strucpf = RMF.StructureProvenanceFactory(rmf)
        filtpf = RMF.FilterProvenanceFactory(rmf)
        samppf = RMF.SampleProvenanceFactory(rmf)
        clustpf = RMF.ClusterProvenanceFactory(rmf)

        self.assertTrue(clustpf.get_is(prov_root))
        clust = clustpf.get(prov_root)
        self.assertEqual(clust.get_members(), 10)

        filt_node = prov_root.get_children()[0]
        self.assertTrue(filtpf.get_is(filt_node))
        filt = filtpf.get(filt_node)
        self.assertEqual(filt.get_method(), "Total score")
        self.assertAlmostEqual(filt.get_threshold(), 250., delta=1e-4)
        self.assertEqual(filt.get_frames(), 4)

        samp_node = filt_node.get_children()[0]
        self.assertTrue(samppf.get_is(samp_node))
        samp = samppf.get(samp_node)
        self.assertEqual(samp.get_frames(), 100)
        self.assertEqual(samp.get_iterations(), 10)
        self.assertEqual(samp.get_replicas(), 8)
        self.assertEqual(samp.get_method(), "Monte Carlo")

        struc_node = samp_node.get_children()[0]
        self.assertTrue(strucpf.get_is(struc_node))
        struc = strucpf.get(struc_node)
        self.assertEqual(struc.get_filename(), 'foo.pdb')
        self.assertEqual(struc.get_chain(), 'X')

if __name__ == '__main__':
    unittest.main()
