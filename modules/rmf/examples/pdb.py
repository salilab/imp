import IMP.atom
import IMP.rmf
m= IMP.Model()

# Create a new IMP.atom.Hierarchy from the contents of the pdb file
h= IMP.atom.read_pdb(IMP.rmf.get_example_path("simple.pdb"), m)

# find the name for a temporary file to use to for writing the hdf5 file
tfn=IMP.create_temporary_file_name("pdb", "hdf5")

print "File name is", tfn

# open the temporary file, clearing any existing contents
rh = IMP.rmf.RootHandle(tfn, True)

# add the hierarchy to the file
IMP.rmf.add_hierarchy(rh, h)

# change a coordinate
IMP.core.XYZ(IMP.atom.get_leaves(h)[0]).set_x(0)

# add the new configuration to the file as frame 1
IMP.rmf.save_frame(rh, 1, h)

# close the file
del rh

# reopen it, don't clear the file when opening it
rh= IMP.rmf.RootHandle(tfn, False)

# hps is a list with one element which is a copy of h
hps= IMP.rmf.create_hierarchies(rh, m)

# load the second configuration into hps
IMP.rmf.load_frame(rh, 1, hps[0])

print "Try running hdf5_display or hdf5_show on", tfn
