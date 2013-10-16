## \example kernel/write_a_restraint.py
# While we do not recommend doing serious work using restraints written in Python, it is often useful when prototyping or testing code. Copy this example and modify as needed.
#

import IMP

# a restraint which checks if particles are sorted in
# increasing order on k.


class MyRestraint(IMP.kernel.Restraint):
    # take the list of particles and the key to use

    def __init__(self, m, ps, k):
        IMP.kernel.Restraint.__init__(self, m, "MyRestraint %1%")
        self.ps = ps
        self.k = k

    def unprotected_evaluate(self, da):
        score = 0
        for i in range(1, len(self.ps)):
            p0 = self.ps[i - 1]
            p1 = self.ps[i]
            if p0.get_value(k) > p1.get_value(k):
                diff = (p0.get_value(k) - p1.get_value(k))
                score = score + diff
                p0.add_to_derivative(k, -1, da)
                p1.add_to_derivative(k, 1, da)
            else:
                if IMP.get_log_level() >= IMP.base.TERSE:
                    print p0.get_name(), "and", p1.get_name(), " are ok"
        return score

    def do_get_inputs(self):
        return self.ps

# some code to create and evaluate it
k = IMP.FloatKey("a key")
m = IMP.kernel.Model()
ps = []
for i in range(0, 10):
    p = IMP.kernel.Particle(m)
    p.add_attribute(k, i)
    ps.append(p)
r = MyRestraint(m, ps, k)
# IMP.base.set_log_level(IMP.base.TERSE)
print r.evaluate(True)
