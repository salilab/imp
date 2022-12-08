import IMP.test
import os
import sys


class EMToolsApplicationTest(IMP.test.ApplicationTestCase):

    def test_mol2pca(self):
        """Simple test of mol2pca application"""
        p = self.run_python_application(
            'mol2pca', [self.get_input_file_name('miniapp.pdb'), 'mini.cmm'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        with open('mini.cmm') as fh:
            lines = fh.readlines()
        markers = [x for x in lines if 'marker id=' in x]
        links = [x for x in lines if 'link id1=' in x]
        self.assertEqual(len(markers), 6)
        self.assertEqual(len(links), 3)
        os.unlink('mini.cmm')

    def test_map2pca(self):
        """Simple test of map2pca application"""
        p = self.run_python_application('map2pca',
                                        [self.get_input_file_name(
                                            'mini.mrc'), '1.0',
                                         'mini.cmm'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        with open('mini.cmm') as fh:
            lines = fh.readlines()
        markers = [x for x in lines if 'marker id=' in x]
        links = [x for x in lines if 'link id1=' in x]
        self.assertEqual(len(markers), 6)
        self.assertEqual(len(links), 3)
        os.unlink('mini.cmm')

    def test_simulate_density(self):
        """Simple test of simulate_density_from_pdb application"""
        p = self.run_python_application('simulate_density_from_pdb',
                                        [self.get_input_file_name(
                                            'miniapp.pdb'), 'mini.mrc',
                                         '1.0', '4.0'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        os.unlink('mini.mrc')

    def test_resample_density(self):
        """Simple test of resample_density application"""
        p = self.run_python_application('resample_density',
                                        [self.get_input_file_name(
                                            'mini.mrc'), '1',
                                         'out.mrc'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        os.unlink('out.mrc')

    def test_estimate_threshold(self):
        """Simple test of estimate_threshold application"""
        p = self.run_python_application(
                  'estimate_threshold_from_molecular_mass',
                  [self.get_input_file_name('mini.mrc'), '400'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)


if __name__ == '__main__':
    IMP.test.main()
