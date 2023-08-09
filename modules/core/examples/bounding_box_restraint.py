import IMP, IMP.core, IMP.algebra, IMP.container

K_BB = 10.0 # bounding box force coefficient in kcal/mol/A^2
RADIUS = 1.0 # particle radius in A

def create_particle(m, bb, radius = RADIUS):
    p = IMP.Particle(m) 
    s = IMP.algebra.Sphere3D(
            IMP.algebra.get_random_vector_in(bb), radius)
    d = IMP.core.XYZR.setup_particle(p, s)
    d.set_coordinates_are_optimized(True)
    return p

bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                            IMP.algebra.Vector3D(10, 10, 10))
m = IMP.Model()
particles = []
for i in range(0,10):
    particles.append(create_particle(m, bb))
hpb = IMP.core.HarmonicUpperBound(0, K_BB)
bbss = IMP.core.BoundingBox3DSingletonScore(hpb, bb)
bbr = IMP.container.SingletonsRestraint(bbss, particles)
