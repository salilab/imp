#  ============================================================================
#
#  Simulated annealing molecular dynamics optimization with cross-correlation,
#  non-bonded interaction, and stereochemical restraints.
#
#  =======================  Maya Topf, 4 Dec 2007 =============================

from modeller import *
from modeller.automodel import refine
from modeller.scripts import complete_pdb
from modeller.optimizers import conjugate_gradients,molecular_dynamics,actions
from modeller import schedule
from rigid import load_rigid,read_selection_list
from random import *
import shutil
import sys, os, os.path
import string


class opt_md:

    # All the possible options of the procedure
    equil_its_for_heating = None
    equil_its_for_cooling = None
    equil_temps_for_heating = None
    equil_temps_for_cooling = None
    cycles = None
    path=None
    code=None
    run_num=None
    rand=None
    em_map_file=None
    input_pdb_file=None
    format=None
    apix=None
    box_size=None
    res=None
    x=None
    y=None
    z=None
    rigid_filename=None
    # The returned structure can be 'FINAL' or 'MINIMAL'
    # Only applied to cooling
    md_return='FINAL'

    def __init__(self):
        # THIS DEFAULT VALUES ARE OVERRIDEN IF THE RUN FUNCTION IS CALLED
        # Number of times to run the heating/cooling cycle
        self.cycles = 4
        # Parameters for heating the system (default values)
        self.equil_its_for_heating= 200 # Number of iterations for heating step
        self.equil_temps_for_heating = (150.0, 250.0, 500.0, 1000.0)
        # Parameters for cooling the system (default values)
        self.equil_its_for_cooling = 400 # Number of iteratios for cooling step
        self.equil_temps_for_cooling = (800.0, 500.0, 250.0, 150.0, 50.0, 0.0)

    def run(self):
        int_seed=randint(0,2**8)
        env = environ(rand_seed=int_seed)
        env.libs.topology.read(file='$(LIB)/top_heav.lib')
        env.libs.parameters.read(file='$(LIB)/par.lib')

        ###################### reading the density ######################
        den = density(env, file=self.path + '/' + self.em_map_file,
                 em_density_format=self.format, em_map_size=self.box_size,
                 density_type='GAUSS', voxel_size=self.apix,
                 resolution=self.res,px=self.x,py=self.y,pz=self.z)

        env.edat.density = den
        env.edat.dynamic_sphere = True

        ###################### read pdb file ######################
        aln = alignment(env)
        mdl2 = model(env, file=self.input_pdb_file)
        aln.append_model(mdl2, align_codes=self.input_pdb_file,
                   atom_files=self.input_pdb_file)
        mdl = model(env, file=self.input_pdb_file)
        aln.append_model(mdl, align_codes=self.code, atom_files=self.code)
        aln.align(gap_penalties_1d=(-600, -400))
        mdl.clear_topology()
        mdl.generate_topology(aln[self.input_pdb_file])
        mdl.transfer_xyz(aln)
        mdl.build(initialize_xyz=False, build_method='INTERNAL_COORDINATES')

        ####################### Remove chain names ######################
        for c in mdl.chains:
            c.name = ' '
        mdl.write(file=self.code+'_ini.pdb')

        ###################### Select the mobile atoms ######################
        sel_all = selection(mdl)
        sel_fixed=[]
        if(self.fixed_filename!=''):
            # read the list of fixed residues
            read_selection_list(self.path,mdl,sel_fixed,self.fixed_filename)
        # select the non fixed, those ones that are going to be optimized
        mobile_atoms = []
        for n in sel_all:
            mobile =True
            for m in sel_fixed:
                if n in m:
                    mobile=False
                    break
            if mobile:
                mobile_atoms.append(n)
#        print "mobile atom ..",n
        print "number of mobile atoms ..",len(mobile_atoms)
        sel_mobile=selection(mobile_atoms) # selection of mobile residues

        ###################### RESTRAINTS FOR MOBILE ATOMS ############
        # STEREOCHEMICAL restraints (based on the model)
        mdl.restraints.make(sel_mobile, restraint_type='STEREO',
                                            spline_on_site=False)
        # MAINCHAIN homology restraints forcing to look in the library
        # (dih_lib_only = True) and ignore the template.
        mdl.restraints.make(sel_mobile, aln=aln,
               restraint_type='PHI-PSI_BINORMAL',spline_on_site=True,
                dih_lib_only=True)
        mdl.restraints.make(sel_mobile, aln=aln,
               restraint_type='OMEGA_DIHEDRAL',spline_on_site=True,
                dih_lib_only=True)
        # SIDECHAIN homology restraints forcing to look in the library
        # (dih_lib_only = True) and ignore the template.
        mdl.restraints.make(sel_mobile, aln=aln,
               restraint_type='CHI1_DIHEDRAL',spline_on_site=True,
                dih_lib_only=True)
        mdl.restraints.make(sel_mobile, aln=aln,
               restraint_type='CHI2_DIHEDRAL',spline_on_site=True,
                dih_lib_only=True)



        ###################### define rigid bodies ######################
        sel_rigid=[]
        rand_rigid=[]
        if(self.rigid_filename != ''):
            load_rigid(self.path,mdl,sel_rigid,rand_rigid,self.rigid_filename)
        for n in sel_rigid:
            include_it=True
            # Only add not fixed rigid bodies. Adding fixed ones is superfluous
            for m in sel_fixed:
                if(n in m):
                    include_it=False
                    break
            if(include_it):
                print "Considering rigid body ...",n
