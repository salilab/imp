import modeller
import modeller.optimizers
import IMP
import IMP.modeller_intf
import IMP.utils
import IMP.xml_loader

model = IMP.Model()

# load document from an XML file
doc = IMP.xml_loader.load_imp_model(model, "gold_model.xml")

# intialize Modeller
modeller.log.level(0,0,0,0,0)
env = modeller.environ()
env.io.atom_files_directory = "../data/"
env.edat.dynamic_sphere = False
env.libs.topology.read(file="$(LIB)/top_heav.lib")
env.libs.parameters.read(file="$(LIB)/par.lib")


# add IMP model and restraints as an energy term to Modeller model
t = env.edat.energy_terms
t.append(IMP.modeller_intf.IMPRestraints(model, model.particles))
        
# get particles for Modeller
mdl = IMP.modeller_intf.create_particles_from_imp(env, model)

atmsel = modeller.selection(mdl)
atoms = mdl.atoms
# atmsel.randomize_xyz(deviation=100.0)

# comment out if you do not want to save the trajectory
model.set_up_trajectory("trajectory.txt");

# opt = modeller.optimizers.conjugate_gradients()
# new_mdl = opt.optimize (atmsel, max_iterations=55, actions=None)
print atmsel.energy()

IMP.utils.show_distances(model.particles)
mdl.write(file="out.pdb", model_format="PDB")
