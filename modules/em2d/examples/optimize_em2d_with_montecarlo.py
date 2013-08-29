## \example em2d/optimize_em2d_with_montecarlo.py
## Example of optimizing an EM2D restraint using Monte Carlo.
##

import IMP
import IMP.base
import IMP.core
import IMP.atom
import IMP.em2d
import IMP.algebra
import IMP.container
import random


# An Optimizer score to get the values of the statistics after a given set
# of evaluations
class WriteStatisticsOptimizerScore(IMP.OptimizerState):
    def __init__(self, restraints):
        IMP.OptimizerState.__init__(self)
        self.count = 0
        self.restraints = restraints
    def update(self):
        if (self.count!=10):
            self.count += 1
            return
        else:
            self.count=0
        o=self.get_optimizer()
        m=o.get_model()
        for r in self.restraints:
            print r.get_name(), r.get_last_score()
        #for i in range(0,m.get_number_of_restraints()):
        #    r=m.get_restraint(i)
        #    print "restraint",r.get_name(),"value",r.evaluate(False)


# Get model from PDB file
IMP.base.set_log_level(IMP.base.TERSE)
m = IMP.Model()
prot =  IMP.atom.read_pdb(IMP.em2d.get_example_path("1z5s.pdb"),m,IMP.atom.ATOMPDBSelector())
IMP.atom.add_radii(prot)

# get the chains
chains = IMP.atom.get_by_type(prot,IMP.atom.CHAIN_TYPE)
print "there are",len(chains),"chains in 1z5s.pdb"

# set the chains as rigid bodies
native_chain_centers = []
rigid_bodies= []
for c in chains:
    atoms=IMP.core.get_leaves(c)
    rbd=IMP.core.RigidBody.setup_particle(c,atoms)
    rigid_bodies.append(rbd)
    print "chain has",rbd.get_number_of_members(), \
                          "atoms","coordinates: ",rbd.get_coordinates()
    native_chain_centers.append(rbd.get_coordinates())

bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-25, -40,-60),
                         IMP.algebra.Vector3D( 25,  40, 60))
# rotate and translate the chains
for rbd in rigid_bodies:
    # rotation
    rotation= IMP.algebra.get_random_rotation_3d()
    transformation1=IMP.algebra.get_rotation_about_point(rbd.get_coordinates(),rotation)
    # translation
    transformation2=IMP.algebra.Transformation3D(IMP.algebra.get_random_vector_in(bb))
    # Apply
    final_transformation = IMP.algebra.compose(transformation1,transformation2)
    IMP.core.transform(rbd,final_transformation)
print "Writing transformed assembly"
IMP.atom.write_pdb (prot,"1z5s-transformed.pdb")

# set distance restraints measusring some distances between rigid bodies
# for the solution.
d01 = IMP.algebra.get_distance(native_chain_centers[0],native_chain_centers[1])
r01 = IMP.core.DistanceRestraint(IMP.core.Harmonic(d01,1),chains[0],chains[1])
r01.set_name("distance 0-1")
d12 = IMP.algebra.get_distance(native_chain_centers[1],native_chain_centers[2])
r12 = IMP.core.DistanceRestraint(IMP.core.Harmonic(d12,1),chains[1],chains[2])
r12.set_name("distance 1-2")
d23 = IMP.algebra.get_distance(native_chain_centers[2],native_chain_centers[3])
r23 = IMP.core.DistanceRestraint(IMP.core.Harmonic(d23,1),chains[2],chains[3])
r23.set_name("distance 2-3")
d30 = IMP.algebra.get_distance(native_chain_centers[3],native_chain_centers[0])
r30 = IMP.core.DistanceRestraint(IMP.core.Harmonic(d30,1),chains[3],chains[0])
r30.set_name("distance 3-0")
print "Distances in the solution: d01 =", \
    d01,"d12 =",d12,"d23 =",d23,"d30 =",d30

