import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import IMP.spb
import math

def create_representation(seq,tmh,topo):

    m=IMP.Model()
    tbr= IMP.core.TableRefiner()
    all=IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))

    def generate_balls(id,seq,tmh,sign):
        nres=len(seq)
        atoms=[]
        for i in range(nres):
            x=2.3*math.cos(math.radians(100.0)*float(i))
            y=2.3*math.sin(math.radians(100.0)*float(i))
            z=1.51*(float(i)-float((nres-1))/2.0)
            # set up residue
            p=IMP.Particle(m)
            r=IMP.atom.Residue.setup_particle(p, IMP.atom.get_residue_type(seq[i]), i+tmh[0])
            rt=r.get_residue_type()
            vol=IMP.atom.get_volume_from_residue_type(rt)
            #rg=IMP.algebra.get_ball_radius_from_volume_3d(vol)
            rg=2.273
            rd=IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(x,y,z),rg))
            # set up atom
            p1=IMP.Particle(m)
            a=IMP.atom.Atom.setup_particle(p1, IMP.atom.AT_CA)
            ad=IMP.core.XYZR.setup_particle(p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(x,y,z),rg))
            r.add_child(a)
            all.add_child(r)
            atoms.append(ad)
            if (i == 0):      begin=p1
            if (i == nres-1): end=p1
        rb=IMP.Particle(m)
        rb=IMP.atom.create_rigid_body(atoms,"TM"+str(id))
        tbr.add_particle(rb,atoms)
        # adjust axis to match topology
        bb=IMP.core.RigidMember(begin).get_internal_coordinates()[0]
        ee=IMP.core.RigidMember(end).get_internal_coordinates()[0]
        d_rbs=IMP.spb.HelixDecorator.setup_particle(rb,bb,ee)
        if ( sign * ( ee - bb ) > 0 ): rot0= -math.pi/2.0
        else :                         rot0=  math.pi/2.0
        return rot0

    rot0=[]
    for i in range(len(seq)):
        rot=generate_balls(i,seq[i],tmh[i],topo[i])
        rot0.append(rot)

    return m,all,tbr,rot0

def create_restraints(m, chain, tbr, TMH):

    def add_excluded_volume():
        lsc= IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s=IMP.atom.Selection(chain, residue_indexes=[(h[0],h[1]+1)])
            lsc.add_particles(s.get_selected_particles())
        evr=IMP.core.ExcludedVolumeRestraint(lsc,1000)
        m.add_restraint(evr)
        m.set_maximum_score(evr, 0.01)

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
        om0=(-156.5, 146.4, -37.9, 13.8, 178.0, 25.5, -161.1, 44.8, 127.4, -60.2, -129.2, 2.4, 161.0)
#  the sigmas
        sig_om0=(10.1, 13.6, 7.50, 16.6, 20.8, 11.2, 10.3, 8.8, 12.3, 14.8, 12.9, 16.2, 17.6)
#  distance cutoff
        dd0=(8.61, 8.57, 7.93, 9.77, 9.14, 8.55, 9.30, 7.96, 9.40, 8.61, 8.97, 8.55, 8.75)
#  and distance sigmas
        sig_dd0=(0.89, 0.99, 0.88, 1.18, 1.47, 1.05, 1.57, 1.13, 1.0, 1.04, 1.65, 0.78, 1.33)
#  the allowed number of sigma
        nsig=3
#  and the number of clusters
        ncl=13
# create allowed intervals (omega in radians)
# and control periodicity
        om_b=[]; om_e=[]; dd_b=[]; dd_e=[]
        for i in range(ncl):
            omb=math.radians(om0[i]-nsig*sig_om0[i])
            ome=math.radians(om0[i]+nsig*sig_om0[i])
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
        nrb= IMP.container.ClosePairContainer(lrb, 25.0)
        ps=  IMP.spb.RigidBodyPackingScore(tbr, om_b, om_e, dd_b, dd_e)
        prs= IMP.container.PairsRestraint(ps, nrb)
        m.add_restraint(prs)
        m.set_maximum_score(prs, .01)

