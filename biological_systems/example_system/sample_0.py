import IMP
import IMP.atom
import IMP.container
import IMP.display
import IMP.statistics
import IMP.example
import IMP.rmf
import IMP.system
import RMF
import parameters
import setup
import os.path

display_restraints=[]


import sys
print "hi"
print sys.argv

# find acceptable conformations of the model
def get_conformations(m, gs, n):
    sampler= IMP.core.MCCGSampler(m)

    sampler.set_bounding_box(parameters.bb)
    # magic numbers, experiment with them and make them large enough for things to work
    sampler.set_number_of_conjugate_gradient_steps(100)
    sampler.set_number_of_monte_carlo_steps(10)
    sampler.set_number_of_attempts(n)
    sampler.set_save_rejected_configurations(True)
    # We don't care to see the output from the sampler
    sampler.set_log_level(IMP.base.SILENT)

    # return the IMP.ConfigurationSet storing all the found configurations that
    # meet the various restraint maximum scores.
    cs= sampler.get_sample()
    # Look at the rejected minimal conformations to understand how the restraints
    # are failing
    print "rejected", \
          sampler.get_rejected_configurations().get_number_of_configurations(), "solutions"
    m.set_gather_statistics(True)
    for i in range(0, sampler.get_rejected_configurations().get_number_of_configurations()):
        sampler.get_rejected_configurations().load_configuration(i)
        m.evaluate(False)
        # show the score for each restraint to see which restraints were causing the
        # conformation to be rejected
        m.show_restraint_score_statistics()
        w= IMP.display.PymolWriter(IMP.system.get_output_path("rejected.%d.pym"%i))
        for g in gs:
            w.add_geometry(g)
    return cs

(i, n) =IMP.system.get_sample_parameters()


# now do the actual work
(m,all)= setup.create_representation()
IMP.atom.show_molecular_hierarchy(all)
setup.create_restraints(m, all)


gs= setup.create_geometry(all)
IMP.base.set_log_level(IMP.base.SILENT)
cs= get_conformations(m, gs, 2000.0/n)
IMP.base.set_log_level(IMP.base.TERSE)

print "found", cs.get_number_of_configurations(), "solutions"

out= RMF.create_rmf_file(IMP.system.get_output_path("configurations_"+str(i)+".rmf"))
IMP.rmf.add_hierarchy(out, all)
for i in range(0, cs.get_number_of_configurations()):
    cs.load_configuration(i)
    IMP.rmf.save_frame(out, i)
    w= IMP.display.PymolWriter(IMP.system.get_output_path("struct.%d.pym"%i))
    for g in gs:
        w.add_geometry(g)
