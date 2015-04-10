
%pythoncode %{
_tmpdir=None

def _get_temporary_file_path(name):
   global _tmpdir
   if not _tmpdir:
       import tempfile
       _tmpdir = tempfile.mkdtemp()
   import os.path
   return os.path.join(_tmpdir, name)

def _get_test_input_file_path(name):
   import sys
   import os.path
   dir= os.path.split(sys.argv[0])[0]
   return os.path.join(dir, "input", name)

def _os_path_split_asunder(path):
    import os.path
    parts = []
    while True:
        newpath, tail = os.path.split(path)
        if newpath == path:
            assert not tail
            if path: parts.append(path)
            break
        parts.append(tail)
        path = newpath
    parts.reverse()
    return parts


def get_example_path(name):
   import sys
   import os.path
   dir= _os_path_split_asunder(sys.argv[0])[:-1]
   dir.append(name)
   return os.path.join(*dir)

def _assert_signatures_equal(sa, sb):
  if sa == sb:
     return
  else:
     import difflib
     for l in difflib.unified_diff(sa.split("\n"), sb.split("\n")):
        stl = str(l)
        print(stl)
     raise RuntimeError("not equal")

suffixes=["rmf", "rmfz"]

try:
  import RMF_HDF5
  HDF5=RMF_HDF5
except:
  pass


%}
#if RMF_HAS_DEPRECATED_BACKENDS
%pythoncode %{
suffixes.extend(["rmf-avro", "rmf3", "rmf-hdf5", "_rmf_test_buffer"])
%}
#endif
