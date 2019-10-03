import unittest
import os
import sys
import subprocess

mock_config = os.environ['MOCK_CONFIG']


class IMPMockTests(unittest.TestCase):

    def test_modules_installed(self):
        """Check modules included in the RPM or .deb"""
        # The scratch module should not be included
        self.assertRaises(ImportError, __import__, 'IMP.scratch')
        # We bundle CGAL now, so everyone should have IMP.cgal
        import IMP.cgal
        # We bundle OpenCV now, so everyone should have IMP.em2d
        import IMP.em2d
        # We build our own Eigen on RHEL, so everyone should have IMP.isd
        import IMP.isd
        # Make sure that npctransport is included and that the underlying
        # protobuf stuff also works
        import IMP.npctransport
        # Ubuntu only supports protobuf with Python 3 in 18.04 or later;
        # our Windows protobuf install is Python 2 only;
        # for RHEL8 we only have Python 3 protobuf wrappers.
        py2only_pb = frozenset(('ubuntu-trusty', 'ubuntu-xenial', 'w32', 'w64'))
        py3only_pb = frozenset(('epel-8-x86_64',))
        if ((sys.version_info[0] == 3 and mock_config not in py2only_pb)
            or (sys.version_info[0] == 2 and mock_config not in py3only_pb)):
            x = IMP.npctransport.Configuration
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
        import IMP.pmi1

    def test_applications_installed(self):
        """Check install of a fairly comprehensive list of applications"""
        emagefit_apps = ['convert_spider_to_jpg', 'emagefit_cluster',
                         'emagefit_dock', 'emagefit', 'emagefit_score']
        idock_apps = ['idock', 'combine_scores', 'recompute_zscore',
                      'cross_links_score', 'cross_links_single_score',
                      'em2d_score', 'em2d_single_score',
                      'interface_cross_links', 'em3d_score',
                      'em3d_single_score', 'saxs_score', 'interface_rtc',
                      'nmr_rtc_score', 'soap_score']
        apps = ['cluster_profiles', 'cnmultifit',
                'complex_to_anchor_graph', 'compute_chi',
                'estimate_threshold_from_molecular_mass', 'foxs',
                'imp_example_app', 'ligand_score', 'map2pca', 'mol2pca',
                'multifit', 'pdb_check', 'pdb_rmf', 'resample_density',
                'compute_rg', 'rmf3_dump', 'rmf_cat', 'rmf_display',
                'rmf_frames', 'rmf_info', 'rmf_interpolate', 'rmf_pdb',
                'rmf_show', 'rmf_signature', 'rmf_simplify', 'rmf_slice',
                'rmf_transform', 'rmf_update',
                'rmf_validate', 'rmf_xml', 'saxs_merge',
                'simulate_density_from_pdb',
                'validate_profile', 'view_density_header']
        # Everyone should have EMageFit and idock now
        apps.extend(emagefit_apps)
        apps.extend(idock_apps)
        for app in apps:
            try:
                p = subprocess.Popen([app, '--help'], stdout=subprocess.PIPE,
                                      stderr=subprocess.STDOUT)
            except OSError:
                raise OSError("Could not run %s" % app)
            out = p.stdout.read()
            p.stdout.close()
            ret = p.wait()
            self.assertTrue(ret == 1 or ret == 0,
                            "Return code for %s app is %d, not 0 or 1; "
                            "output is %s" % (app, ret, out))

if __name__ == '__main__':
    # Note we use unittest rather than IMP.test, since the latter requires
    # a unittest2 Python module to be installed
    unittest.main()
