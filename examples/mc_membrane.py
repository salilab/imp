import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import IMP.membrane
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
            if (i == 0):      begin=p1
            if (i == nres-1): end=p1
            r.add_child(a)
            all.add_child(r)
            atoms.append(ad)
        rb=IMP.Particle(m)
        rb=IMP.atom.create_rigid_body(atoms,"TM"+str(id))
        tbr.add_particle(rb,atoms)
        # adjust axis to match topology
        bb=IMP.core.RigidMember(begin).get_internal_coordinates()[0]
        ee=IMP.core.RigidMember(end).get_internal_coordinates()[0]
        d_rbs=IMP.membrane.HelixDecorator.setup_particle(rb,bb,ee)
        if ( sign * ( ee - bb ) > 0 ): rot0= -math.pi/2.0
        else :                         rot0=  math.pi/2.0
        return rot0

    rot0=[]
    for i in range(len(seq)):
        rot=generate_balls(i,seq[i],tmh[i],topo[i])
        rot0.append(rot)
    return m,all,tbr,rot0

def create_restraints(m, chain, tbr, TMH, rot0, topo):

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
        ps=  IMP.membrane.RigidBodyPackingScore(tbr, om_b, om_e, dd_b, dd_e)
        prs= IMP.container.PairsRestraint(ps, nrb)
        m.add_restraint(prs)
        m.set_maximum_score(prs, .001)

## DOPE/GQ scoring
    def add_DOPE():
        IMP.atom.add_dope_score_data(chain)
        dsc=IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_indexes=[(h[0],h[1]+1)])
            dsc.add_particles(s.get_selected_particles())
        dpc=IMP.container.ClosePairContainer(dsc, 15.0, 0.0)
        f=IMP.membrane.SameHelixPairFilter()
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
        hub= IMP.core.HarmonicUpperBound(8.0,1000.0)
        sd=  IMP.core.DistancePairScore(hub)
        kc=  IMP.core.KClosePairsPairScore(sd,tbr,1)
        ir=  IMP.core.PairRestraint(kc,[rb0,rb1])
        m.add_restraint(ir)
        m.set_maximum_score(ir,.01)
        return ir

    def add_diameter_restraint(diameter):
        lrb= IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
            rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
            lrb.add_particle(rb)
        hub=IMP.core.HarmonicUpperBound(0,1000)
        dr=IMP.core.DiameterRestraint(hub, lrb, diameter)
        m.add_restraint(dr)
        m.set_maximum_score(dr,.01)

    def add_depth_restraint(range):
        lrb= IMP.container.ListSingletonContainer(m)
        for h in TMH:
            s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
            rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
            lrb.add_particle(rb)
        well=IMP.core.HarmonicWell(range, 1000.0)
        ass=IMP.core.AttributeSingletonScore(well,IMP.FloatKey("z"))
        sr=IMP.container.SingletonsRestraint(ass, lrb)
        m.add_restraint(sr)
        m.set_maximum_score(sr,.01)

    def add_tilt_restraint(range0,rot0):
        laxis=(1.0,0.0,0.0)
        zaxis=(0.0,0.0,1.0)
        for i,h in enumerate(TMH):
            s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
            rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
            if ( rot0[i] > 0 ): range=[math.pi-range0[1],math.pi-range0[0]]
            else              : range=[range0[0],range0[1]]
            well=IMP.core.HarmonicWell(range, 1000.0)
            tss=IMP.membrane.TiltSingletonScore(well, laxis, zaxis)
            sr=IMP.core.SingletonRestraint(tss, rb)
            m.add_restraint(sr)
            m.set_maximum_score(sr,.01)

# assembling all the restraints
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
    add_packing_restraint()
    add_DOPE()
    add_diameter_restraint(35.0)
    add_depth_restraint([-5.0,5.0])
    add_tilt_restraint([0,math.radians(40)],rot0)
    ir0=add_interacting_restraint(TMH[1],TMH[2])
    #ir1=add_interacting_restraint(TMH[0],TMH[1])

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
TMH= [[24,48], [75,94], [220,238]]#, [254,276]]

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
topo=[1.0,-1.0,1.0]#,-1.0]

print "creating representation"
(m,chain,tbr,rot0)=create_representation(seq,TMH,topo)

print "creating restraints"
create_restraints(m,chain,tbr,TMH,rot0,topo)

print "creating movers"
mc= IMP.core.MonteCarlo(m)
for i,h in enumerate(TMH):
    s0=IMP.atom.Selection(chain, atom_type = IMP.atom.AT_CA, residue_index = h[0])
    rb=IMP.core.RigidMember(s0.get_selected_particles()[0]).get_rigid_body()
    if ( i == 0 ): mv= IMP.membrane.RigidBodyNewMover(rb, 0.0, 0.0, 0.5, 0.05, 0.05, 0.05)
    if ( i == 1 ): mv= IMP.membrane.RigidBodyNewMover(rb, 0.5, 0.0, 0.5, 0.05, 0.05, 0.05)
    if ( i > 1 ):  mv= IMP.membrane.RigidBodyNewMover(rb, 0.5, 0.5, 0.5, 0.05, 0.05, 0.05)
    mc.add_mover(mv)

mc.set_kt(3.0)
mc.set_return_best(False)
#score=m.evaluate(False)
#display(m,chain,TMH,"initial.score_"+str(score)+".pym")

print "sampling"
for steps in range(1000):
    mc.optimize(1000)
    score=m.evaluate(False)
    display(m,chain,TMH,"conf_"+str(steps)+".score_"+str(score)+".pym")
