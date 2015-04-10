"""Use the Python pygments library to perform extra checks on C++ grammar."""

from pygments import token
from pygments.lexers.compiled import CppLexer
import re
import os


def check_header_file(fh_name, project_name, errors):
    """Check a single C++ header file"""
    _check_file(fh_name, project_name, True, errors)


def check_cpp_file(fh_name, project_name, errors):
    """Check a single C++ source file"""
    _check_file(fh_name, project_name, False, errors)


def _check_file(fh_name, project_name, header, errors):
    fh, filename = fh_name
    s = tokenize_file(fh)
    check_tokens(s, filename, project_name, header, errors)


def tokenize_file(fh):
    """Use the Python pygments library to tokenize a C++ file"""
    code = fh.read()
    c = CppLexer()
    scan = []
    for (index, tok, value) in c.get_tokens_unprocessed(code):
        scan.append((tok, value))
    return scan


def check_tokens(scan, filename, project_name, header, errors):
    if filename.find("test_") == -1:
        # we don't do it for python tests
        check_comment_header(scan, filename, errors)
    if header:
        # Handle older versions of pygments which concatenate \n and # tokens
        if len(scan) >= 3 and scan[2][0] == token.Comment.Preproc \
           and scan[2][1] == '\n#':
            scan[2] = (token.Comment.Preproc, '#')
            scan.insert(2, (token.Comment.Text, '\n'))
        check_header_start_end(scan, filename, project_name, errors)


def check_comment_header(scan, filename, errors):
    if len(scan) < 1 or scan[0][0] not in (token.Comment,
                                           token.Comment.Multiline):
        errors.append('%s:1: First line should be a comment ' % filename +
                      'with a copyright notice and a description of the file')


def have_header_guard(scan):
    return len(scan) >= 11 \
        and scan[4][0] == token.Comment.Preproc \
        and scan[4][1].startswith('ifndef') \
        and scan[7][0] == token.Comment.Preproc \
        and scan[7][1].startswith('define') \
        and scan[-3][0] == token.Comment.Preproc \
        and scan[-3][1].startswith('endif') \
        and scan[-2][0] in (token.Comment, token.Comment.Multiline)


def get_header_guard(filename, project_name):
    """Get prefix and suffix for header guard"""
    guard_prefix = project_name.replace(".", "").upper()
    guard_suffix = os.path.split(filename)[1].replace(".", "_").upper()
    return guard_prefix, guard_suffix


def check_header_start_end(scan, filename, project_name, errors):
    guard_prefix, guard_suffix = get_header_guard(filename, project_name)
    header_guard = guard_prefix + '_' + guard_suffix
    bad = False
    if not len(scan) >= 11:
        bad = True
    if not scan[4][0] == token.Comment.Preproc:
        bad = True
    if not scan[4][1].startswith('ifndef'):
        errors.append('%s:%d: Header guard missing #ifndef.'
                      % (filename, 1))
        bad = True
    if not scan[7][0] == token.Comment.Preproc:
        bad = True
    if not scan[7][1].startswith('define'):
        errors.append('%s:%d: Header guard missing #definer.'
                      % (filename, 1))
        bad = True
    if not scan[-3][0] == token.Comment.Preproc and not scan[-4][0] == token.Comment.Preproc:
        bad = True
    if not scan[-3][1].startswith('endif') and not scan[-4][1].startswith('endif'):
        errors.append('%s:%d: Header guard missing #endif.'
                      % (filename, 1))
        bad = True
    if not scan[-2][0] in (token.Comment, token.Comment.Multiline) and not scan[-3][0] in (token.Comment, token.Comment.Multiline):
        errors.append('%s:%d: Header guard missing closing comment.'
                      % (filename, 1))
        bad = True

    guard = scan[4][1][7:]
    if not guard.startswith(guard_prefix):
        errors.append('%s:%d: Header guard does not start with "%s".'
                      % (filename, 1, guard_prefix))
        bad = True
    if not guard.replace("_", "").endswith(guard_suffix.replace("_", "")):
        errors.append('%s:%d: Header guard does not end with "%s".'
                      % (filename, 1, guard_suffix))
        bad = True
    if not scan[7][1] == 'define ' + guard:
        errors.append('%s:%d: Header guard does define "%s".'
                      % (filename, 1, guard))
        bad = True
    if not scan[-2][1] == '/* %s */' % guard and not scan[-3][1] == '/* %s */' % guard:
        errors.append('%s:%d: Header guard close does not have a comment of "/* %s */".'
                      % (filename, 1, guard))
        bad = True
    if bad:
        errors.append('%s:%d: Missing or incomplete header guard.'
                      % (filename, 1) + """
Header files should start with a comment, then a blank line, then the rest
of the file wrapped with a header guard. This must start with %s
and end with %s - in between can be placed extra qualifiers, e.g. for a
namespace. For example,

/** Copyright and file description */

#ifndef %s
#define %s
...
#endif /* %s */
""" % (guard_prefix, guard_suffix, header_guard, header_guard, header_guard))
