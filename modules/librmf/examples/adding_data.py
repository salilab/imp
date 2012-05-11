import IMP.atom
import RMF
import IMP.rmf
m= IMP.Model()

# Create a new IMP.atom.Hierarchy from the contents of the pdb file
h= IMP.atom.read_pdb(RMF.get_example_path("simple.pdb"), m)

# find the name for a temporary file to use to for writing the hdf5 file
tfn=IMP.create_temporary_file_name("pdb", ".rmf")

print "File name is", tfn

# open the temporary file, clearing any existing contents
rh = RMF.create_rmf_file(tfn)

my_kc= rh.add_category("my data");

# add the hierarchy to the file
IMP.rmf.add_hierarchy(rh, h)

# change a coordinate
IMP.core.XYZ(IMP.atom.get_leaves(h)[0]).set_x(0)

# add the new configuration to the file as frame 1
IMP.rmf.save_frame(rh, 1)

# create my key
my_key= RMF.get_float_key_always(rh, my_kc, "my score", True)

# make up scores
rh.get_root_node().set_value(my_key, 3, 0)
rh.get_root_node().set_value(my_key, 5, 1)
