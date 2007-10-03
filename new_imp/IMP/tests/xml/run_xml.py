import modeller
import modeller.optimizers

# set the appropriate search path
import sys

sys.path.append("../python_libs/")
import IMP_Modeller_Intf
import imp2
import load_imp_xml_model

model = imp2.Model()

# load document from an XML file
doc = load_imp_xml_model.load_imp_model(model, "test.xml")

# intialize Modeller
modeller.log.level(0,0,0,0,0)
env = modeller.environ()
env.io.atom_files_directory = '../data/'
env.edat.dynamic_sphere = False
env.libs.topology.read(file='$(LIB)/top_heav.lib')
env.libs.parameters.read(file='$(LIB)/par.lib')


# add IMP model and restraints as an energy term to Modeller model
t = env.edat.energy_terms
t.append(IMP_Modeller_Intf.IMP_Restraints(model, model.particles))

# get particles for Modeller
mdl = IMP_Modeller_Intf.Create_Particles_From_IMP(env, model)

atmsel = modeller.selection(mdl)
atoms = mdl.atoms
atmsel.randomize_xyz(deviation=100.0)

model.set_up_trajectory("trajectory.txt")
opt = modeller.optimizers.conjugate_gradients()
new_mdl = opt.optimize (atmsel, max_iterations=55, actions=None)
print atmsel.energy()

IMP_Modeller_Intf.Show_Distances(model.particles)
mdl.write(file='out_test_xml.pdb', model_format='PDB')
