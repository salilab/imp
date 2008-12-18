"""Use the Python pygments library to perform extra checks on C++ grammar."""

from pygments import token
from pygments.lexers.compiled import CppLexer
import re
import os

def check_header_file(fh_name, errors):
    """Check a single C++ header file"""
    _check_file(fh_name, True, errors)

def check_cpp_file(fh_name, errors):
    """Check a single C++ source file"""
    _check_file(fh_name, False, errors)


def _check_file(fh_name, header, errors):
    fh, filename = fh_name
    s = tokenize_file(fh)
    check_tokens(s, filename, header, errors)

def tokenize_file(fh):
    """Use the Python pygments library to tokenize a C++ file"""
    code = fh.read()
    c = CppLexer()
    scan = []
    for (index, tok, value) in c.get_tokens_unprocessed(code):
        scan.append((tok, value))
    return scan

def check_tokens(scan, filename, header, errors):
    check_comment_header(scan, filename, errors)
    check_eol(scan, filename, errors)
    if header:
        check_header_start_end(scan, filename, errors)

def check_comment_header(scan, filename, errors):
    if len(scan) < 1 or scan[0][0] != token.Comment:
        errors.append('%s:1: First line should be a comment ' % filename + \
                      'with a copyright notice and a description of the file')

def check_eol(scan, filename, errors):
    if len(scan) > 0 and ((scan[-1][0] != token.Comment.Preproc \
                           and scan[-1][0] != token.Text) \
                          or scan[-1][1] != '\n'):
        errors.append('%s:999: No end-of-line character at the ' % filename + \
                      'end of the last line in the file')
        # Add an EOL so other checks don't complain
        scan.append((token.Text, '\n'))

def have_header_guard(scan):
    return len(scan) >= 10 \
           and scan[3][0] == token.Comment.Preproc \
           and scan[3][1].startswith('ifndef') \
           and scan[6][0] == token.Comment.Preproc \
           and scan[6][1].startswith('define') \
           and scan[-3][0] == token.Comment.Preproc \
           and scan[-3][1].startswith('endif') \
           and scan[-2][0] == token.Comment

def header_guard_ok(scan, guard_prefix, guard_suffix):
    """Make sure the guard has the correct prefix and suffix, and is consistent
       between the #ifndef, #define and #endif lines"""
    guard = scan[3][1][7:]
    return guard.startswith(guard_prefix) and guard.endswith(guard_suffix) \
           and scan[6][1] == 'define ' + guard \
           and scan[-2][1] == '/* %s */' % guard

def get_header_guard(filename):
    """Get prefix and suffix for header guard"""
    guard_prefix = "IMP"
    module = 'IMP'
    m = re.search('modules\/(\w+)\/', filename)
    if m:
        module = m.group(1)
        guard_prefix += module.upper()
    base = os.path.basename(filename)
    # For convenience remove leading module name qualifier if present
    for prefix in (module, '_'):
        if base.startswith(prefix):
            base = base[len(prefix):]
    def repl(match):
        return match.group(1).upper() + '_' + match.group(2)
    # Convert CamelCase into CAPS_SEPARATED_BY_UNDERSCORES
    guard_suffix = re.subn('([a-z]+|[A-Z]{2,})([A-Z0-9])', repl,
                           base)[0].upper()[:-2] + '_H'
    return guard_prefix, guard_suffix

def check_header_start_end(scan, filename, errors):
    guard_prefix, guard_suffix = get_header_guard(filename)
    if not have_header_guard(scan) \
       or not header_guard_ok(scan, guard_prefix, guard_suffix):
        header_guard = guard_prefix + '_' + guard_suffix
        errors.append('%s:%d: Missing or incomplete header guard.' \
                      % (filename, 1) + """
Header files should start with a comment, then a blank line, then the rest
of the file wrapped with a header guard. This must start with %s
and end with %s - in between can be placed extra qualifiers, e.g. for a
namespace. For example,

/** Copyright and file description */

#ifndef %s
#define %s
...
#endif  /* %s */
""" % (guard_prefix, guard_suffix, header_guard, header_guard, header_guard))
