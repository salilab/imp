import IMP.test
import sys
import os
import re
import RMF
import IMP.rmf


class Tests(IMP.test.ApplicationTestCase):

    def test_usage(self):
        """Test pdb_rmf usage"""
        p = self.run_application('pdb_rmf', [])
        out, err = p.communicate()
        self.assertEqual(p.returncode, 1)
        self.assertIn('Make an RMF file from a PDB.', err)
        self.assertIn('Wrong number of positional arguments', err)

    def test_help(self):
        """Test pdb_rmf help"""
        p = self.run_application('pdb_rmf', ['-h'])
        out, err = p.communicate()
        self.assertApplicationExitedCleanly(p.returncode, err)
        self.assertIn('Make an RMF file from a PDB.', err)

    def test_simple(self):
        """Test pdb_rmf on a simple PDB"""
        pdb = self.get_input_file_name('mini.pdb')
        rmf = 'test.rmf'
        p = self.run_application('pdb_rmf', [pdb, rmf])
        out, err = p.communicate()
        self.assertApplicationExitedCleanly(p.returncode, err)
        f = RMF.open_rmf_file_read_only(rmf)
        m = IMP.Model()
        h = IMP.rmf.create_hierarchies(f, m)
        self.assertEqual(len(h), 1)
        self.assertEqual(len(IMP.atom.get_leaves(h[0])), 38)
        del f
        os.unlink(rmf)


if __name__ == '__main__':
    IMP.test.main()
