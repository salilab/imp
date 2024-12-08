## \example cloning.py
# RMF includes functions to help cloning files an extracting parts. They are
# also exposed through the [rmf_slice](\ref rmf_slice) application.

import RMF

in_name = RMF.get_example_path("3U7W.rmf")
out_name = RMF._get_temporary_file_path("3U7W.out.rmfz")

in_file = RMF.open_rmf_file_read_only(in_name)
out_file = RMF.create_rmf_file(out_name)

print("creating file", out_name)

# copy over info like the creator, this isn't essential
RMF.clone_file_info(in_file, out_file)
# copy over the hierarchy
# This is necessary in order to copy the data
RMF.clone_hierarchy(in_file, out_file)

# copy all the static data
RMF.clone_static_frame(in_file, out_file)

# go through and copy each frame
for f in in_file.get_frames():
    print("cloning frame", f)
    in_file.set_current_frame(f)
    out_file.add_frame(in_file.get_name(f), in_file.get_type(f))
    RMF.clone_loaded_frame(in_file, out_file)
