import IMP.atom
import IMP.hdf5
m= IMP.Model()
h= IMP.atom.read_pdb(IMP.hdf5.get_example_path("simple.pdb"), m)

# create a tmp file and overwrite the contents
# sorry about the silliness necessary to get binary access to a temp file
tfn=IMP.create_temporary_file_name("pdb", "hdf5")

print "File name is", tfn
rh = IMP.hdf5.RootHandle(tfn, True)
# write the hierarchy to the file
IMP.hdf5.add_hierarchy(rh, h)

# change a coordinate
IMP.core.XYZ(IMP.atom.get_leaves(h)[0]).set_x(0)
# add the new configuration to the file
IMP.hdf5.save_frame(rh, 1, h)

# close the file
del rh

# reopen it
rh= IMP.hdf5.RootHandle(tfn, False)

# hps is a list with one element which is a copy of h
hps= IMP.hdf5.create_hierarchies(rh, m)

# load the second configuration into hps
IMP.hdf5.load_frame(rh, 1, hps[0])

print "Try running hdf5_display or hdf5_show on", tfn
