import IMP.test
import os
import sys
import shutil
import StringIO

class Tests(IMP.test.ApplicationTestCase):
    def test_wrong_number_args(self):
        """Test idock.py with wrong number of arguments"""
        p = self.run_python_application('idock.py', [])
        out, err = p.communicate()
        self.assertNotEqual(p.returncode, 0)

    def test_em2d_without_pixel_size(self):
        """Test idock.py with class averages but no pixel size"""
        p = self.run_python_application('idock.py',
                                        ['--em2d', 'foo', 'file1', 'file2'])
        out, err = p.communicate()
        self.assertIn('please specify pixel size', err)
        self.assertNotEqual(p.returncode, 0)

    def test_no_data(self):
        """Test idock.py with no experimental data"""
        p = self.run_python_application('idock.py',
                                        ['file1', 'file2'])
        out, err = p.communicate()
        self.assertIn('please provide', err)
        self.assertNotEqual(p.returncode, 0)

    def test_idock_init(self):
        """Test IDock class init"""
        app = self.import_python_application('idock.py')
        dock = app.IDock()

    def make_idock_with_captured_subprocess(self):
        """Make an IDock instance. Capture the command lines of any
           subprocesses, rather than actually running them."""
        app = self.import_python_application('idock.py')
        class Opts(object):
            pass
        d = app.IDock()
        d.opts = Opts()
        d.ligand = d.receptor = ''
        def _run_binary(*args, **keys):
            d.run_binary_args = args
            d.run_binary_keys = keys
        old_run_binary = app._run_binary
        app._run_binary = _run_binary
        return app, old_run_binary, d

    def test_run_patch_dock_binary(self):
        """Test run_patch_dock_binary()"""
        app, old_run_binary, dock = self.make_idock_with_captured_subprocess()
        try:
            dock.opts.patch_dock = 'pd_dir'
            dock.run_patch_dock_binary('pd', ['foo', 'bar'])
            self.assertEqual(dock.run_binary_args, ('pd_dir', 'pd',
                                                    ['foo', 'bar']))
        finally:
            app._run_binary = old_run_binary

    def test_run_fiber_dock_binary(self):
        """Test run_fiber_dock_binary()"""
        app, old_run_binary, dock = self.make_idock_with_captured_subprocess()
        try:
            dock.opts.fiber_dock = 'fd_dir'
            dock.run_fiber_dock_binary('fd', ['foo', 'bar'])
            self.assertEqual(dock.run_binary_args, ('fd_dir', 'fd',
                                                    ['foo', 'bar']))
        finally:
            app._run_binary = old_run_binary

    def test_count_lines(self):
        """Test _count_lines()"""
        app = self.import_python_application('idock.py')
        open('foo', 'w').write('foo\nbar\nbaz\n')
        self.assertEqual(app._count_lines('foo'), 3)
        os.unlink('foo')

    def test_count_fiber_dock_out(self):
        """Test _count_fiber_dock_out()"""
        app = self.import_python_application('idock.py')
        open('foo', 'w').write("""
After Refinement:
########### Program Parameters ##################
#################################################
    Sol # |     glob |     aVdW |     rVdW |      ACE |   inside |    aElec |    rElec |   laElec |   lrElec |       hb |      piS |   catpiS |    aliph |     prob | BBdeform |
1       |   -20.01 |   -16.03 |     3.97 |     5.64 |    15.82 |    -8.53 |     0.00 |    -8.41 |     4.76 |     0.00 |    -5.00 |     0.00 |     0.00 |     0.00 |     0.00 |2.57 0.76 1.77 33.15 35.83 73.77 | |
2       |   -14.43 |   -15.32 |     9.84 |     1.44 |    15.25 |   -57.83 |   130.98 |   -26.15 |    26.80 |     0.00 |   -16.50 |     0.00 |    -2.00 |     0.00 |     0.00 |1.14 0.80 -0.11 9.81 62.06 63.74 | |
""")
        self.assertEqual(app._count_fiber_dock_out('foo'), 2)
        os.unlink('foo')

    def test_run_binary(self):
        """Test _run_binary()"""
        class DummySubprocess(object):
            args = None
            class Popen(object):
                def __init__(self, *args, **keys):
                    DummySubprocess.args = self.args = args
                def wait(self):
                    if 'bad' in self.args[0][0]:
                        return 1
                    else:
                        return 0
        app = self.import_python_application('idock.py')
        oldsubproc = app.subprocess
        old_stdout = sys.stdout
        try:
            app.subprocess = DummySubprocess
            self.assertRaises(OSError, app._run_binary, 'testpath', 'bad', [])
            app._run_binary('testpath', 'bin', ['arg1', 'arg2'])
            self.assertEqual(app.subprocess.args,
                             ([os.path.join('testpath', 'bin'),
                               'arg1', 'arg2'],))
            sys.stdout = StringIO.StringIO()
            app._run_binary('', 'bin', ['arg1', 'arg2'])
            self.assertEqual(app.subprocess.args,
                             (['bin', 'arg1', 'arg2'],))
            self.assertEqual(sys.stdout.getvalue().rstrip('\r\n'),
                             'bin arg1 arg2')
            sys.stdout = StringIO.StringIO()
            app._run_binary('', 'bin', ['arg1', 'arg2'], out_file='foo')
            self.assertEqual(sys.stdout.getvalue().rstrip('\r\n'),
                             'bin arg1 arg2 > foo')
            os.unlink('foo')
        finally:
            sys.stdout = old_stdout
            app.subprocess = oldsubproc

    def test_make_patch_dock_surfaces(self):
        """Test make_patch_dock_surfaces()"""
        app, old_run_binary, dock = self.make_idock_with_captured_subprocess()
        try:
            dock.opts.patch_dock = 'pd_dir'
            dock.receptor = 'testrecep'
            dock.ligand = 'testlig'
            dock.make_patch_dock_surfaces()
            self.assertEqual(dock.run_binary_args,
                             ('pd_dir', 'buildMS.pl', ['testrecep', 'testlig']))
        finally:
            app._run_binary = old_run_binary

    def test_make_patch_dock_parameters(self):
        """Test make_patch_dock_parameters()"""
        app, old_run_binary, dock = self.make_idock_with_captured_subprocess()
        try:
            dock.opts.patch_dock = 'pd_dir'
            dock.opts.type = 'AA'
            dock.receptor = 'testrecep'
            dock.ligand = 'testlig'

            dock.opts.precision = 1
            dock.make_patch_dock_parameters()
            self.assertEqual(dock.run_binary_args, ('pd_dir', 'buildParams.pl',
                                        ['testrecep', 'testlig', '4.0', 'AA']))

            dock.opts.precision = 2
            dock.make_patch_dock_parameters()
            self.assertEqual(dock.run_binary_args, ('pd_dir', 'buildParams.pl',
                                        ['testrecep', 'testlig', '2.0', 'AA']))

            dock.opts.precision = 3
            dock.make_patch_dock_parameters()
            self.assertEqual(dock.run_binary_args,
                             ('pd_dir', 'buildParamsFine.pl',
                                        ['testrecep', 'testlig', '2.0', 'AA']))
        finally:
            app._run_binary = old_run_binary

    def test_get_filename(self):
        """Test get_filename()"""
        app = self.import_python_application('idock.py')
        class Opts(object):
            pass
        dock = app.IDock()
        dock.opts = Opts()
        dock.opts.prefix = 'foo'
        self.assertEqual(dock.get_filename('bar'), 'foobar')

    def test_do_patch_dock_docking(self):
        """Test do_patch_dock_docking()"""
        def do_dock():
            app, old_run_binary, dock \
                     = self.make_idock_with_captured_subprocess()
            dock.opts.prefix = 'foo'
            dock.opts.patch_dock = 'pd_dir'
            dock.do_patch_dock_docking()
            return app, old_run_binary, dock
        if os.path.exists('foodocking.res'):
            os.unlink('foodocking.res')
        app, old_run_binary, dock = do_dock()
        try:
            self.assertEqual(dock.run_binary_args,
                             ('pd_dir', 'patch_dock.Linux',
                                           ['params.txt', 'foodocking.res']))
        finally:
            app._run_binary = old_run_binary

        # Recovery should be skipped if results file is too short
        open('foodocking.res', 'w').write('foo\n' * 36)
        app, old_run_binary, dock = do_dock()
        try:
            self.assertEqual(dock.run_binary_args,
                             ('pd_dir', 'patch_dock.Linux',
                                           ['params.txt', 'foodocking.res']))
        finally:
            app._run_binary = old_run_binary

        open('foodocking.res', 'w').write('foo\n' * 37)
        app, old_run_binary, dock = do_dock()
        try:
            self.assertFalse(hasattr(dock, 'run_binary_args'))
        finally:
            app._run_binary = old_run_binary
        os.unlink('foodocking.res')

    def test_make_transformation_file(self):
        """Test make_transformation_file()"""
        app = self.import_python_application('idock.py')
        class Opts(object):
            pass
        dock = app.IDock()
        dock.opts = Opts()
        dock.opts.prefix = 'foo'
        open('foodocking.res', 'w').write("""
Program parameters
******
   # | score | pen.  | Area    | as1   | as2   | as12  | ACE     | hydroph | Energy  |cluster| dist. || Ligand Transformation
""" + "  1 | 14906 | -3.60 | 1816.10 |     0 |  2279 |     0 |  229.45 | 1248.53 |    0.00 |     0 | 0.00 || 2.56 0.12 -0.27 36.00 -10.44 47.54\n" * 6000)

        dock.opts.precision = 1
        n = dock.make_transformation_file()
        lines = open('trans_pd').readlines()
        self.assertEqual(n, 5000)
        self.assertEqual(len(lines), 5000)
        self.assertEqual(lines[0].rstrip('\r\n'),
                         '1 2.56 0.12 -0.27 36.00 -10.44 47.54')

        dock.opts.precision = 2
        n = dock.make_transformation_file()
        lines = open('trans_pd').readlines()
        self.assertEqual(n, 6000)
        self.assertEqual(len(lines), 6000)
        os.unlink('trans_pd')
        os.unlink('foodocking.res')

    def test_run_patch_dock(self):
        """Test run_patch_dock()"""
        app = self.import_python_application('idock.py')
        class Dummy(app.IDock):
            def make_patch_dock_surfaces(self):
                self.calls.append('surface')
            def make_patch_dock_parameters(self):
                self.calls.append('params')
            def do_patch_dock_docking(self):
                self.calls.append('docking')
            def make_transformation_file(self):
                self.calls.append('transforms')
                return 42
        class Opts(object):
            pass
        dock = Dummy()
        dock.opts = Opts()
        dock.receptor = 'testrecep'
        dock.ligand = 'testlig'
        dock.calls = []
        dock.opts.type = 'other'
        n = dock.run_patch_dock()
        self.assertEqual(n, 42)
        self.assertEqual(dock.calls, ['surface', 'params', 'docking',
                                      'transforms'])
        self.assertEqual(dock.ligand, 'testlig')
        self.assertEqual(dock.receptor, 'testrecep')

        # Check for switch when type='AA'
        dock = Dummy()
        dock.opts = Opts()
        dock.receptor = 'testrecep'
        dock.ligand = 'testlig'
        dock.calls = []
        dock.opts.saxs_receptor_pdb = 'saxsrecep'
        dock.opts.saxs_ligand_pdb = 'saxslig'
        dock.opts.type = 'AA'
        dock.run_patch_dock()
        self.assertEqual(dock.ligand, 'testrecep')
        self.assertEqual(dock.receptor, 'testlig')
        self.assertEqual(dock.opts.saxs_receptor_pdb, 'saxslig')
        self.assertEqual(dock.opts.saxs_ligand_pdb, 'saxsrecep')

    def test_parse_args(self):
        """Test IDock.parse_args()"""
        app = self.import_python_application('idock.py')
        old_sys_argv = sys.argv
        try:
            # Instantiate all scorers
            sys.argv = [sys.argv[0], '--cxms', 'cxms.txt',
                        '--em2d', 'em2d_1.pgm', '--em2d', 'em2d_2.pgm',
                        '--pixel_size', '2.0',
                        '--em3d', 'em3d.mrc',
                        '--receptor_rtc', 'receptor.rtc',
                        '--ligand_rtc', 'ligand.rtc',
                        '--saxs', 'saxs.txt',
                        '--saxs_receptor_pdb', 'receptor.saxs',
                        '--saxs_ligand_pdb', 'ligand.saxs',
                        'receptor.pdb', 'ligand.pdb']
            idock = app.IDock()
            scorers = idock.parse_args()
            self.assertEqual(len(scorers), 5)
        finally:
            sys.argv = old_sys_argv

    def test_get_scorers(self):
        """Test IDock.get_scorers()"""
        app = self.import_python_application('idock.py')
        class Opts(object):
            pass
        d = app.IDock()
        d.opts = Opts()
        d.receptor = 'testrecep'
        d.ligand = 'testlig'
        d.opts.prefix = 'pre'
        d.opts.type = 'other'
        d.opts.receptor_rtc = 'testrecep_rtc'
        d.opts.ligand_rtc = 'testrecep_rtc'
        d.opts.saxs_file = 'testsaxs'
        d.opts.saxs_receptor_pdb = d.opts.saxs_ligand_pdb = None
        d.opts.class_averages = []
        d.opts.map_file = 'test.mrc'
        d.opts.cross_links_file = None
        scorers = d.get_scorers(None)
        self.assertEqual(len(scorers), 3)

    def test_scorer(self):
        """Test Scorer class"""
        app = self.import_python_application('idock.py')
        class MyScorer(app.Scorer):
            def _run_score_binary(self):
                self.run_score = True
        class MockIDock(object):
            receptor = 'testrecep'
            ligand = 'testlig'
            def get_filename(self, f):
                return f
        if os.path.exists('testoutput.res'):
            os.unlink('testoutput.res')
        # Score should be recalculated if output file does not exist
        s = MyScorer(MockIDock(), 'testoutput')
        self.assertEqual(s.output_file, 'testoutput.res')
        s.score(10)
        self.assertEqual(s.run_score, True)

        open('testoutput.res', 'w').write('foo\n' * 8)
        # Also if the file contains fewer transforms than input
        s = MyScorer(MockIDock(), 'testoutput')
        s.score(10)
        self.assertEqual(s.run_score, True)

        # Skip score if number of transforms is sufficient
        s = MyScorer(MockIDock(), 'testoutput')
        s.score(8)
        self.assertEqual(hasattr(s, 'run_score'), False)
        os.unlink('testoutput.res')

    def test_recompute_zscore(self):
        """Test Scorer.recompute_zscore()"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.prefix = ''
        class MyScorer(app.Scorer):
            def __init__(self, idock):
                app.Scorer.__init__(self, idock, "my_score")
        s = MyScorer(idock)
        open('my_score.res', 'w').write("""
