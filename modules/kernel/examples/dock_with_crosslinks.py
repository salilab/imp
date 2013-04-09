## \example kernel/dock_with_crosslinks.py
## This example docks several proteins using excluded volume and crosslinking terms. To set the pdbs and crosslinks to use, edit the data at the start of the python script.
##

import IMP
import IMP.core
import IMP.algebra
import IMP.base
import IMP.atom
import IMP.container
import random
import sys
try:
    import networkx as nx
    import networkx.algorithms
except:
    print "Script requires networkx to run"
    sys.exit()

IMP.base.add_bool_flag("test", "Run a minimal test on the script")
IMP.base.setup_from_argv(sys.argv, "Dock several proteins using excluded volume and crosslinking")

# remove internal checks
IMP.base.set_check_level(IMP.base.USAGE)
pdbs=[IMP.get_example_path('dock_data/chainf.pdb'),
      IMP.get_example_path('dock_data/chaind.pdb')]
xlinks=[
    (256, "D", 89, "F", 17, 0.1),
    (259, "D", 84, "F", 17, 0.1),
    (259, "D", 89, "F", 17, 0.1),
    (269, "D", 89, "F", 17, 0.1),
    (128, "F", 275, "D", 17, 0.1),
    (130, "F", 275, "D", 17, 0.1),
    (144, "F", 269, "D", 17, 0.1),
    (150, "F", 256, "D", 17, 0.1),
    (150, "F", 259, "D", 17, 0.1),
    (150, "F", 269, "D", 17, 0.1),
    (150, "F", 179, "D", 17, 0.1),
    (166, "F", 186, "D", 17, 0.1),
    (170, "D", 89, "F", 17, 0.1),
    (179, "D", 84, "F", 17, 0.1)]


def read_pdbs(list_pdb_file):
    """read pdbs from an external list file
    create a simplified representation"""
    chains=[]
    chain_id={}
    for pdb in pdbs:
        prot = IMP.atom.read_pdb(pdb, m,
                     IMP.atom.CAlphaPDBSelector())
        ch=IMP.atom.get_by_type(prot, IMP.atom.CHAIN_TYPE)
        chains+=ch
        print ch

    for c in chains:
        chain_id[c]=IMP.atom.Chain(c).get_id()
    return chains,chain_id

def setup_radii(chains):
    "setup the radii for each residue"
    for c in chains:
        IMP.atom.create_rigid_body(c)
        for res in c.get_children():
            dres=IMP.core.XYZR(res)
            rt=IMP.atom.Residue(res).get_residue_type()
            vol=IMP.atom.get_volume_from_residue_type(rt)
            rg=IMP.algebra.get_ball_radius_from_volume_3d(vol)
            dres.set_radius(rg)


def add_excluded_volume(chains):
    """add excluded volume score on the coarse grained c-alpha model
    residues are represented by beads with conveniently defined excluded volume radius"""
    rs = IMP.RestraintSet('excluded_volume')
    kappa_=0.1
    lsc= IMP.container.ListSingletonContainer(m)
    IMP.atom.get_by_type
    for c in chains:
        lsc.add_particles(IMP.atom.get_leaves(c))
    evr=IMP.core.ExcludedVolumeRestraint(lsc,kappa_)
    rs.add_restraint(evr)
    m.add_restraint(rs)
    restraints['excluded_volume']=rs


def setup_move_them_all_MonteCarlo_internal(chains,mc_dx=0.3,mc_dang=0.1,mc_kt=1.0):
    """setup rigid body monte carlo move set
    mc_dx is the traslation move
    mc_dang is the rotation angle"""
    mc=IMP.core.MonteCarlo(m)

    mc.set_return_best(False)
    mc.set_kt(10)
    for c in chains:
        p=IMP.atom.get_leaves(c)[0]
        rb=IMP.core.RigidMember(p).get_rigid_body()
        mv= IMP.core.RigidBodyMover(rb, mc_dx, mc_dang)
        mc.add_mover(mv)
    return mc

def setup_move_them_all_MonteCarlo_external(connected_chain_list,rb_ext_list=[],
                                            mc_dx=0.3,mc_dang=0.1,mc_kt=1.0,return_best=False):
    """setup rigid body monte carlo move set
    mc_dx is the traslation move
    mc_dang is the rotation angle"""

    print 'RIGID EXT BODY LIST', [r.get_name() for r in rb_ext_list]
    for rb in rb_ext_list:
        IMP.core.RigidBody.teardown_particle(rb)

    rb_ext_list=[]

    mc=IMP.core.MonteCarlo(m)
    mc.set_return_best(return_best)
    mc.set_kt(mc_kt)

    print connected_chain_list
    for c in connected_chain_list:
        rb_tmp_list=[]
        for s in c:
            p=IMP.atom.get_leaves(s)[0]
            rba=IMP.core.RigidMember(p).get_rigid_body()
            rb_tmp_list.append(rba)

        print 'RIGID BODY LIST', [r.get_name( ) for r in rb_tmp_list]
        p=IMP.Particle(m)
        p.set_name("root rigid body "+str(chain_id[s]))
        rb=IMP.core.RigidBody.setup_particle(p,rb_tmp_list)
        rb_ext_list.append(rb)
        mv= IMP.core.RigidBodyMover(rb, mc_dx, mc_dang)
        mc.add_mover(mv)
    return mc,rb_ext_list

