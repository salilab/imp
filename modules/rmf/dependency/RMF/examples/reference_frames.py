## \example reference_frames.py
# Show how to properly extract coordinates in the presence of rigid bodies.
from __future__ import print_function
import RMF

# print out transformed coordinates


def print_coordinates(reference_frame, coords):
    print(reference_frame.get_global_coordinates(coords))

# walk through the hierarchy visiting each node and extracting coordinates


def visit(node, reference_frame, reference_frame_factory, particle_factory,
          segment_factory, ball_factory):
    if reference_frame_factory.get_is(node):
        reference_frame = RMF.CoordinateTransformer(
            reference_frame, reference_frame_factory.get(node))
        print("reference frame is now", reference_frame)
    elif segment_factory.get_is(node):
        segment = segment_factory.get(node)
        print("segment", node.get_name())
        # silliness
        coords = segment.get_coordinates()
        for i in range(0, len(coords[0])):
            print_coordinates(reference_frame, [coords[0][i],
                                                coords[1][i],
                                                coords[2][i]])
    elif particle_factory.get_is(node):
        particle = particle_factory.get(node)
        print("particle", node.get_name(), print_coordinates(reference_frame, particle.get_coordinates()))
    elif ball_factory.get_is(node):
        particle = ball_factory.get(node)
        print("ball", node.get_name(), print_coordinates(reference_frame, particle.get_coordinates()))
    for c in node.get_children():
        visit(c, reference_frame, reference_frame_factory, particle_factory,
              segment_factory, ball_factory)

fh = RMF.open_rmf_file_read_only(RMF.get_example_path("reference_frames.rmf3"))


visit(fh.get_root_node(), RMF.CoordinateTransformer(),
      RMF.ReferenceFrameFactory(fh),
      RMF.ParticleFactory(fh),
      RMF.SegmentFactory(fh),
      RMF.BallFactory(fh))