receptorPdb (str) 2p4e.pdb
ligandPdb (str) antibody_cut.pdb
     # | 1-CC   |filter| Zscore | Transformation
     1 |  0.196 |  +   |   2.45 | 2.423 0.1092 -0.2944 36.17 -8.459 49.66
     2 |  0.221 |  +   |   3.19 | 2.674 0.4152 -0.7746 33.23 -4.204 31.47
     3 |  0.233 |  +   |   3.53 | 2.622 0.5735 -0.7227 34.3 -2.353 32.4
     4 |  0.212 |  +   |   2.94 | 2.551 0.06816 -0.1646 38.22 -10.09 46.22
     5 |  0.208 |  +   |   2.82 | 2.722 0.3805 -0.8429 33.87 -5.112 29.73
     6 |  0.211 |  +   |    2.9 | 2.496 -0.04911 -0.258 37.57 -10.18 47.23
""")
        open('transforms', 'w').write("""
2 2.674 0.4152 -0.7746 33.23 -4.204 31.47
3 2.622 0.5735 -0.7227 34.3 -2.353 32.4
1 2.423 0.1092 -0.2944 36.17 -8.459 49.66
6 2.496 -0.04911 -0.258 37.57 -10.18 47.23
""")
        s.recompute_zscore("transforms")
        lines = open('my_scoref.res').readlines()
        self.assertEqual(len(lines), 5)
        self.assertEqual(lines[1].strip(' \r\n'),
          '2 |  0.221 |  +  | 0.424 |  2.674 0.4152 -0.7746 33.23 -4.204 31.47')

        s.reverse_zscores = True
        s.recompute_zscore("transforms")
        lines = open('my_scoref.res').readlines()
        self.assertEqual(len(lines), 5)
        self.assertEqual(lines[1].strip(' \r\n'),
         '2 |  0.221 |  +  | -0.424 |  2.674 0.4152 -0.7746 33.23 -4.204 31.47')
        os.unlink('my_score.res')
        os.unlink('my_scoref.res')
        os.unlink('transforms')

    def get_dummy_idock_for_scorer(self):
        app = self.import_python_application('idock.py')
        class Opts(object):
            pass
        idock = app.IDock()
        idock.opts = Opts()
        idock.receptor = 'testrecep'
        idock.ligand = 'testlig'
        idock.opts.prefix = ''
        return app, idock

    def run_scorer_score(self, scorer):
        app = self.import_python_application('idock.py')
        class Dummy:
            def _run_binary(cls, *args):
                cls.score_args = args
            _run_binary = classmethod(_run_binary)
        old_run_binary = app._run_binary
        app._run_binary = Dummy._run_binary
        try:
            scorer.score(10)
        finally:
            app._run_binary = old_run_binary
        return Dummy.score_args

    def test_nmr_scorer(self):
        """Test NMRScorer class"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.receptor_rtc = idock.opts.ligand_rtc = None
        self.assertIsNone(app.NMRScorer.check_options(idock, None))
        idock.opts.receptor_rtc = 'r_rtc'
        idock.opts.ligand_rtc = 'l_rtc'
        idock.opts.type = 'other'
        s = app.NMRScorer(idock)
        s2 = app.NMRScorer.check_options(idock, None)
        self.assertIsInstance(s2, app.NMRScorer)
        self.assertEqual(s.transforms_needed, 5000)
        self.assertEqual(s.reverse_zscores, False)
        self.assertEqual(s.receptor_rtc, 'r_rtc')
        self.assertEqual(s.ligand_rtc, 'l_rtc')
        idock.opts.type = 'AA'
        s = app.NMRScorer(idock)
        self.assertEqual(s.receptor_rtc, 'l_rtc')
        self.assertEqual(s.ligand_rtc, '-')
        self.assertEqual(str(s), 'NMR score')
        self.assertEqual(self.run_scorer_score(s),
                         (None, 'nmr_rtc_score', ['testrecep', 'testlig',
                          'trans_pd', 'l_rtc', '-', '-o', 'nmr_rtc_score.res']))

    def test_saxs_scorer(self):
        """Test SAXSScorer class"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.saxs_file = None
        self.assertIsNone(app.SAXSScorer.check_options(idock, None))
        idock.opts.saxs_file = 'test.saxs'
        idock.opts.saxs_receptor_pdb = idock.opts.saxs_ligand_pdb = None
        s = app.SAXSScorer(idock)
        s2 = app.SAXSScorer.check_options(idock, None)
        self.assertIsInstance(s2, app.SAXSScorer)
        self.assertEqual(s.transforms_needed, 5000)
        self.assertEqual(s.reverse_zscores, False)
        self.assertEqual(s.saxs_file, 'test.saxs')
        self.assertEqual(s.saxs_receptor, 'testrecep')
        self.assertEqual(s.saxs_ligand, 'testlig')
        idock.opts.saxs_receptor_pdb = 'testrecep.s'
        idock.opts.saxs_ligand_pdb = 'testlig.s'
        s = app.SAXSScorer(idock)
        self.assertEqual(s.saxs_receptor, 'testrecep.s')
        self.assertEqual(s.saxs_ligand, 'testlig.s')
        self.assertEqual(str(s), 'SAXS score')
        self.assertEqual(self.run_scorer_score(s),
                         (None, 'saxs_score', ['testrecep.s', 'testlig.s',
                          'trans_pd', 'test.saxs', '-o', 'saxs_score.res']))

    def test_em2d_scorer(self):
        """Test EM2DScorer class"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.class_averages = []
        self.assertIsNone(app.EM2DScorer.check_options(idock, None))
        idock.opts.class_averages = ['test1.pgm', 'test2.pgm']
        idock.opts.pixel_size = 4.0
        s = app.EM2DScorer(idock)
        s2 = app.EM2DScorer.check_options(idock, None)
        self.assertIsInstance(s2, app.EM2DScorer)
        self.assertEqual(s.transforms_needed, 5000)
        self.assertEqual(s.reverse_zscores, False)
        self.assertEqual(s.class_averages, ['test1.pgm', 'test2.pgm'])
        self.assertAlmostEqual(s.pixel_size, 4.0, delta=1e-6)
        self.assertEqual(str(s), 'EM2D score')
        self.assertEqual(self.run_scorer_score(s),
                         (None, 'em2d_score', ['testrecep', 'testlig',
                          'trans_pd', 'test1.pgm', 'test2.pgm', '-o',
                          'em2d_score.res', '-n', '200', '-s', '4.0']))

    def test_em3d_scorer(self):
        """Test EM3DScorer class"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.map_file = None
        self.assertIsNone(app.EM3DScorer.check_options(idock, None))
        idock.opts.map_file = 'test.mrc'
        s = app.EM3DScorer(idock)
        s2 = app.EM3DScorer.check_options(idock, None)
        self.assertIsInstance(s2, app.EM3DScorer)
        self.assertEqual(s.transforms_needed, 1000)
        self.assertEqual(s.reverse_zscores, True)
        self.assertEqual(s.map_file, 'test.mrc')
        self.assertEqual(str(s), 'EM3D score')
        self.assertEqual(self.run_scorer_score(s),
                         (None, 'em3d_score', ['testrecep', 'testlig',
                          'trans_pd', 'test.mrc', '-o', 'em3d_score.res',
                          '-s']))

    def test_cxms_scorer(self):
        """Test CXMSScorer class"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.cross_links_file = None
        self.assertIsNone(app.CXMSScorer.check_options(idock, None))
        idock.opts.cross_links_file = 'test.cxms'
        s = app.CXMSScorer(idock)
        s2 = app.CXMSScorer.check_options(idock, None)
        self.assertIsInstance(s2, app.CXMSScorer)
        self.assertEqual(s.transforms_needed, 2000)
        self.assertEqual(s.reverse_zscores, True)
        self.assertEqual(s.cross_links_file, 'test.cxms')
        self.assertEqual(str(s), 'CXMS score')
        self.assertEqual(self.run_scorer_score(s),
                         (None, 'cross_links_score', ['testrecep', 'testlig',
                          'trans_pd', 'test.cxms', '-o', 'cxms_score.res']))

    def test_get_all_scores_filename(self):
        """Test IDock.get_all_scores_filename()"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.prefix = 'prefix_'
        idock.opts.cross_links_file = 'test.cxms'
        idock.opts.map_file = 'test.mrc'
        scorers = [app.CXMSScorer(idock), app.EM3DScorer(idock)]
        self.assertEqual(idock.get_all_scores_filename(scorers, 'combined_',
                                                       '.res'),
                         'prefix_combined_cxms_em3d.res')

    def test_get_filtered_scores_1(self):
        """Test get_filtered_scores() with a single method"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.cross_links_file = 'test.cxms'
        s = app.CXMSScorer(idock)
        open(s.output_file, 'w').write("""
     # | 1-CC   |filter| Zscore | Transformation
     1 |  0.196 |  +   |   2.45 | 2.423 0.1092 -0.2944 36.17 -8.459 49.66
     2 |  0.221 |  -   |   3.19 | 2.674 0.4152 -0.7746 33.23 -4.204 31.47
     3 |  0.233 |  +   |   3.53 | 2.622 0.5735 -0.7227 34.3 -2.353 32.4
""")
        idock.get_filtered_scores([s])
        lines = open('trans_for_cluster').readlines()
        self.assertEqual(len(lines), 2)
        nums = [line.split()[0] for line in lines]
        self.assertEqual(nums, ['1', '3'])
        scores = [line.split()[1] for line in lines]
        self.assertEqual(scores, ['2.45', '3.53'])
        os.unlink('trans_for_cluster')
        os.unlink(s.output_file)

    def test_get_filtered_scores_2(self):
        """Test get_filtered_scores() with two methods"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.cross_links_file = 'test.cxms'
        idock.opts.map_file = 'test.mrc'
        s1 = app.CXMSScorer(idock)
        s2 = app.EM3DScorer(idock)
        open(s1.output_file, 'w').write("""
     # | 1-CC   |filter| Zscore | Transformation
     1 |  0.196 |  -   |   2.45 | 2.423 0.1092 -0.2944 36.17 -8.459 49.66
     2 |  0.221 |  +   |   3.19 | 2.674 0.4152 -0.7746 33.23 -4.204 31.47
     3 |  0.233 |  +   |   3.53 | 2.622 0.5735 -0.7227 34.3 -2.353 32.4
