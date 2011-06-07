import IMP.domino
import IMP.core

m= IMP.Model()
m.set_log_level(IMP.SILENT)
ds= [IMP.core.XYZR.setup_particle(IMP.Particle(m)) for i in range(0,3)]
for d in ds:
    d.set_radius(1)

k=1
h= IMP.core.Harmonic(0,k)
r0= IMP.core.SingletonRestraint(IMP.core.DistanceToSingletonScore(h, IMP.algebra.Vector3D(0,0,0)),
                                ds[0], "0 at origin")
m.add_restraint(r0)

r1= IMP.core.SingletonRestraint(IMP.core.AttributeSingletonScore(h,
                                                                 IMP.core.XYZ.get_xyz_keys()[0]),
                                ds[1], "1 on axis")
m.add_restraint(r1)

for pr in [(0,1), (1,2), (0,2)]:
    r= IMP.core.PairRestraint(IMP.core.HarmonicSphereDistancePairScore(0,k),
                              (ds[pr[0]], ds[pr[1]]),
                              "R for "+str(pr))
    m.add_restraint(r)


bb= IMP.algebra.BoundingBox2D(IMP.algebra.Vector2D(0,0),
                              IMP.algebra.Vector2D(4,4))

covers=[]
for i in range(0,6):
    cur=IMP.algebra.get_grid_interior_cover_by_spacing(bb, 4.0/2**i)
    print cur
    covers.append([IMP.algebra.Vector3D(x[0], x[1], 0) for x in cur])


def setup(cover, scale):
    pst= IMP.domino.ParticleStatesTable()
    st= IMP.domino.XYZStates(cover)
    for p in ds:
        pst.set_particle_states(p, st)
    for r in m.get_restraints():
        r.set_maximum_score(.5*scale**2)
    lf= IMP.domino.ListSubsetFilterTable(pst)
    fs=[IMP.domino.RestraintScoreSubsetFilterTable(m, pst),
        lf]
    sampler= IMP.domino.DominoSampler(m, pst)
    sampler.set_subset_filter_tables(fs)
    sampler.set_log_level(IMP.SILENT)
    return (sampler, lf, pst)

(sampler,lf, pst)= setup(covers[0], 4.0)

subset=IMP.domino.Subset(ds)
ac= sampler.get_sample_assignments(subset)

print ac

def get_mapping(cover0, cover1):
    nn= IMP.algebra.NearestNeighbor3D(cover0)
    ret=[[] for c in cover0]
    for i,p in enumerate(cover1):
        nns=nn.get_nearest_neighbor(p)
        ret[nns].append(i)
    return ret

mw= IMP.display.PymolWriter("mapping.pym")
def display_mapping(index, cover0, cover1, mapping):
    mw.set_frame(index)
    for i,c in enumerate(mapping):
        for p in c:
            g= IMP.display.PointGeometry(cover1[p])
            g.set_color(IMP.display.get_display_color(i))
            g.set_name("fine")
            mw.add_geometry(g)
    for i,c in enumerate(cover0):
        g= IMP.display.PointGeometry(c)
        g.set_color(IMP.display.get_display_color(i))
        g.set_name("coarse")
        mw.add_geometry(g)





for i in range(1,len(covers)):
    scale= 4.0/2**i
    print scale
    mapping=get_mapping(covers[i-1], covers[i])
    print mapping
    display_mapping(i-1, covers[i-1], covers[i], mapping)
    (sampler, lf, pst)= setup(covers[i], scale)
    lac= ac
    cac=[]
    for a in lac:
        for i,p in enumerate(subset):
            s= a[i]
            allowed= mapping[s]
            lf.set_allowed_states(p, allowed)
        ccac=sampler.get_sample_assignments(subset)
        print a, ccac
        cac= cac+ccac
    ac= list(set(cac))
print ac
sw= IMP.display.PymolWriter("solutions.pym")
for i,a in enumerate(ac):
    IMP.domino.load_particle_states(subset, a, pst)
    sw.set_frame(i)
    for p in ds:
        g= IMP.display.XYZRGeometry(p)
        sw.add_geometry(g)
