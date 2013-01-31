## \example externals.py
## Show how to create an RMF file that has references to external files.
## And how to read it back, sort of.
import RMF
import os.path
import sys

input_pdb= os.path.join(os.path.split(sys.argv[0])[0],"simple.pdb")
rmf_name= RMF._get_temporary_file_path("externals.rmf")
output_pdb= RMF._get_temporary_file_path("simple.pdb")
print "file name is", rmf_name

rmf= RMF.create_rmf_file(rmf_name)

# copy simple.pdb to output dir
open(output_pdb, "w").write(open(input_pdb, "r").read())
pdb_relpath="./simple.pdb"

rpf= RMF.ReferenceFrameFactory(rmf)
ef= RMF.ExternalFactory(rmf)
# add a couple transformed copies of simple
c0= rmf.get_root_node().add_child("copy0", RMF.REPRESENTATION)
rb0= rpf.get(c0)
rb0.set_translation([0,0,0])
rb0.set_rotation([1,0,0,0])
external= c0.add_child("simple.pdb", RMF.REPRESENTATION)
ed= ef.get(external)
ed.set_path(pdb_relpath)

# add another, reusing the same reference node
c1= rmf.get_root_node().add_child("copy1", RMF.REPRESENTATION)
rb1= rpf.get(c1)
rb1.set_translation([20,0,0])
rb1.set_rotation([.5,.5,.5,.5])
c1.add_child(external)
