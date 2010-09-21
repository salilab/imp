import IMP
import IMP.core as core
import IMP.atom as atom
import IMP.em2d as em2d
import IMP.em as em
import IMP.algebra as alg
import IMP.container
import random


# An Optimizer score to get the values of the statistics after a given set
# of evaluations
class WriteStatisticsOptimizerScore(IMP.OptimizerState):
    count =0
    def __init__(self):
        IMP.OptimizerState.__init__(self)
        count = 0
    def update(self):
        if (self.count!=10):
            self.count += 1
            return
        else:
            self.count=0
        o=self.get_optimizer()
        m=o.get_model()
        m.show_restraint_score_statistics()
        m.show_all_statistics()
        for i in range(0,m.get_number_of_restraints()):
            r=m.get_restraint(i)
            print "restraint",r.get_name(),"value",r.evaluate(False)
    def do_show(self, stream):
        print >> stream, ps


# Get model from PDB file
IMP.set_log_level(IMP.TERSE)
m = IMP.Model()
ssel = atom.ATOMPDBSelector()
prot =  atom.read_pdb(em2d.get_example_path("1z5s.pdb"),m,ssel)
atom.add_radii(prot)

# get the chains
chains = atom.get_by_type(prot,atom.CHAIN_TYPE)
print "there are",len(chains),"chains in 1z5s.pdb"

# set the chains as rigid bodies
chains_centers = []
rigid_bodies= []
for c in chains:
    atoms=core.get_leaves(c)
    core.RigidBody.setup_particle(c,atoms)
    rbd= core.RigidBody(c)
    rigid_bodies.append(rbd)
    print "chain has",rbd.get_number_of_members(), \
                          "atoms","coordinates: ",rbd.get_coordinates()
    chains_centers.append(rbd.get_coordinates())

# rotate and translate the chains
for rbd in rigid_bodies:
    # rotation
    rotation= alg.get_random_rotation_3d()
    transformation1=alg.get_rotation_about_point(rbd.get_coordinates(),rotation)
    # translation
    x=random.uniform(-25,25)
    y=random.uniform(-40,40)
    z=random.uniform(-60,60)
    transformation2=alg.Transformation3D(alg.Vector3D(x,y,z))
    # Apply
    final_transformation = alg.compose(transformation1,transformation2)
    core.transform(rbd,final_transformation)
print "Writing transformed assembly"
atom.write_pdb (prot,"1z5s-transformed.pdb")

# set distance restraints measusring some distances between rigid bodies
# for the solution.
d01 = alg.get_distance(chains_centers[0],chains_centers[1])
r01 = core.DistanceRestraint(core.Harmonic(d01,1),chains[0],chains[1])
r01.set_name("distance 0-1")
d12 = alg.get_distance(chains_centers[1],chains_centers[2])
r12 = core.DistanceRestraint(core.Harmonic(d12,1),chains[1],chains[2])
r12.set_name("distance 1-2")
d23 = alg.get_distance(chains_centers[2],chains_centers[3])
r23 = core.DistanceRestraint(core.Harmonic(d23,1),chains[2],chains[3])
r23.set_name("distance 2-3")
d30 = alg.get_distance(chains_centers[3],chains_centers[0])
r30 = core.DistanceRestraint(core.Harmonic(d30,1),chains[3],chains[0])
r30.set_name("distance 3-0")
print "Distances in the solution: d01 =", \
    d01,"d12 =",d12,"d23 =",d23,"d30 =",d30

# set distance restraints
print "adding distance restraints "
distance_restraints = [r01,r12,r23,r30]
distance_restraints_set = IMP.RestraintSet()
for r in distance_restraints:
    distance_restraints_set.add_restraint(r)
distance_restraints_set.set_weight(1)
for i in range(0,distance_restraints_set.get_number_of_restraints()):
    m.add_restraint(distance_restraints_set.get_restraint(i))
print "model has ",m.get_number_of_restraints(),"restraints"

# set em2D restraint
srw = em.SpiderImageReaderWriter()
selection_file="all-1z5s-projections.sel"
images_to_read_names=em2d.read_selection_file(selection_file)
em_images =em.read_images(images_to_read_names,srw)
print len(em_images),"images read"

em2d_restraint = em2d.Em2DRestraint()
apix=1.5
resolution=1
n_projections=20
coarse_registration_method = 1
save_match_images = False
em2d_restraint.initialize(apix,resolution,n_projections,
                coarse_registration_method,save_match_images)
em2d_restraint.set_images(em_images)
em2d_restraint.set_name("em2d restraint")
container = IMP.container.ListSingletonContainer(core.get_leaves(prot))
em2d_restraint.set_particles(container)
em2d_restraints_set=IMP.RestraintSet()
em2d_restraints_set.add_restraint(em2d_restraint)
em2d_restraints_set.set_weight(1000) # weight for the em2D restraint

print "adding em2d restraint "
for i in range(0,em2d_restraints_set.get_number_of_restraints()):
    m.add_restraint(em2d_restraints_set.get_restraint(i))
# Add all restraints to a model
print "model has ",m.get_number_of_restraints(),"restraints"


# MONTECARLO OPTIMIZATION
s=core.MonteCarlo(m)
# Add movers for the rigid bodies
movers=[]
for rbd in rigid_bodies:
    movers.append(core.RigidBodyMover(rbd,5,2))
s.add_movers(movers)
print "MonteCarlo sampler has",s.get_number_of_movers(),"movers"
# Add an optimizer state to save intermediate configurations of the hierarchy
o_state=atom.WritePDBOptimizerState(chains,"intermediate-step-%1%.pdb")
o_state.set_skip_steps(10)
s.add_optimizer_state(o_state)

ostate2 = WriteStatisticsOptimizerScore()
s.add_optimizer_state(ostate2)

# Perform optimization
m.set_gather_statistics(True)
temperatures=[200,100,60,40,20,5]
optimization_steps = 2000
for temp in temperatures:
    s.optimize(optimization_steps)
atom.write_pdb(prot,"solution.pdb")


# Check that the optimization achieves distances close to th solution
print "*** End optimization ***"
new_centers = []
for rbd in rigid_bodies:
    print "chain has",rbd.get_number_of_members(), \
        "atoms","coordinates: ",rbd.get_coordinates()
    new_centers.append(rbd.get_coordinates())

d01 = alg.get_distance(new_centers[0],new_centers[1])
d12 = alg.get_distance(new_centers[1],new_centers[2])
d23 = alg.get_distance(new_centers[2],new_centers[3])
d30 = alg.get_distance(new_centers[3],new_centers[0])
print "Distances at the end of the optimization: d01 =", \
    d01,"d12 =",d12,"d23 =",d23,"d30 =",d30
