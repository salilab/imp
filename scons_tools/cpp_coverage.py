import sys
import os
import glob
import environment

class _CoverageTester(object):
    def __init__(self, env, coverage, test_type, output_file):
        self._env = env
        self._test_type = None
        self._files = []
        self._output_file = output_file
        self._coverage = coverage
        self._name = name = environment.get_current_name(env)
        if test_type.startswith('module'):
            self._test_type = 'module'
            self.add('modules/%s/src' % name, '*.cpp', report=True)
            self.add('modules/%s/src/internal' % name, '*.cpp', report=True)
        elif test_type.startswith('application'):
            self._test_type = 'application'
            self.add('applications/%s' % name, '*.cpp', report=True)

    def add(self, directory, pattern, report):
        self._files.append([directory, pattern, report])

    def Execute(self, *args, **keys):
        self._cleanup_coverage_files()
        ret = self._env.Execute(*args, **keys)
        if self._test_type:
            self._report()
        self._cleanup_coverage_files()
        return ret

    def _cleanup_coverage_files(self):
        for dir, pattern, report in self._files:
            for f in glob.glob(os.path.join(dir, '*.gcda')):
                os.unlink(f)

    def _report(self):
        if self._coverage == 'lines':
            self._report_lines()

    def _report_lines(self):
        outfile = self._output_file + '.cppcoverage'
        fh = open(outfile, 'w')
        print >> fh, "%-41s Stmts   Exec  Cover   Missing" % "Name"
        divider = "-" * 71
        print >> fh, divider
        total_statements = total_executed = 0
        for dir, pattern, report in self._files:
            if report:
                self._run_gcov(dir, pattern)
                covs = glob.glob("*.gcov")
                covs.sort()
                for cov in covs:
                    source, statements, executed, missing \
                                 = self._parse_gcov_file(cov)
                    # Skip system and non-IMP headers
                    # Note that we currently report only on cpp files
                    # (not headers). This is because gcov coverage information
                    # on headers is stored in the cpp file that includes them,
                    # so for each header we need to merge coverage info from
                    # all cpp files (todo).
                    if not source.startswith('/') and source.endswith('.cpp'):
                        self._report_gcov_file(fh, source, statements, executed,
                                               missing)
                        total_statements += statements
                        total_executed += executed
                    os.unlink(cov)
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
        for line in open(cov):
            spl = line.split(':', 2)
            calls = spl[0].strip()
            line_number = int(spl[1])
            if calls == '#####':
                missing.append(line_number)
            if line_number == 0:
                if spl[2].startswith('Source:'):
                    source = os.path.normpath(spl[2][7:].strip())
            elif calls != '-':
                executable_statements += 1
        return (source, executable_statements,
                executable_statements - len(missing), missing)

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

    def _run_gcov(self, dir, pattern):
        import subprocess
        # Note that since gcov insists on dumping .gcov in the current
        # directory and that .cpp and coverage files are in the same
        # directory, this probably won't work with parallel builds (todo: fix)
        # or out of tree builds.
        cmd = 'gcov -l -p -o %s %s' % (dir, os.path.join(dir, pattern))
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
        out = p.stdout.read()
        err = p.stderr.read()
        ret = p.wait()
        if ret != 0:
            raise OSError("'%s' failed with code %d, error %s" \
                          % (cmd, ret, err))
