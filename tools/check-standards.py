#!/usr/bin/python

import sys
import os.path
from reindent import Reindenter
import re

def check_c_file(filename, errors):
    """Check each modified C file to make sure it adheres to the standards"""
    fh = file(filename, "r")
    srch = re.compile('\s+$')
    url = re.compile('https?://')
    blank = False
    for (num, line) in enumerate(fh):
        line = line.rstrip('\r\n')
        # No way to split URLs, so let them exceed 80 characters:
        if len(line) > 80 and not url.search(line):
            errors.append('%s:%d: Line is longer than 80 characters.' \
                          % (filename, num+1))
        if line.find('\t') >= 0:
            errors.append('%s:%d: Line contains tabs.' % (filename, num+1))
        if srch.match(line):
            errors.append('%s:%d: Line has trailing whitespace' \
                          % (filename, num+1))
        blank = (len(line) == 0)
        if blank and num == 0:
            errors.append('%s:1: File has leading blank line(s)' % filename)
    if blank:
        errors.append('%s:1000: File has trailing blank line(s)' % filename)

def check_python_file(filename, errors):
    """Check each modified Python file to make sure it adheres to the
       standards"""
    fh = file(filename, "r")
    r = Reindenter(fh)
    if r.run():
        errors.append('Python file ' + filename + ' has odd indentation; ' \
                      + 'please run through reindent.py first.')

def check_modified_file(filename, errors):
    """Check each modified file to make sure it adheres to the standards"""
    if filename.endswith('.h') or filename.endswith('.cpp') \
       or filename.endswith('.c'):
        check_c_file(filename, errors)
    elif filename.endswith('.py') or filename.endswith('SConscript') \
         or filename.endswith('SConstruct'):
        check_python_file(filename, errors)

def get_all_files():
    modfiles = []
    for root, dirs, files in os.walk('.'):
        if '.sconf_temp' not in root:
            for f in files:
                if not f.startswith('.'):
                    modfiles.append(os.path.join(root, f))
    excludes = ['./kernel/pyext/IMP_wrap.h', './kernel/pyext/IMP_wrap.cc',
                './impEM/pyext/IMPEM_wrap.h', './impEM/pyext/IMPEM_wrap.cc',
                './impEM/pyext/IMPEM.py', './kernel/pyext/IMP.py',
                './kernel/pyext/IMP/__init__.py']
    for ex in excludes:
        try:
            modfiles.remove(ex)
        except ValueError:
            pass
    return modfiles

def main():
    errors = []

    modfiles = get_all_files()
    for filename in modfiles:
        check_modified_file(filename, errors)

    if len(errors) > 0:
        sys.stderr.write("The following problems needed to be fixed:\n\n")
        sys.stderr.write("\n".join(errors))
        sys.stderr.write("\n")
        sys.exit(1)

if __name__ == '__main__':
    main()