## DOPE/GQ scoring
    def add_DOPE():
        IMP.atom.add_dope_score_data(chain)
        dsc=IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_indexes=[(h[0],h[1]+1)])
            dsc.add_particles(s.get_selected_particles())
        dpc=IMP.container.ClosePairContainer(dsc, 15.0, 0.0)
        f=IMP.spb.SameHelixPairFilter()
        dpc.add_pair_filter(f)
        dps=IMP.atom.DopePairScore(15.0)#, IMP.atom.get_data_path("dope_new.lib"))
        dope=IMP.container.PairsRestraint(dps, dpc)
        m.add_restraint(dope)
        m.set_maximum_score(dope, .01)

    def add_interacting_restraint(h0, h1):
        s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h0[0])
        s1=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h1[0])
        rb0=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
        rb1=IMP.core.RigidMember(s1.get_selected_particles()[0]).get_rigid_body()
        lpc= IMP.container.ListPairContainer(m)
        lpc.add_particle_pair([rb0,rb1])
        hub= IMP.core.HarmonicUpperBound(3.454,1)
        sd=  IMP.core.SphereDistancePairScore(hub)
        kc=  IMP.core.KClosePairsPairScore(sd,tbr,1)
        ir=  IMP.container.PairsRestraint(kc,lpc)
        m.add_restraint(ir)
        m.set_maximum_score(ir,.01)
        return ir

    def add_diameter_restraint(diameter):
        lrb= IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
            rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
            lrb.add_particle(rb)
        h=IMP.core.HarmonicUpperBound(0,1000)
        r=IMP.core.DiameterRestraint(h, lrb, diameter)
        m.add_restraint(r)
        m.set_maximum_score(r,.01)


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
        length=1.6*(TMH[i+1][0]-TMH[i][1]+1)+7.4
        dr=add_distance_restraint(p0,p1,length,1000)
        #rdr=add_distance_restraint(rb0,rb1,30.0,1000)
        rset.add_restraint(dr)
    add_packing_restraint()
    add_DOPE()
    add_diameter_restraint(35.0)
    ir0=add_interacting_restraint(TMH[1],TMH[2])
    ir1=add_interacting_restraint(TMH[0],TMH[3])
    rset.add_restraint(ir0)
    rset.add_restraint(ir1)
    return rset

# creating the discrete states for domino
def create_discrete_states(m,chain,TMH):
     # sampling parameters
# bin size
    Dtilt=math.radians(30)
    Dswing=math.radians(90)
    Drot=math.radians(90)
    Dx=5.0
# nbin
    itilt=int(math.radians(30)/Dtilt)
    iswing=int(math.radians(360)/Dswing)
    irot=int(math.radians(360)/Drot)
    ix=int(15.0/Dx)
    iz=int(5.0/Dx)
#   store initial rotation to have the right topology
#   TO_DO!!!
    rot0=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), math.pi/2.0)
    trs0=[]; trs1=[]; trs2=[];
    for i in range(irot):
        rotz=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,0,1), float(i)*Drot)
        for t in range(itilt+1):
            tilt=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), float(t)*Dtilt)
            rot1=IMP.algebra.compose(tilt,rotz)
            for s in range(iswing):
                if ( t == 0 ) and ( s != 0 ) : break
                swing=IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,0,1), float(s)*Dswing)
                rot2=IMP.algebra.compose(swing,rot1)
                for dz in range(-iz,iz+1):
                    trs0.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.compose(rot2,rot0),
                                            IMP.algebra.Vector3D(0,0,float(dz)*Dx))))
                    for dx in range(-ix,ix+1):
                        if ( float(dx)*Dx >= 6.0 ):
                            trs1.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.compose(rot2,rot0),
                                        IMP.algebra.Vector3D(float(dx)*Dx,0,float(dz)*Dx))))
                        for dy in range(-ix,ix+1):
                            trs2.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(IMP.algebra.compose(rot2,rot0),
                                        IMP.algebra.Vector3D(float(dx)*Dx,float(dy)*Dx,float(dz)*Dx))))
                            #print i, t, s, dx, dy, dz

    pst= IMP.domino.ParticleStatesTable()
    pstate=[]
    pstate.append(IMP.domino.RigidBodyStates(trs0))
    pstate.append(IMP.domino.RigidBodyStates(trs1))
    pstate.append(IMP.domino.RigidBodyStates(trs2))
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
    print ig,jt
