import RMF


def add(path):
    f = RMF.create_rmf_file(path)
    f0 = f.add_frame("frame", RMF.FRAME)
    cat = f.get_category("cat")
    k = f.get_key(cat, "key", RMF.int_tag)
    k1 = f.get_key(cat, "key1", RMF.int_tag)
    n = f.get_root_node()
    #n.set_frame_value(k, 1)
    #n.set_frame_value(k1, 3)
    f1 = f.add_frame("frame1", RMF.FRAME)
    n1 = n.add_child("child", RMF.REPRESENTATION)
    #n1.set_frame_value(k, 2)
    n1.set_frame_value(k1, 4)
    f2 = f.add_frame("frame2", RMF.FRAME)
    n1.set_frame_value(k, 6)
    RMF.show_hierarchy_with_values(n)


def check(path):
    f = RMF.open_rmf_file_read_only(path)
    f.set_current_frame(RMF.FrameID(0))
    cat = f.get_category("cat")
    k = f.get_key(cat, "key", RMF.int_tag)
    k1 = f.get_key(cat, "key1", RMF.int_tag)
    n = f.get_root_node()
    RMF.show_hierarchy_with_values(n)
    n1 = n.get_children()[0]
    #assert(n.get_value(k) == 1)
    f.set_current_frame(RMF.FrameID(1))
    RMF.show_hierarchy_with_values(n)
    #assert(n.get_value(k1) is None)
    assert(n1.get_value(k) is None)
    f.set_current_frame(RMF.FrameID(2))
    RMF.show_hierarchy_with_values(n)
    assert(n1.get_value(k1) is None)

for suffix in RMF.suffixes:
    path = RMF._get_temporary_file_path("late_add." + suffix)
    print(path)

    add(path)
    check(path)
