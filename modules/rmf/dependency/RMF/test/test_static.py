import RMF
for suffix in RMF.suffixes:
    path = RMF._get_temporary_file_path("alias." + suffix)
    f = RMF.create_rmf_file(path)
    f0 = f.add_frame("frame", RMF.FRAME)
    cat = f.get_category("cat")
    k = f.get_key(cat, "key", RMF.int_tag)
    n = f.get_root_node()
    n.set_static_value(k, 1)
    del f
    del n
    f = RMF.open_rmf_file_read_only(path)
    f.set_current_frame(f0)
    n = f.get_root_node()
    assert(n.get_value(k) == 1)