""")
        open(s2.output_file, 'w').write("""
     # | Score    |filter| Zscore |  CC    | Escore   | Map transformation                          | Ligand Transformation
     1 |    0.000 |  +   |  0.000 |  0.000 |    0.000 | 0.0000 0.0000 0.0000 0.0000 0.0000 0.0000 | 2.4228 0.1092 -0.2944 36.1673 -8.4593 49.6622
     2 |    0.000 |  +   |  0.000 |  0.000 |    0.000 | 0.0000 0.0000 0.0000 0.0000 0.0000 0.0000 | 2.6739 0.4152 -0.7746 33.2264 -4.2045 31.4729
     3 |    0.000 |  +   |  0.000 |  0.000 |    0.000 | 0.0000 0.0000 0.0000 0.0000 0.0000 0.0000 | 2.6217 0.5735 -0.7227 34.2971 -2.3534 32.4020
""")
        idock.get_filtered_scores([s1, s2])
        lines = open('trans_for_cluster').readlines()
        self.assertEqual(len(lines), 2)
        nums = [line.split()[0] for line in lines]
        # Only 2 and 3 are OK in both methods
        self.assertEqual(nums, ['2', '3'])
        os.unlink('combined_cxms_em3d.res')
        os.unlink('trans_for_cluster')
        os.unlink(s1.output_file)
        os.unlink(s2.output_file)

    def test_get_clustered_transforms(self):
        """Test get_clustered_transforms()"""
        app, old_run_binary, dock = self.make_idock_with_captured_subprocess()
        try:
            dock.opts.patch_dock = 'pd_dir'
            dock.receptor = 'testrecep'
            dock.ligand = 'testlig'
            dock.opts.prefix = ''
            dock.opts.cross_links_file = 'test.cxms'
            dock.opts.map_file = 'test.mrc'
            s1 = app.CXMSScorer(dock)
            s2 = app.EM3DScorer(dock)
            out_file = 'clustered_cxms_em3d.res'
            out = dock.get_clustered_transforms([s1, s2])
            self.assertEqual(dock.run_binary_args,
                             ('pd_dir', 'interface_cluster.linux',
                              ['testrecep.ms', 'testlig', 'trans_for_cluster',
                               '4.0', out_file]))
            self.assertEqual(out, out_file)
        finally:
            app._run_binary = old_run_binary

    def test_get_transforms_for_fiber_dock(self):
        """Test get_transforms_for_fiber_dock()"""
        app, idock = self.get_dummy_idock_for_scorer()
        class MockScorer1(object):
            short_name = 'test1'
            transforms_needed = 10
        class MockScorer2(object):
            short_name = 'test2'
            transforms_needed = 3
        in_transforms = 'foo'
        open(in_transforms, 'w').write("""
