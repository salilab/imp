import RMF
import sys
import os.path
fname= os.path.join(os.path.split(sys.argv[0])[0],"bd_trajectory.rmf")
rmf= RMF.open_rmf_file_read_only(fname)
scf= RMF.ScoreConstFactory(rmf)
def traverse(n):
    if n.get_type() == RMF.FEATURE and scf.get_is(n, RMF.ANY_FRAME):
        print n.get_name()
        d= scf.get(n, RMF.ANY_FRAME)
        print d.get_all_scores()
    for c in n.get_children():
        traverse(c)

traverse(rmf.get_root_node())
