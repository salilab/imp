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
    tr0= IMP.algebra.Transformation3D(IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(1,0,0), 0.0), IMP.algebra.Vector3D(-5,0,0))
    IMP.core.transform(rbs[0],tr0)
    tr1= IMP.algebra.Transformation3D(IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(1,0,0), 0.0), IMP.algebra.Vector3D(5,0,0))
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
        ncl=5
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
# initializing data
        IMP.membrane.add_dope_score_data(chain)
# creating containers
        dsc= IMP.container.ListSingletonContainer(m)
        for i in range(len(tmb)):
            s=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), residue_indexes=[(tmb[i],tme[i]+1)])
            dsc.add_particles(s.get_selected_particles())
# exclude pairs of atoms belonging to the same residue
# for consistency with MODELLER DOPE score
        dpc= IMP.container.ClosePairContainer(dsc, 15.0, 0.0)


        dps= IMP.membrane.DopePairScore(15.0)
        d=   IMP.container.PairsRestraint(dps, dpc)
        m.add_restraint(d)

    add_excluded_volume()
    for i in range(len(tmb)-1):
        s0=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tme[i])
        s1=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tmb[i+1])
        p0=s0.get_selected_particles()[0]
        p1=s1.get_selected_particles()[0]
        length=(tmb[i+1]-tme[i])*3.0
        add_distance_restraint(p0,p1,length,100)
    add_packing_restraint()
    add_DOPE()
    return m.get_restraints()

# creating the discrete states for domino
def  create_discrete_states(m,chain,tmb)
    trs= [ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                            Vector2D(0,0,0))),
          ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                            Vector3D(5,0,0))),
          ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                            Vector3D(10,0,0))),
          ReferenceFrame3D(Transformation3D(get_identity_rotation_3d(),
                                            Vector3D(15,0,0)))]
    pstate= IMP.domino.RigidBodyStates(trs)
    pst= IMP.domino.ParticleStatesTable()
# getting rigid bodies
    rbs=[]
    for i in range(len(tmb)):
        s0=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tmb[i])
        rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        rbs.append(rb)
    pst.set_particle_states(rbs[0], pstate)
    pst.set_particle_states(rbs[1], pstate)
    return pst

def create_sampler(m, pst):
    s=IMP.domino.DominoSampler(m, pst)
    s.set_log_level(IMP.VERBOSE)
    # the following lines recreate the defaults and so are optional
#    filters=[]
    # do not allow particles with the same ParticleStates object
    # to have the same state index
#    filters.append(IMP.domino.ExclusionSubsetFilterTable(pst))
    # filter states that score worse than the cutoffs in the Model
#    filters.append(IMP.domino.RestraintScoreSubsetFilterTable(m, pst))
#    filters[-1].set_log_level(IMP.SILENT)
#    mf=MyFilterTable(ps[1], 0)
    # try with and without this line
#    filters.append(mf)
#    states= IMP.domino.BranchAndBoundSubsetStatesTable(pst, filters)
    #states.set_log_level(IMP.SILENT);
#    s.set_subset_states_table(states)
#    s.set_subset_filter_tables(filters)
    return s

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

print "creating representation"
# TMH boundaries
tmb=[38,80]
tme=[73,106]
(m,chain)=create_representation(tmb,tme)

print "creating score function"
rs=create_restraints(m,chain,tmb,tme)

print "creating discrete states"
pst=create_discrete_states(m,chain,tmb)

print "creating sampler"
s=create_sampler(m, pst)

print "sampling"
cs=s.get_sample()

print "found ", cs.get_number_of_configurations(), "solutions"
for i in range(cs.get_number_of_configurations()):
    cs.load_configuration(i)
    print "solution number:",i," is:", m.evaluate(False)

#print "creating visualization"
#display(m,chain,tmb,tme)
#IMP.atom.write_pdb(chain,"test.pdb")
