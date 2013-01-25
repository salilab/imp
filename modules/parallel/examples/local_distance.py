## \example parallel/local_distance.py
## This is a simple demonstration of the parallel functionality. It uses the
## module to make a simple plot of score versus distance for a harmonic
## distance restraint, dividing the work between several slaves.
##

import IMP.parallel
import tasks

# Set up a Manager to keep track of slaves and our tasks
m = IMP.parallel.Manager()

# Add slaves (two processors on the local machine)
s = IMP.parallel.LocalSlave()
m.add_slave(s)
s = IMP.parallel.LocalSlave()
m.add_slave(s)

# Generate a context (an environment on each slave in which tasks will be
# run). Provide a setup function for this context. Note that setup functions
# and task functions are sent across the network to the slaves using Python's
# 'pickle' module, which requires them to be defined in a separate module
# (tasks.py in this case).
c = m.get_context(tasks.setup)

# Add 10 tasks with different input parameters
for dist in range(0, 10):
    c.add_task(tasks.Task(dist))

# Run all 10 tasks, distributed between the two slaves. Get the results in
# the order they are returned (not necessarily the order they were created).
# This produces a simple table of score versus distance for a harmonic distance
# restraint (see tasks.py for the actual calculations).
for x in c.get_results_unordered():
    print x
