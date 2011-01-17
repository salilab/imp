import IMP.display
import IMP.atom

Segment= IMP.algebra.Segment3D
Cylinder= IMP.algebra.Cylinder3D

IMP.set_log_level(IMP.TERSE)
m= IMP.Model()
hs=[]
# used to test of two molecules are touching one another
ps= IMP.core.KClosePairsPairScore(IMP.core.SphereDistancePairScore(IMP.core.HarmonicUpperBound(10,1)),
                                  IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))


chain_colors={}
#51
print "reading"
for i in range(0,46):
    name= IMP.display.get_module_example_path("ensemble/aligned-"+str(i)+".pdb")
    h= IMP.atom.read_pdb(name, m, IMP.atom.CAlphaPDBSelector())
    if i==0:
        base=IMP.atom.Selection(h, atom_type=IMP.atom.AT_CA).get_selected_particles()
        print "  base", len(base)
    hs.append(h)

def add_markers(h, c, w):
    """Add markers to a the passed conformation. The marker locations are chosen
    pretty thoughtlessly and don't really illustrate the technique well."""
    def add_marker(s, name):
        g= IMP.display.XYZRGeometry(s.get_selected_particles()[0])
        g.set_name(name)
        g.set_color(c)
        w.add_geometry(g)
    s= IMP.atom.Selection(h, chain='B', residue_index=317, atom_type=IMP.atom.AT_CA)
    add_marker(s, "m0")
    s= IMP.atom.Selection(h, chain='G', residue_index=212, atom_type=IMP.atom.AT_CA)
    add_marker(s, "m1")
    s= IMP.atom.Selection(h, chain='I', residue_index=237, atom_type=IMP.atom.AT_CA)
    add_marker(s, "m2")
    s= IMP.atom.Selection(h, chain='F', residue_index=101, atom_type=IMP.atom.AT_CA)
    add_marker(s, "m3")

def get_nice_name(h):
    nm= h.get_name()
    return nm[nm.find('-')+1:nm.rfind('.')]

def add_axis(h, c, w):
    for hc in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
        rb= IMP.core.RigidMember(hc).get_rigid_body()
        g= IMP.display.ReferenceFrameGeometry(rb.get_reference_frame())
        g.set_name(get_nice_name(h)+"_orient")
        if c:
            g.set_color(c)
        else:
            g.set_color(chain_colors[IMP.atom.Chain(hc).get_id()])
        w.add_geometry(g)

def add_skeleton(h, c, r, w):
    for hc0 in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
        for hc1 in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
            if hc1 < hc0:
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


def chaincmp(p0, p1):
    return  cmp(IMP.core.XYZ(p0).get_x()+IMP.core.XYZ(p0).get_y(),
                IMP.core.XYZ(p1).get_x()+ IMP.core.XYZ(p1).get_y())

def mycmp(h0, h1):
    p0=IMP.atom.Selection(h0, chain='I', residue_index=237, atom_type=IMP.atom.AT_CA).get_selected_particles()[0]
    p1=IMP.atom.Selection(h1, chain='I', residue_index=237, atom_type=IMP.atom.AT_CA).get_selected_particles()[0]
    return  cmp(IMP.core.XYZ(p0).get_z(), IMP.core.XYZ(p1).get_z())


print "creating rigid bodies"
base_chains={}
for hc in IMP.atom.get_by_type(hs[0], IMP.atom.CHAIN_TYPE):
    c= IMP.atom.Chain(hc)
    base_chains[c.get_id()]=c

for i, h in enumerate(hs):
    print " ",i
    for hc in IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE):
        c=IMP.atom.Chain(hc)
        if h==hs[0]:
            crb=IMP.atom.create_rigid_body(hc)
        else:
            # make sure the rigid bodies have equivalent defining reference frames
            crb=IMP.atom.create_compatible_rigid_body(hc, base_chains[c.get_id()])
            tr= IMP.core.get_transformation_aligning_first_to_second(IMP.atom.get_leaves(base_chains[c.get_id()]),
                                                                     IMP.atom.get_leaves(hc))
            brb= IMP.core.RigidMember(base_chains[c.get_id()]).get_rigid_body()


chains= IMP.atom.get_by_type(hs[0], IMP.atom.CHAIN_TYPE)
chains.sort(chaincmp)
for i,c in enumerate(chains):
    id= IMP.atom.Chain(c).get_id()
    #f= i/float(len(chains))
    color= IMP.display.get_display_color(i)
    #IMP.display.get_jet_color(f)
    chain_colors[id]=color

w= IMP.display.PymolWriter("markers.pym")
add_markers(hs[0], IMP.display.Color(1,1,1), w)
hso= hs[1:]
hso.sort(mycmp)
print "adding markers"
for i,h in enumerate(hso):
    print " ",i
    c= IMP.display.get_interpolated_rgb(IMP.display.Color(1,0,0), IMP.display.Color(0,0,1), i/50.)
    add_markers(h, c, w)
w= IMP.display.PymolWriter("axis.pym")
print "adding axis"
add_axis(hs[0], IMP.display.Color(1,1,1), w)
for i,h in enumerate(hs[1:]):
    print " ",i
    add_axis(h, None, w)

w= IMP.display.PymolWriter("skeletons.pym")
add_skeleton(hs[0], IMP.display.Color(1,1,1), 5, w)
print "adding skeleton"
for i,h in enumerate(hs[1:]):
    print " ",i
    add_skeleton(h, None, 1, w)
