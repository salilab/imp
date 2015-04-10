## \example rmf_decorator.py
# The example shows how to use the decorators by printing the
# coordates and radius of every node that has them.
from __future__ import print_function
import RMF


def traverse(nh, pf):
    # check if the current node has coordinates and radius
    if pf.get_is(nh):
        # create a decorator for the current node
        d = pf.get(nh)
        print(nh.get_name(), d.get_coordinates())
    # recurse on the children
    children = nh.get_children()
    for c in children:
        traverse(c, pf)

fch = RMF.open_rmf_file_read_only(RMF.get_example_path("simple.rmf3"))
pf = RMF.ParticleFactory(fch)
fch.set_current_frame(RMF.FrameID(0))
traverse(fch.get_root_node(), pf)
