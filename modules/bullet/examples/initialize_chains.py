import IMP
import IMP.core
import IMP.container
import IMP.bullet
import IMP.display

radius=10
stiffness=10
pn=10
pm=10
pbb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                               IMP.algebra.Vector3D(pn*30, pn*30, pm*5*radius))

def add_geometry(w, chains):
    for (i,c) in enumerate(chains):
        color= IMP.display.get_display_color(i)
        cn= "chain "+str(i)
        for p in c:
            sg= IMP.display.XYZRGeometry(IMP.core.XYZR(p))
            sg.set_name(cn)
            sg.set_color(color)
            w.add_geometry(sg)
    bbg= IMP.display.BoundingBoxGeometry(pbb)
    bbg.set_name("bounding box")
    w.add_geometry(bbg)

def create():
    m= IMP.Model()
    m.set_log_level(IMP.SILENT)
    filters=[]
    chains=[]
    all=[]
    link= IMP.core.HarmonicDistancePairScore(2*radius,stiffness)
    #rso= IMP.RestraintSet("non-ev")
    #m.add_restraint(rso)
    for i in range(0,pn):
        for j in range(0,pn):
            chains.append([])
            p= IMP.Particle(m)
            chains[-1].append(p)
            d= IMP.core.XYZR.setup_particle(p)
            d.set_radius(radius)
            d.set_coordinates(IMP.algebra.Vector3D(30*i, 30*j, 0))
            d.set_coordinates_are_optimized(False)
            for k in range(1, pm):
                p= IMP.Particle(m)
                chains[-1].append(p)
                d= IMP.core.XYZR.setup_particle(p)
                d.set_radius(radius)
                d.set_coordinates(IMP.algebra.get_random_vector_in(pbb))
                d.set_coordinates_are_optimized(True)
            cpc= IMP.container.ConsecutivePairContainer(chains[-1])
            filters.append(IMP.container.InContainerPairFilter(cpc))
            pr= IMP.container.PairsRestraint(link, cpc)
            m.add_restraint(pr)
            pr.set_name("link chain "+str(i)+" "+str(j))
            all=all+chains[-1]
    alll= IMP.container.ListSingletonContainer(all)
    cpc= IMP.container.ClosePairContainer(alll, 0, 1)
    cpc.add_pair_filters(filters)
    m.add_restraint(IMP.container.PairsRestraint(IMP.core.SoftSpherePairScore(stiffness), cpc, "excluded volume"))
    ass= IMP.core.AttributeSingletonScore(IMP.core.HarmonicLowerBound(0,10*stiffness),
                                          IMP.FloatKey("z"))
    sr=IMP.container.SingletonsRestraint(ass, alll)
    m.add_restraint(sr)
    sr.set_name("floor")
    return (m, all, chains)


def initialize_model(m, all, chains):
    opt= IMP.bullet.ResolveCollisionsOptimizer(m)
    opt.set_log_level(IMP.VERBOSE)
    w=IMP.display.PymolWriter()
    w.set_log_level(IMP.SILENT)
    log= IMP.display.WriteOptimizerState(w, "frame.%1%.pym")
    add_geometry(log, chains)
    opt.add_optimizer_state(log)
    opt.optimize(10)


def display(chains, name):
    w= IMP.display.PymolWriter(name)
    add_geometry(w, chains)


# to view the animation of the optimization do something like
# pymol ` ls frame.*.pym| sort -t\. -n -k 2`
(m, all, chains)= create()
display(chains, "initial.pym")
initialize_model(m, all, chains)
display(chains, "final.pym")
