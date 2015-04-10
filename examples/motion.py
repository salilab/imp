## \example motion.py
# Do a silly little simulation to show things moving around

from __future__ import print_function
import RMF
import random

number_of_particles = 10
number_of_frames = 100
minimum_radius = .5
maximum_radius = 3
box_size = 15

# find the name for a temporary file to use to for writing the hdf5 file
tfn = RMF._get_temporary_file_path("motion.rmf3")
print("File is", tfn)

# open the temporary file, clearing any existing contents
fh = RMF.create_rmf_file(tfn)

tf = RMF.TypedFactory(fh)
pf = RMF.ParticleFactory(fh)
bf = RMF.BondFactory(fh)
sf = RMF.SegmentFactory(fh)
rf = RMF.RepresentationFactory(fh)
scf = RMF.ScoreFactory(fh)

coords = []
radii = []
particles = []


def get_distance(v0, r0, v1, r1):
    return (
        ((v0[0] - v1[0]) ** 2 + (v0[1] - v1[1])
         ** 2 + (v0[1] - v1[1]) ** 2) ** .5 - r0 - r1
    )


def intersects(c, r, cs, rs):
    for oc, orad in zip(cs, rs):
        if get_distance(c, r, oc, orad) < 0:
            # print c,r, "intersects", oc, orad
            return True
    return False


def too_far(c, r, oc, orad):
    d = get_distance(c, r, oc, orad)
    if d > .5:
        return True
    return False


def random_coordinates():
    return RMF.Vector3(random.uniform(0, box_size),
                       random.uniform(0, box_size),
                       random.uniform(0, box_size))

for p in range(0, number_of_particles):
    cur = fh.get_root_node().add_child(str(p), RMF.REPRESENTATION)
    tf.get(cur).set_type_name(str(p % 3))
    pd = pf.get(cur)
    pd.set_static_radius(random.uniform(minimum_radius, maximum_radius))
    pd.set_static_mass(pd.get_radius())
    curcoord = random_coordinates()
    while intersects(curcoord, pd.get_radius(), coords, radii):
        curcoord = random_coordinates()
    radii.append(pd.get_radius())
    coords.append(curcoord)
    particles.append(cur)

bn = fh.get_root_node().add_child("bonds", RMF.ORGANIZATIONAL)

bonds = []
print("creating bonds")
for i in range(10):
    bd0 = random.choice(particles)
    bd1 = random.choice(particles)
    if bd0 == bd1:
        continue
    bp = (particles.index(bd0), particles.index(bd1))
    tries = 0
    while too_far(coords[bp[0]], radii[bp[0]],
                  coords[bp[1]], radii[bp[1]])\
        or intersects(coords[bp[0]], radii[bp[0]],
                      coords[:bp[0]] + coords[bp[0] + 1:],
                      radii[:bp[0]] + radii[bp[0] + 1:]):
        coords[bp[0]] = random_coordinates()
        tries = tries + 1
        if tries == 100:
            break
    if tries < 100:
        b = bn.add_child("bond", RMF.BOND)
        bd = bf.get(b)
        bd.set_static_bonded_0(bd0.get_id().get_index())
        bd.set_static_bonded_1(bd1.get_id().get_index())

        bonds.append(bp)
        coords[bonds[-1][0]] = coords[bonds[-1][1]]

print("created", len(bonds), "bonds")
fn = fh.get_root_node().add_child("restraints", RMF.ORGANIZATIONAL)
features = []
feature_nodes = []
print("creating features")
for i in range(4):
    f0 = random.choice(particles)
    f1 = random.choice(particles)
    if f0 == f1:
        continue
    f = fn.add_child("r", RMF.FEATURE)
    rd = rf.get(f)
    rd.set_representation([f0, f1])
    features.append((particles.index(f0), particles.index(f1)))
    feature_nodes.append(f)


def get_particle_distance(p0, p1):
    pd0 = pf.get(p0)
    pd1 = pf.get(p1)
    v0 = pd0.get_coordinates()
    v1 = pd1.get_coordinates()
    r0 = pd0.get_radius()
    r1 = pd1.get_radius()
    return (
        ((v0[0] - v1[0]) ** 2 + (v0[1] - v1[1])
         ** 2 + (v0[1] - v1[1]) ** 2) ** .5 - r0 - r1
    )

bn = fh.get_root_node().add_child("box", RMF.ORGANIZATIONAL)
# draw box
bs = box_size
for e in [(RMF.Vector3(0, 0, 0), RMF.Vector3(0, 0, bs)),
          (RMF.Vector3(0, 0, 0), RMF.Vector3(0, bs, 0)),
          (RMF.Vector3(0, 0, 0), RMF.Vector3(bs, 0, 0)),
          (RMF.Vector3(bs, bs, bs), RMF.Vector3(0, bs, bs)),
          (RMF.Vector3(bs, bs, bs), RMF.Vector3(bs, 0, bs)),
          (RMF.Vector3(bs, bs, bs), RMF.Vector3(bs, bs, 0)),
          (RMF.Vector3(0, 0, bs), RMF.Vector3(0, bs, bs)),
          (RMF.Vector3(0, 0, bs), RMF.Vector3(bs, 0, bs)),
          (RMF.Vector3(0, bs, 0), RMF.Vector3(0, bs, bs)),
          (RMF.Vector3(0, bs, 0), RMF.Vector3(bs, bs, 0)),
          (RMF.Vector3(bs, 0, 0), RMF.Vector3(bs, bs, 0)),
          (RMF.Vector3(bs, 0, 0), RMF.Vector3(bs, 0, bs))
          ]:
    en = bn.add_child("edge", RMF.GEOMETRY)
    sd = sf.get(en)
    sd.set_static_coordinates_list(e)


def write_frame(name):
    fh.add_frame(name, RMF.FRAME)
    for c, p in zip(coords, particles):
        pd = pf.get(p)
        pd.set_frame_coordinates(c)
    for f, p in zip(feature_nodes, features):
        s = get_particle_distance(particles[p[0]], particles[p[1]])
        sd = scf.get(f)
        sd.set_frame_score(s)

write_frame("initial")

for i in range(0, number_of_frames):
    for i in range(len(coords)):
        c = coords[i]
        cn = RMF.Vector3(c[0] + random.uniform(-.2, .2),
                         c[1] + random.uniform(-.2, .2),
                         c[2] + random.uniform(-.2, .2))
        if intersects(cn, radii[i], coords[:i] + coords[i + 1:], radii[:i] + radii[i + 1:]):
            continue
        bad = False
        for b in [b for b in bonds if b[0] == i or b[1] == i]:
            if b[1] == i:
                b = [b[1], b[0]]
            if too_far(cn, radii[i], coords[b[1]], radii[b[1]]):
                bad = False
                break
        coords[i] = cn
    write_frame(str(i))
