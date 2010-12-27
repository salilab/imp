import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import IMP.membrane
import math

class SameResidueFilter(IMP.PairFilter):
    def __init__(self):
        IMP.PairFilter.__init__(self)
    def get_contains_particle_pair(self, pp):
        diff= pp[0].get_value(IMP.IntKey("num"))-pp[1].get_value(IMP.IntKey("num"))
        if diff==0:
            return True
        return False
    def get_input_particles(self, p):
        return [p]
    def get_input_containers(self, p):
        return []
    def do_show(self, out):
        pass

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
#    tr0= IMP.algebra.Transformation3D(IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(1,0,0), 0.0), IMP.algebra.Vector3D(-5,0,0))
#    IMP.core.transform(rbs[0],tr0)
#    tr1= IMP.algebra.Transformation3D(IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(1,0,0), 0.0), IMP.algebra.Vector3D(5,0,0))
#    IMP.core.transform(rbs[1],tr1)
    return (m, chain)

def create_restraints(m, chain, tmb, tme):
    def add_excluded_volume():
        lsc= IMP.container.ListSingletonContainer(m)
        for i in range(len(tmb)):
            s=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), residue_indexes=[(tmb[i],tme[i]+1)])
            lsc.add_particles(s.get_selected_particles())
        nbl= IMP.container.ClosePairContainer(lsc, 0, IMP.core.RigidClosePairsFinder(), 2.0)
        ps= IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1000))
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
## DOPE/GQ scoring
    def add_DOPE():
# initializing data
        IMP.membrane.add_dope_score_data(chain)
# creating containers
        dsc= IMP.container.ListSingletonContainer(m)
        for i in range(len(tmb)):
            s=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), residue_indexes=[(tmb[i],tme[i]+1)])
            ps=s.get_selected_particles()
            for p in ps:
                p.add_attribute(IMP.IntKey("num"), IMP.atom.get_residue(IMP.atom.Atom(p)).get_index())
            dsc.add_particles(ps)
        dpc= IMP.container.ClosePairContainer(dsc, 15.0, 0.0)
# exclude pairs of atoms belonging to the same residue
# for consistency with MODELLER DOPE score
        f= SameResidueFilter()
        dpc.add_pair_filter(f)
        dps= IMP.membrane.DopePairScore(15.0)
        d=   IMP.container.PairsRestraint(dps, dpc)
        m.add_restraint(d)

# assembling all the restraints
    add_excluded_volume()
    for i in range(len(tmb)-1):
        s0=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tme[i])
        s1=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tmb[i+1])
        p0=s0.get_selected_particles()[0]
        p1=s1.get_selected_particles()[0]
        length=(tmb[i+1]-tme[i])*3.0
        add_distance_restraint(p0,p1,length,1000)
    add_packing_restraint()
    add_DOPE()
    return m.get_restraints()

# creating the discrete states for domino
def  create_discrete_states(m,chain,tmb):
    rot00=  IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), math.pi/2.0)
    rot01=  IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), -math.pi/2.0)
    trs0=[]
    trs1=[]
    for i in range(0,4):
        rotz=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,0,1), i*math.pi/2)
        for t in range(0,5):
            tilt=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), t*math.pi/18)
            rot1=IMP.algebra.compose(tilt,rotz)
            for s in range(0,4):
                if ( t == 0 ) and ( s != 0 ):
                    break
                swing=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,0,1), s*math.pi/2)
                rot2=IMP.algebra.compose(swing,rot1)
                rot_p =IMP.algebra.compose(rot2,rot00)
                rot_m =IMP.algebra.compose(rot2,rot01)
                for dz in range(0,2):
                    trs0.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(rot_p,IMP.algebra.Vector3D(0,0,1.0*dz))))
                    for dx in range(0,5):
                        if ( dx >= 0 ):
                            trs1.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(rot_p,IMP.algebra.Vector3D(9.0+1.0*dx,0,1.0*dz))))

    pstate0= IMP.domino.RigidBodyStates(trs0)
    pstate1= IMP.domino.RigidBodyStates(trs1)
    pst= IMP.domino.ParticleStatesTable()
# getting rigid bodies
    rbs=[]
    for i in range(len(tmb)):
        s0=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), atom_type = IMP.atom.AT_CA, residue_index = tmb[i])
        rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        rbs.append(rb)
    pst.set_particle_states(rbs[0], pstate0)
    pst.set_particle_states(rbs[1], pstate1)
    return pst

# setting up domino (and filters)
def create_sampler(m, pst):
    s=IMP.domino.DominoSampler(m, pst)
    # the following lines recreate the defaults and so are optional
    filters=[]
    # do not allow particles with the same ParticleStates object
    # to have the same state index
    filters.append(IMP.domino.ExclusionSubsetFilterTable(pst))
    # filter states that score worse than the cutoffs in the Model
    filters.append(IMP.domino.RestraintScoreSubsetFilterTable(m, pst))
    states= IMP.domino.BranchAndBoundSubsetStatesTable(pst, filters)
    s.set_subset_states_table(states)
    s.set_subset_filter_tables(filters)
    return s

def display(m,chain,tmb,tme,name):
    m.update()
    w= IMP.display.PymolWriter(name)
    for i in range(len(tmb)):
        jj=0
        s=IMP.atom.Selection(IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE), residue_indexes=[(tmb[i],tme[i]+1)])
        ps=s.get_selected_particles()
        for p in ps:
            jj+=1
            g= IMP.display.XYZRGeometry(p)
            g.set_name("TM"+str(i))
            c=i
            if ( jj == 1 ) : c=10
            g.set_color(IMP.display.get_display_color(c))
            w.add_geometry(g)

# Here starts the real job...
#IMP.set_log_level(IMP.VERBOSE)

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
score=[]
for i in range(cs.get_number_of_configurations()):
    cs.load_configuration(i)
    ss=m.evaluate(False)
    score.append(ss)
    print "** solution number:",i," is:",ss

topscore = 30
print "visualizing the top ",topscore
for i in range(0,topscore):
    low=min(score)
    ii=score.index(low)
    score[ii]=10000000.
    print "** solution number:",i," is:",low
    cs.load_configuration(ii)
    display(m,chain,tmb,tme,"sol_"+str(i)+".score_"+str(low)+".pym")