receptorPdb (str) 2p4e.pdb
ligandPdb (str) antibody_cut.pdb
18901 | -4.9370 |     1 |    9 |2.44620 0.74390 2.01370 32.03100 36.50100 74.97570
25924 | -4.6170 |     2 |    3 |0.91100 0.68300 -0.12270 15.18620 66.08760 62.19710
11663 | -4.4110 |     3 |    3 |2.07170 0.92170 2.35490 28.79510 33.55630 67.50950
23569 | -4.0800 |     4 |   11 |-1.67500 -0.39110 3.01710 2.30350 13.13610 -1.50930
""")
        fn = idock.get_transforms_for_fiber_dock([MockScorer1(), MockScorer2()],
                                                 in_transforms)
        lines = open(fn).readlines()
        self.assertEqual(len(lines), 3)
        self.assertEqual(lines[0].rstrip('\r\n'),
               "18901  2.44620 0.74390 2.01370 32.03100 36.50100 74.97570")
        self.assertEqual(fn, 'trans_for_fd_test1_test2')
        os.unlink(fn)
        os.unlink(in_transforms)

    def test_add_hydrogens(self):
        """Test add_hydrogens()"""
        app, old_run_binary, dock = self.make_idock_with_captured_subprocess()
        try:
            fn = dock.add_hydrogens('foo.pdb')
            self.assertEqual(dock.run_binary_args,
                             (None, 'reduce',
                              ['-OH', '-HIS', '-NOADJ', '-NOROTMET',
                               'foo.pdb']))
            self.assertEqual(dock.run_binary_keys, {'out_file':'foo.pdb.HB'})
            self.assertEqual(fn, 'foo.pdb.HB')
        finally:
            app._run_binary = old_run_binary

    def test_make_fiber_dock_parameters(self):
        """Test make_fiber_dock_parameters()"""
        class MockScorer(object):
            short_name = 'mock'
        app, old_run_binary, dock = self.make_idock_with_captured_subprocess()
        try:
            dock.opts.fiber_dock = 'fd_dir'
            dock.opts.prefix = ''
            dock.opts.type = 'AA'
            params, out = dock.make_fiber_dock_parameters([MockScorer()],
                                          'recep.HB', 'lig.HB', 'test_trans')
            self.assertEqual(dock.run_binary_args,
                             ('fd_dir', 'buildFiberDockParams.pl',
                              ['recep.HB', 'lig.HB', 'U', 'U', 'AA',
                               'test_trans', 'fd_res_mock', '0', '50', '0.8',
                               '0', 'glpk', 'params_fd_mock.txt']))
            self.assertEqual(params, 'params_fd_mock.txt')
            self.assertEqual(out, 'fd_res_mock.ref')
        finally:
            app._run_binary = old_run_binary

    def test_do_fiber_dock_docking(self):
        """Test do_fiber_dock_docking()"""
        def do_dock():
            app, old_run_binary, dock \
                           = self.make_idock_with_captured_subprocess()
            dock.opts.fiber_dock = 'fd_dir'
            dock.opts.prefix = ''
            dock.do_fiber_dock_docking('in_trans', 'params', 'fd_out')
            return app, old_run_binary, dock

        if os.path.exists('fd_out'):
            os.unlink('fd_out')
        open('in_trans', 'w').write('trans\n' * 3)
        app, old_run_binary, dock = do_dock()
        try:
            self.assertEqual(dock.run_binary_args,
                             ('fd_dir', 'FiberDock', ['params']))
        finally:
            app._run_binary = old_run_binary
        # FiberDock should still be run if fd_out is too short
        open('fd_out', 'w').write('foo\n')
        app, old_run_binary, dock = do_dock()
        try:
            self.assertEqual(dock.run_binary_args,
                             ('fd_dir', 'FiberDock', ['params']))
        finally:
            app._run_binary = old_run_binary
        # FiberDock should be skipped if fd_out is long enough
        open('fd_out', 'w').write("""
    Sol # |     glob |     aVdW |     rVdW |      ACE |   inside |    aElec |    rElec |   laElec |   lrElec |       hb |      piS |   catpiS |    aliph |     prob | BBdeform |
