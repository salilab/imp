import IMP
import IMP.atom
import IMP.container
import IMP.display
import IMP.statistics

k=100

def create_representation():
    m= IMP.Model()
    resolution=1000
    all=IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
    all.set_name("the universe")
    ps=IMP.core.HarmonicSphereDistancePairScore(0,k)
    def create_protein(name, ds):
        h=IMP.atom.create_protein(m, name, resolution, ds)
        leaves= IMP.atom.get_leaves(h)
        all.add_child(h)
        if len(leaves)==2:
            m.add_restraint(IMP.core.PairRestraint(ps, leaves))
        elif len(leaves)>2:
            cr=IMP.core.ConnectivityRestraint(ps, IMP.container.ListSingletonContainer(leaves))
            m.add_restraint(cr)
    create_protein("Nup85", 570)
    ct= IMP.atom.Selection(all, molecule="Nup85", terminus= IMP.atom.Selection.C)
    d= IMP.core.XYZ(ct.get_particles()[0])
    d.set_coordinates(IMP.algebra.Vector3D(0,0,0))
    d.set_coordinates_are_optimized(False)
    create_protein("Nup84", 460)
    create_protein("Nup145C", 442)
    create_protein("Nup120", [0, 500, 761])
    create_protein("Nup133", [0, 450, 288+450+1])
    create_protein("Seh1", 351)
    create_protein("Sec13", 379)
    return (m, all)

def create_restraints(m, all):
    # actually, just call IMP.atom.create_connectivity_restraint, but...
    def add_connectivity_restraint(s):
        tr= IMP.core.TableRefiner()
        rps=[]
        for sc in s:
            ps= sc.get_particles()
            rps.append(ps[0])
            tr.add_particle(ps[0], ps)
        score= IMP.core.KClosePairsPairScore(IMP.core.HarmonicSphereDistancePairScore(0,1),
                                             tr)
        r= IMP.core.ConnectivityRestraint(score, IMP.container.ListSingletonContainer(rps))
        m.add_restraint(r)
    def add_distance_restraint(s0, s1):
        m.add_restraint(IMP.atom.create_distance_restraint(s0,s1, 0, k))
    evr=IMP.atom.create_excluded_volume_restraint([all])
    m.add_restraint(evr)
    S= IMP.atom.Selection
    s0=S(hierarchy=all, molecule="Nup145C", residue_indexes=[(0,423)])
    s1=S(hierarchy=all, molecule="Nup84", molecules=[])
    s2=S(hierarchy=all, molecule="Sec13")
    add_connectivity_restraint([s0,s1,s2])
    add_distance_restraint(S(hierarchy=all, molecule="Nup145C", residue_indexes=[(0,423)]),
                           S(hierarchy=all, molecule="Nup85"))
    add_distance_restraint(S(hierarchy=all, molecule="Nup145C", residue_indexes=[(0,423)]),
                           S(hierarchy=all, molecule="Nup120", residue_indexes= [(500, 762)]))
    add_distance_restraint(S(hierarchy=all, molecule="Nup84"),
                           S(hierarchy=all, molecule="Nup133", residue_indexes=[(450, 288+450+1)]))
    add_distance_restraint(S(hierarchy=all, molecule="Nup85"),
                           S(hierarchy=all, molecule="Seh1"))
    add_distance_restraint(S(hierarchy=all, molecule="Nup145C", residue_indexes=[(0,423)]),
                           S(hierarchy=all, molecule="Sec13"))


(m,all)= create_representation()
IMP.atom.show_molecular_hierarchy(all)
create_restraints(m, all)


gs=[]
for i, l in enumerate(IMP.atom.get_leaves(all)):
    g= IMP.display.XYZRGeometry(l)
    g.set_color(IMP.display.get_display_color(i))
    g.set_name(IMP.atom.get_molecule_name(l))
    gs.append(g)
sampler= IMP.core.MCCGSampler(m)
#try:
#    import IMP.bullet
#    lopt= IMP.bullet.ResolveCollisionsOptimizer(m)
#sampler.set_local_optimizer(lopt)
#except:
#lopt= IMP.core.ConjugateGradients(m)

bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                             IMP.algebra.Vector3D(1000, 1000, 1000))
#for l in IMP.atom.get_leaves(all):
#    IMP.core.XYZ(l).set_coordinates(IMP.algebra.get_random_vector_in(bb))
#lopts= IMP.display.WriteOptimizerState(IMP.display.PymolWriter(), "step.%1%.pym")
#lopt.add_optimizer_state(lopts)
#for g in gs:
#    lopts.add_geometry(g)
#lopt.optimize(100)

m.set_maximum_score(5)
sampler.set_bounding_box(bb)
sampler.set_number_of_conjugate_gradient_steps(100)
sampler.set_number_of_monte_carlo_steps(10)
sampler.set_number_of_attempts(100)
sampler.set_log_level(IMP.SILENT)
cs= sampler.get_sample()
print "found", cs.get_number_of_configurations(), "solutions"
for i in range(0, cs.get_number_of_configurations()):
    cs.load_configuration(i)
    w= IMP.display.PymolWriter("config.%d.pym"%i)
    for g in gs:
        w.add_geometry(g)


embed= IMP.statistics.ConfigurationSetXYZEmbedding(cs, IMP.container.ListSingletonContainer(IMP.atom.get_leaves(all)), True)
cluster= IMP.statistics.get_lloyds_kmeans(embed, 10, 10000)
for i in range(cluster.get_number_of_clusters()):
    center= cluster.get_cluster_center(i)
    cs.load_configuration(i)
    w= IMP.display.PymolWriter("cluster.%d.pym"%i)
    for g in gs:
        w.add_geometry(g)
