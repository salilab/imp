import atexit
import pickle
import glob
import shutil
import os
import sys
import re
from SCons.Script import Dir
import python_coverage
import cpp_coverage
from python_coverage import coverage

def _get_all_python_coverage(covdir):
    cov = coverage.coverage(branch=True,
                            data_file=os.path.join(covdir, '.coverage'))
    python_coverage.setup_excludes(cov)
    cov.file_locator.relative_dir = Dir("#/build").abspath + '/'
    cov.combine()
    morfs = cov.data.lines.keys()
    morfs.sort()
    return cov, morfs

def _get_module_python_coverage(f):
    cov = coverage.coverage(branch=True, data_file=f)
    python_coverage.setup_excludes(cov)
    cov.file_locator.relative_dir = Dir("#/build").abspath + '/'
    cov.load()
    morfs = cov.data.lines.keys()
    morfs.sort()
    return cov, morfs

def _get_module_morfs(covdir, modtype, modname):
    fn = os.path.join(covdir, 'morfs.' + modtype + '.' + modname)
    return pickle.load(open(fn, 'rb'))

def _report_python_module(cov, covdir, morfs, name):
    try:
        cov.html_report(morfs, directory=os.path.join(covdir, 'python', name))
    except coverage.CoverageException, detail:
        print >> sys.stderr, "Python coverage of %s failed: %s" \
                             % (name, str(detail))

def _build_python_coverage(env, single, global_cov):
    covdir = Dir("#/build/coverage").abspath
    # Return if no coverage to report (i.e. no tests were run)
    coverage_files = glob.glob('%s/.coverage*' % covdir)
    if len(coverage_files) == 0:
        return
    print >> sys.stderr, "Generating Python HTML coverage report... ",

    if single:
        cov, morfs = _get_all_python_coverage(covdir)
        cov.html_report(morfs, directory=os.path.join(covdir, 'python'))
    else:
        r = re.compile('\.coverage\.(\w+)\.(\w+)')
        if global_cov:
            cov, morfs = _get_all_python_coverage(covdir)
            for f in coverage_files:
                m = r.search(f)
                mod_morfs = _get_module_morfs(covdir, m.group(1), m.group(2))
                _report_python_module(cov, covdir, mod_morfs, m.group(2))
        else:
            for f in coverage_files:
                m = r.search(f)
                cov, morfs = _get_module_python_coverage(f)
                _report_python_module(cov, covdir, morfs, m.group(2))

    print >> sys.stderr, "Done"

def _extract_info(out_info, all_info):
    """Extract lcov info for a single module or application from the
       'all of IMP' info file"""
    info_dir, fname = os.path.split(out_info)
    test_type, name, info = fname.split('.')
    files = os.path.join(info_dir, 'files.' + fname[:-5])
    want_files = pickle.load(open(files, 'rb'))
    def filter_filename(fname):
        return fname in want_files

    fin = open(all_info)
    fout = open(out_info, 'w')
    record = []
    write_record = False
    lines_written = False
    for line in fin:
        if line.startswith('SF:'):
            write_record = filter_filename(line.rstrip('\r\n')[3:])
            if test_type == 'module' and write_record:
                line = cpp_coverage.map_module_path(line, name)
                # Exclude auto-generated files that *only* live
                # in build/include
                write_record = os.path.exists(line.rstrip('\r\n')[3:])
        record.append(line)
        if line.startswith('end_of_record'):
            if write_record:
                fout.writelines(record)
                lines_written = True
            record = []
    fin.close()
    fout.close()
    # lcov falls over if given a 0-byte .info file
    if not lines_written:
        os.unlink(out_info)

def _build_cpp_coverage(env, single, group_cov):
    import subprocess
    def call(args):
        r = subprocess.call(args)
        if r != 0:
            raise OSError("%s failed with exit code %d" % (args[0], r))

    covdir = Dir("#/build/coverage").abspath
    # Return if no coverage to report (i.e. no tests were run)
    info_files = glob.glob('%s/*.info' % covdir)
    if len(info_files) == 0:
        return

    if group_cov:
        # Combine all info files
        args = ['lcov']
        for d in info_files:
            args.extend(['-a', d])
        args.extend(['-o', 'all.info'])
        call(args)
        # Make new module or application info files containing only
        # their own files
        for d in info_files:
            _extract_info(d, 'all.info')
        os.unlink('all.info')
        # Some of the .info files may have been deleted, so refresh the list
        info_files = glob.glob('%s/*.info' % covdir)

    cwd = os.path.abspath(env['repository'])
    if single:
        call(['genhtml', '--demangle-cpp', '--legend', '-p', cwd,
              '--no-branch-coverage',
              '-o', os.path.join(covdir, 'cpp')] + info_files)
    else:
        r = re.compile('\w+\.(\w+)\.info')
        for f in info_files:
            m = r.search(f)
            call(['genhtml', '--demangle-cpp', '--legend', '-p', cwd,
                  '--no-branch-coverage',
                  '-o', os.path.join(covdir, 'cpp', m.group(1)), f])

    print >> sys.stderr, "Generated C++ HTML coverage report."

def _build_html_coverage(env):
    single = 'single' in env['html_coverage']
    group_cov = 'group' in env['html_coverage']
    if env.get('pycoverage', None):
        _build_python_coverage(env, single, group_cov)
    if env.get('cppcoverage', None):
        _build_cpp_coverage(env, single, group_cov)

def register(env):
    """Set up HTML coverage"""
    covdir = Dir("#/build/coverage").abspath
    shutil.rmtree(covdir, ignore_errors=True)
    os.makedirs(covdir)
    atexit.register(_build_html_coverage, env)