1       |   -20.01 |   -16.03 |     3.97 |     5.64 |    15.82 |    -8.53 |     0.00 |    -8.41 |     4.76 |     0.00 |    -5.00 |     0.00 |     0.00 |     0.00 |     0.00 |2.57 0.76 1.77 33.15 35.83 73.77 | |
2       |   -14.43 |   -15.32 |     9.84 |     1.44 |    15.25 |   -57.83 |   130.98 |   -26.15 |    26.80 |     0.00 |   -16.50 |     0.00 |    -2.00 |     0.00 |     0.00 |1.14 0.80 -0.11 9.81 62.06 63.74 | |
3       |    13.94 |   -18.35 |     8.34 |    14.71 |    15.73 |   -19.89 |    59.33 |   -17.14 |    24.47 |    -0.30 |    -4.00 |     0.00 |     0.00 |     0.00 |     0.00 |2.05 1.10 2.19 29.46 32.01 66.74 | |
""")
        app, old_run_binary, dock = do_dock()
        try:
            self.assertEqual(hasattr(dock, 'run_binary_args'), False)
        finally:
            app._run_binary = old_run_binary
        os.unlink('fd_out')
        os.unlink('in_trans')

    def test_convert_fiber_dock_to_score(self):
        """Test convert_fiber_dock_to_score()"""
        app, idock = self.get_dummy_idock_for_scorer()
        class MockScorer(object):
            short_name = 'mock'
        open('firedock.output', 'w').write("""
