## \example all_scores.py
## Show how to traverse an RMF file and print out all the scores in
## bulk. \c inf is used as a placeholder for values that are not there.
import RMF
import sys
import os.path
fname= os.path.join(os.path.split(sys.argv[0])[0],"bd_trajectory.rmf")
rmf= RMF.open_rmf_file_read_only(fname)
scf= RMF.ScoreConstFactory(rmf)
def traverse(n):
    if n.get_type() == RMF.FEATURE and scf.get_is(n):
        print n.get_name()
        d= scf.get(n, RMF.ANY_FRAME)
        print d.get_all_scores()
    for c in n.get_children():
        traverse(c)
rmf.set_current_frame(RMF.ALL_FRAMES)
traverse(rmf.get_root_node())
