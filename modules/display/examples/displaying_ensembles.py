## \example display/displaying_ensembles.py
## The script shows a couple experiments with trying to visualize an ensembe of structures. The ensemble is fairly tight on the assembly scale, but there is significant variation between the location and orientation of the individual proteins (which were modeled as rigid bodies). To save space, the models have had their sidechain atoms removed.

import IMP.display
import IMP.atom

Segment= IMP.algebra.Segment3D
Cylinder= IMP.algebra.Cylinder3D

# turn off internal checks to speed things up
IMP.base.set_check_level(IMP.base.USAGE)

def read(m, beyond_file):
    print "reading"
    hs=[]
    for i in range(0,beyond_file):
        # create a simplified version for each chain to speed up computations
        name= IMP.display.get_example_path("ensemble/aligned-"+str(i)+".pdb")
        h= IMP.atom.read_pdb(name, m, IMP.atom.CAlphaPDBSelector())
        hr= IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        hs.append(hr)
        for c in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
            simp= IMP.atom.create_simplified_along_backbone(IMP.atom.Chain(c), 4)
            hr.add_child(simp)
        IMP.atom.destroy(h)
        if i==0:
            base=IMP.atom.get_leaves(hr)
        print " ",i
    return hs

def add_markers(h, c, w):
    """Add markers to a the passed conformation. The marker locations are chosen
    pretty thoughtlessly and don't really illustrate the technique well."""
    def add_marker(s, name):
        g= IMP.core.XYZRGeometry(s.get_selected_particles()[0])
        g.set_name(name)
        g.set_color(c)
        w.add_geometry(g)
    s= IMP.atom.Selection(h, chain='B', residue_index=317)
    add_marker(s, "m0")
    s= IMP.atom.Selection(h, chain='G', residue_index=212)
    add_marker(s, "m1")
    s= IMP.atom.Selection(h, chain='I', residue_index=237)
    add_marker(s, "m2")
    s= IMP.atom.Selection(h, chain='F', residue_index=101)
    add_marker(s, "m3")

def get_nice_name(h):
    nm= h.get_name()
    return nm[nm.find('-')+1:nm.rfind('.')]

def add_axis(h, c, w, chain_colors):
    """Add a coordinate axis to show the relative orientation of the protein"""
    for hc in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
        rb= IMP.core.RigidMember(hc).get_rigid_body()
        g= IMP.display.ReferenceFrameGeometry(rb.get_reference_frame())
        g.set_name(get_nice_name(h)+"_orient")
        if c:
            g.set_color(c)
        else:
            g.set_color(chain_colors[IMP.atom.Chain(hc).get_id()])
        w.add_geometry(g)

def add_skeleton(h, c, r, w, chain_colors):
    """Show the connectivity skeleton of the conformation to give an idea of
    how things are layed out"""
    for hc0 in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
        for hc1 in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
            if hc1 <= hc0:
                continue
            d= ps.evaluate((hc0, hc1), None)
            if d < 1:
                d0= IMP.core.XYZ(hc0)
                d1= IMP.core.XYZ(hc1)
                mp= .5*(d0.get_coordinates()+d1.get_coordinates())
                g= IMP.display.CylinderGeometry(Cylinder(Segment(d0.get_coordinates(), mp), r))
                if c:
                    g.set_color(c)
                else:
                    g.set_color(chain_colors[IMP.atom.Chain(d0).get_id()])
                g.set_name(get_nice_name(h)+"_skel")
                w.add_geometry(g)
                g= IMP.display.CylinderGeometry(Cylinder(Segment(d1.get_coordinates(), mp), r))
                if c:
                    g.set_color(c)
                else:
                    g.set_color(chain_colors[IMP.atom.Chain(d1).get_id()])
                g.set_name(get_nice_name(h)+"_skel")
                w.add_geometry(g)

IMP.base.set_log_level(IMP.base.TERSE)
m= IMP.Model()

# change to 46 to display all of them
hs= read(m, 3)

# used to test of two molecules are touching one another
ps= IMP.core.KClosePairsPairScore(IMP.core.SphereDistancePairScore(IMP.core.HarmonicUpperBound(10,1)),
                                  IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
ps.set_log_level(IMP.base.SILENT)


print "creating rigid bodies"
base_chains={}
for hc in IMP.atom.get_by_type(hs[0], IMP.atom.CHAIN_TYPE):
    c= IMP.atom.Chain(hc)
    base_chains[c.get_id()]=c

for i, h in enumerate(hs):
    for hc in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
        c=IMP.atom.Chain(hc)
        if h==hs[0]:
            crb=IMP.atom.create_rigid_body(hc)
        else:
            # make sure the rigid bodies have equivalent defining reference frames
            # if we just used IMP.atom.create_rigid_body, globular proteins are likely
            # to have different axis computed when starting in different orientations
            crb=IMP.atom.create_compatible_rigid_body(hc, base_chains[c.get_id()])
    print " ",i

chains= IMP.atom.get_by_type(hs[0], IMP.atom.CHAIN_TYPE)
chains.sort(lambda x,y: cmp(IMP.core.XYZ(x).get_x()+IMP.core.XYZ(x).get_y(),
                IMP.core.XYZ(y).get_x()+ IMP.core.XYZ(y).get_y()))
chain_colors={}
for i,c in enumerate(chains):
    id= IMP.atom.Chain(c).get_id()
    #f= i/float(len(chains))
    color= IMP.display.get_display_color(i)
    #IMP.display.get_jet_color(f)
    chain_colors[id]=color

w= IMP.display.PymolWriter("markers.pym")
add_markers(hs[0], IMP.display.Color(1,1,1), w)
hso= hs[1:]


# sort them spatially so the colors are nicely arranged and allow one to visually connect
# the position of one end with that of the other
hso.sort(lambda h0, h1:  cmp(IMP.core.XYZ(IMP.atom.Selection(h0, chain='I',
                                                             residue_index=237).get_selected_particles()[0]).get_z(),
                             IMP.core.XYZ(IMP.atom.Selection(h1, chain='I',
                                                             residue_index=237).get_selected_particles()[0]).get_z()))
print "adding markers",
for i,h in enumerate(hso):
    c= IMP.display.get_interpolated_rgb(IMP.display.Color(1,0,0), IMP.display.Color(0,0,1), i/50.)
    add_markers(h, c, w)
    print " ",i
w= IMP.display.PymolWriter("axis.pym")
print "adding axis",
add_axis(hs[0], IMP.display.Color(1,1,1), w, chain_colors)
for i,h in enumerate(hs[1:]):
    add_axis(h, None, w, chain_colors)
    print i,

w= IMP.display.PymolWriter("skeletons.pym")
add_skeleton(hs[0], IMP.display.Color(1,1,1), 5, w, chain_colors)
print "adding skeleton",
for i,h in enumerate(hs[1:]):
    add_skeleton(h, None, 1, w, chain_colors)
    print " ",i
