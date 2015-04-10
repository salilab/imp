from __future__ import print_function
import RMF

RMF.set_log_level("trace")
path = RMF._get_temporary_file_path("writing.rmf")
print(path)
fh = RMF.create_rmf_file(path)
fh.add_frame("frame", RMF.FRAME)
fn = fh.get_root_node().add_child("frag", RMF.REPRESENTATION)

pf = RMF.ParticleFactory(fh)
ff = RMF.FragmentFactory(fh)

pf.get(fn).set_radius(1.0)
pf.get(fn).set_mass(2.0)
pf.get(fn).set_coordinates(RMF.Vector3(1, 2, 3))
ff.get(fn).set_residue_indexes([1, 2, 3, 4])
print("closing")
del fn
del fh
print("opening")
fh = RMF.open_rmf_file_read_only(path)
fh.set_current_frame(RMF.FrameID(0))
RMF.show_info(fh)
fh.set_current_frame(RMF.FrameID(0))
RMF.show_hierarchy_with_values(fh.get_root_node())

"""fn = fh.get_root_node().get_children()[0]
pf = RMF.ParticleFactory(fh)
assert(pf.get_is(fn))
"""
