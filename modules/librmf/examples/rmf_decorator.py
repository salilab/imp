import RMF
def traverse(nh, pf):
    # check if the current node has coordinates and radius
    if pf.get_is(nh):
        # create a decorator for the current node
        d=pf.get(nh)
        print nh.get_name(), d.get_coordinates()
    # recurse on the children
    children= nh.get_children()
    for c in children:
        traverse(c, pf)

fch= RMF.open_rmf_file_read_only(RMF.get_example_path("simple.rmf"))
pf= RMF.ParticleConstFactory(fch, 0)
traverse(fch.get_root_node(), pf)