#################################################
    Sol # |   inside |     aVdW |     rVdW |      ACE |     glob |    aElec |    rElec |   laElec |   lrElec |       hb |      piS |   catpiS |    aliph |     prob | BBdeform |
1       |    15.82 |   -16.03 |     3.97 |     5.64 |   -20.01 |    -8.53 |     0.00 |    -8.41 |     4.76 |     0.00 |    -5.00 |     0.00 |     0.00 |     0.00 |     0.00 |2.57 0.76 1.77 33.15 35.83 73.77
2       |    15.25 |   -15.32 |     9.84 |     1.44 |   -14.43 |   -57.83 |   130.98 |   -26.15 |    26.80 |     0.00 |   -16.50 |     0.00 |    -2.00 |     0.00 |     0.00 |1.14 0.80 -0.11 9.81 62.06 63.74
3       |    15.73 |   -18.35 |     8.34 |    14.71 |    13.94 |   -19.89 |    59.33 |   -17.14 |    24.47 |    -0.30 |    -4.00 |     0.00 |     0.00 |     0.00 |     0.00 |2.05 1.10 2.19 29.46 32.01 66.74 | |
""")
        open('fiberdock.output', 'w').write("""
#################################################
    Sol # |     glob |     aVdW |     rVdW |      ACE |   inside |    aElec |    rElec |   laElec |   lrElec |       hb |      piS |   catpiS |    aliph |     prob | BBdeform |
