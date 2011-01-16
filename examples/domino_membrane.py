import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import IMP.membrane
import math

def mysign(x):
    if x >= 0.0:
        return 1
    else:
        return -1

def create_representation(TMH,topo):
    m=IMP.Model()
    #mp0= IMP.atom.read_pdb('2K9P_OPM.pdb', m, IMP.atom.NonWaterNonHydrogenPDBSelector())
    #mp0= IMP.atom.read_pdb('2K9P_OPM.pdb', m, IMP.atom.CAlphaPDBSelector())
    mp0= IMP.atom.read_pdb('1rwt_OPM.pdb', m, IMP.atom.CAlphaPDBSelector())
    chain=IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)[0]
#   updating CA radius to match residue volume
    for p in IMP.atom.get_by_type(chain, IMP.atom.ATOM_TYPE):
        res=IMP.atom.get_residue(IMP.atom.Atom(p))
        rt=res.get_residue_type()
        vol=IMP.atom.get_volume_from_residue_type(rt)
        rg=IMP.algebra.get_ball_radius_from_volume_3d(vol)
        IMP.core.XYZR(p).set_radius(rg)
#   visualize initial configuration
    display(m,chain,TMH,"initial.pym")
#   rotation to make z the principal axis
    rt= IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), math.pi/2.0)
    tr= IMP.algebra.Transformation3D(rt, IMP.algebra.Vector3D(0,0,0))
#   make rigid bodies
    sign=[]
    tbr= IMP.core.TableRefiner()
    for i,h in enumerate(TMH):
        s=IMP.atom.Selection(chain, residue_indexes=[(h[0],h[1]+1)])
        p=s.get_selected_particles()
        rb=IMP.atom.create_rigid_body(p)
#       set the reference frame so that z is the principal axis
        rb.set_reference_frame(IMP.algebra.ReferenceFrame3D(tr))
#       add to TableRefiner
        tbr.add_particle(rb,p)
#       initialize membrane decorator
        s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
        s1=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[1])
        bb=s0.get_selected_particles()[0]
        bb=IMP.core.XYZ(bb).get_coordinates()[2]
        ee=s1.get_selected_particles()[0]
        ee=IMP.core.XYZ(ee).get_coordinates()[2]
        sign.append(mysign((ee-bb)*topo[i]))
        d_rbs=IMP.membrane.HelixDecorator.setup_particle(rb,bb,ee)
#        print " Rigid #",i," number of members=",rb.get_number_of_members()
#        print "              begin=",d_rbs.get_begin()," end=",d_rbs.get_end()
    return (m, chain, tbr, sign)

def create_restraints(m, chain, tbr, TMH):

    def add_excluded_volume():
        lsc= IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s=IMP.atom.Selection(chain, residue_indexes=[(h[0],h[1]+1)])
            lsc.add_particles(s.get_selected_particles())
        evr=IMP.core.ExcludedVolumeRestraint(lsc,1000)
        m.add_restraint(evr)
        m.set_maximum_score(evr, .01)

    def add_distance_restraint(s0, s1, x0, k):
        hub= IMP.core.HarmonicUpperBound(x0,k)
        df= IMP.core.DistancePairScore(hub)
        dr= IMP.core.PairRestraint(df, IMP.ParticlePair(s0, s1))
        m.add_restraint(dr)
        m.set_maximum_score(dr, .01)
        return dr

    def add_packing_restraint():
## if the helices are interacting, apply a filter on the crossing angle
## first define the allowed intervals, by specifying the center
## of the distributions (Walters and DeGrado PNAS (2007) 103:13658)
        om0=(-156.5, 146.4, -37.9, 13.8, 178.0, 25.5)
#  the sigmas
        sig_om0=(10.1, 13.6, 7.50, 16.6, 20.8, 11.2)
#  distance cutoff
        dd0=(8.61, 8.57, 7.93, 9.77, 9.14, 8.55)
#  and distance sigmas
        sig_dd0=(0.89, 0.99, 0.88, 1.18, 1.47, 1.05)
#  the allowed number of sigma
        nsig=2
#  and the number of clusters
        ncl=6
# create allowed intervals (omega in radians)
# and control periodicity
        om_b=[]; om_e=[]; dd_b=[]; dd_e=[]
        for i in range(ncl):
            omb=(om0[i]-nsig*sig_om0[i])/180.*math.pi
            ome=(om0[i]+nsig*sig_om0[i])/180.*math.pi
            ddb=dd0[i]-nsig*sig_dd0[i]
            dde=dd0[i]+nsig*sig_dd0[i]
            om_b.append(max(omb,-math.pi))
            om_e.append(min(ome,math.pi))
            dd_b.append(ddb)
            dd_e.append(dde)
            if ( omb < -math.pi ):
                om_b.append(2*math.pi+omb)
                om_e.append(math.pi)
                dd_b.append(ddb)
                dd_e.append(dde)
            if ( ome > math.pi ):
                om_b.append(-math.pi)
                om_e.append(-2*math.pi+ome)
                dd_b.append(ddb)
                dd_e.append(dde)
        lrb= IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
            rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
            lrb.add_particle(rb)
        nrb= IMP.container.ClosePairContainer(lrb, 15.0)
        ps=  IMP.membrane.RigidBodyPackingScore(tbr, om_b, om_e, dd_b, dd_e)
        prs= IMP.container.PairsRestraint(ps, nrb)
        m.add_restraint(prs)
        m.set_maximum_score(prs, .01)