def setup_move_them_all_MonteCarlo_internal_2(chains,rb_ext_list=[],
                                              mc_dx=0.3,mc_dang=0.1,mc_kt=1.0):
    """setup rigid body monte carlo move set
    mc_dx is the traslation move
    mc_dang is the rotation angle"""

    print 'RIGID EXT BODY LIST', [r.get_name() for r in rb_ext_list]
    for rb in rb_ext_list:
        IMP.core.RigidBody.teardown_particle(rb)

    rb_ext_list=[]

    mc=IMP.core.MonteCarlo(m)

    mc.set_return_best(False)
    mc.set_kt(10)
    for c in chains:
        p=IMP.atom.get_leaves(c)[0]
        rb=IMP.core.RigidMember(p).get_rigid_body()
        rb_ext_list.append(rb)
        mv= IMP.core.RigidBodyMover(rb, mc_dx, mc_dang)
        mc.add_mover(mv)
    return mc,rb_ext_list

def shuffle_configuration(chains,bounding_box_length=200.0):
    "shuffle configuration, used to restart the optimization"
    hbbl=bounding_box_length/2
    for c in chains:
        p=IMP.atom.get_leaves(c)[0]
        rb=IMP.core.RigidMember(p).get_rigid_body()
        ub = IMP.algebra.Vector3D(-hbbl,-hbbl,-hbbl)
        lb = IMP.algebra.Vector3D( hbbl, hbbl, hbbl)
        bb = IMP.algebra.BoundingBox3D(ub, lb)
        translation = IMP.algebra.get_random_vector_in(bb)
        rotation = IMP.algebra.get_random_rotation_3d()
        transformation = IMP.algebra.Transformation3D(rotation, translation)
        rb.set_reference_frame(IMP.algebra.ReferenceFrame3D(transformation))


def add_restraints():
    """Handle the restraints defined above
    sintax: resid1 chain1 resid2 chain2 max_distance strength_of_potential
    example:     226 H 272 N 15.0 10.0"""
    rs=IMP.RestraintSet('cross_links')
    restraints={}
    restraints_map={}
    for tokens in xlinks:
        r1=tokens[0]
        c1=tokens[1]
        r2=tokens[2]
        c2=tokens[3]
        distance=tokens[4]
        strength=tokens[5]
        try:
            s1=IMP.atom.Selection(chains, residue_index=r1, chains=c1)
            p1=s1.get_selected_particles()[0]
        except:
            print "residue %d of chain %s not found" % (r1,c1)
            continue
        try:
            s2=IMP.atom.Selection(chains, residue_index=r2, chains=c2)
            p2=s2.get_selected_particles()[0]
        except:
            print "residue %d of chain %s not found" % (r2,c2)
            continue
        d=distance
        k=strength
        hub= IMP.core.HarmonicUpperBound(d,k)
        df= IMP.core.DistancePairScore(hub)
        dr= IMP.core.PairRestraint(df, (p1, p2))
        rs.add_restraint(dr)
        a='cross_link-'+`r1`+'.'+c1+'-'+`r2`+'.'+c2+'.lower'
        restraints_map
        restraints[a]=dr
        restraints_map[a]=(c1,c2)


    m.add_restraint(rs)
    restraints['cross_links']=rs
    return restraints,restraints_map

def setup_md(temp=300.0, tau=0.01):
    md=IMP.atom.MolecularDynamics()
    md.set_model(m)
    md.assign_velocities(temp)
    md.set_time_step(1.0)
    st = IMP.atom.LangevinThermostatOptimizerState(md.get_simulation_particles(), temp,tau)
    md.add_optimizer_state(st)
    return md

def init_pdb(prefix='models'):
    "append models to a pdb file"
    pdbfile = prefix+'.pdb'
    flpdb=open(pdbfile,'w')
    flpdb.close()

def write_pdb(prefix='models'):
    "append zero score models (solutions) to a pdb file"
    pdbfile = prefix+'.pdb'
    flpdb=open(pdbfile,'a')
    tmpfile=".tmp.pdb"
    IMP.atom.write_pdb(chains,tmpfile)
    fltmp=open(tmpfile,'r')
    pdb=fltmp.read()
    flpdb.write(pdb)
    flpdb.close()
    fltmp.close()

def chain_pair_list(chains,nop_cutoff=0):
    pair_list=[]
    for c in chains:
        sc1= IMP.container.ListSingletonContainer(m)
        sc1.add_particles(IMP.atom.get_leaves(c))
        for s in chains:
            if chain_id[s]>chain_id[c]:
                sc2= IMP.container.ListSingletonContainer(m)
                sc2.add_particles(IMP.atom.get_leaves(s))
                aaa= IMP.container.CloseBipartitePairContainer(sc1,sc2,0.0)
                m.update()
                nop=aaa.get_number_of_particle_pairs()
                if nop > nop_cutoff :
                    pair_list.append((c,s))
                else:
                    pair_list.append((c,c))
                    pair_list.append((s,s))
    return pair_list