1       |   -20.01 |   -16.03 |     3.97 |     5.64 |    15.82 |    -8.53 |     0.00 |    -8.41 |     4.76 |     0.00 |    -5.00 |     0.00 |     0.00 |     0.00 |     0.00 |2.57 0.76 1.77 33.15 35.83 73.77 | |
2       |   -14.43 |   -15.32 |     9.84 |     1.44 |    15.25 |   -57.83 |   130.98 |   -26.15 |    26.80 |     0.00 |   -16.50 |     0.00 |    -2.00 |     0.00 |     0.00 |1.14 0.80 -0.11 9.81 62.06 63.74 | |
3       |    13.94 |   -18.35 |     8.34 |    14.71 |    15.73 |   -19.89 |    59.33 |   -17.14 |    24.47 |    -0.30 |    -4.00 |     0.00 |     0.00 |     0.00 |     0.00 |2.05 1.10 2.19 29.46 32.01 66.74 | |
""")
        for in_fn in ('fiberdock.output', 'firedock.output'):
            fn = idock.convert_fiber_dock_to_score([MockScorer()], in_fn)
            self.assertEqual(fn, 'fd_res_mock.res')
            lines = open(fn).readlines()
            self.assertEqual(len(lines), 4)
            self.assertEqual(lines[1].strip(' \r\n'),
                "1 | -20.010 |  +  | -0.886 | 2.57 0.76 1.77 33.15 35.83 73.77")
            os.unlink(fn)
            os.unlink(in_fn)

    def test_run_fiber_dock(self):
        """Test run_fiber_dock()"""
        app = self.import_python_application('idock.py')
        class Dummy(app.IDock):
            def get_transforms_for_fiber_dock(self, scorers, transforms_file):
                return transforms_file + '_fd'
            def add_hydrogens(self, in_pdb):
                return in_pdb + '.HB'
            def make_fiber_dock_parameters(self, scorers, receptorH, ligandH,
                                           trans_for_fd):
                return 'tparams', trans_for_fd + '_' + receptorH + '_' + ligandH
            def do_fiber_dock_docking(self, trans_for_fd, params, fd_out):
                self.params = params
            def convert_fiber_dock_to_score(self, scorers, fd_out):
                return self.params + '_' + scorers[0] + '_' + fd_out

        class Opts(object):
            pass
        dock = Dummy()
        dock.opts = Opts()
        dock.receptor = 'testrecep'
        dock.ligand = 'testlig'
        fn = dock.run_fiber_dock(['mockscore'], 'in_trans')
        self.assertEqual(fn,
                    ('in_trans_fd',
                     'tparams_mockscore_in_trans_fd_testrecep.HB_testlig.HB'))

    def test_combine_final_scores(self):
        """Test combine_final_scores()"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.prefix = ''
        idock.opts.map_file = 'test.mrc'
        s = app.EM3DScorer(idock)
        open('em3d_scoref.res', 'w').write("""
     # | 1-CC   |filter| Zscore | Transformation
     1 |  0.196 |  -   |   2.45 | 2.423 0.1092 -0.2944 36.17 -8.459 49.66
     2 |  0.221 |  +   |   3.19 | 2.674 0.4152 -0.7746 33.23 -4.204 31.47
     3 |  0.233 |  +   |   3.53 | 2.622 0.5735 -0.7227 34.3 -2.353 32.4
""")
        open('fd_res.res', 'w').write("""
    # | Score | filt| ZScore | Transformation
     1 | -20.010|  -   |  -0.270| 2.423 0.1092 -0.2944 36.17 -8.459 49.66
     2 | -14.430|  +   |  -0.019| 2.674 0.4152 -0.7746 33.23 -4.204 31.47
     3 | 13.940 |  +   |  1.254 | 2.622 0.5735 -0.7227 34.3 -2.353 32.4
""")
        fn = idock.combine_final_scores([s], 'fd_res.res')
        lines = open('combined_final.res').readlines()
        self.assertEqual(len(lines), 3)
        self.assertEqual(lines[1].strip(' \r\n'),
                '2 |  3.171 |  +  | -1.000 |  0.221 |  3.190 | -14.430 |'
                ' -0.019 |  2.674 0.4152 -0.7746 33.23 -4.204 31.47')
        os.unlink('em3d_scoref.res')
        os.unlink('fd_res.res')
        os.unlink('combined_final.res')

    def test_write_results(self):
        """Test write_results()"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.prefix = ''
        class MockScorer(object):
            short_name = 'mock'
        open('comb_final', 'w').write("""
     # |  Score | filt| ZScore | Score0 | Zscore0 |Score1 | Zscore1 |Transformation
 18901 | -5.225 |  +  | -3.318 | 16.304 | -1.454 |  0.685 | -1.829 |   2.4462 0.7439 2.0137 32.0310 36.5010 74.9757
 25924 | -3.976 |  +  | -4.525 | 15.486 | -2.746 |  0.670 | -1.029 |   0.9110 0.6830 -0.1227 15.1862 66.0876 62.1971
 11663 | -2.537 |  +  | -1.611 | 16.079 | -1.809 |  0.677 | -1.402 |  2.0717 0.9217 2.3549 28.7951 33.5563 67.5095
