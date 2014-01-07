import IMP
import IMP.core
import IMP.isd
m=IMP.Model()
r=IMP.isd.ConstantRestraint(m)
m.add_restraint(r)
print "=== calling m.evaluate"
m.evaluate(False)
print "=== calling scoring function"
sf = IMP.core.RestraintsScoringFunction([r])
sf.evaluate(False)
