#!/usr/bin/env python
import sys,os
from glob import glob

import IMP
import IMP.atom
import IMP.container
import IMP.saxs
import IMP.core
import IMP.isd
import IMP.isd.TBLReader
import IMP.isd.TALOSReader
import IMP.isd.utils
from IMP.isd.Entry import Entry

from math import sqrt,exp
from StringIO import StringIO

from random import random

kB= (1.381 * 6.02214) / 4184.0

class PyMDMover:
    def __init__(self, cont, md, n_md_steps):
        self.cont = cont
        self.md = md
        self.n_md_steps = n_md_steps
        self.velkeys=[IMP.FloatKey("vx"),
                    IMP.FloatKey("vy"), IMP.FloatKey("vz")]

    def store_move(self):
        self.oldcoords=[]
        #self.oldvel=[]
        for i in xrange(self.cont.get_number_of_particles()):
            p=self.cont.get_particle(i)
            #self.oldvel.append([p.get_value(k) for k in self.velkeys])
            d=IMP.core.XYZ(p)
            self.oldcoords.append(d.get_coordinates())

    def propose_move(self, prob):
        self.md.optimize(self.n_md_steps)

    def reset_move(self):
        for i in xrange(self.cont.get_number_of_particles()):
            p=self.cont.get_particle(i)
            #for j,k in enumerate(self.velkeys):
            #    p.set_value(k, self.oldvel[i][j])
            d=IMP.core.XYZ(p)
            d.set_coordinates(self.oldcoords[i])

    def get_number_of_steps(self):
        return self.n_md_steps

    def set_number_of_steps(self, nsteps):
        self.n_md_steps = nsteps

class PyMC(IMP.Optimizer):
    debug =True

    def __init__(self,model):
        self.m=model

    def add_mover(self,mv):
        self.mv = mv

    def set_kt(self,kT):
        self.kT=kT

    def set_return_best(self,thing):
        pass
    def set_move_probability(self,thing):
        pass

    def get_energy(self):
        pot=self.m.evaluate(False)
        #pot=self.get_restraints().evaluate(False)
        kin=self.mv.md.get_kinetic_energy()
        return pot+kin

    def metropolis(self, old, new):
        deltaE=new-old
        if self.debug:
            print ": old %f new %f deltaE %f new_epot: %f" % (old,new,deltaE,
                self.m.evaluate(False)),
        kT=self.kT
        if deltaE < 0:
            return True
        else:
            return exp(-deltaE/kT) > random()

    def optimize(self,nsteps):
        self.naccept = 0
        print "=== new MC call"
        #store initial coordinates
        self.mv.store_move()
        for i in xrange(nsteps):
            print "MC step %d " % i,
            #draw new velocities
            self.mv.md.assign_velocities(self.kT / kB)
            #get total energy
            old=self.get_energy()
            #make a MD move
            self.mv.propose_move(1)
            #get new total energy
            new=self.get_energy()
            if self.metropolis(old,new):
                #move was accepted: store new conformation
                self.mv.store_move()
                self.naccept += 1
                print "accepted "
            else:
                #move rejected: restore old conformation
                self.mv.reset_move()
                print " "

    def get_number_of_forward_steps(self):
        return self.naccept

