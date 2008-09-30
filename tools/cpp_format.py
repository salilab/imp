"""Use the Python pygments library to perform extra checks on C++ grammar."""

from pygments import token
from pygments.lexers.compiled import CppLexer

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

def check_comment_header(scan, filename, errors):
    if len(scan) < 1 or scan[0][0] != token.Comment:
        errors.append('%s:1: First line should be a comment ' % filename + \
                      'with a copyright notice and a description of the file')
