#!/usr/bin/env python
import sys,os

import IMP
import IMP.atom
import IMP.container
import IMP.isd

from math import sqrt

kB= (1.381 * 6.02214) / 4184.0

class sfo():
    "shared functions object, published on all nodes"

    def hello(self):
        return "hello world"

    def init_model(self, wd, initpdb, restraints, ff_temp=300.0):
        "loads pdb and restraints and creates particles and nuisances"
        #IMP.set_log_level(IMP.SILENT)
        IMP.set_check_level(IMP.NONE)
        os.chdir(wd)
        print "loading protein and force field"
        # Create an IMP model and add a heavy atom-only protein from a PDB file
        m = IMP.Model()
        #prot = IMP.atom.read_pdb("./1G6J_MODEL1_imp.pdb", m)
        prot = IMP.atom.read_pdb(initpdb, m)
        # Read in the CHARMM heavy atom topology and parameter files
        ff = IMP.atom.CHARMMParameters("top.lib", "par.lib")
        # equivalent:
        # ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
        #
        # Using the CHARMM libraries, determine the ideal topology (atoms and their
        # connectivity) for the PDB file's primary sequence
        topology = ff.create_topology(prot)
        # Typically this modifies the C and N termini of each chain in the protein by
        # applying the CHARMM CTER and NTER patches. Patches can also be manually
        # applied at this point, e.g. to add disulfide bridges.
        topology.apply_default_patches()
        # Each atom is mapped to its CHARMM type. These are needed to look up bond
        # lengths, Lennard-Jones radii etc. in the CHARMM parameter file. Atom types
        # can also be manually assigned at this point using the CHARMMAtom decorator.
        topology.add_atom_types(prot)
        # Generate and return lists of bonds, angles, dihedrals and impropers for
        # the protein. Each is a Particle in the model which defines the 2, 3 or 4
        # atoms that are bonded, and adds parameters such as ideal bond length
        # and force constant. Note that bonds and impropers are explicitly listed
        # in the CHARMM topology file, while angles and dihedrals are generated
        # automatically from an existing set of bonds. These particles only define the
        # bonds, but do not score them or exclude them from the nonbonded list.
        bonds = topology.add_bonds(prot)
        angles = ff.create_angles(bonds)
        dihedrals = ff.create_dihedrals(bonds)
        impropers = topology.add_impropers(prot)
        # Maintain stereochemistry by scoring bonds, angles, dihedrals and impropers
        #
        # Score all of the bonds. This is done by combining IMP 'building blocks':
        # - A ListSingletonContainer simply manages a list of the bond particles.
        # - A BondSingletonScore, when given a bond particle, scores the bond by
        #   calculating the distance between the two atoms it bonds, subtracting the
        #   ideal value, and weighting the result by the bond's "stiffness", such that
        #   an "ideal" bond scores zero, and bonds away from equilibrium score non-zero.
        #   It then hands off to a UnaryFunction to actually penalize the value. In
        #   this case, a Harmonic UnaryFunction is used with a mean of zero, so that
        #   bond lengths are harmonically restrained.
        # - A SingletonsRestraint simply goes through each of the bonds in the
        #   container and scores each one in turn.
        cont = IMP.container.ListSingletonContainer(bonds, "bonds")
        bss = IMP.atom.BondSingletonScore(IMP.core.Harmonic(0, 1))
        m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))
        # Score angles, dihedrals, and impropers. In the CHARMM forcefield, angles and
        # impropers are harmonically restrained, so this is the same as for bonds.
        # Dihedrals are scored internally by a periodic (cosine) function.
        cont = IMP.container.ListSingletonContainer(angles, "angles")
        bss = IMP.atom.AngleSingletonScore(IMP.core.Harmonic(0,1))
        m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))
        #dihedrals
        cont = IMP.container.ListSingletonContainer(dihedrals, "dihedrals")
        bss = IMP.atom.DihedralSingletonScore()
        m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))
        #impropers
        cont = IMP.container.ListSingletonContainer(impropers, "impropers")
        bss = IMP.atom.ImproperSingletonScore(IMP.core.Harmonic(0,1))
        m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))
        # Add non-bonded interaction (in this case, Lennard-Jones). This needs to
        # know the radii and well depths for each atom, so add them from the forcefield
        # (they can also be assigned manually using the XYZR or LennardJones
        # decorators):
        ff.add_radii(prot)
        ff.add_well_depths(prot)
        # Get a list of all atoms in the protein, and put it in a container
        atoms = IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
        cont = IMP.container.ListSingletonContainer(atoms)
        # Add a restraint for the Lennard-Jones interaction. Again, this is built from
        # a collection of building blocks. First, a ClosePairContainer maintains a list
        # of all pairs of Particles that are close. A StereochemistryPairFilter is used
        # to exclude atoms from this list that are bonded to each other or are involved
        # in an angle or dihedral (1-3 or 1-4 interaction). Then, a
        # LennardJonesPairScore scores a pair of atoms with the Lennard-Jones potential.
        # Finally, a PairsRestraint is used which simply applies the
        # LennardJonesPairScore to each pair in the ClosePairContainer.
        #non bonded list exclusion
        nbl = IMP.container.ClosePairContainer(cont, 4.0)
        pair_filter = IMP.atom.StereochemistryPairFilter()
        pair_filter.set_bonds(bonds)
        pair_filter.set_angles(angles)
        pair_filter.set_dihedrals(dihedrals)
        nbl.add_pair_filter(pair_filter)
        #setup pair score
        ps = IMP.isd.RepulsiveDistancePairScore(0,1)
        #should weight the ff restraint by a temperature, set to 300K.
        pr = IMP.container.PairsRestraint(ps, nbl)
        rs = IMP.RestraintSet('phys')
        rs.add_restraint(pr)
        rs.set_weight(1.0/(kB*ff_temp))
        m.add_restraint(rs)
        sigma=IMP.isd.Nuisance.setup_particle(IMP.Particle(m),10)
        gamma=IMP.isd.Nuisance.setup_particle(IMP.Particle(m),10)
        rs = IMP.RestraintSet('NOE')
        print "reading data restraints"
        for i,line in enumerate(open(restraints)):
            if i % 100 == 0:
               print i
            tokens=line.split()
            r1=int(tokens[2])
            at1=tokens[5]
            r2=int(tokens[8])
            at2=tokens[11]
            dist=float(tokens[13])
            #find corresponding atoms
            try:
                p0=IMP.core.XYZ(IMP.atom.Selection(hierarchy=prot,
                    residue_index=r1,
                    atom_type=IMP.atom.AtomType(at1)
                    ).get_selected_particles()[0])
            except:
                print "atom %d %s not found" % (r1,at1)
                continue
            try:
                p1=IMP.core.XYZ(IMP.atom.Selection(hierarchy=prot,
                    residue_index=r2,
                    atom_type=IMP.atom.AtomType(at2)
                    ).get_selected_particles()[0])
            except:
                print "atom %d %s not found" % (r2,at2)
                continue
            #create lognormal restraint using gamma_data = 1
            ln=IMP.isd.NOERestraint(p0,p1,sigma,gamma,1*dist**-6)
            rs.add_restraint(ln)
        print i, "restraints read"
        #set weight of rs and add to model. 
        #Weight is 1.0 cause sigma particle already has this role.
        rs.set_weight(1.0)
        m.add_restraint(rs)
        self._m = m
        self._p={}
        self._p['sigma'] = sigma
        self._p['gamma'] = gamma
        self._p['prot'] = prot

    def m(self,name,*args,**kw):
        "wrapper to call methods of m"
        func=getattr(self._m,name)
        return func(*args, **kw)

    def init_simulation(self,lambda_temp=1.0, tau=500.0):
        "prepare md and mc sims"
        self.inv_temp = lambda_temp
        self.md_tau = tau
        self._md = self._setup_md(1/(kB*lambda_temp), tau)
        self._mc_sigma, self._nm_sigma = self._setup_mc(self._p['sigma'],
                lambda_temp)
        self._mc_gamma, self._nm_gamma = self._setup_mc(self._p['gamma'],
                lambda_temp)

    def do_md(self,nsteps):
        "perform md simulation on protein for nsteps"
        for i in IMP.atom.get_leaves(self._p['prot']):
            IMP.core.XYZR(i).set_coordinates_are_optimized(True)
        self._p['sigma'].set_is_optimized(IMP.FloatKey("nuisance"),False)
        self._p['gamma'].set_is_optimized(IMP.FloatKey("nuisance"),False)
        self._md.optimize(nsteps)

    def do_mc_and_update_stepsize(self,nsteps):
        """perform mc on nuisances for nsteps, updating stepsizes to target 
        50% acceptance. Don't make nsteps too small (say << 50).
        """
        prot = self._p['prot']
        sigma = self._p['sigma']
        gamma = self._p['gamma']
        for i in IMP.atom.get_leaves(prot):
            IMP.core.XYZR(i).set_coordinates_are_optimized(False)
        sigma.set_is_optimized(IMP.FloatKey("nuisance"),True)
        gamma.set_is_optimized(IMP.FloatKey("nuisance"),False)
        self._mc_and_update(nsteps,self._mc_sigma,self._nm_sigma)
        sigma.set_is_optimized(IMP.FloatKey("nuisance"),False)
        gamma.set_is_optimized(IMP.FloatKey("nuisance"),True)
        self._mc_and_update(nsteps,self._mc_gamma,self._nm_gamma)

    def write_pdb(self, name='prot.pdb'):
        IMP.atom.write_pdb(self._p['prot'], name)

    def init_stats(self, prefix='p01'):
        self.prefix=prefix
        self.statfile = prefix+'_stats.txt'
        flstat=open(self.statfile,'w')
        flstat.write("Step Time Temp Potential Kinetic "
                "Total Sigma Gamma MC_accept_s MC_accept_g "
                "MC_stepsize_s MC_stepsize_g\n")
        flstat.close()
        self.naccept_s=0
        self.naccept_g=0

    def write_stats(self,stepno,nsteps):
        flstat=open(self.statfile,'a')
        kinetic   = self._md.get_kinetic_energy() 
        potential = self._m.evaluate(False)
        temp = self._md.get_kinetic_temperature(kinetic)
        si=self._p['sigma'].get_nuisance()
        ga=self._p['gamma'].get_nuisance()
        acc_s,st_s = self._get_mc_stat(self._mc_sigma, self._nm_sigma,
                self.naccept_s,nsteps)
        acc_g,st_g = self._get_mc_stat(self._mc_gamma, self._nm_gamma,
                self.naccept_g,nsteps)
        self.naccept_s = self._mc_sigma.get_number_of_forward_steps()
        self.naccept_g = self._mc_gamma.get_number_of_forward_steps()
        for i in [stepno, stepno*100*2.0/1000.0, temp, potential, kinetic,
                kinetic+potential,si,ga,acc_s,acc_g,st_s,st_g]:
            flstat.write("%10f " % i)
        flstat.write('\n')
        flstat.close()
        IMP.atom.write_pdb(self._p['prot'],self.prefix+"_%05d.pdb" % stepno)

    def set_temp(self, inv_temp):
        "sets temperature of mc and md sims (used in replica exchange)"
        self._md.set_therm(2, 1.0/(kB*inv_temp), self.md_tau)
        self._md.rescale_vel(sqrt(self.inv_temp/inv_temp))
        self.inv_temp = inv_temp
        self._mc.set_temperature(1/self.inv_temp)
    
    def get_temp(self):
        return self.inv_temp


    def _setup_md(self,temp=300.0, tau=500):
        ## Molecular Dynamics (from MAX BONOMI)
        md=IMP.atom.MolecularDynamics()
        md.set_model(self._m)
        md.assign_velocities(temp)
        md.set_time_step(1.0)
        ## therm legend
        # 0 :: nve
        # 1 :: rescale velocities
        # 2 :: berendsen
        # 3 :: langevin
        #md.set_therm(0,0,0)
        #md.set_therm(1,300,0)
        md.set_therm(2,temp,tau)
        #md.set_therm(3,300,0.01)
        # metadynamics setup
        #md.mtd_setup(0.003, 10.0, -200.0, 400.0)
        return md

    def _setup_mc(self,particle,beta=1.676972322):
        "monte carlo on nuisance parameter"
        mc = IMP.core.MonteCarlo(self._m)
        cont=IMP.container.ListSingletonContainer(self._m)
        cont.add_particle(particle)
        nm_particle=IMP.core.NormalMover(cont,
                IMP.FloatKeys([IMP.FloatKey("nuisance")]),0.1)
        cont=IMP.container.ListSingletonContainer(self._m)
        #why is this returning an int?
        mc.add_mover(nm_particle)
        #set same temp as MD, careful with units
        mc.set_temperature(1.0/beta)
        #allow to go uphill
        mc.set_return_best(False)
        #update particle each time
        mc.set_move_probability(1.0)
        return (mc,nm_particle)

    def _mc_and_update(self,nsteps,mc,nm):
        before = mc.get_number_of_forward_steps()
        mc.optimize(nsteps)
        after = mc.get_number_of_forward_steps()
        accept = float(after-before)/nsteps
        if 0.4 < accept < 0.6:
            return
        if accept < 0.05:
            accept = 0.05
        if accept > 1.0:
            accept = 1.0
        nm.set_sigma(nm.get_sigma()*2*accept)

        "helper class to gather and print statistics on a simulation"

    def _get_mc_stat(self,mc,nm,nacc,nsteps):
        "return acceptance rate and stepsize"
        stepsize = nm.get_sigma()
        n_ok=mc.get_number_of_forward_steps() - nacc
        return 100*n_ok/nsteps, stepsize

