import IMP
import IMP.restrainer

IMP.base.set_log_level(IMP.base.VERBOSE)

# Create restrainer object
restrainer = IMP.restrainer.Main()

# Add representation, restraint, optimization and display to restrainer
rep = restrainer.add_representation(
    IMP.restrainer.get_example_path(
        'input/nup84_representation.xml'))
rsr = restrainer.add_restraint(
    IMP.restrainer.get_example_path(
        'input/nup84_restraint.xml'))

# BEGIN ADDED PART
import IMP.rmf
import RMF

rmf = RMF.create_rmf_file('nup84_simulation.rmf')
rmf.set_description("Simulate nup84.\n")

model = restrainer.get_model()
root_hierarchy = rep.get_root_imp_hierarchy()

IMP.rmf.add_hierarchy(rmf, root_hierarchy)
IMP.rmf.add_restraints(rmf, model.get_restraints())
IMP.rmf.save_frame(rmf, 0)

os = IMP.rmf.SaveOptimizerState(rmf)
os.update_always("initial conformation")
restrainer.log = os
# END ADDED PART

opt = restrainer.add_optimization(
    IMP.restrainer.get_example_path(
        'input/nup84_optimization.xml'))
disp = restrainer.add_display(
    IMP.restrainer.get_example_path(
        'input/nup84_display.xml'),
    'some_log_name')

# =======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
# =======================================================================###

# Save the initial state in Chimera format
# restrainer.log.write('initial.py')

# Perform optimization
restrainer.optimize()

# Save the optimized state in Chimera format
# restrainer.log.write('optimized.py')
