from __future__ import print_function
import RMF

name = RMF._get_temporary_file_path("children.rmf")
f = RMF.create_rmf_file(name)

ch = f.get_root_node().add_child("hi", RMF.REPRESENTATION)

chb = f.get_root_node().get_children()[0]
print(chb)
print(chb.get_name())
assert(isinstance(chb, type(ch)))
