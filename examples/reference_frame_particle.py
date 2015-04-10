## \example reference_frame_particle.py
# This simple example makes an RMF file with several rigid copies of the
# same thing
from __future__ import print_function
import RMF

file_name = RMF._get_temporary_file_path("reference_frame.rmfz")
print("file is", file_name)
fh = RMF.create_rmf_file(file_name)
fh.add_frame("first frame", RMF.FRAME)

rh = fh.get_root_node()

reference_frame_factory = RMF.ReferenceFrameFactory(fh)
ball_factory = RMF.BallFactory(fh)
particle_factory = RMF.ParticleFactory(fh)
color_factory = RMF.ColoredFactory(fh)
bond_factory = RMF.BondFactory(fh)

origin = rh.add_child("origin", RMF.REPRESENTATION)
pd = particle_factory.get(origin)
pd.set_radius(1)
pd.set_mass(2)
pd.set_coordinates(RMF.Vector3(0, 0, 0))

for i in range(0, 3):
    c = rh.add_child(str(i), RMF.REPRESENTATION)
    pd = particle_factory.get(c)
    pd.set_radius(1)
    pd.set_mass(2)
    coords = [0, 0, 0]
    coords[i] = 2
    pd.set_coordinates(RMF.Vector3(*coords))


frame = rh.add_child("frame", RMF.REPRESENTATION)
rbo = reference_frame_factory.get(frame)
rbo.set_translation(RMF.Vector3(5, 0, 0))
rbo.set_rotation(RMF.Vector4(1, 0, 0, 0))
pd = particle_factory.get(frame)
pd.set_radius(5)
pd.set_mass(2)
pd.set_coordinates(RMF.Vector3(0, 0, 0))

for i in range(0, 3):
    ch = frame.add_child("site", RMF.GEOMETRY)
    coords = [0, 0, 0]
    coords[i] = 5
    v = RMF.Vector3(*coords)
    ball_factory.get(ch).set_coordinates(v)
    ball_factory.get(ch).set_radius(1)
    bch = frame.add_child("bond", RMF.BOND)
    bond_factory.get(bch).set_bonded_0(ch.get_id().get_index())
    bond_factory.get(bch).set_bonded_1(frame.get_id().get_index())