""")
        fn = idock.write_results([MockScorer()], 'comb_final')
        self.assertEqual(fn, 'results_mock.txt')
        lines = open('results_mock.txt').readlines()
        self.assertEqual(len(lines), 6)
        self.assertEqual(lines[3].strip(' \r\n'),
            "1 |  -3.976 |  +  | -4.525 | 15.486 | -2.746 |  0.670 | -1.029 "
            "|   0.9110 0.6830 -0.1227 15.1862 66.0876 62.1971")
        os.unlink('comb_final')
        os.unlink('results_mock.txt')

    def test_main(self):
        """Test IDock.main()"""
        app = self.import_python_application('idock.py')
        class MockDock(app.IDock):
            def run_patch_dock_binary(s, binary, args):
                # Skip surface/parameters; copy example PatchDock output
                # instead of running PatchDock itself
                if 'patch_dock' in binary:
                    shutil.copy(self.get_input_file_name('docking.res'), '.')
                elif 'interface_cluster' in binary:
                    shutil.copy(self.get_input_file_name('clustered_cxms.res'),
                                '.')
            def run_fiber_dock_binary(s, binary, args):
                # Skip parameters; copy example FiberDock output
                # instead of running FiberDock itself
                if 'Params' not in binary:
                    shutil.copy(self.get_input_file_name('fd_res_cxms.ref'),
                                '.')
            def add_hydrogens(self, in_pdb):
                # Skip running reduce
                return in_pdb + '.HB'

        old_sys_argv = sys.argv
        try:
            sys.argv = [sys.argv[0], self.get_input_file_name('testrecep.pdb'),
                        self.get_input_file_name('testlig.pdb'),
                        '--cxms',
                        self.get_input_file_name('testcxms.txt')]
            dock = MockDock()
            dock.main()
            lines = open('results_cxms.txt').readlines()
            self.assertEqual(len(lines), 10)
            self.assertEqual(lines[3].strip(' \r\n'),
                 '1 |  -1.603 |  +  | -1.301 |  0.004 |  0.579 | -10.200 |'
                 ' -2.182 |   -0.9194 -1.0958 -2.0672 -2.7843 -1.1249 -3.0764')
            for f in ['clustered_cxms.res', 'combined_final.res',
                      'cxms_score.res', 'cxms_scoref.res',
                      'docking.res', 'fd_res_cxms.ref',
                      'fd_res_cxms.res', 'results_cxms.txt',
                      'trans_for_cluster', 'trans_for_fd_cxms']:
                os.unlink(f)
        finally:
            sys.argv = old_sys_argv

if __name__ == '__main__':
    IMP.test.main()