#        for at in n:
#          print "atom ...",at
                mdl.restraints.rigid_bodies.append(rigid_body(n))
        # Remove duplicated restraints
        mdl.restraints.unpick_redundant()
        # Write all the restraints to a file
        mdl.restraints.write(file=self.code+'.rsr')

        ###################### MOLECULAR DYNAMICS ANNEALING ################
        print "MD annealing"
        scal_for_annealing = physical.values(default=1.0, em_density=10000)
        cap=0.39
        timestep=5.0
        icount=0
        trc_file = open('MD'+self.run_num+'.trc', "a")
        ###################### loop for number of cycles
        a,b = 1,self.cycles
        while  a <= b:
            equil_its=self.equil_its_for_heating
            equil_equil=20
            equil_temps= self.equil_temps_for_heating
            trc_step=5
            init_vel = True
            # during heating simulations, FINAL structure is always returned.
            # If the MINIMAL would be returned, no heating would be achieved.
            MD =  molecular_dynamics(cap_atom_shift=cap, md_time_step=timestep,
                               md_return='FINAL', output='REPORT',
                               schedule_scale=scal_for_annealing)

            ###################### heating the system
            for (its, equil, temps) in [(equil_its, equil_equil, equil_temps)]:
                for temp in temps:
                    # optimize the mobile atoms
                    MD.optimize(sel_mobile, max_iterations=its,
                      temperature=temp, init_velocities=init_vel,
                      equilibrate=equil,
                      actions=[actions.trace(trc_step,trc_file)])
                    # print progress
                    scal = physical.values(default=0.0, em_density=1.0)
                    (molpdf, terms) = sel_all.energy(schedule_scale=scal)
                    print "HEATING: iteration number= %s  step= %d %d  "\
                    "temp= %d  EM score= %.3f" %(a,icount,its,int(temp),-molpdf)

                    icount=icount+equil_its
                init_vel=False

            ###################### cooling the system
            equil_its=self.equil_its_for_cooling
            equil_temps= self.equil_temps_for_cooling
            # during the cooling MD simulations the structure returned can be
            # FINAL or MINIMAL acording to the option selected
            MD =  molecular_dynamics(cap_atom_shift=cap, md_time_step=timestep,
                             md_return=self.md_return, output='REPORT',
                             schedule_scale=scal_for_annealing)

            for (its, equil, temps) in [(equil_its, equil_equil, equil_temps)]:
                for temp in temps:

                    MD.optimize(sel_mobile, max_iterations=its,
                      temperature=temp, init_velocities=init_vel,
                      equilibrate=equil,
                      actions=[actions.trace(trc_step,trc_file)])

                    scal = physical.values(default=0.0, em_density=1.0)
                    (molpdf, terms) = sel_all.energy(schedule_scale=scal)
                    print "COOLING: iteration number= %s  step= %d %d " \
                    "temp= %d  EM score= %.3f" %(a,icount,its,int(temp),-molpdf)

                    icount=icount+equil_its

            filename='md'+self.run_num+'_'+str(a)+'.pdb'
            sel_all.write(file=filename)
            a+=1

        trc_file.close()

        print "MD FINAL: step= %d: energy all (with scaling 1:10000)" % icount
        eval = sel_all.energy(schedule_scale=scal_for_annealing)

        ################## final minimization with and without CC restraints.
        ################## Optimize all atoms for refinement
        CG = conjugate_gradients()
        print " final conjugate_gradients"

        CG.optimize(sel_all, output='REPORT', max_iterations=200,
                schedule_scale=scal_for_annealing)
        eval = sel_all.energy(schedule_scale=scal_for_annealing)

        scal = physical.values(default=1.0, em_density=0.0)
        CG.optimize(sel_all, output='REPORT', max_iterations=200,
                schedule_scale=scal)
        eval = sel_all.energy(schedule_scale=scal)

        sel_all.write(file='final'+self.run_num+'_mdcg.pdb')

        os.system("rm -f *.MRC")
