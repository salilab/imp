import sys
from run_optimization import *


############### INPUT PARAMETERS used in both CG and MD ##################
class general_parameters:
    # rigid bodies (they move as a whole body).
    # If '' all the protein is flexible
    rigid_filename = 'rigid-sses.txt'
    # Parts of the protein kept fixed (the don't move). If ''
    # all the protein is mobile
    fixed_filename = ''
    os.system("mkdir ./flex-EM-1iok")
    path = '.' # directory path
    code = 'grol'               # 4 letter code of the structure
    input_pdb_file = 'P0A6F5.B99990006.pdb' # input model for optimization
    em_map_file = 'groel_subunit_8.mrc'   # name of EM density map (mrc)
    format='MRC'              # map format: MRC or XPLOR
    apix=2.7                # voxel size: A/pixel
    box_size=28             # size of the density map (cubic)
    res=11.5                   # resolution
    x= 0 ; y= -18.9  ; z= -2.7  # origin of the map


############### SPECIFIC PARAMETERS FOR MD (not used if CG) ##################
class md_parameters:
    md_return='FINAL'
    equil_its_for_heating = 100 # iterations for the heating  step
    equil_its_for_cooling = 200 # Iterations for the cooling step
    equil_temps_for_heating = (500,1000,1500,1800)
    equil_temps_for_cooling = (1200.0,1000.0,600.0,200.0,100.0,50.0,25.)
    cycles = 10 # number of cycles to repeat heating-cooling

############### RUN OPTIMIZATION ##################
optimization = sys.argv[1]          # type of optimization:CG / MD
num_of_runs = 10             # number of runs (models)
initial_dir = 1
run_optimization(optimization,num_of_runs,initial_dir,
                          general_parameters,md_parameters)

# After finishing the fitting, you can optionally run the refinement script
# modeller_refinement to optimize geometrical distornions introduced by the
# MD for all atoms.