class sfo_common:
    """nonspecific methods used across all shared function objects.
    Rules:
        - Their name starts with the name of the parent function (e.g.
                init_model_* )
        - they don't store anything in the class, but instead
                return all created objects.
                Exceptions: the model, which is self._m
                            the statistics class, which is self.stat
        - they store what they have to store in the model (e.g. restraints)
        - they don't print out anything except for long routines (e.g. NOE
          parsing)
        - the prior RestraintSet is added to the model when it is created, so
          that when it is passed to another function, it is not added twice.
    """

    def hello(self):
        return "hello world"

    def set_checklevel(self,value):
        IMP.base.set_check_level(value)

    def set_loglevel(self,value):
        IMP.base.set_log_level(value)

    def m(self,name,*args,**kw):
        "wrapper to call methods of m"
        func=getattr(self._m,name)
        return func(*args, **kw)

    def init_model_base(self, wd):
        "moves to wd and creates model"
        os.chdir(wd)
        # Create an IMP model
        self._m = IMP.Model()

    def init_model_charmm_protein_and_ff(self, initpdb, top, par, selector, pairscore,
            ff_temp=300.0, disulfides=None, representation='custom'):
        """creates a CHARMM protein representation.
        creates the charmm force field, bonded and nonbonded.
        - initpdb: initial structure in pdb format
        - top is a CHARMM top.lib, read if representation=='custom' (default)
        - par is a CHARMM par.lib
        - selector is an instance of
            one of the selectors of IMP.atom, for example
            IMP.atom.NonWaterNonHydrogenPDBSelector().
        - pairscore is an instance of a Pair Score to score the interaction
            between two atoms. usually, it's either
            LennardJonesDistancePairScore(0,1) or
            RepulsiveDistancePairScore(0,1)
        - ff_temp is the temperature at which the force field should be
          simulated.
        - disulfides: if not None, a list of tuples corresponding to residue
                      numbers that should be cross-linked. Residues should be
                      cysteines, and residue numbering should start at 0.
        - representation: 'full' : all-atom CHARMM force field
                          'heavy': heavy atom CHARMM forcefield with polar H
                          'calpha': only C alphas, ball and stick model with
                                    bondlength 3.78 angstrom, beads at VdW
                                    contact, and harmonic restraint between
                                    them.
                          'custom' (default) : read given CHARMM top and par
                                               files.
        Returns: prot, ff, rsb, rs
            - prot: the protein
            - ff: the force field
            - rsb: the RestraintSet on bonded interactions
            - rs: the RestraintSet on nonbonded interactions. Both are weighted
              using ff_temp.
        """
        m=self._m
        prot = IMP.atom.read_pdb(initpdb, m, selector)
        if not prot.get_is_valid(True):
            raise ValueError, "invalid hierarchy!"
        if representation == 'custom':
            # Read in the CHARMM heavy atom topology and parameter files
            ff = IMP.atom.CHARMMParameters(top,par)
        elif representation == 'heavy':
            ff= IMP.atom.get_heavy_atom_CHARMM_parameters()
        elif representation == 'full':
            ff= IMP.atom.get_all_atom_CHARMM_parameters()
        elif representation == 'calpha':
            pass
        else:
            raise NotImplementedError, representation
        if representation == 'calpha':
            print "setting up simplified C alpha force field"
            # set radii
            for ca in IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE):
                IMP.core.XYZR(ca.get_particle()).set_radius(1.89)
                #IMP.atom.Mass(ca.get_particle()).set_mass(1.)
            #create bonds by getting pairs and applying a pairscore
            pairs=[]
            for chain in prot.get_children():
                residues=[(chain.get_child(i),chain.get_child(i+1))
                            for i in xrange(chain.get_number_of_children()-1)]
                residues=[(i.get_child(0).get_particle(),
                           j.get_child(0).get_particle())
                            for (i,j) in residues]
                pairs.extend(residues)
            bonds=[]
            for (i,j) in pairs:
                #check, because it fails if you try twice
                if IMP.atom.Bonded.particle_is_instance(i):
                    bi = IMP.atom.Bonded(i)
                else:
                    bi = IMP.atom.Bonded.setup_particle(i)
                if IMP.atom.Bonded.particle_is_instance(j):
                    bi = IMP.atom.Bonded(i)
                else:
                    bj = IMP.atom.Bonded.setup_particle(j)
                bond=IMP.atom.create_custom_bond(bi,bj,3.78,10) #stiff
                bonds.append(bond)
            bonds_container = IMP.container.ListSingletonContainer(bonds)
            hdps = IMP.core.Harmonic(0,1)
            bs = IMP.atom.BondSingletonScore(hdps)
            br = IMP.container.SingletonsRestraint(bs, bonds_container)
            rsb = IMP.RestraintSet("bonded")
            rsb.add_restraint(br)
            rsb.set_weight(1.0/(kB*ff_temp))
            m.add_restraint(rsb)
            nonbonded_pair_filter = IMP.atom.StereochemistryPairFilter()
            nonbonded_pair_filter.set_bonds(bonds)
            ff=None
        else:
            print "setting up CHARMM forcefield"
            #
            # Using the CHARMM libraries, determine the ideal topology (atoms and their
            # connectivity) for the PDB file's primary sequence
            topology = ff.create_topology(prot)
            # Typically this modifies the C and N termini of each chain in the protein by
            # applying the CHARMM CTER and NTER patches. Patches can also be manually
            # applied at this point, e.g. to add disulfide bridges.
            topology.apply_default_patches()
            #disulfides
            if disulfides:
                s=topology.get_segment(0)
                dis=ff.get_patch('DISU')
                for (i,j) in disulfides:
                    self.find_atom((i, 'SG'), prot)
                    self.find_atom((j, 'SG'), prot)
                    r0=s.get_residue(i)
                    r1=s.get_residue(j)
                    if i==0:
                        r0.set_patched(False)
                    if j==0:
                        r1.set_patched(False)
                    dis.apply(r0,r1)
                    print "added disulfide bridge between cysteines %d and %d" % (i,j)
            # Make the PDB file conform with the topology; i.e. if it contains extra
            # atoms that are not in the CHARMM topology file, remove them; if it is
            # missing atoms (e.g. sidechains, hydrogens) that are in the CHARMM topology,
            # add them and construct their Cartesian coordinates from internal coordinate
            # information.
            topology.setup_hierarchy(prot)
            # Set up and evaluate the stereochemical part (bonds, angles, dihedrals,
            # impropers) of the CHARMM forcefield
            r = IMP.atom.CHARMMStereochemistryRestraint(prot, topology)
            rsb = IMP.RestraintSet("bonded")
            rsb.add_restraint(r)
            m.add_restraint(rsb)
            #
            # Add non-bonded interaction (in this case, Lennard-Jones). This needs to
            # know the radii and well depths for each atom, so add them from the forcefield
            # (they can also be assigned manually using the XYZR or LennardJones
            # decorators):
            ff.add_radii(prot)
            ff.add_well_depths(prot)
            nonbonded_pair_filter = r.get_pair_filter()
        # Get a list of all atoms in the protein, and put it in a container
        atoms = IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
        cont = IMP.container.ListSingletonContainer(atoms)
        # Add a restraint for the Lennard-Jones interaction. This is built from
        # a collection of building blocks. First, a ClosePairContainer maintains a list
        # of all pairs of Particles that are close. Next, all 1-2, 1-3 and 1-4 pairs
        # from the stereochemistry created above are filtered out.
        # Then, a LennardJonesPairScore scores a pair of atoms with the Lennard-Jones
        # potential. Finally, a PairsRestraint is used which simply applies the
        # LennardJonesPairScore to each pair in the ClosePairContainer.
        nbl = IMP.container.ClosePairContainer(cont, 4.0)
        nbl.add_pair_filter(nonbonded_pair_filter)
        #should weight the ff restraint by a temperature, set to 300K.
        pr = IMP.container.PairsRestraint(pairscore, nbl)
        rs = IMP.RestraintSet('phys')
        rs.add_restraint(pr)
        rs.set_weight(1.0/(kB*ff_temp))
        m.add_restraint(rs)
        return prot, ff, rsb, rs

    def init_model_setup_scale(self, default, lower=-1, upper=-1):
        """sets up a Scale particle to the initial default value. It can
        optionnally be constrained between two positive bounds, or else its
        range  is 0 to infinity.
        """
        m=self._m
        scale = IMP.isd.Scale.setup_particle(IMP.Particle(m),default,lower,upper)
        #constrain it also for the optimizers
        if lower != -1 or upper != -1:
            m.add_score_state(IMP.core.SingletonConstraint(
                            IMP.isd.ScaleRangeModifier(),None,scale))
        return scale

    def init_model_jeffreys_kappa(self, scales, prior_rs=None):
        """given a list of scales, returns a RestraintSet('prior') with weight
        1.0 that contains a list of vonMisesKappaJeffreysRestraint on each scale.
        If argument prior_rs is used, add them to that RestraintSet instead.
        """
        if not prior_rs:
            prior_rs = IMP.RestraintSet('prior')
            self._m.add_restraint(prior_rs)
            prior_rs.set_weight(1.0)
        for i in scales:
            prior_rs.add_restraint(IMP.isd.vonMisesKappaJeffreysRestraint(i))
        return prior_rs

    def init_model_jeffreys(self, scales, prior_rs=None):
        """given a list of scales, returns a RestraintSet('prior') with weight
        1.0 that contains a list of JeffreysRestraint on each scale.
        If argument prior_rs is used, add them to that RestraintSet instead.
        """
        if not prior_rs:
            prior_rs = IMP.RestraintSet('prior')
            self._m.add_restraint(prior_rs)
            prior_rs.set_weight(1.0)
        for i in scales:
            prior_rs.add_restraint(IMP.isd.JeffreysRestraint(i))
        return prior_rs

    def init_model_conjugate_kappa(self, scales, c, R, prior_rs=None):
        """given a list of scales, returns a RestraintSet('prior') with weight
        1.0 that contains a list of vonMisesKappaConjugateRestraint on each
        scale. If argument prior_rs is used, add them to that RestraintSet
        instead.
        """
        if not (0 <= R <= c):
            raise ValueError, "parameters R and c should satisfy 0 <= R <= c"
        if not prior_rs:
            prior_rs = IMP.RestraintSet('prior')
            self._m.add_restraint(prior_rs)
            prior_rs.set_weight(1.0)
        for i in scales:
            prior_rs.add_restraint(IMP.isd.vonMisesKappaConjugateRestraint(i, c, R))
        return prior_rs

    def find_atom(self, atom, prot):
        """scans the prot hierarchy and tries to find atom = (resno, name)
        assumes that resno follows the same numbering as the sequence.
        Stores already found atoms for increased speed.
        """
        if not hasattr(self,'__memoized'):
            self.__memoized={prot:{}}
        try:
            return self.__memoized[prot][atom]
        except:
            pass
        try:
            sel=IMP.atom.Selection(hierarchy=prot,
                residue_index=atom[0],
                atom_type=IMP.atom.AtomType(atom[1])
                ).get_selected_particles()
            if len(sel) > 1:
                print "found multiple atoms for atom %d %s!" % atom
                return
            p0=IMP.core.XYZ(sel[0])
        except:
            print "atom %d %s not found" % atom
            return
        self.__memoized[prot][atom] = p0
        return p0

    def init_model_vonMises_restraint_full(self, atoms, data, kappa):
        """
        Sets up a vonMises torsion angle restraint using the given kappa
        particle as concentration parameter. Returns the restraint.
        data is a list of observations.
        """
        return IMP.isd.TALOSRestraint(atoms,data,kappa)

    def init_model_vonMises_restraint_mean(self, prot, atoms, data, kappa):
        """
        Sets up a vonMises torsion angle restraint using the given kappa
        particle as concentration parameter. Returns the restraint.
        data is (mean, standard deviation).
        """
        raise NotImplementedError

    def init_model_NOE_restraint(self, prot, atoms, distance, sigma, gamma):
        """
        Sets up a lognormal distance restraint using the given sigma and gamma.
        Returns the restraint.
        assumes atoms = (atom1, atom2)
        where atom1 is (resno, atomname) and resno is the residue sequence
        number.
        """
        #find corresponding atoms
        p0=self.find_atom(atoms[0], prot)
        p1=self.find_atom(atoms[1], prot)
        #create lognormal restraint using gamma_data = 1
        ln = IMP.isd.NOERestraint(p0,p1,sigma,gamma,distance**(-6))
        return ln

    def init_model_ambiguous_NOE_restraint(self, prot, contributions, distance,
            sigma, gamma):
        """Reads an ambiguous NOE restraint. contributions is a list of
        (atom1, atom2) pairs, where atom1 is (resno, atomname).  Sets up a
        lognormal distance restraint using the given sigma and gamma.
        Returns the restraint.
        """
        #create pairs list
        pairs=[(self.find_atom(i, prot).get_particle(),
                self.find_atom(j, prot).get_particle()) for (i,j) in
                contributions]
        pairs = IMP.container.ListPairContainer(pairs)
        #create restraint
        ln = IMP.isd.AmbiguousNOERestraint(pairs, sigma, gamma, distance**(-6))
        return ln

    def init_model_NOEs(self, prot, seqfile, tblfile, name='NOE', prior_rs=None,
            bounds_sigma=(1.0,0.1,100), bounds_gamma=(1.0,0.1,100),
            verbose=True, sequence_match=(1,1)):
        """read TBL file and store NOE restraints, using one sigma and one gamma
        for the whole dataset. Creates the necessary uninformative priors.
        - prot: protein hierarchy
        - seqfile: a file with 3-letter sequence
        - tblfile: a TBL file with the restraints
        - name: an optional name for the restraintset
        - prior_rs: when not None, add new sigma and gamma to this
                    RestraintSet instance.
        - bounds_sigma or gamma: tuple of (initial value, lower, upper bound)
            bounds can be -1 to set to default range [0,+inf]
        - verbose: be verbose (default True)
        - sequence_match : (noe_start, sequence_start)
        Returns: data_rs, prior_rs, sigma, gamma
        """
        #prior
        if verbose:
            print "Prior for the NOE Scales"
        sigma=self.init_model_setup_scale(*bounds_sigma)
        gamma=self.init_model_setup_scale(*bounds_gamma)
        prior_rs = self.init_model_jeffreys([sigma,gamma], prior_rs)
        #likelihood
        rs = IMP.RestraintSet(name)
        #use the TBLReader to parse the TBL file.
        sequence = IMP.isd.utils.read_sequence_file(seqfile,
                first_residue_number=sequence_match[1])
        tblr = IMP.isd.TBLReader.TBLReader(sequence,
                sequence_match=sequence_match)
        restraints = tblr.read_distances(tblfile, 'NOE')['NOE']
        for i,restraint in enumerate(restraints):
            if verbose and i % 100 == 0:
                print "\r%d" % i,
                sys.stdout.flush()
            #a restraint is (contributions, dist, upper, lower, volume)
            #where contributions is a tuple of contributing pairs
            #and a pair is (c1, c2), where c1 is of the form (resno, atname)
            if len(restraint[0]) > 1:
                ln = self.init_model_ambiguous_NOE_restraint(prot, restraint[0],
                        restraint[1], sigma, gamma)
            else:
                ln = self.init_model_NOE_restraint(prot, restraint[0][0],
                        restraint[1], sigma, gamma)
            rs.add_restraint(ln)
        if verbose:
            print "\r%d NOE restraints read" % i
        #set weight of rs and add to model.
        #Weight is 1.0 cause sigma particle already has this role.
        rs.set_weight(1.0)
        self._m.add_restraint(rs)
        return rs, prior_rs, sigma, gamma

    def init_model_NOEs_marginal(self, prot, seqfile, tblfile, name='NOE',
            verbose=True, sequence_match=(1,1)):
        """read TBL file and store NOE restraints, using the marginal of the
        lognormal with one sigma and one gamma, for the whole dataset.
        - prot: protein hierarchy
        - seqfile: a file with 3-letter sequence
        - tblfile: a TBL file with the restraints
        - name: an optional name for the restraintset
        - verbose: be verbose (default True)
        - sequence_match : (noe_start, sequence_start)
        Returns: data_rs
        """
        #likelihood
        rs = IMP.RestraintSet(name)
        #use the TBLReader to parse the TBL file.
        sequence = IMP.isd.utils.read_sequence_file(seqfile,
                first_residue_number=sequence_match[1])
        tblr = IMP.isd.TBLReader.TBLReader(sequence,
                sequence_match=sequence_match)
        restraints = tblr.read_distances(tblfile, 'NOE')['NOE']
        ln = IMP.isd.MarginalNOERestraint()
        for i,restraint in enumerate(restraints):
            if verbose and i % 100 == 0:
                print "\r%d" % i,
                sys.stdout.flush()
            #a restraint is (contributions, dist, upper, lower, volume)
            #where contributions is a tuple of contributing pairs
            #and a pair is (c1, c2), where c1 is of the form (resno, atname)
            #residue numbers start at 0
            pairs=[(self.find_atom(i, prot).get_particle(),
                    self.find_atom(j, prot).get_particle()) for (i,j) in
                    restraint[0]]
            pairs = IMP.container.ListPairContainer(pairs)
            ln.add_contribution(pairs, restraint[4])
        rs.add_restraint(ln)
        if verbose:
            print "\r%d NOE contributions added" % (len(restraints))
        rs.set_weight(1.0)
        self._m.add_restraint(rs)
        return rs

    def init_model_HBonds_marginal(self, prot, seqfile, tblfile, name='NOE',
            verbose=True):
        """read TBL file and store lognormal restraints, using the marginal of the
        lognormal with one sigma and gamma=1, for the whole dataset.
        - prot: protein hierarchy
        - seqfile: a file with 3-letter sequence
        - tblfile: a TBL file with the restraints
        - name: an optional name for the restraintset
        - verbose: be verbose (default True)
        Returns: data_rs
        """
        #likelihood
        rs = IMP.RestraintSet(name)
        #use the TBLReader to parse the TBL file.
        sequence = IMP.isd.utils.read_sequence_file(seqfile)
        tblr = IMP.isd.TBLReader.TBLReader(sequence)
        restraints = tblr.read_distances(tblfile, 'HBond')['HBond']
        ln = IMP.isd.MarginalHBondRestraint()
        for i,restraint in enumerate(restraints):
            if verbose and i % 100 == 0:
                print "\r%d" % i,
                sys.stdout.flush()
            #a restraint is (contributions, dist, upper, lower, volume)
            #where contributions is a tuple of contributing pairs
            #and a pair is (c1, c2), where c1 is of the form (resno, atname)
            #residue numbers start at 0
            pairs=[(self.find_atom(i, prot).get_particle(),
                    self.find_atom(j, prot).get_particle()) for (i,j) in
                    restraint[0]]
            pairs = IMP.container.ListPairContainer(pairs)
            ln.add_contribution(pairs, restraint[4])
        rs.add_restraint(ln)
        if verbose:
            print "\r%d Hbond contributions added" % (len(restraints))
        rs.set_weight(1.0)
        self._m.add_restraint(rs)
        return rs

    def init_model_TALOS(self, prot, seqfile, talos_data, fulldata=True,
            sequence_match=(1,1),name='TALOS', prior_rs=None,
            bounds_kappa=(1.0, 0.1,10), verbose=True, prior='jeffreys',
            keep_all=False):
        """read TALOS dihedral angle data, and create restraints for phi/psi
        torsion angles, along with the prior for kappa, which is a scale for the
        whole dataset, compare to 1/sigma**2 in the NOE case.
        - prot: protein hierarchy
        - seqfile: a file with 3-letter sequence
        - talos_data: either a file (pred.tab), or a folder (pred/) in which all
                      files in the form res???.tab can be found. If possible,
                      try to provide the folder, as statistics are more
                      accurate in that case.
        - fulldata : either True or False, whether the data is the full TALOS
                     output (predAll.tab or pred/ folder), or just the averages
                     (pred.tab)
        - sequence_match : (talos_no, sequence_no) to adjust for different
                           residue numberings
        - name: an optional name for the restraintset
        - prior_rs: when not None, add new kappa(s) to this RestraintSet instance.
        - bounds_kappa: tuple of (initial value, lower, upper bound)
                        bounds can be -1 to set to default range [0,+inf]
        - verbose: be verbose (default True)
        - prior: either 'jeffreys' or a tuple (R,c), which signifies to use the
                 conjugate prior of the von Mises restraint, with parameters R
                 and c.  Good values are R=0 and c=10. Default: jeffreys prior.
        - keep_all: in case of a folder for 'talos_data', whether to keep
                    candidates marked as 'outliers' by TALOS, or to include them.
        Returns: data_rs, prior_rs, kappa

        """
        #prior
        if verbose:
            print "Prior for von Mises Kappa"
        kappa=self.init_model_setup_scale(*bounds_kappa)
        prior_rs = self.init_model_jeffreys_kappa([kappa], prior_rs)
        #likelihood
        if verbose:
            print "reading data"
        rs=IMP.RestraintSet(name)
        sequence= IMP.isd.utils.read_sequence_file(seqfile,
                first_residue_no=sequence_match[1])
        if fulldata:
            talosr=IMP.isd.TALOSReader.TALOSReader(sequence, True, keep_all,
                    sequence_match=sequence_match)
            if os.path.isdir(talos_data):
                #using pred/res???.tab files
                for i,res in enumerate(glob(os.path.join(talos_data,'res???.tab'))):
                    if verbose and i % 100:
                        print "\r%d" % i,
                        sys.stdout.flush()
                    talosr.read(res)
            else:
                #using predAll.tab file
                talosr.read(talos_data)
        else:
            #using pred.tab file and correcting for estimates
            talosr=IMP.isd.TALOSReader.TALOSReader(sequence, False, keep_all,
                    sequence_match=sequence_match)
            talosr.read(talos_data)
        #get harvested data and create restraints
        data = talosr.get_data()
        if verbose:
            print "\rread dihedral data for %d residues" % len(data)
            print "creating restraints"
        avgR=[]
        for resno,datum in data.iteritems():
            phidata=datum['phi']
            psidata=datum['psi']
            num=datum['num']
            res=IMP.atom.Residue(IMP.atom.get_residue(prot, resno))
            phi=IMP.atom.get_phi_dihedral_atoms(res)
            psi=IMP.atom.get_psi_dihedral_atoms(res)
            if fulldata:
                r=self.init_model_vonMises_restraint_full(phi, phidata, kappa)
                rs.add_restraint(r)
                if verbose:
                    avgR.append(r.get_R0())
                r=self.init_model_vonMises_restraint_full(psi, psidata, kappa)
                rs.add_restraint(r)
                if verbose:
                    avgR.append(r.get_R0())
            else:
                r=self.init_model_vonMises_restraint_mean(phi, phidata, kappa)
                rs.add_restraint(r)
                if verbose:
                    avgR.append(r.get_R0())
                r=self.init_model_vonMises_restraint_mean(psi, psidata, kappa)
                rs.add_restraint(r)
                if verbose:
                    avgR.append(r.get_R0())
        if verbose:
            print "%s Restraints created. Average R0: %f" % \
                (len(avgR), sum(avgR)/float(len(avgR)))
        rs.set_weight(1.0)
        self._m.add_restraint(rs)
        return rs, prior_rs, kappa

    def init_model_standard_SAXS_restraint(self, prot, profilefile, name='SAXS',
            ff_type=IMP.saxs.HEAVY_ATOMS):
        """read experimental SAXS profile and apply restraint the standard
        way (like foxs)
        Returns: a restraintset and the experimental profile
        """
        rs = IMP.RestraintSet(name)
        saxs_profile = IMP.saxs.Profile(profilefile)
        particles = IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
        saxs_restraint = IMP.saxs.Restraint(particles, saxs_profile, ff_type)
        rs.add_restraint(saxs_restraint)
        rs.set_weight(1.0)
        self._m.add_restraint(rs)
        return rs, saxs_profile

    def _setup_md(self,prot, temperature=300.0, thermostat='berendsen',
            coupling=500, md_restraints=None, timestep=1.0, recenter=1000,
            momentum=1000):
        """setup molecular dynamics
        - temperature: target temperature
        - thermostat: one of 'NVE', rescale_velocities',
                                'berendsen', 'langevin'
        - coupling: coupling constant (tau (fs) for berendsen,
                                        gamma (/fs) for langevin)
        - md_restraints: if not None, specify the terms of the energy to be used
                            during the md steps via a list of restraints.
        - timestep: in femtoseconds.
        - recenter: recenter the molecule every so many steps (Langevin only)
        - momentum: remove angular momentum every so many steps (Berendsen only)
        Returns: an instance of md and an instance of an OptimizerState (the
        thermostat), or None if NVE.
        """
        ## Molecular Dynamics (from MAX BONOMI)
        md=IMP.atom.MolecularDynamics()
        md.set_model(self._m)
        md.assign_velocities(temperature)
        md.set_time_step(timestep)
        if thermostat == 'NVE':
            os = None
        elif thermostat == 'rescale_velocities':
            os=IMP.atom.VelocityScalingOptimizerState(
                    IMP.atom.get_leaves(prot), temperature, 0)
            md.add_optimizer_state(os)
        elif thermostat == 'berendsen':
            os=IMP.atom.BerendsenThermostatOptimizerState(
                    IMP.atom.get_leaves(prot), temperature, coupling)
            md.add_optimizer_state(os)
            mom = IMP.atom.RemoveRigidMotionOptimizerState(
                    IMP.atom.get_leaves(prot), momentum)
            md.add_optimizer_state(mom)
        elif thermostat == 'langevin':
            os=IMP.atom.LangevinThermostatOptimizerState(
                    IMP.atom.get_leaves(prot), temperature, coupling)
            md.add_optimizer_state(os)
            #cen = IMP.atom.RemoveTranslationOptimizerState(
            #        IMP.atom.get_leaves(prot), recenter)
            #md.add_optimizer_state(cen)
        else:
            raise NotImplementedError, thermostat

        if md_restraints:
            md.set_restraints(md_restraints)

        return md, os

    def _setup_normal_mover(self, particle, floatkey, stepsize):
        """setup NormalMover to move particle's floatkey attribute
        by a gaussian with standard deviation 'stepsize'
        Returns: mover instance.
        """
        cont=IMP.container.ListSingletonContainer(self._m)
        cont.add_particle(particle)
        nm=IMP.core.NormalMover(cont,
                IMP.FloatKeys([floatkey]),stepsize)
        return nm

    def _setup_md_mover(self, md, particles, temperature, n_md_steps=10):
        """setup MDMover using md and particles.
        - md: md instance
        - particles: particles to move, usually the leaves of the protein
          hierarchy
        - n_md_steps: number of md steps to perform on each move
        Returns: mover instance.
        """
        cont=IMP.container.ListSingletonContainer(self._m)
        cont.add_particles(particles)
        return IMP.atom.MDMover(cont, md, temperature, n_md_steps)

    def _setup_mc(self, mover, temperature=300.0, mc_restraints=None):
        """setup monte carlo using a certain mover.
        - mover: mover to use, NormalMover or MDMover usually.
        - temperature: target temperature.
        - mc_restraints: if not None, list of restraints for the metropolis
          criterion.
        Returns: mc instance.
        """
        mc = IMP.core.MonteCarlo(self._m)
        #why is this returning an int?
        mc.add_mover(mover)
        #set same temp as MD, careful with units
        mc.set_kt(kB*temperature)
        #allow to go uphill
        mc.set_return_best(False)
        #update all particles each time
        mc.set_move_probability(1.0)
        if mc_restraints:
            #tell mc to only use restraints involving the particle
            mc.set_restraints(mc_restraints)
        return mc

    def init_simulation_setup_protein_hmc_hopper(self, prot, temperature=300.0,
            gamma=0.01, n_md_steps=10,
            md_restraints=None, mc_restraints=None, timestep=1.0,
            sd_threshold=0.0, sd_stepsize=0.01, sd_maxsteps=100):
        """setup hybrid monte-carlo on protein. Uses basin hopping with steepest
        descent minimization.
        - prot: protein hierarchy.
        - temperature: target temperature.
        - gamma: coupling constant for langevin (/fs)
        - n_md_steps: number of md steps per mc step
        - md_restraints: if not None, specify the terms of the energy to be used
                            during the md steps.
        - mc_restraints: if not None, use these energy terms for the metropolis
                            criterion.
        - timestep: time step for md, in femtoseconds.
        - sd_threshold: stop steepest descent after energy difference drops
                below that threshold
        - sd_stepsize: stepsize to use for the steepest descent, in angstrom.
        - sd_maxsteps: maximum number of steps for steepest descent
        Returns: hmc, mdmover, md and OptimizerState (thermostat)
        """
        raise NotImplementedError
        md, os = self._setup_md(prot, temperature=temperature,
                thermostat='langevin', coupling=gamma,
                md_restraints=md_restraints, timestep=timestep)
        particles=IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
        mdmover = self._setup_md_mover(md, particles, temperature, n_md_steps)
        hmc = self._setup_mc(mdmover, temperature, mc_restraints)
        sd=IMP.core.SteepestDescent(self._m)
        sd.set_threshold(sd_threshold)
        sd.set_step_size(sd_stepsize)
        hmc.set_local_optimizer(sd)
        hmc.set_local_steps(sd_maxsteps)
        hmc.set_use_basin_hopping(True)
        return hmc, mdmover, md, os

    def init_simulation_setup_protein_hmc_nve(self, prot, temperature=300.0,
            n_md_steps=100, md_restraints=None, mc_restraints=None,
            timestep=1.0):
        """setup hybrid monte-carlo on protein. Uses NVE MD and tries the full
        - prot: protein hierarchy.
        - temperature: target temperature.
        - coupling: coupling constant (tau (fs) for berendsen,
                                        gamma (/fs) for langevin)
        - n_md_steps: number of md steps per mc step
        - md_restraints: if not None, specify the terms of the energy to be used
                            during the md steps.
        - mc_restraints: if not None, use these energy terms for the metropolis
                            criterion.
        - timestep: time step for md, in femtoseconds.
        - sd_threshold: stop steepest descent after energy difference drops
                below that threshold
        - sd_stepsize: stepsize to use for the steepest descent, in angstrom.
        - sd_maxsteps: maximum number of steps for steepest descent
        Returns: hmc, mdmover and md
        """
        prot = self._p['prot']
        md, os = self._setup_md(prot, temperature=temperature,
                thermostat='NVE', coupling=None, timestep=1.0,
                md_restraints=md_restraints)
        particles=IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
        cont=IMP.container.ListSingletonContainer(self._m)
        cont.add_particles(particles)
        mdmover = PyMDMover(cont, md, n_md_steps)
        mdmover.m=self._m
        #mdmover = IMP.atom.MDMover(cont, md, temperature, n_md_steps)
        mc = PyMC(self._m)
        #mc = IMP.core.MonteCarlo(self._m)
        #why is this returning an int?
        mc.add_mover(mdmover)
        #set same temp as MD, careful with units
        mc.set_kt(kB*temperature)
        #allow to go uphill
        mc.set_return_best(False)
        #update all particles each time
        mc.set_move_probability(1.0)
        return mc, mdmover, md

    def init_simulation_setup_scale_mc(self, scale, temperature=300.0,
            mc_restraints=None, floatkey=IMP.FloatKey("scale"), nm_stepsize=0.1):
        """sets up monte carlo on scale, at a certain target temperature,
        optionnally using a certain set of restraints only.
        - scale: scale particle
        - temperature: target temperature
        - mc_restraints: optional set of restraints from which the energy should
          be drawn instead of the energy of the complete system.
        - floatkey: the floatkey to move.
        - nm_stepsize: the stepsize of the normal mover
        Returns: mc instance, nm instance.
        """
        nm = self._setup_normal_mover(scale, floatkey, nm_stepsize)
        mc = self._setup_mc(nm, temperature, mc_restraints)
        return mc, nm

    def _mc_and_update_nm(self, nsteps, mc, nm, stats_key,
            adjust_stepsize=True):
        """run mc using a normal mover on a single particle,
        update stepsize and store nsteps, acceptance and stepsize in the
        statistics instance self.stat by using the key stats_key.
        """
        #do the monte carlo
        mc.optimize(nsteps)
        naccept = mc.get_number_of_forward_steps()
        #increment the counter for this simulation
        self.stat.increment_counter(stats_key, nsteps)
        #get the acceptance rate, stepsize
        accept = float(naccept)/nsteps
        self.stat.update(stats_key, 'acceptance', accept)
        stepsize = nm.get_sigma()
        self.stat.update(stats_key, 'stepsize', stepsize)
        #update stepsize if needed and requested
        if adjust_stepsize:
            if 0.4 < accept < 0.6:
                return
            if accept < 0.05:
                accept = 0.05
            if accept > 1.0:
                accept = 1.0
            nm.set_sigma(stepsize*2*accept)

    def _hmc_and_update_md(self, nsteps, hmc, mv, stats_key,
            adjust_stepsize=True):
        """run hmc, update stepsize and print statistics. Updates number of MD
        steps to reach a target acceptance between 0.4 and 0.6, sends
        statistics to self.stat. MD steps are always at least 10 and at most 500.
        """
        hmc.optimize(nsteps)
        naccept = hmc.get_number_of_forward_steps()
        self.stat.increment_counter(stats_key, nsteps)
        accept = float(naccept)/nsteps
        self.stat.update(stats_key, 'acceptance', accept)
        mdsteps = mv.get_number_of_steps()
        self.stat.update(stats_key, 'n_md_steps', mdsteps)
        if adjust_stepsize:
            if 0.4 < accept < 0.6:
                return
            mdsteps = int(mdsteps *2**(accept-0.5))
            if mdsteps > 500:
                mdsteps = 500
            if mdsteps < 10:
                mdsteps = 10
            mv.set_nsteps(mdsteps)

    def get_pdb(self, prot):
        """returns a string corresponding to the pdb structure of hierarchy
        prot.
        """
        output = StringIO()
        IMP.atom.write_pdb(prot, output)
        return output.getvalue()

    def get_netcdf(self, prot):
        raise NotImplementedError

    def do_md_protein_statistics(self, md_key, nsteps, md_instance,
            temperature=300.0, prot_coordinates=None):
        """updates statistics for md simulation: target temp, kinetic energy,
        kinetic temperature, writes coordinates and increments counter.
        - md_key: stats md key
        - nsteps: number of steps performed.
        - md_instance: instance of the MolecularDynamics class.
        - temperature: target temperature
        - prot_coordinates: protein coordinates to be passed to the stats class,
          (should be a string)
        """
        self.stat.update(md_key, 'target_temp', temperature)
        kinetic = md_instance.get_kinetic_energy()
        self.stat.update(md_key, 'E_kinetic', kinetic)
        self.stat.update(md_key, 'instant_temp',
                         md_instance.get_kinetic_temperature(kinetic))
        self.stat.update_coordinates(md_key, 'protein', prot_coordinates)
        self.stat.increment_counter(md_key, nsteps)

    def init_stats_add_mc_category(self, stat, name='mc', coord='particle'):
        """shortcut for a frequent series of operations on MC simulations'
        statistics. Creates an entry for acceptance, stepsize and one
        coordinate set printed in the statistics file.
        """
        #create category
        mc_key = stat.add_category(name=name)
        #giving None as argument is a way to create a static entry.
        stat.add_entry(mc_key, entry=Entry('temperature', '%10G', None))
        stat.add_entry(mc_key, entry=Entry('acceptance', '%10G', None))
        stat.add_entry(mc_key, entry=Entry('stepsize', '%10G', None))
        #special call to add coordinates to be dumped
        stat.add_entry(mc_key, entry=Entry(coord, '%10G', None))
        #add the counter to the output
        stat.add_entry(mc_key, name='counter')
        return mc_key

    def init_stats_add_md_category(self, stat, name='md', coord='protein'):
        """shortcut for a frequent series of operations on MD simulations'
        statistics. Creates an entry for target temp, instantaneous temp,
        kinetic energy, and one set of coordinates called 'protein' by
        default.
        """
        #create category
        md_key = stat.add_category(name=name)
        #giving None as argument is a way to create a static entry.
        stat.add_entry(md_key, entry=Entry('target_temp', '%10G', None))
        stat.add_entry(md_key, entry=Entry('instant_temp', '%10G', None))
        stat.add_entry(md_key, entry=Entry('E_kinetic', '%10G', None))
        #special call to add coordinates to be dumped
        stat.add_coordinates(md_key, coord)
        #add the counter to the output
        stat.add_entry(md_key, name='counter')
        return md_key

    def init_stats_add_hmc_category(self, stat, name='hmc', coord='protein'):
        """shortcut for a frequent series of operations on HMC simulations'
        statistics. Adds acceptance, number of MD steps and a trajectory for
        a protein.
        """
        #create category
        hmc_key = stat.add_category(name=name)
        #giving None as argument is a way to create a static entry.
        stat.add_entry(hmc_key, entry=Entry('temperature', '%10G', None))
        stat.add_entry(hmc_key, entry=Entry('acceptance', '%10G', None))
        stat.add_entry(hmc_key, entry=Entry('n_md_steps', '%10G', None))
        stat.add_entry(hmc_key, entry=Entry('E_kinetic', '%10G', None))
        #special call to add coordinates to be dumped
        stat.add_coordinates(hmc_key, coord)
        #add the counter to the output
        stat.add_entry(hmc_key, name='counter')
        return hmc_key

    def rescale_velocities(self, particles, factor):
        """rescale the velocities of a bunch of particles having vx vy and vz
        floatkeys
        """
        keys=[IMP.FloatKey("vx"), IMP.FloatKey("vy"), IMP.FloatKey("vz")]
        for p in particles:
            for k in keys:
                p.set_value(k, p.get_value(k)*factor)
