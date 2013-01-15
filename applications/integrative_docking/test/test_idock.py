import IMP.test
import os
import sys

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
        self.assertIn('please provide one or more types', err)
        self.assertNotEqual(p.returncode, 0)

    def test_idock_init(self):
        """Test IDock class init"""
        app = self.import_python_application('idock.py')
        dock = app.IDock('x', 'y', 'z')
        self.assertEqual(dock.opts, 'x')
        self.assertEqual(dock.receptor, 'y')
        self.assertEqual(dock.ligand, 'z')

    def make_idock_with_captured_subprocess(self):
        """Make an IDock instance. Capture the command lines of any
           subprocesses, rather than actually running them."""
        app = self.import_python_application('idock.py')
        class Opts(object):
            pass
        d = app.IDock(Opts(), '', '')
        def _run_binary(*args):
            d.run_binary_args = args
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


    def test_run_binary(self):
        """Test _run_binary()"""
        class DummySubprocess(object):
            args = None
            class Popen(object):
                def __init__(self, *args):
                    DummySubprocess.args = self.args = args
                def wait(self):
                    if 'bad' in self.args[0][0]:
                        return 1
                    else:
                        return 0
        app = self.import_python_application('idock.py')
        oldsubproc = app.subprocess
        try:
            app.subprocess = DummySubprocess
            self.assertRaises(OSError, app._run_binary, 'testpath', 'bad', [])
            app._run_binary('testpath', 'bin', ['arg1', 'arg2'])
            self.assertEqual(app.subprocess.args,
                             (['testpath/bin', 'arg1', 'arg2'],))
            app._run_binary('', 'bin', ['arg1', 'arg2'])
            self.assertEqual(app.subprocess.args,
                             (['bin', 'arg1', 'arg2'],))
        finally:
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
        dock = app.IDock(Opts(), 'y', 'z')
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
        dock = app.IDock(Opts(), 'y', 'z')
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
        dock = Dummy(Opts(), 'testrecep', 'testlig')
        dock.calls = []
        dock.opts.type = 'other'
        n = dock.run_patch_dock()
        self.assertEqual(n, 42)
        self.assertEqual(dock.calls, ['surface', 'params', 'docking',
                                      'transforms'])
        self.assertEqual(dock.ligand, 'testlig')
        self.assertEqual(dock.receptor, 'testrecep')

        # Check for switch when type='AA'
        dock = Dummy(Opts(), 'testrecep', 'testlig')
        dock.calls = []
        dock.opts.saxs_receptor_pdb = 'saxsrecep'
        dock.opts.saxs_ligand_pdb = 'saxslig'
        dock.opts.type = 'AA'
        dock.run_patch_dock()
        self.assertEqual(dock.ligand, 'testrecep')
        self.assertEqual(dock.receptor, 'testlig')
        self.assertEqual(dock.opts.saxs_receptor_pdb, 'saxslig')
        self.assertEqual(dock.opts.saxs_ligand_pdb, 'saxsrecep')

    def test_get_scorers(self):
        """Test IDock.get_scorers()"""
        app = self.import_python_application('idock.py')
        class Opts(object):
            pass
        d = app.IDock(Opts(), 'testrecep', 'testlig')
        d.opts.prefix = 'pre'
        d.opts.type = 'other'
        d.opts.receptor_rtc = 'testrecep_rtc'
        d.opts.ligand_rtc = 'testrecep_rtc'
        d.opts.saxs_file = 'testsaxs'
        d.opts.saxs_receptor = d.opts.saxs_ligand = None
        d.opts.class_averages = []
        d.opts.map_file = 'test.mrc'
        d.opts.cross_links_file = None
        scorers = d.get_scorers()
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
        if os.path.exists('testoutput'):
            os.unlink('testoutput')
        # Score should be recalculated if output file does not exist
        s = MyScorer(MockIDock(), 'testoutput')
        s.score(10)
        self.assertEqual(s.run_score, True)

        open('testoutput', 'w').write('foo\n' * 8)
        # Also if the file contains fewer transforms than input
        s = MyScorer(MockIDock(), 'testoutput')
        s.score(10)
        self.assertEqual(s.run_score, True)

        # Skip score if number of transforms is sufficient
        s = MyScorer(MockIDock(), 'testoutput')
        s.score(8)
        self.assertEqual(hasattr(s, 'run_score'), False)
        os.unlink('testoutput')

    def get_dummy_idock_for_scorer(self):
        app = self.import_python_application('idock.py')
        class Opts(object):
            pass
        idock = app.IDock(Opts(), 'testrecep', 'testlig')
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
        idock.opts.receptor_rtc = 'r_rtc'
        idock.opts.ligand_rtc = 'l_rtc'
        idock.opts.type = 'other'
        s = app.NMRScorer(idock)
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
        idock.opts.saxs_file = 'test.saxs'
        idock.opts.saxs_receptor = idock.opts.saxs_ligand = None
        s = app.SAXSScorer(idock)
        self.assertEqual(s.saxs_file, 'test.saxs')
        self.assertEqual(s.saxs_receptor, 'testrecep')
        self.assertEqual(s.saxs_ligand, 'testlig')
        idock.opts.saxs_receptor = 'testrecep.s'
        idock.opts.saxs_ligand = 'testlig.s'
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
        idock.opts.class_averages = ['test1.pgm', 'test2.pgm']
        idock.opts.pixel_size = 4.0
        s = app.EM2DScorer(idock)
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
        idock.opts.map_file = 'test.mrc'
        s = app.EM3DScorer(idock)
        self.assertEqual(s.map_file, 'test.mrc')
        self.assertEqual(str(s), 'EM3D score')
        self.assertEqual(self.run_scorer_score(s),
                         (None, 'em3d_score', ['testrecep', 'testlig',
                          'trans_pd', 'test.mrc', '-o', 'em3d_score.res',
                          '-s']))

    def test_cxms_scorer(self):
        """Test CXMSScorer class"""
        app, idock = self.get_dummy_idock_for_scorer()
        idock.opts.cross_links_file = 'test.cxms'
        s = app.CXMSScorer(idock)
        self.assertEqual(s.cross_links_file, 'test.cxms')
        self.assertEqual(str(s), 'CXMS score')
        self.assertEqual(self.run_scorer_score(s),
                         (None, 'cross_links_score', ['testrecep', 'testlig',
                          'trans_pd', 'test.cxms', '-o', 'cxms_score.res']))

if __name__ == '__main__':
    IMP.test.main()
