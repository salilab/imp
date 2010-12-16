import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import IMP.membrane
import math

def create_representation():
    m=IMP.Model()
#   only CA
    mp0= IMP.atom.read_pdb('2K9P_OMP.pdb', m, IMP.atom.CAlphaPDBSelector())
#   all-atom
#    mp0= IMP.atom.read_pdb('2K9P_OMP.pdb', m, IMP.atom.NonWaterNonHydrogenPDBSelector())
    chains= IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)
#   number of TMH
    n_TMH=2
#   TMH boundaries
    b=[38,80]
    e=[73,106]
#   select particles and make rigid bodies
    print "Making rigid bodies"
    rbs=[]
    for i in [0,n_TMH-1]:
        s=IMP.atom.Selection(chains[0], residue_indexes=[(b[i],e[i]+1)])
        p=s.get_selected_particles()
        rbs.append(IMP.atom.create_rigid_body(p))
#   rotation to make z the principal axis
    rt= IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), math.pi/2.0)
    tr= IMP.algebra.Transformation3D(rt, IMP.algebra.Vector3D(0,0,0))
#   set the new reference frame
    for i in [0,n_TMH-1]:
        rbs[i].set_reference_frame(IMP.algebra.ReferenceFrame3D(tr))
        bb=rbs[i].get_member(0).get_coordinates()[2]
        ee=rbs[i].get_member(rbs[i].get_number_of_members()-1).get_coordinates()[2]
#   initialize decorator
        d_rbs=IMP.membrane.HelixDecorator.setup_particle(rbs[i],bb,ee)
        print " Rigid #",i," number of residues=",rbs[i].get_number_of_members()
        print "              begin=",d_rbs.get_begin()," end=",d_rbs.get_end()
#   trial translation+rotation
    tr0= IMP.algebra.Transformation3D(IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(1,0,0), -math.pi/6), IMP.algebra.Vector3D(-10,0,0))
#    rbs[0].set_transformation(IMP.algebra.compose(tr0, rbs[0].get_transformation()))
    IMP.core.transform(rbs[0],tr0)
    tr1= IMP.algebra.Transformation3D(IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(1,0,0), math.pi/6), IMP.algebra.Vector3D(10,0,0))
#    rbs[1].set_transformation(IMP.algebra.compose(tr1, rbs[1].get_transformation()))
    IMP.core.transform(rbs[1],tr1)
    return (m, rbs)

def create_restraints(m, rbs):
    def add_excluded_volume(rb):
#        print "defining excluded volume"
        lsc= IMP.container.ListSingletonContainer(m)
        for i in [0,len(rb)-1]:
            lsc.add_particles(rb[i].get_members())
        nbl= IMP.container.ClosePairContainer(lsc, 0, IMP.core.RigidClosePairsFinder(), 2.0)
        ps= IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1))
        evr= IMP.container.PairsRestraint(ps, nbl)
        m.add_restraint(evr)
        m.set_maximum_score(evr, .01)
    def add_distance_restraint(s0, s1, x0, k):
#       print "defining distance restraint"
        hub= IMP.core.HarmonicUpperBound(x0,k)
        df= IMP.core.DistancePairScore(hub)
        r= IMP.core.PairRestraint(df, IMP.ParticlePair(s0, s1))
        m.add_restraint(r)
#       set tolerance
        m.set_maximum_score(r, .01)
    def add_packing_restraint(rb):
## if the rigid bodies are close, apply a filter on the crossing angle
## first define the allowed intervals, by specifying the center
## of the distributions
        om0=[-156.5, 146.4, -37.9, 13.8, 178.0, 25.5]
#  the sigmas
        sig_om0=[10.1, 13.6, 7.50, 16.6, 20.8, 11.2]
#  distance cutoff
        dd0=[8.61, 8.57, 7.93, 9.77, 9.14, 8.55]
#  and distance sigmas
        sig_dd0=[0.89, 0.99, 0.88, 1.18, 1.47, 1.05]
#  the number of sigmas
        nsig=2
#  and the number of clusters
        ncl=5
# create allowed intervals (omega in radians)
        om_b=[]
        om_e=[]
        dd_b=[]
        dd_e=[]
        for i in [0, ncl-1]:
            om_b.append((om0[i]-nsig*sig_om0[i])/180.*math.pi)
            om_e.append((om0[i]+nsig*sig_om0[i])/180.*math.pi)
            dd_b.append(dd0[i]-nsig*sig_dd0[i])
            dd_e.append(dd0[i]+nsig*sig_dd0[i])
        lrb= IMP.container.ListSingletonContainer(m)
        for i in [0,len(rb)-1]:
            lrb.add_particle(rb[i])
        nrb= IMP.container.ClosePairContainer(lrb, 12.0, 2.0)
        ps=  IMP.membrane.RigidBodyPackingScore(om_b, om_e, dd_b, dd_e)
        prs= IMP.container.PairsRestraint(ps, nrb)
        m.add_restraint(prs)
        m.set_maximum_score(prs, .01)
## ?? def add_penetration_restraint(rb): ??
## restraint on the penetration depth

## DOPE/GQ scoring

    add_excluded_volume(rbs)
    for i in [0,len(rbs)-2]:
        p0=rbs[i].get_member(rbs[i].get_number_of_members()-1)
        p1=rbs[i+1].get_member(0)
        add_distance_restraint(p0,p1,20.0,100)
    add_packing_restraint(rbs)
    return m.get_restraints()

#def create_discrete_states

#def create_sampler

def display(m):
    m.update()
    w= IMP.display.PymolWriter("out.pym")
    for i in [0,1]:
        j=1
        for p in rbs[i].get_members():
            g= IMP.display.XYZRGeometry(p)
            g.set_name(str(i))
            g.set_color(IMP.display.get_display_color(j))
            w.add_geometry(g)
            j=2

# Here starts the real job...
IMP.set_log_level(IMP.VERBOSE)
print "creating representation"
(m,rbs)=create_representation()

print "creating score function"
rs=create_restraints(m,rbs)

#print "optimizer"
#o= IMP.core.ConjugateGradients()
#o.set_model(m)
#o.optimize(500)

print "creating visualization"
display(m)
