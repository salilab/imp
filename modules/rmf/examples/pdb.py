## \example rmf/pdb.py
# Write a PDB to an RMF file.
#

import IMP.atom
import IMP.rmf
import RMF
import sys

IMP.setup_from_argv(sys.argv, "pdb")

m = IMP.Model()

# Create a new IMP.atom.Hierarchy from the contents of the pdb file
h = IMP.atom.read_pdb(IMP.rmf.get_example_path("simple.pdb"), m)

tfn = "pdb.rmf"

print("File name is", tfn)

# open the file, clearing any existing contents
rh = RMF.create_rmf_file(tfn)

# add the hierarchy to the file
IMP.rmf.add_hierarchies(rh, [h])

# add the current configuration to the file as frame 0
IMP.rmf.save_frame(rh)

# change a coordinate
IMP.core.XYZ(IMP.atom.get_leaves(h)[0]).set_x(0)

# add the new configuration to the file as frame 1
IMP.rmf.save_frame(rh)

# close the file
del rh

# reopen it, don't clear the file when opening it
rh = RMF.open_rmf_file_read_only(tfn)

# hps is a list with one element which is a copy of h
hps = IMP.rmf.create_hierarchies(rh, m)

IMP.atom.show_molecular_hierarchy(hps[0])

# load the second configuration into hps
IMP.rmf.load_frame(rh, RMF.FrameID(0))

print("Try running rmf_display or rmf_show on", tfn)
