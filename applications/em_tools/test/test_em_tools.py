import IMP.test
import os
import sys

class EMToolsApplicationTest(IMP.test.ApplicationTestCase):
    def test_mol2pca(self):
        """Simple test of mol2pca application"""
        p = self.run_python_application('mol2pca.py',
                          [self.get_input_file_name('mini.pdb'), 'mini.cmm'])
        p.stdin.close()
        out = p.stdout.readlines()
        ret, err = p.wait()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(ret, err)
        lines = open('mini.cmm').readlines()
        markers = [x for x in lines if 'marker id=' in x]
        links = [x for x in lines if 'link id1=' in x]
        self.assertEqual(len(markers), 6)
        self.assertEqual(len(links), 3)
        os.unlink('mini.cmm')

    def test_map2pca(self):
        """Simple test of map2pca application"""
        p = self.run_python_application('map2pca.py',
                          [self.get_input_file_name('mini.mrc'), '1.0',
                           'mini.cmm'])
        p.stdin.close()
        out = p.stdout.readlines()
        ret, err = p.wait()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(ret, err)
        lines = open('mini.cmm').readlines()
        markers = [x for x in lines if 'marker id=' in x]
        links = [x for x in lines if 'link id1=' in x]
        self.assertEqual(len(markers), 6)
        self.assertEqual(len(links), 3)
        os.unlink('mini.cmm')

    def test_simulate_density(self):
        """Simple test of simulate_density_from_pdb application"""
        p = self.run_python_application('simulate_density_from_pdb.py',
                          [self.get_input_file_name('mini.pdb'), 'mini.mrc',
                           '1.0', '4.0'])
        p.stdin.close()
        out = p.stdout.readlines()
        ret, err = p.wait()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(ret, err)
        os.unlink('mini.mrc')

    def test_resample_density(self):
        """Simple test of resample_density application"""
        p = self.run_python_application('resample_density.py',
                          [self.get_input_file_name('mini.mrc'), '1',
                           'out.mrc'])
        p.stdin.close()
        out = p.stdout.readlines()
        ret, err = p.wait()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(ret, err)
        os.unlink('out.mrc')

if __name__ == '__main__':
    IMP.test.main()