#   set filters
    filters=[]
    # do not allow particles with the same ParticleStates object
    # to have the same state index
    filters.append(IMP.domino.ExclusionSubsetFilterTable(pst))
    # filter states that score worse than the cutoffs in the Model
    filters.append(IMP.domino.RestraintScoreSubsetFilterTable(m, pst))
    states= IMP.domino.BranchAndBoundSubsetStatesTable(pst, filters)
    s.set_use_cross_subset_filtering(True)
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
#IMP.set_log_level(IMP.VERBOSE)

# TM regions
TMH= [[24,48], [75,94], [220,238], [254,276]]

# define TMH sequences
seq0=("M","L","I","H","N","W","I","L","T","F","S","I","F","R","E","H","P","S","T","V","F","Q","I","F","T","K","C","I","L","V","S","S","S","F","L","L","F","Y","T","L","L","P","H","G","L","L","E","D","L","M","R","R","V","G","D","S","L","V","D","L","I","V","I","C","E","D","S","Q","G","Q","H","L","S","S","F","C","L","F","V","A","T","L","Q","S","P","F","S","A","G","V","S","G","L","C","K","A","I","L","L","P","S","K","Q","I","H","V","M","I","Q","S","V","D","L","S","I","G","I","T","N","S","L","T","N","E","Q","L","C","G","F","G","F","F","L","N","V","K","T","N","L","H","C","S","R","I","P","L","I","T","N","L","F","L","S","A","R","H","M","S","L","D","L","E","N","S","V","G","S","Y","H","P","R","M","I","W","S","V","T","W","Q","W","S","N","Q","V","P","A","F","G","E","T","S","L","G","F","G","M","F","Q","E","K","G","Q","R","H","Q","N","Y","E","F","P","C","R","C","I","G","T","C","G","R","G","S","V","Q","C","A","G","L","I","S","L","P","I","A","I","E","F","T","Y","Q","L","T","S","S","P","T","C","I","V","R","P","W","R","F","P","N","I","F","P","L","I","A","C","I","L","L","L","S","M","N","S","T","L","S","L","F","S","F","S","G","G","R","S","G","Y","V","L","M","L","S","S","K","Y","Q","D","S","F","T","S","K","T","R","N","K","R","E","N","S","I","F","F","L","G","L","N","T","F","T","D","F","R","H","T","I","N","G","P","I","S","P","L","M","R","S","L","T","R","S","T","V","E")

# storing sequences of TMH
seq=[]
for h in TMH:
    tmp=[]
    for j in range(h[0],h[1]+1):
        tmp.append(seq0[j-1])
    seq.append(tmp)

# define the topology
topo=[1.0,-1.0,1.0,-1.0]

print "creating representation"
(m,chain,tbr,rot0_ang)=create_representation(seq,TMH,topo)

print "creating restraints"
rset=create_restraints(m,chain,tbr,TMH)

print "creating discrete states"
pst=create_discrete_states(m,chain,TMH)

print "creating sampler"
s=create_sampler(m, rset, pst)


print "sampling"
ass=IMP.domino.Subset(pst.get_particles())
cs=s.get_sample_states(ass)

print "found ", len(cs), "solutions"

score=[]
for v in cs:
    IMP.domino.load_particle_states(ass,v,pst)
    ss=m.evaluate(False)
    score.append(ss)

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
