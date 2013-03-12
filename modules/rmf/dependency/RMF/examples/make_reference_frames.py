## \example make_rigid_body.py
## This simple example makes an RMF file with several rigid copies of the same thing
import RMF

file_name= RMF._get_temporary_file_path("reference_frame.rmfa")
print "file is", file_name
fh= RMF.create_rmf_file(file_name)

rh= fh.get_root_node()

reference_frame_factory = RMF.ReferenceFrameFactory(fh)
segment_factory = RMF.SegmentFactory(fh)
color_factory= RMF.ColoredFactory(fh)

# first make a copy at the origin
origin= rh.add_child("origin", RMF.REPRESENTATION)
rbo= reference_frame_factory.get(origin)
rbo.set_translation([0,0,0])
rbo.set_rotation([1,0,0,0])
x= origin.add_child("x", RMF.GEOMETRY)
sx= segment_factory.get(x)
sx.set_coordinates([[0,1], [0,0], [0,0]])
cx= color_factory.get(x)
cx.set_rgb_color([1,0,0])
y= origin.add_child("y", RMF.GEOMETRY)
sy= segment_factory.get(y)
sy.set_coordinates([[0,0], [0,1], [0,0]])
cy= color_factory.get(y)
cy.set_rgb_color([0,1,0])
z= origin.add_child("z", RMF.GEOMETRY)
sz= segment_factory.get(z)
sz.set_coordinates([[0,0], [0,0], [0,1]])
cz= color_factory.get(z)
cz.set_rgb_color([0,0,1])


# now we add another copy
# this will result in one off of the origin and rotated
remote= rh.add_child("remote", RMF.REPRESENTATION)
rbr= reference_frame_factory.get(remote)
rbr.set_translation([1,0,0])
rbr.set_rotation([.5,.5,.5,.5])
remote.add_child(x)
remote.add_child(y)
remote.add_child(z)
