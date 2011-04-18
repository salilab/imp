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
IMP.hdf5.write_hierarchy(h, rh)

# change a coordinate
IMP.core.XYZ(IMP.atom.get_leaves(h)[0]).set_x(0)
# add the new configuration to the file
IMP.hdf5.save_configuration(h, rh, 1)

# hps is a list with one element which is a copy of h
hps= IMP.hdf5.read_all_hierarchies(rh, m)

# load the second configuration into hps
IMP.hdf5.load_configuration(rh, hps[0], 1)

print "Try running hdf5_display or hdf5_show on", tfn
