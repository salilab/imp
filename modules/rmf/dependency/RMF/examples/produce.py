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
rmf= RMF.create_rmf_file(sys.argv[1])

n= rmf.get_root_node()

cat= rmf.get_category("mydata")
key= rmf.get_int_key(cat, "mykey")

while True:
    cnf=rmf.get_number_of_frames()
    rmf.set_current_frame(cnf)
    n.set_value(key, cnf)
    print "wrote frame", cnf
    time.sleep(1)