## DOPE/GQ scoring
    def add_DOPE():
        IMP.membrane.add_dope_score_data(chain)
        dsc=[]
        for i,h in enumerate(TMH):
            dsc.append(IMP.container.ListSingletonContainer(m))
            s=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_indexes=[(h[0],h[1]+1)])
            dsc[i].add_particles(s.get_selected_particles())
        for i in range(len(TMH)-1):
            for j in range(i+1,len(TMH)):
                dpc= IMP.container.CloseBipartitePairContainer(dsc[i], dsc[j], 15.0, 0.0)
                dps= IMP.membrane.DopePairScore(15.0)
                dope=IMP.container.PairsRestraint(dps, dpc)
                m.add_restraint(dope)

# assembling all the restraints
    rset=IMP.RestraintSet()
    add_excluded_volume()
    for i in range(len(TMH)-1):
        s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = TMH[i][1])
        s1=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = TMH[i+1][0])
        p0=s0.get_selected_particles()[0]
        p1=s1.get_selected_particles()[0]
        rb0=IMP.core.RigidMember(p0).get_rigid_body()
        rb1=IMP.core.RigidMember(p1).get_rigid_body()
        length=1.8*(TMH[i+1][0]-TMH[i][1]+1)+7.4
        dr=add_distance_restraint(p0,p1,length,1000)
        rdr=add_distance_restraint(rb0,rb1,30.0,1000)
        rset.add_restraint(dr)
    add_packing_restraint()
    add_DOPE()
    return rset

# creating the discrete states for domino
def  create_discrete_states(m,chain,TMH,sign):
#   store initial rotation to have the right topology
    rot0=[]
    for i in range(len(TMH)):
        rot0.append(IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), sign[i]*math.pi/2.0))
    trs0=[]; trs1=[]; trs2=[]
    for i in range(0,1):
        rotz=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,0,1), i*math.pi/2)
        for t in range(0,6):
            tilt=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), t*math.pi/18)
            rot1=IMP.algebra.compose(tilt,rotz)
            for s in range(0,1):
                if ( t == 0 ) and ( s != 0 ) : break
                swing=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,0,1), s*math.pi/2)
                rot2=IMP.algebra.compose(swing,rot1)
                trs0.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.compose(rot2,rot0[0]),
                                        IMP.algebra.Vector3D(0,0,0))))
                for dz in range(0,1):
                    for dx in range(-15,15):
                        if ( dx >= 5 ):
                            trs1.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.compose(rot2,rot0[1]),
                                        IMP.algebra.Vector3D(dx,0,dz))))
                        for dy in range(-15,15):
                            trs2.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.compose(rot2,rot0[2]),
                                        IMP.algebra.Vector3D(dx,dy,dz))))

    pst= IMP.domino.ParticleStatesTable()
    pstate=[]
    pstate.append(IMP.domino.RigidBodyStates(trs0))
    pstate.append(IMP.domino.RigidBodyStates(trs1))
    pstate.append(IMP.domino.RigidBodyStates(trs2))
    for i,h in enumerate(TMH):
        s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
        rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        pst.set_particle_states(rb, pstate[i])
    return pst

# setting up domino (and filters)
def create_sampler(m, rset, pst):
    s=IMP.domino.DominoSampler(m, pst)
    ig= IMP.domino.get_interaction_graph(rset, pst)
    jt= IMP.domino.get_junction_tree(ig)
#   set filters
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

def display(m,chain,TMH,name):
    m.update()
    w= IMP.display.PymolWriter(name)
    for i,h in enumerate(TMH):
        jj=0
        s=IMP.atom.Selection(chain, residue_indexes=[(h[0],h[1]+1)])
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
IMP.set_log_level(IMP.VERBOSE)

# TMH definition and topology
# 1rwt definition
TMH=[[64,82],[124,142],[179,199]]
topo=[+1.0, -1.0, +1.0]
# 2k9p definition
#TMH=[[38,73],[80,106]]
#topo=[-1.0,+1.0]

print "creating representation"
(m,chain,tbr,sign)=create_representation(TMH,topo)

print "creating score function"
rset=create_restraints(m,chain,tbr,TMH)

print "creating discrete states"
pst=create_discrete_states(m,chain,TMH,sign)

print "creating sampler"
s=create_sampler(m, rset, pst)

print "sampling"
ass=IMP.domino.Subset(pst.get_particles())
cs=s.get_sample_states(ass)

print "found ", len(cs), "solutions"

score=[]
for i,v in enumerate(cs):
    IMP.domino.load_particle_states(ass,v,pst)
    ss=m.evaluate(False)
    score.append(ss)
#    print "** solution number:",i," is:",ss

topscore = 30
topscore = min(topscore,len(cs))
print "visualizing the top ",topscore
for i in range(0,topscore):
    low=min(score)
    ii=score.index(low)
    score[ii]=10000000.
    print "** solution number:",i," is:",low
    IMP.domino.load_particle_states(ass,cs[ii],pst)
    display(m,chain,TMH,"sol_"+str(i)+".score_"+str(low)+".pym")
