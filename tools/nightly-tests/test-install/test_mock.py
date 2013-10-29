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
        import IMP.restrainer
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
        apps = ['cluster_profiles', 'cnmultifit.py', 'combine_scores',
                'complex_to_anchor_graph.py', 'compute_chi',
                'cross_links_score',
                'cross_links_single_score', 'em2d_score', 'em2d_single_score',
                'em3d_score', 'em3d_single_score',
                'estimate_threshold_from_molecular_mass.py', 'foxs',
                'idock.py', 'imp_example_app', 'interface_cross_links',
                'interface_rtc', 'ligand_score', 'map2pca.py', 'mol2pca.py',
                'multifit.py', 'nmr_rtc_score', 'pdb_check', 'pdb_rmf',
                'recompute_zscore', 'resample_density.py', 'rg',
                'rmf_avro_dump', 'rmf_cat', 'rmf_color', 'rmf_display',
                'rmf_frames', 'rmf_info', 'rmf_interpolate', 'rmf_pdb',
                'rmf_show', 'rmf_slice', 'rmf_transform', 'rmf_update',
                'rmf_validate', 'rmf_xml', 'saxs_merge.py', 'saxs_score',
                'simulate_density_from_pdb.py', 'soap_score',
                'soap_single_score', 'validate_profile',
                'view_density_header.py' ]
        # RHEL systems don't include EMageFit; Fedora does
        if mock_config.startswith('fedora'):
            apps.extend(emagefit_apps)
        else:
            for app in emagefit_apps:
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
                         "Return code for %s app is %d, not 0 or 1; " \
                         "output is %s" % (app, ret, out))

if __name__ == '__main__':
    # Note we use unittest rather than IMP.test, since the latter requires
    # a unittest2 Python module to be installed
    unittest.main()
