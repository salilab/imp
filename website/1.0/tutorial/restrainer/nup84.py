import IMP
import IMP.restrainer
import IMP.core

IMP.base.set_log_level(IMP.base.VERBOSE)

rep = IMP.restrainer.XMLRepresentation('representation.xml').run()
rsr = IMP.restrainer.XMLRestraint('restraint.xml').run()
disp = IMP.restrainer.XMLDisplay('display.xml').run()

model = rep.to_model()

log = disp.create_log(rep, 'bead_log_%03d.py')
log.write('initial.py')

rsr.add_to_representation(rep)

opt = IMP.core.ConjugateGradients()
opt.set_model(model)
opt.add_optimizer_state(log)

opt.optimize(10)
log.write('optimized.py')
