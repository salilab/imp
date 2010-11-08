import shutil
import sys, os, os.path
import string
import math
from modeller import *
from CG import opt_cg
from MD import opt_md

# This functions runs an optimization given the 2 classes of parameters
# optimization -- type of optimization: CG / MD
# num_of_runs --- number of runs
# initial_dir --- initial directory name
def run_optimization(optimization,num_of_runs,initial_dir,
                              general_parameters,md_parameters):
    #  CG
    # ----
    if optimization == 'CG':
        print '=============================================='
        print 'CONJUGATE GRADIENTS OPTIMIZATION. Parameteres'
        print '=============================================='
        for i in range(initial_dir,initial_dir+num_of_runs):
            scratch = general_parameters.path + '/' + str(i) + '/'
            os.system('mkdir -p ' + scratch)
            os.system('cp ' + general_parameters.path + '/'+
                      general_parameters.input_pdb_file + ' ' + scratch)
            os.chdir(scratch)
            opt=opt_cg()
            # Set paramters
            opt.run_num=str(i)
            opt.rand=55*i
            for j in dir(general_parameters):
                setattr(opt,j,getattr(general_parameters,j))
            # Run
            opt.run()

    #  MD
    # ----
    elif optimization == 'MD':
        print '=============================================='
        print 'MOLECULAR DYNAMICS OPTIMIZATION. Parameteres'
        print '=============================================='
        for i in range(initial_dir,initial_dir+num_of_runs):
            scratch = general_parameters.path + '/' + str(i) + '/'
            general_parameters.input_pdb_file = 'final' + str(i) + '_cg.pdb'
            os.chdir(scratch)
            opt=opt_md()
            # Set paramters
            opt.run_num=str(i)
            opt.rand=10*i
            for j in dir(general_parameters):
                if (j!='__doc__' and j!='__module__'):
                    setattr(opt,j,getattr(general_parameters,j))
                    print j,getattr(opt,j)
            for j in dir(md_parameters):
                if (j!='__doc__' and j!='__module__'):
                    setattr(opt,j,getattr(md_parameters,j))
                    print j,getattr(opt,j)
            # Run
            opt.run()
