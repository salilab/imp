#  ============================================================================
#
#  Conjugate gradients minimization with cross-correlation, non-bonded
#  interaction and stereochemical restraints.
#
#  =======================  Maya Topf, 4 Dec 2007 =============================

from modeller import *
from modeller.automodel import refine
from modeller.scripts import complete_pdb
from modeller.optimizers import conjugate_gradients,actions
from modeller import schedule
from rigid import load_rigid,read_selection_list
from random import *
import shutil
import sys, os, os.path
import string

class opt_cg:

    # All the possible options of the procedure
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


#############
# NEW SCHEDULE
    CG = conjugate_gradients()
    sched = schedule.schedule(2,
     [
      schedule.step(CG, 2, physical.values(default=0.01, em_density=0.1)),
      schedule.step(CG, 5, physical.values(default=0.1 , em_density=1)),
      schedule.step(CG, 10, physical.values(default=0.2, em_density=10)),
      schedule.step(CG, 50,
                physical.values(default=0.5, soft_sphere=0.1, em_density=50)),
      schedule.step(CG, 9999,
                physical.values(default=0.5, soft_sphere=1.0, em_density=100)),
      schedule.step(CG, 9999,
                physical.values(default=1.0, soft_sphere=1.0, em_density=1000))
     ])
#############


    def __init__(self):
        pass

    def run(self):
        int_seed=randint(0,2**8)
        env = environ(rand_seed=int_seed)
        env.libs.topology.read(file='$(LIB)/top_heav.lib')
        env.libs.parameters.read(file='$(LIB)/par.lib')
        log.verbose()

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

        ###################### RESTRAINTS FOR MOBILE ATOMS ###############
        # STEREOCHEMICAL restraints (based on the model).
        mdl.restraints.make(sel_mobile,
                      restraint_type='STEREO', spline_on_site=False)

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

        ###################### MODEL RANDOMIZATION ######################
        # Select the flexible atoms
        # flexible: "those that are not fixed neither belong to to a rigid body"
        # Only the flexible atoms are randomized
        flexible_atoms = []
        for n in sel_mobile:
            flexible = True
            for m in sel_rigid:
                if n in m:
                    flexible = False
                    break
            if(flexible):
#        print "flexible atom ...",n
                flexible_atoms.append(n)
        print "number of flexible atoms ...",len(flexible_atoms)
        sel_flex=selection(flexible_atoms)
        sel_flex.randomize_xyz(deviation=1.0)
        # randomize the rigid bodies if requested
        seed()
        max_trans = 10
        max_ang = 30
        for num in sel_rigid:
            if rand_rigid[sel_rigid.index(num)]==1:
                rand_rot_x = uniform(-1,1)
                rand_rot_y = uniform(-1,1)
                rand_rot_z = uniform(-1,1)
                rand_ang = uniform(0,max_ang)
                rand_trans_x = uniform(-max_trans,max_trans)
                rand_trans_y = uniform(-max_trans,max_trans)
                rand_trans_z = uniform(-max_trans,max_trans)
                num.rotate_mass_center([rand_rot_x,rand_rot_y,rand_rot_z],
                                                                  rand_ang)
                num.translate([rand_trans_x,rand_trans_y,rand_trans_z])
        # Write final randomized model
        mdl.write(file=self.code+'_rand.pdb')

        ###################### CG MINIMIZATION ######################
        print "Conjugate_gradients"
        print "directory num %s" % self.run_num
        trc_step=5
        trc_file = open('CG'+self.run_num+'.trc', "a")
        i=0
        # trim the schedule
        self.sched.make_for_model(mdl)
        for step in self.sched:
            i = i+1
            step.optimize(sel_all, output='REPORT', max_iterations=200,
                      actions=actions.trace(trc_step,trc_file))
            # Write PDB file each optimization step
            pdb_file='cg'+self.run_num+'_'+str(i)+'.pdb'
            sel_all.write(file=pdb_file)
        trc_file.close()

        ###################### PRINT FINAL ENERGY AND CCC ######################
        print "final energy all "
        scal = physical.values(default=1.0, em_density=10000)
        eval = sel_all.energy(schedule_scale=scal)
        print "final cc "
        scal = physical.values(default=0.0, em_density=1.0)
        eval = sel_all.energy(schedule_scale=scal)
        # Write final model
        sel_all.write(file='final'+self.run_num+'_cg.pdb')
        os.system("rm -f *.MRC")
