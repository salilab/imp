## \example kernel/write_an_optimizer_state.py
## While we do not recomment doing serious work using optimizer states written it python, it is often useful when prototyping or testing code. Copy this example and modify as needed.

import IMP

# an optimizer state which prints out the last scores of some restraints.
class MyOptimizerState(IMP.OptimizerState):
    def __init__(self, rs):
        IMP.OptimizerState.__init__(self, rs[0].get_model(),
                                    "MyOptimizerState%1%")
        self.rs = rs
    def update(self):
        for r in self.rs:
            print r.get_name(), r.get_last_score()

# some code to create and evaluate it
k= IMP.FloatKey("a key")
m= IMP.Model()
# we don't have any real restraints in the kernel
r0=IMP.kernel._ConstRestraint(m, [], 1)
r0.set_name("restraint 0")

r1=IMP.kernel._ConstRestraint(m, [], 2)
r1.set_name("restraint 1")

rs = IMP.RestraintSet([r0, r1], 1.0)
sf = rs.create_scoring_function()

os= MyOptimizerState([r0, r1])
os.set_name("python optimizer state")
# we don't have any optimizers either
co= IMP.kernel._ConstOptimizer(m)
co.set_scoring_function(sf)
co.add_optimizer_state(os)
print co.optimize(100)

# this is needed to clean up memory properly for some reason
co.remove_optimizer_state(os)
del os
del m
