import RMF
RMF.set_log_level("trace")
for suffix in RMF.suffixes:
    path = RMF._get_temporary_file_path("dag." + suffix)
    print(path)
    f = RMF.create_rmf_file(path)
    root = f.get_root_node()
    ch = root.add_child("hi", RMF.REPRESENTATION)
    chch = ch.add_child("there", RMF.REPRESENTATION)
    root.add_child(chch)
    RMF.show_hierarchy(root)
    RMF.show_info(f)

    del f
    del root
    del ch
    del chch

    f = RMF.open_rmf_file_read_only(path)
