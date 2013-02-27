#!/usr/bin/env python

from modeller import *
from modeller.automodel import *
from modeller.automodel import refine
from modeller.optimizers import conjugate_gradients,molecular_dynamics,actions
from modeller import schedule

############## COMMAND TO RUN THIS SCRIPT ##########
class md_parameters:
    md_return='MINIMAL'
    equilibrate = 50
    temperature = 3 # temperature for MD
    input_file = ''
    max_iterations = 50000
    skip_its_for_writing=25000
    input_pdb_file = 'final3_mdcg.pdb'
    output_pdb_file = 'final3_mdcg_refined-50000its.pdb'
params = md_parameters()
# log.verbose()
log.minimal()
env = environ()
env.libs.topology.read(file='$(LIB)/top_heav.lib')
env.libs.parameters.read(file='$(LIB)/par.lib')

###################### read pdb file ######################
aln = alignment(env)
mdl2 = model(env, file=params.input_pdb_file)
aln.append_model(mdl2, align_codes=params.input_pdb_file,
           atom_files=params.input_pdb_file)
mdl = model(env, file=params.input_pdb_file)
aln.append_model(mdl,align_codes=params.input_pdb_file,
           atom_files=params.input_pdb_file)
aln.align(gap_penalties_1d=(-600, -400))
mdl.clear_topology()
mdl.generate_topology(aln[params.input_pdb_file])
mdl.transfer_xyz(aln)
mdl.build(initialize_xyz=False, build_method='INTERNAL_COORDINATES')

# restraints
all_atoms = selection(mdl)
mdl.restraints.make(all_atoms,restraint_type='STEREO',
                  spline_on_site=False,dih_lib_only=True)
mdl.restraints.make(all_atoms, aln=aln,restraint_type='PHI-PSI_BINORMAL',
                  spline_on_site=True, dih_lib_only=True)
mdl.restraints.make(all_atoms, aln=aln,
       restraint_type='OMEGA_DIHEDRAL',spline_on_site=True,
        dih_lib_only=True)
mdl.restraints.make(all_atoms, aln=aln,
       restraint_type='CHI1_DIHEDRAL',spline_on_site=True,
        dih_lib_only=True)
mdl.restraints.make(all_atoms, aln=aln,
       restraint_type='CHI2_DIHEDRAL',spline_on_site=True,
        dih_lib_only=True)
mdl.restraints.make(all_atoms, aln=aln,
       restraint_type='CHI3_DIHEDRAL',spline_on_site=True,
        dih_lib_only=True)
mdl.restraints.make(all_atoms, aln=aln,
       restraint_type='CHI4_DIHEDRAL',spline_on_site=True,
        dih_lib_only=True)
mdl.restraints.make(all_atoms, aln=aln,
       restraint_type='PHI_DIHEDRAL',spline_on_site=True,
        dih_lib_only=True)
mdl.restraints.make(all_atoms, aln=aln,
       restraint_type='PSI_DIHEDRAL',spline_on_site=True,
        dih_lib_only=True)
mdl.restraints.condense()

# energy data
env.edat.dynamic_lennard = True


# Prepare optimizer
scaling_factors = physical.values(default=1.0, em_density=0)
MD =  molecular_dynamics(output='REPORT',cap_atom_shift=0.0001,
              md_time_step=0.05,temperature=params.temperature,
                        init_velocities=True,md_return=params.md_return,
                        equilibrate = params.equilibrate,
                       schedule_scale=scaling_factors)
w=actions.write_structure(params.skip_its_for_writing, 'md-%03d.pdb',
            write_all_atoms=True, first=True, last=True, start=0)
# optimize
MD.optimize(all_atoms, max_iterations=params.max_iterations,actions=[w])

for c in mdl.chains:
    c.name = 'A'
mdl.remark =""
mdl.reorder_atoms()
mdl.write(params.output_pdb_file, model_format='PDB')
