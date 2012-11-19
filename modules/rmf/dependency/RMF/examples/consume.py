## \example create.py
## This example creates an RMF file that tries to exploit as many
## capabilities of RMF as possible.
import RMF
import time
import sys

if len(sys.argv) ==1:
    print "Run with the name of the file to create as an argument"
    sys.exit(0)

print "path is", sys.argv[1]
rmf= RMF.open_rmf_file_read_only(sys.argv[1])

n= rmf.get_root_node()

cat= rmf.get_category("mydata")
key= rmf.get_int_key(cat, "mykey")

while True:
    rmf.reload()
    rmf.set_current_frame(rmf.get_number_of_frames()-1)
    print "number of frames is", rmf.get_number_of_frames(), "and value is", n.get_value(key)
    time.sleep(1)
