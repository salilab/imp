import RMF
def traverse(nh, pf, frame):
    # check if the current node has coordinates and radius
    if pf.get_is(nh, frame):
        # create a decorator for the current node
        d=pf.get(nh, frame)
        print nh.get_name(), d.get_coordinates()
    # recurse on the children
    children= nh.get_children()
    for c in children:
        traverse(c, pf, frame)

fch= RMF.open_rmf_file_read_only(RMF.get_example_path("simple.rmf"))
pf= RMF.ParticleConstFactory(fch)
traverse(fch.get_root_node(), pf, 0)