def chain_pair_list_based_on_restraint(chains,nop_cutoff=0):
    pair_list=[]
    id_chain=invert_dict(chain_id)
    for i in restraints.keys():
        (chaina_id,chainb_id)=restraints_map[i]
        chaina=invert_dict(chaina_id)
        chainb=invert_dict(chainb_id)


    for c in chains:
        sc1= IMP.container.ListSingletonContainer(m)
        sc1.add_particles(IMP.atom.get_leaves(c))
        for s in chains:
            if chain_id[s]>chain_id[c]:
                sc2= IMP.container.ListSingletonContainer(m)
                sc2.add_particles(IMP.atom.get_leaves(s))
                aaa= IMP.container.CloseBipartitePairContainer(sc1,sc2,0.0)
                m.update()
                nop=aaa.get_number_of_particle_pairs()
                if nop > nop_cutoff :
                    pair_list.append((c,s))
                else:
                    pair_list.append((c,c))
                    pair_list.append((s,s))
    return pair_list


def create_graph(chains,nop_cutoff=0):
    connected=[]
    while len(connected)<=1:
        G=nx.Graph()
        pair_list=chain_pair_list(chains,nop_cutoff)
        G.add_edges_from(pair_list)
        connected=nx.algorithms.connected_components(G)
        G.clear()
        nop_cutoff+=10
    return connected,nop_cutoff

def invert_dict(d):
    inv = {}
    for k, v in d.iteritems():
        keys = inv.setdefault(v, [])
        keys.append(k)
    return inv


m = IMP.Model()
chain_id={}
rb_ext_list=[]
restraints={}
restraints_map={}



init_pdb("models")
init_pdb("solutions")
init_pdb("best")
chains,chain_id=read_pdbs("pdb.list")
write_pdb("best")
setup_radii(chains)
add_excluded_volume(chains)
restraints,restraints_map=add_restraints()
shuffle_configuration(chains,300.0)



delta_r=5.0
delta_a=0.2
delta_r_best=0.5
delta_a_best=0.1
temp=1.0


mc_int=setup_move_them_all_MonteCarlo_internal(chains,delta_r,delta_a,temp)
write_pdb("models")



nsteps_int=100
nsteps_ext=100

cg=IMP.core.ConjugateGradients()
cg.set_model(m)

number_of_steps = 10000

if IMP.base.get_bool_flag("test"):
    number_of_steps = 10

for steps in range(number_of_steps):

    nop_cutoff=10000000
    connected_chains_list,nop_cutoff=create_graph(chains,nop_cutoff)
    mc_int,rb_ext_list = setup_move_them_all_MonteCarlo_external(connected_chains_list,rb_ext_list,delta_r,delta_a,temp,return_best=False)
    mc_int.optimize(nsteps_ext)
    print ' INTERNAL', m.evaluate(False)
    write_pdb("models")


    nop_cutoff=0
    connected_chains_list,nop_cutoff=create_graph(chains,nop_cutoff)
    mc_ext,rb_ext_list = setup_move_them_all_MonteCarlo_external(connected_chains_list,rb_ext_list,delta_r,delta_a,temp,return_best=False)
    mc_ext.optimize(nsteps_ext)
    print ' EXTERNAL', m.evaluate(False)
    write_pdb("models")

    nfs=mc_ext.get_number_of_forward_steps()
    acceptance=float(nfs)/nsteps_ext

    if acceptance<0.05:
        delta_r=delta_r/2
    if acceptance>0.3:
        delta_r=delta_r*2
    print 'ACCEPTANCE 1', acceptance, delta_r, delta_a


    nop_cutoff=10000000
    connected_chains_list,nop_cutoff=create_graph(chains,nop_cutoff)
    mc_int,rb_ext_list = setup_move_them_all_MonteCarlo_external(connected_chains_list,rb_ext_list,delta_r_best,delta_a_best,temp,return_best=True)
    mc_int.optimize(nsteps_ext)
    print ' INTERNAL', m.evaluate(False)
    write_pdb("models")


    nop_cutoff=0
    connected_chains_list,nop_cutoff=create_graph(chains,nop_cutoff)
    mc_ext,rb_ext_list = setup_move_them_all_MonteCarlo_external(connected_chains_list,rb_ext_list,delta_r_best,delta_a_best,temp,return_best=True)
    mc_ext.optimize(nsteps_ext)
    print ' EXTERNAL', m.evaluate(False)
    write_pdb("models")

    nfs=mc_ext.get_number_of_forward_steps()
    acceptance=float(nfs)/nsteps_ext

    if acceptance<0.5:
        delta_r_best=delta_r_best/2

    if acceptance>0.8:
        delta_r_best=delta_r_best*2
    print 'ACCEPTANCE 2', acceptance, delta_r_best, delta_a_best

    cg.optimize(10)

    score=m.evaluate(False)
    for i in restraints.keys():
        print i, restraints[i].evaluate(False)


    if score==0.0:
        write_pdb("solutions")
        shuffle_configuration(chains,300.0)
        exit()




print 'ALL DONE'
