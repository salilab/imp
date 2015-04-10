## \example alternatives.py
# The RMF format supports storing alternative representations of the structure
# through the RMF::decorator::Alternatives decorator and associated types.
# Multiple versions of the hierarchy are stored, all representing the same
# biological entity. Different representations can be used for different
# purposes based on resolution or other criteria.

from __future__ import print_function
import RMF

tfn = RMF._get_temporary_file_path("alternatives.rmf3")
print("File is", tfn)

f = RMF.create_rmf_file(tfn)
f.add_frame("frame", RMF.FRAME)

# make a trivial set of multiple resolution represntations
rh = f.get_root_node()

pf = RMF.ParticleFactory(f)
gpf = RMF.GaussianParticleFactory(f)
rff = RMF.ReferenceFrameFactory(f)


def frange(x, y, jump):
    while x < y:
        yield x
        x += jump


def create_hierarchy(radius):
    root = f.add_node(str(radius), RMF.REPRESENTATION)
    for i in frange(0., 8, radius):
        for j in frange(0., 8, radius):
            for k in frange(0., 8, radius):
                center = RMF.Vector3(
                    i + radius / 2,
                    j + radius / 2,
                    k + radius / 2)
                n = root.add_child(
                    str(i) + "-" + str(j) + "-" + str(k),
                    RMF.REPRESENTATION)
                d = pf.get(n)
                d.set_static_radius(radius)
                d.set_frame_coordinates(center)
                d.set_static_mass(radius ** 3)
    return root


def create_gmm(radius=1.):
    root = f.add_node("gmm" + str(radius), RMF.REPRESENTATION)
    for i in frange(0., 8, radius):
        for j in frange(0., 8, radius):
            center = RMF.Vector3(
                i + radius / 2,
                j + radius / 2,
                4)

            n = root.add_child(
                str(i) + "-" + str(j),
                RMF.REPRESENTATION)
            d = gpf.get(n)
            d.set_variances(RMF.Vector3(radius / 2, radius / 2, 4))
            d.set_static_mass(radius ** 3)
            rf = rff.get(n)
            rf.set_rotation(RMF.Vector4(1, 0, 0, 0))
            rf.set_translation(center)
    return root

n = create_hierarchy(4)
rh.add_child(n)

af = RMF.AlternativesFactory(f)
da = af.get(n)
da.add_alternative(create_hierarchy(1), RMF.PARTICLE)
da.add_alternative(create_hierarchy(2), RMF.PARTICLE)
da.add_alternative(create_hierarchy(4), RMF.PARTICLE)
da.add_alternative(create_gmm(), RMF.GAUSSIAN_PARTICLE)

RMF.show_hierarchy(n)

print("particles", RMF.get_resolutions(n, RMF.PARTICLE))
print("gaussian particles", RMF.get_resolutions(n, RMF.GAUSSIAN_PARTICLE))