# set distance restraints
print "adding distance restraints "
for r in [r01,r12,r23,r30]:
    m.add_restraint(r)
print "model has ",m.get_number_of_restraints(),"restraints"

# set em2D restraint
srw = IMP.em2d.SpiderImageReaderWriter()
selection_file=IMP.em2d.get_example_path("all-1z5s-projections.sel")
images_to_read_names=[IMP.base.get_relative_path(selection_file, x) for x in
        IMP.em2d.read_selection_file(selection_file)]
em_images =IMP.em2d.read_images(images_to_read_names,srw)
print len(em_images),"images read"

em2d_restraint = IMP.em2d.Em2DRestraint()
apix=1.5 # sampling rate of the available EM images
# resolution at which you want to generate the projections of the model
# In principle you want "perfect" projections, so use the highest resolution
resolution=1
# Number of projections to use for the initial registration
#  (coarse registration) to estimate the registration parameters
n_projections=20
params = IMP.em2d.Em2DRestraintParameters(apix,resolution,n_projections)

# This method (recommended) uses preprocessing of the images and projections
# to speed-up the registration
params.coarse_registration_method = IMP.em2d.ALIGN2D_PREPROCESSING
# use true if you want to save the projections from the model that best
# match the Em images
params.save_match_images = False

score_function=IMP.em2d.EM2DScore()
em2d_restraint = IMP.em2d.Em2DRestraint()
em2d_restraint.setup(score_function, params)
em2d_restraint.set_images(em_images)
em2d_restraint.set_name("em2d restraint")
container = IMP.container.ListSingletonContainer(IMP.core.get_leaves(prot))
em2d_restraint.set_particles(container)
em2d_restraints_set=IMP.kernel.RestraintSet()

# The next two lines are commented, because the optimization of the example
# is expensive. To run the full example, uncomment them (It can take a few
# hours).
#em2d_restraints_set.add_restraint(em2d_restraint)
#em2d_restraints_set.set_weight(1000) # weight for the em2D restraint

print "adding em2d restraint "
m.add_restraint(em2d_restraints_set)
# Add all restraints to a model
print "model has ",m.get_number_of_restraints(),"restraints"


# MONTECARLO OPTIMIZATION
s=IMP.core.MonteCarlo(m)
# Add movers for the rigid bodies
movers=[]
for rbd in rigid_bodies:
    movers.append(IMP.core.RigidBodyMover(rbd,5,2))
s.add_movers(movers)
print "MonteCarlo sampler has",s.get_number_of_movers(),"movers"
# Add an optimizer state to save intermediate configurations of the hierarchy
o_state=IMP.atom.WritePDBOptimizerState(chains,"intermediate-step-%1%.pdb")
o_state.set_skip_steps(10)
s.add_optimizer_state(o_state)

ostate2 = WriteStatisticsOptimizerScore(m.get_restraints())
s.add_optimizer_state(ostate2)

# Perform optimization
temperatures=[200,100,60,40,20,5]
# 200 steps recommended for accurate optimization; a smaller number is used
# here for demonstration purposes
optimization_steps = 10
for T in temperatures:
    s.set_kt(T)
    s.optimize(optimization_steps)
IMP.atom.write_pdb(prot,"solution.pdb")


# Check that the optimization achieves distances close to the solution
print "*** End optimization ***"
new_centers = []
for rbd in rigid_bodies:
    print "chain has",rbd.get_number_of_members(), \
        "atoms","coordinates: ",rbd.get_coordinates()
    new_centers.append(rbd.get_coordinates())

d01 = IMP.algebra.get_distance(new_centers[0],new_centers[1])
d12 = IMP.algebra.get_distance(new_centers[1],new_centers[2])
d23 = IMP.algebra.get_distance(new_centers[2],new_centers[3])
d30 = IMP.algebra.get_distance(new_centers[3],new_centers[0])
print "Distances at the end of the optimization: d01 =", \
    d01,"d12 =",d12,"d23 =",d23,"d30 =",d30
