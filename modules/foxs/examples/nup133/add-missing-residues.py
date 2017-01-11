# This is a MODELLER script to fill in missing residues for
# the 3KFO structure. It generates 5 candidate models; fit
# each one against the SAXS profile using FoXS and pick the
# one with the best fit.

# Comparative modeling by the automodel class
from modeller import *              # Load standard Modeller classes
from modeller.automodel import *    # Load the automodel class

log.verbose()    # request verbose output
env = environ()  # create a new MODELLER environment to build this model in

# script_dir is the directory this script is in
import sys
import os
script_dir = os.path.dirname(sys.argv[0])

# directories for input atom files
env.io.atom_files_directory = [script_dir, '../atom_files']

# Renumber output model to get correct residue numbers and chain IDs
# (Modeller default is to not assign a chain ID, and start residue numbers at 1)
class MyModel(automodel):
    def special_patches(self, aln):
        self.rename_segments(segment_ids='A', renumber_residues=944)

a = MyModel(env,
            alnfile  = os.path.join(script_dir,
                                    'alignment.ali'),   # alignment filename
            knowns   = '3KFO',              # codes of the templates
            sequence = '3KFO-fill')         # code of the target
a.starting_model= 1                 # index of the first model
a.ending_model  = 5                 # index of the last model
                                    # (determines how many models to calculate)
a.make()                            # do the actual comparative modeling
