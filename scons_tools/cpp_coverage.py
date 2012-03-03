import sys
import os
import shutil
import glob
import fnmatch
import tempfile
import environment

class _TempDir(object):
    """Simple RAII-style class to make a temporary directory for gcov"""
    def __init__(self):
        self._origdir = os.getcwd()
        self.tmpdir = tempfile.mkdtemp()
        # Fool gcov into thinking this dir is the IMP top-level dir
        for subdir in ('build', 'modules'):
            os.symlink(os.path.join(self._origdir, subdir),
                       os.path.join(self.tmpdir, subdir))
    def __del__(self):
        shutil.rmtree(self.tmpdir, ignore_errors=True)


class _CoverageTester(object):
    def __init__(self, env, coverage, test_type, output_file):
        self._env = env
        self._test_type = None
        self._sources = []
        self._headers = []
        self._header_callcounts = {}
        self._output_file = output_file
        self._coverage = coverage
        self._name = name = environment.get_current_name(env)
        if test_type.startswith('module'):
            self._test_type = 'module'
            self.add_source('modules/%s/src' % name, '*.cpp', report=True)
            self.add_source('modules/%s/src/internal' % name, '*.cpp',
                            report=True)
            self.add_source('build/src', 'IMP_%s_wrap.cpp' % name, report=False)
            if name == 'kernel':
                h = 'IMP'
            else:
                h = 'IMP/' + name
            self.add_header('build/include/%s' % h, '*.h', report=True)
            self.add_header('build/include/%s/internal' % h, '*.h', report=True)
        elif test_type.startswith('application'):
            self._test_type = 'application'
            self.add_source('applications/%s' % name, '*.cpp', report=True)
            self.add_header('applications/%s' % name, '*.h', report=True)

    def add_source(self, directory, pattern, report):
        self._sources.append([directory, pattern, report])

    def add_header(self, directory, pattern, report):
        self._headers.append([directory, pattern, report])

    def Execute(self, *args, **keys):
        self._cleanup_coverage_files()
        ret = self._env.Execute(*args, **keys)
        if self._test_type:
            self._report()
        self._cleanup_coverage_files()
        return ret

    def _cleanup_coverage_files(self):
        for dir, pattern, report in self._sources:
            # todo: glob pattern to fix parallel runs
            for f in glob.glob(os.path.join(dir, '*.gcda')):
                os.unlink(f)

    def _report(self):
        if self._coverage == 'lines':
            self._report_lines()

    def _report_lines(self):
        t = _TempDir()
        outfile = self._output_file + '.cppcoverage'
        fh = open(outfile, 'w')
        print >> fh, "%-41s Stmts   Exec  Cover   Missing" % "Name"
        divider = "-" * 71
        print >> fh, divider
        total_statements = total_executed = 0
        for dir, pattern, report in self._sources:
            # Run gcov in a temporary directory so that parallel builds work
            self._run_gcov(dir, pattern, running_dir=t.tmpdir)
            covs = glob.glob(os.path.join(t.tmpdir, "*.gcov"))
            covs.sort()
            for cov in covs:
                ret = self._parse_gcov_file(cov)
                if ret:
                    source, statements, executed, missing = ret
                    self._report_gcov_file(fh, source, statements, executed,
                                           missing)
                    total_statements += statements
                    total_executed += executed
                os.unlink(cov)
        headers = self._header_callcounts.keys()
        headers.sort()
        for header in headers:
            statements, executed, missing \
               = self._summarize_header(self._header_callcounts[header])
            self._report_gcov_file(fh, header, statements, executed,
                                   missing)
            total_statements += statements
            total_executed += executed
        print >> fh, divider
        self._report_gcov_file(fh, 'TOTAL', total_statements,
                               total_executed, [])
        fh.close()
        print >> sys.stderr, \
                 "\nC++ coverage of %s %s written to %s." \
                 % (self._name, self._test_type, outfile)

    def _get_missing_ranges(self, missing):
        ranges = []
        start_range = None
        end_range = None
        def add_range():
            if start_range is not None:
                if end_range == start_range:
                    ranges.append('%d' % start_range)
                else:
                    ranges.append('%d-%d' % (start_range, end_range))
        for line in missing:
            if end_range is not None and end_range == line - 1:
                end_range = line
            else:
                add_range()
                start_range = line
                end_range = line
        add_range()
        return ", ".join(ranges)

    def _parse_gcov_file(self, cov):
        executable_statements = 0
        missing = []
        header_callcounts = None
        for line in open(cov):
            spl = line.split(':', 2)
            calls = spl[0].strip()
            line_number = int(spl[1])
            if line_number == 0:
                if spl[2].startswith('Source:'):
                    source = os.path.normpath(spl[2][7:].strip())
                    header_callcounts = self._match_header(source)
                    if header_callcounts is None \
                       and not self._match_source(source):
                        return None
            else:
                if header_callcounts is not None:
                    self._update_header_callcounts(header_callcounts,
                                                   line_number, calls)
                if calls == '#####':
                    missing.append(line_number)
                if calls != '-':
                    executable_statements += 1
        # Ignore header info for now; we'll accumulate it and display at
        # the end of all source (cpp) files
        if header_callcounts is None:
            return (source, executable_statements,
                    executable_statements - len(missing), missing)

    def _summarize_header(self, header_callcounts):
        executable_statements = 0
        missing = []
        for n, line in enumerate(header_callcounts):
            if line >= 0:
                executable_statements += 1
            if line == 0:
                missing.append(n + 1)
        return (executable_statements, executable_statements - len(missing),
                missing)

    def _update_header_callcounts(self, header_callcounts, line_number, calls):
        # All new lines are marked as non-executable (-1)
        while line_number > len(header_callcounts):
            header_callcounts.append(-1)

        # If this gcov file says the line was executable but not called
        # (call count '#####') then override another gcov file that said it was
        # non-executable (no effect if it was called in another gcov file)
        if calls == '#####':
            if header_callcounts[line_number - 1] == -1:
                header_callcounts[line_number - 1] = 0

        # Non-executable gcov lines (call count of '-') have no effect,
        # since all lines start that way (and this shouldn't override another
        # file that says the line is executable)

        # Executable lines that were called (positive integer call count)
        # override non-executable status, and accumulate
        elif calls != '-':
            count = int(calls)
            if header_callcounts[line_number - 1] == -1:
                header_callcounts[line_number - 1] = count
            else:
                header_callcounts[line_number - 1] += count

    def _match_header(self, fn):
        for dir, patt, report in self._headers:
            if report and fnmatch.fnmatch(fn, os.path.join(dir, patt)):
                if fn not in self._header_callcounts:
                    self._header_callcounts[fn] = []
                return self._header_callcounts[fn]

    def _match_source(self, fn):
        for dir, patt, report in self._sources:
            if report and fnmatch.fnmatch(fn, os.path.join(dir, patt)):
                return True

    def _report_gcov_file(self, fh, source, statements, executed, missing):
        if statements == 0:
            cover = 0
        else:
            cover = float(executed) * 100. / float(statements)

        if len(source) > 40:
            source = "[..]" + source[-36:]

        print >> fh, "%-40s %6d %6d %5d%%   %s" \
              % (source, statements, executed, cover,
                 self._get_missing_ranges(missing))

    def _run_gcov(self, dir, pattern, running_dir=None):
        import subprocess
        # Note that gcov expects to find the .cpp file in the same directory
        # as the coverage info, so annotated output probably won't work
        # with out of tree builds
        cmd = 'gcov -l -p -o %s %s' % (dir, os.path.join(dir, pattern))
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE, cwd=running_dir)
        out = p.stdout.read()
        err = p.stderr.read()
        ret = p.wait()
        if ret != 0:
            raise OSError("'%s' failed with code %d, error %s" \
                          % (cmd, ret, err))
