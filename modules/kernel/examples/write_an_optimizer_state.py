## \example kernel/write_an_optimizer_state.py
## While we do not recomment doing serious work using optimizer states written it python, it is often useful when prototyping or testing code. Copy this example and modify as needed.

import IMP

# an optimizer state which prints out model statistics.
class MyOptimizerState(IMP.OptimizerState):
    def __init__(self):
        IMP.OptimizerState.__init__(self)
    def update(self):
        self.get_optimizer().get_model().show_restraint_score_statistics()

# some code to create and evaluate it
k= IMP.FloatKey("a key")
m= IMP.Model()
# we don't have any real restraints in the kernel
r0=IMP.kernel._ConstRestraint(1)
r0.set_name("restraint 0")
m.add_restraint(r0)
r1=IMP.kernel._ConstRestraint(2)
r1.set_name("restraint 1")
m.add_restraint(r1)

os= MyOptimizerState()
os.set_name("python optimizer state")
# we don't have any optimizers either
co= IMP.kernel._ConstOptimizer(m)
co.add_optimizer_state(os)
m.set_gather_statistics(True)
# so we only see the statistics
IMP.base.set_log_level(IMP.base.SILENT)
print co.optimize(100)

# this is needed to clean up memory properly for some reason
co.remove_optimizer_state(os)
del os
del m
