import IMP
import IMP.atom
import IMP.container
import IMP.display
import IMP.statistics
import IMP.example
import IMP.system
import parameters

display_restraints=[]

def create_representation():
    print "creating representation"
    m= IMP.Model()
    all=IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
    all.set_name("the universe")
    def create_protein(name, ds):
        h=IMP.atom.create_protein(m, name, parameters.resolution, ds)
        leaves= IMP.atom.get_leaves(h)
        all.add_child(h)
        r=IMP.atom.create_connectivity_restraint([IMP.atom.Selection(c)\
                                                  for c in h.get_children()],
                                                 parameters.k)
        if r:
            m.add_restraint(r)
            display_restraints.append(r)
            m.set_maximum_score(r, parameters.k)
    def create_protein_from_pdbs(name, files):
        def create_from_pdb(file):
            sls=IMP.base.SetLogState(IMP.base.NONE)
            t=IMP.atom.read_pdb( IMP.get_example_path("data/"+file), m,
                                 IMP.atom.ATOMPDBSelector())
            del sls
            #IMP.atom.show_molecular_hierarchy(t)
            c=IMP.atom.Chain(IMP.atom.get_by_type(t, IMP.atom.CHAIN_TYPE)[0])
            if c.get_number_of_children()==0:
                IMP.atom.show_molecular_hierarchy(t)
            # there is no reason to use all atoms, just approximate the pdb shape instead
            s=IMP.atom.create_simplified_along_backbone(c,
                                                        parameters.resolution/2.0)
            IMP.atom.destroy(t)
            # make the simplified structure rigid
            rb=IMP.atom.create_rigid_body(s)
            rb.set_coordinates_are_optimized(True)
            return s
        if len(files) >1:
            p= IMP.Particle(m)
            h= IMP.atom.Hierarchy.setup_particle(p)
            h.set_name(name)
            for i, f in enumerate(files):
                c=create_from_pdb(f)
                h.add_child(c)
                c.set_name(name+" chain "+str(i))
            r=IMP.atom.create_connectivity_restraint([IMP.atom.Selection(c)\
                                                      for c in h.get_children()],
                                                     parameters.k)
            if r:
                m.add_restraint(r)
                display_restraints.append(r)
                m.set_maximum_score(r, parameters.k)
        else:
            h= create_from_pdb(files[0])
            h.set_name(name)
        all.add_child(h)
    create_protein("Nup85", 570)
    ct= IMP.atom.Selection(all, molecule="Nup85", terminus= IMP.atom.Selection.C)
    d= IMP.core.XYZ(ct.get_selected_particles()[0])
    d.set_coordinates(IMP.algebra.Vector3D(0,0,0))
    d.set_coordinates_are_optimized(False)
    create_protein("Nup84", 460)
    create_protein("Nup145C", 442)
    create_protein("Nup120", [0, 500, 761])
    create_protein("Nup133", [0, 450, 778, 1160])
    create_protein_from_pdbs("Seh1", ["seh1.pdb"])
    create_protein_from_pdbs("Sec13", ["sec13.pdb"])
    return (m, all)

def create_restraints(m, all):
    print "creating restraints"
    def add_connectivity_restraint(s):
        r= IMP.atom.create_connectivity_restraint(s, parameters.k)
        m.add_restraint(r)
        m.set_maximum_score(r, parameters.k)
        display_restraints.append(r)
    def add_distance_restraint(s0, s1):
        r=IMP.atom.create_distance_restraint(s0,s1, 0, parameters.k)
        m.add_restraint(r)
        m.set_maximum_score(r, parameters.k)
        display_restraints.append(r)
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
                           S(hierarchy=all, molecule="Nup120",
                             residue_indexes= [(500, 762)]))
    add_distance_restraint(S(hierarchy=all, molecule="Nup84"),
                           S(hierarchy=all, molecule="Nup133",
                             residue_indexes=[(778, 1160)]))
    add_distance_restraint(S(hierarchy=all, molecule="Nup85"),
                           S(hierarchy=all, molecule="Seh1"))
    add_distance_restraint(S(hierarchy=all, molecule="Nup145C",
                             residue_indexes=[(0,423)]),
                           S(hierarchy=all, molecule="Sec13"))
    for l in IMP.atom.get_leaves(all):
        r= IMP.example.ExampleRestraint(l, parameters.k)
        m.add_restraint(r)
        # make sure rigid bodies are as not all particles in them can be on the x,y plane
        m.set_maximum_score(.5*parameters.resolution**2*parameters.k)


def create_geometry(all):
    print "creating geometry"
    gs=[]
    for i in range(all.get_number_of_children()):
        color= IMP.display.get_display_color(i)
        n= all.get_child(i)
        name= n.get_name()
        for l in IMP.atom.get_leaves(n):
            g= IMP.core.XYZRGeometry(l)
            g.set_color(color)
            g.set_name(name)
            gs.append(g)
    # also display the restraints to see which particles they connect
    for r in display_restraints:
        try:
            g= IMP.display.create_restraint_geometry(r)
            gs.append(g)
        except:
            pass
    return gs
