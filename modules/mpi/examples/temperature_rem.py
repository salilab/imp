import IMP
import IMP.base
import IMP.mpi
import IMP.core
import sys

IMP.base.setup_from_argv(sys.argv, "Temperature MPI example")

# min and max temperature
TEMPMIN_ = 1.0
TEMPMAX_ = 5.0

# initialize Replica Exchange class
rem = IMP.mpi.ReplicaExchange()
# get number of replicas
nproc = rem.get_number_of_replicas()
# create array of temperatures, in geometric progression
temp = rem.create_temperatures(TEMPMIN_, TEMPMAX_, nproc)
# get replica index
myindex = rem.get_my_index()
# set initial value of the parameter (temperature) to exchange
rem.set_my_parameter("temp", [temp[myindex]])

# create model
m = IMP.kernel.Model()

# add 2 particles
ps = []
for i in range(2):
    p = IMP.kernel.Particle(m)
    d = IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(0.0, 0.0, 0.0))
    d.set_coordinates_are_optimized(True)
    ps.append(p)

# add harmonic restraint to distance
h = IMP.core.Harmonic(5.0, 1.0)
ds = IMP.core.DistanceRestraint(h, ps[0], ps[1])
m.add_restraint(ds)

# movers
movers = []
for p in ps:
    movers.append(IMP.core.BallMover([p], 0.5))
# serial mover
sm = IMP.core.SerialMover(movers)

# sampler
mc = IMP.core.MonteCarlo(m)
mc.set_kt(temp[myindex])
mc.set_return_best(False)
mc.add_mover(sm)

# prepare output
log = open("log" + str(myindex), "w")

# start sampling loop
for istep in range(0, 100):
    # do optimization
    score = mc.optimize(100)

    # get my replica index and temperature
    myindex = rem.get_my_index()
    mytemp = rem.get_my_parameter("temp")[0]
    # score divided by kbt
    myscore = score / mytemp

    # printout
    log.write("%4d %2d %6.3f\n" % (istep, myindex, score))

    # get my friend index and temperature
    findex = rem.get_friend_index(istep)
    ftemp = rem.get_friend_parameter("temp", findex)[0]
    # score divided by kbt
    fscore = score / ftemp

    # try exchange
    flag = rem.do_exchange(myscore, fscore, findex)
    # if accepted, change temperature
    if (flag):
        mc.set_kt(ftemp)
