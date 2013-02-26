## \example kernel/dependency_graph.py
## When trying to understand what is going on in \imp, it can often be useful to view the dependency graph, that is, the graph showing how various entities relate to one another. In it, an arrow leads from an IMP::Container or IMP::Particle to an IMP::Restraint if the IMP::Restraint reads from that container or particle. Similarly, an arrow leads from an IMP::Container or IMP::Particle to an IMP::ScoreState if the score state reads from it, and an arrow leads from an IMP::ScoreState to an IMP::Container or IMP::Particle if the score state updates the particle.
##
## The resulting pruned depenency graph is:
## \dotgraph{ \dot
## digraph pruned_dependencies {
## 0[label="Nup85-12"];
## 1[label="Fragment [213-218)"];
## 2[label="Nup145C-4"];
## 3[label="Atom k distance restraint 2"];
## 4[label="Nup1202-8"];
## 5[label="Model 0"];
## 6[label="Fragment [239-244)"];
## 7[label="Nup1332-10"];
## 8[label="Nup1201-12"];
## 9[label="Connectivity particles"];
## 10[label="Nup84-14"];
## 11[label="Atom k distance restraint 3"];
## 12[label="MovedSingletonContainer2"];
## 13[label="Nup1333-16"];
## 14[label="Atom k distance restraint 5"];
## 15[label="Atom k distance restraint 4"];
## 16[label="normalize rigid bodies"];
## 17[label="Atom k distance restraint 0"];
## 18[label="ListSingletonContainer2"];
## 19[label="Chain A rigid body"];
## 20[label="ConnectivityRestraint 0"];
## 21[label="MovedSingletonContainer1"];
## 22[label="ListSingletonContainer1"];
## 23[label="HarmonicUpperBoundSphereDistancePairScore2 and ConnectingPairContainer"];
## 24[label="HarmonicUpperBoundSphereDistancePairScore1 and ConnectingPairContainer"];
## 25[label="Sec13"];
## 26[label="Hierarchy EV"];
## 27[label="Chain A rigid body rigid body positions"];
## 28[label="rigid bodies list"];
## 29[label="Atom k distance restraint 1"];
## 30[label="HarmonicUpperBoundSphereDistancePairScore0 and ConnectingPairContainer"];
## 31[label="ConnectingPairContainer"];
## 32[label="ConnectingPairContainer"];
## 33[label="Chain A rigid body"];
## 34[label="ListSingletonContainer0"];
## 35[label="ConnectivityRestraint 1"];
## 36[label="MovedSingletonContainer0"];
## 37[label="Chain A rigid body rigid body positions"];
## 38[label="ConnectingPairContainer"];
## 39[label="Seh1"];
## 40[label="Hierarchy EV particles"];
## 16->19 ;
## 16->33 ;
## 27->6 ;
## 27->39 ;
## 37->25 ;
## 37->1 ;
## 4->3 ;
## 2->3 ;
## 11->5 ;
## 17->5 ;
## 23->5 ;
## 14->5 ;
## 35->5 ;
## 15->5 ;
## 30->5 ;
## 24->5 ;
## 29->5 ;
## 26->5 ;
## 3->5 ;
## 20->5 ;
## 13->11 ;
## 10->11 ;
## 2->12 ;
## 18->12 ;
## 25->14 ;
## 2->14 ;
## 1->14 ;
## 33->14 ;
## 0->15 ;
## 6->15 ;
## 19->15 ;
## 39->15 ;
## 28->16 ;
## 8->17 ;
## 4->17 ;
## 13->20 ;
## 7->20 ;
## 22->21 ;
## 10->21 ;
## 2->23 ;
## 38->23 ;
## 31->24 ;
## 10->24 ;
## 0->26 ;
## 13->26 ;
## 8->26 ;
## 7->26 ;
## 4->26 ;
## 2->26 ;
## 6->26 ;
## 19->26 ;
## 10->26 ;
## 40->26 ;
## 1->26 ;
## 33->26 ;
## 19->27 ;
## 0->29 ;
## 2->29 ;
## 0->30 ;
## 32->30 ;
## 21->31 ;
## 22->31 ;
## 10->31 ;
## 0->32 ;
## 36->32 ;
## 34->32 ;
## 25->35 ;
## 2->35 ;
## 10->35 ;
## 1->35 ;
## 33->35 ;
## 0->36 ;
## 34->36 ;
## 33->37 ;
## 12->38 ;
## 2->38 ;
## 18->38 ;
## }
## \enddot
## }

import IMP
import IMP.atom
import IMP.container

def create_representation():
    m= IMP.Model()
    all=IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
    all.set_name("the universe")
    def create_protein(name, ds):
        h=IMP.atom.create_protein(m, name, 10, ds)
        leaves= IMP.atom.get_leaves(h)
        all.add_child(h)
        r=IMP.atom.create_connectivity_restraint([IMP.atom.Selection(c)\
                                                  for c in h.get_children()],
                                                 1)
        if r:
            m.add_restraint(r)
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
                                                        10.0/2.0)
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
                                                     1)
            if r:
                m.add_restraint(r)
        else:
            h= create_from_pdb(files[0])
            h.set_name(name)
        all.add_child(h)
    create_protein("Nup85", 570)
    create_protein("Nup84", 460)
    create_protein("Nup145C", 442)
    create_protein("Nup120", [0, 500, 761])
    create_protein("Nup133", [0, 450, 778, 1160])
    create_protein_from_pdbs("Seh1", ["seh1.pdb"])
    create_protein_from_pdbs("Sec13", ["sec13.pdb"])
    return (m, all)

def create_restraints(m, all):
    def add_connectivity_restraint(s):
        r= IMP.atom.create_connectivity_restraint(s, 1)
        m.add_restraint(r)
    def add_distance_restraint(s0, s1):
        r=IMP.atom.create_distance_restraint(s0,s1, 0, 1)
        m.add_restraint(r)
    evr=IMP.atom.create_excluded_volume_restraint([all])
    m.add_restraint(evr)
    S= IMP.atom.Selection
    s0=S(hierarchy=all, molecule="Nup145C", residue_indexes=[(0,423)])
    s1=S(hierarchy=all, molecule="Nup84")
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

# now do the actual work
(m,all)= create_representation()
create_restraints(m, all)

# we can get the full dependency graph for the whole model with all the restraints
# but it is pretty complex
dg= IMP.get_dependency_graph(m)
IMP.show_graphviz(dg);

# better thing to do is to get the "pruned" graph
pdg= IMP.get_pruned_dependency_graph(m)

IMP.show_graphviz(pdg)
