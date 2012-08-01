import sys
import RMF
import time

if len(sys.argv)==1:
    print "Must pass file name"
    sys.exit(0)

f= RMF.open_rmf_file(sys.argv[1])

num_frames=-1
while True:
    print "locking...",
    l= RMF.FileLock(f)
    print "locked"
    f.reload()
    if f.get_number_of_frames() != num_frames:
        num_frames=f.get_number_of_frames()
        print num_frames
    del l
    time.sleep(1)
