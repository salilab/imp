## \example rmf/link.py
# This example is like module/rmf/pdb.py except that instead of creating a
# new hierarchy from the rmf file, it simply links the existing hierarchy
# to the file. This mechanism can be used for loading multiple
# conformations for scoring or other analysis without having to set up
# restraints and things each time.

import IMP.atom
import IMP.rmf
import RMF
import sys

IMP.setup_from_argv(sys.argv, "link")

m = IMP.Model()

# Create a new IMP.atom.Hierarchy from the contents of the pdb file
h = IMP.atom.read_pdb(IMP.rmf.get_example_path("simple.pdb"), m)

tfn = "link.rmf"

print("File name is", tfn)

# open the file, clearing any existing contents
rh = RMF.create_rmf_file(tfn)

# add the hierarchy to the file
IMP.rmf.add_hierarchies(rh, [h])

# add the current configuration to the file as frame 0
IMP.rmf.save_frame(rh)

# close the file
del rh

# reopen it, don't clear the file when opening it
rh = RMF.open_rmf_file_read_only(tfn)

# link to the existing pdb hierarchy
IMP.rmf.link_hierarchies(rh, [h])

# load the same coordinates in, ok, that is not very exciting
IMP.rmf.load_frame(rh, RMF.FrameID(0))

print("Try running rmf_display or rmf_show on", tfn)
