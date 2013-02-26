## \example domino/multiscale.py
## We are interested in applying domino to problems systematically in a multiscale manner. This script experiments with those approaches.

import IMP.domino
import IMP.core


# Use faster built-in 'set' type on newer Pythons; fall back to the older
# 'sets' module on older Pythons
try:
    x = set
    del x
except NameError:
    import sets
    set = sets.Set

m= IMP.Model()
m.set_log_level(IMP.base.SILENT)
ds= [IMP.core.XYZR.setup_particle(IMP.Particle(m)) for i in range(0,3)]
for i,d in enumerate(ds):
    d.set_radius(1)
    IMP.display.Colored.setup_particle(d, IMP.display.get_display_color(i))

k=1
h= IMP.core.Harmonic(0,k)
r0= IMP.core.SingletonRestraint(IMP.core.DistanceToSingletonScore(h, IMP.algebra.Vector3D(0,0,0)),
                                ds[0], "0 at origin")
r0.set_model(m)

r1= IMP.core.SingletonRestraint(IMP.core.AttributeSingletonScore(h,
                                                                 IMP.core.XYZ.get_xyz_keys()[0]),
                                ds[1], "1 on axis")
r1.set_model(m)

rs=[r0, r1]
for pr in [(0,1), (1,2), (0,2)]:
    r= IMP.core.PairRestraint(IMP.core.HarmonicSphereDistancePairScore(0,k),
                              (ds[pr[0]], ds[pr[1]]),
                              "R for "+str(pr))
    r.set_model(m)
    rs.append(r)


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
    for r in rs:
        r.set_maximum_score(.5*scale**2)
    lf= IMP.domino.ListSubsetFilterTable(pst)
    rc= IMP.domino.RestraintCache(pst)
    rc.add_restraints(rs);
    fs=[IMP.domino.RestraintScoreSubsetFilterTable(rc),
        lf]
    sampler= IMP.domino.DominoSampler(m, pst)
    sampler.set_subset_filter_tables(fs)
    sampler.set_log_level(IMP.base.SILENT)
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





for curi in range(1,len(covers)):
    scale= 4.0/2**curi
    print scale
    mapping=get_mapping(covers[curi-1], covers[curi])
    print mapping
    display_mapping(curi-1, covers[curi-1], covers[curi], mapping)
    (sampler, lf, pst)= setup(covers[curi], scale)
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
    print "for scale", scale, "got", ac
    sw= IMP.display.PymolWriter("solutions."+str(curi)+".pym")
    for i,a in enumerate(ac):
        IMP.domino.load_particle_states(subset, a, pst)
        sw.set_frame(i)
        for p in ds:
            g= IMP.core.XYZRGeometry(p)
            sw.add_geometry(g)
        for c in covers[curi]:
            g= IMP.display.PointGeometry(c)
            g.set_color(IMP.display.Color(1,1,1))
            g.set_name("grid")
            sw.add_geometry(g)
