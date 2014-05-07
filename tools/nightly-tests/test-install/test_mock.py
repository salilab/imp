import unittest
import os
import subprocess

mock_config = os.environ['MOCK_CONFIG']


class IMPMockTests(unittest.TestCase):

    def test_modules_installed(self):
        """Check modules included in the RPM"""
        # RHEL systems don't include cgal, em2d; Fedora does
        if mock_config.startswith('fedora'):
            import IMP.cgal
            import IMP.em2d
        else:
            self.assertRaises(ImportError, __import__, 'IMP.cgal')
            self.assertRaises(ImportError, __import__, 'IMP.em2d')
        # The scratch module should not be included
        self.assertRaises(ImportError, __import__, 'IMP.scratch')
        # We build our own Eigen on RHEL, so everyone should have IMP.isd
        import IMP.isd
        # Check that most other modules (particularly those with many
        # dependencies) are present
        import IMP.cnmultifit
        import IMP.domino
        import IMP.em
        import IMP.gsl
        import IMP.kinematics
        import IMP.kmeans
        import IMP.misc
        import IMP.multifit
        import IMP.parallel
        import IMP.rmf
        import IMP.rotamer
        import IMP.saxs
        import IMP.statistics
        import IMP.symmetry
        import IMP.test

    def test_applications_installed(self):
        """Check install of a fairly comprehensive list of applications"""
        emagefit_apps = ['convert_spider_to_jpg.py', 'emagefit_cluster.py',
                         'emagefit_dock.py', 'emagefit.py', 'emagefit_score.py']
        idock_apps = ['idock.py', 'combine_scores', 'recompute_zscore',
                      'cross_links_score', 'cross_links_single_score',
                      'em2d_score', 'em2d_single_score',
                      'interface_cross_links', 'em3d_score',
                      'em3d_single_score', 'saxs_score', 'interface_rtc',
                      'nmr_rtc_score', 'soap_score']
        apps = ['cluster_profiles', 'cnmultifit.py',
                'complex_to_anchor_graph.py', 'compute_chi',
                'estimate_threshold_from_molecular_mass.py', 'foxs',
                'imp_example_app', 'ligand_score', 'map2pca.py', 'mol2pca.py',
                'multifit.py', 'pdb_check', 'pdb_rmf', 'resample_density.py',
                'rg', 'rmf3_dump', 'rmf_cat', 'rmf_display',
                'rmf_frames', 'rmf_info', 'rmf_interpolate', 'rmf_pdb',
                'rmf_show', 'rmf_signature', 'rmf_simplify', 'rmf_slice',
                'rmf_transform', 'rmf_update',
                'rmf_validate', 'rmf_xml', 'saxs_merge.py',
                'simulate_density_from_pdb.py',
                'validate_profile', 'view_density_header.py']
        # RHEL systems don't include EMageFit and idock; Fedora does
        if mock_config.startswith('fedora'):
            apps.extend(emagefit_apps)
            apps.extend(idock_apps)
        else:
            for app in emagefit_apps + idock_apps:
                self.assertRaises(OSError, subprocess.call, app)
        for app in apps:
            try:
                p = subprocess.Popen([app, '--help'], stdout=subprocess.PIPE,
                                     stderr=subprocess.STDOUT)
            except OSError:
                raise OSError("Could not run %s" % app)
            out = p.stdout.read()
            ret = p.wait()
            self.assert_(ret == 1 or ret == 0,
                         "Return code for %s app is %d, not 0 or 1; "
                         "output is %s" % (app, ret, out))

if __name__ == '__main__':
    # Note we use unittest rather than IMP.test, since the latter requires
    # a unittest2 Python module to be installed
    unittest.main()
