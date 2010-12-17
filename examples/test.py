import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import IMP.membrane
import math

def create_representation(tmb,tme):
    m=IMP.Model()
#   only CA
    mp0= IMP.atom.read_pdb('2K9P_OMP.pdb', m, IMP.atom.CAlphaPDBSelector())
#   all-atom
#    mp0= IMP.atom.read_pdb('2K9P_OMP.pdb', m, IMP.atom.NonWaterNonHydrogenPDBSelector())
    chain=IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)[0]
#   select particles and make rigid bodies
    print "Making rigid bodies"
    rbs=[]
    for i in range(len(tmb)):
        s=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), residue_indexes=[(tmb[i],tme[i]+1)])
        p=s.get_selected_particles()
        rbs.append(IMP.atom.create_rigid_body(p))
#   rotation to make z the principal axis
    rt= IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), math.pi/2.0)
    tr= IMP.algebra.Transformation3D(rt, IMP.algebra.Vector3D(0,0,0))
#   set the new reference frame
    for i in range(len(tmb)):
        rbs[i].set_reference_frame(IMP.algebra.ReferenceFrame3D(tr))
#   initialize decorator
        s0=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tmb[i])
        s1=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tme[i])
        bb=s0.get_selected_particles()[0]
        bb=IMP.core.XYZ(bb).get_coordinates()[2]
        ee=s1.get_selected_particles()[0]
        ee=IMP.core.XYZ(ee).get_coordinates()[2]
        d_rbs=IMP.membrane.HelixDecorator.setup_particle(rbs[i],bb,ee)
        print " Rigid #",i," number of members=",rbs[i].get_number_of_members()
        print "              begin=",d_rbs.get_begin()," end=",d_rbs.get_end()
#   trial translation+rotation
    tr0= IMP.algebra.Transformation3D(IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(1,0,0), -math.pi/6), IMP.algebra.Vector3D(-5,0,0))
#    rbs[0].set_transformation(IMP.algebra.compose(tr0, rbs[0].get_transformation()))
    IMP.core.transform(rbs[0],tr0)
    tr1= IMP.algebra.Transformation3D(IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(1,0,0), math.pi/6), IMP.algebra.Vector3D(5,0,0))
#    rbs[1].set_transformation(IMP.algebra.compose(tr1, rbs[1].get_transformation()))
    IMP.core.transform(rbs[1],tr1)
    return (m, chain)

def create_restraints(m, chain, tmb, tme):
    def add_excluded_volume():
        lsc= IMP.container.ListSingletonContainer(m)
        for i in range(len(tmb)):
            s=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), residue_indexes=[(tmb[i],tme[i]+1)])
            lsc.add_particles(s.get_selected_particles())
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
    def add_packing_restraint():
## if the rigid bodies are close, apply a filter on the crossing angle
## first define the allowed intervals, by specifying the center
## of the distributions (Walters and DeGrado PNAS (2007) 103:13658)
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
        ncl=6
# create allowed intervals (omega in radians)
        om_b=[]
        om_e=[]
        dd_b=[]
        dd_e=[]
        for i in range(ncl):
            om_b.append((om0[i]-nsig*sig_om0[i])/180.*math.pi)
            om_e.append((om0[i]+nsig*sig_om0[i])/180.*math.pi)
            dd_b.append(dd0[i]-nsig*sig_dd0[i])
            dd_e.append(dd0[i]+nsig*sig_dd0[i])
        lrb= IMP.container.ListSingletonContainer(m)
        for i in range(len(tmb)):
            s0=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tmb[i])
            rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
            lrb.add_particle(rb)
        nrb= IMP.container.ClosePairContainer(lrb, 12.0, 1.0)
        ps=  IMP.membrane.RigidBodyPackingScore(om_b, om_e, dd_b, dd_e)
        prs= IMP.container.PairsRestraint(ps, nrb)
        m.add_restraint(prs)
        m.set_maximum_score(prs, .01)
## ?? def add_penetration_restraint(rb): ??
## restraint on the penetration depth

## DOPE/GQ scoring
    def add_DOPE():
        dsc= IMP.container.ListSingletonContainer(m)
        for i in range(len(tmb)):
            s=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), residue_indexes=[(tmb[i],tme[i]+1)])
            dsc.add_particles(s.get_selected_particles())
#       initializing data
        IMP.membrane.add_dope_score_data(chain)
        dpc= IMP.container.ClosePairContainer(dsc, 15.0, 0.0)
        dps= IMP.membrane.DopePairScore("dope.lib")
        d=   IMP.container.PairsRestraint(dps, dpc)
        m.add_restraint(d)

    add_excluded_volume()
    for i in range(len(tmb)-1):
        s0=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tme[i])
        s1=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tmb[i+1])
        p0=s0.get_selected_particles()[0]
        p1=s1.get_selected_particles()[0]
        add_distance_restraint(p0,p1,20.0,100)
    add_packing_restraint()
    add_DOPE()
    return m.get_restraints()

#def create_discrete_states

#def create_sampler

def display(m,chain,tmb,tme):
    m.update()
    w= IMP.display.PymolWriter("out.pym")
    for i in range(len(tmb)):
        j=1
        s=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), residue_indexes=[(tmb[i],tme[i]+1)])
        ps=s.get_selected_particles()
        for p in ps:
            g= IMP.display.XYZRGeometry(p)
            g.set_name(str(i))
            g.set_color(IMP.display.get_display_color(j))
            w.add_geometry(g)
            j=2

# Here starts the real job...
IMP.set_log_level(IMP.VERBOSE)
# TMH boundaries
tmb=[38,80]
tme=[73,106]
print "creating representation"
(m,chain)=create_representation(tmb,tme)

print "creating score function"
rs=create_restraints(m,chain,tmb,tme)

#print "optimizer"
#o= IMP.core.ConjugateGradients()
#o.set_model(m)
#o.optimize(500)

print "creating visualization"
display(m,chain,tmb,tme)
