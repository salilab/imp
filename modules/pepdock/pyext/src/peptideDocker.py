import IMP
import subprocess
import random
import IMP.domino
import IMP.core
import IMP.rmf
import RMF

import time
import IMP.algebra
import types
import re
import sys
import operator
import os
import atomicDominoUtilities

class PeptideDocker:

    def __init__(self, parameterFileName):
        self.createdMdOptimizer = 0
        self.fixedNonFlexibleAtoms = 0
        self.readParameters(parameterFileName)
        self.wroteNativeProtein = 0

    def loadHelpers(self):
        self.loadDockingHelpers()
        self.loadMdHelpers()

    def readParameters(self, parameterFileName):
        self.parameters = atomicDominoUtilities.readParameters(parameterFileName)

    #Processing specific to peptide docking and this script
    def loadDockingHelpers(self):
        outputDir = self.getParam("output_directory")
        mkdirProcess = subprocess.Popen(['mkdir', '-p', self.getParam('output_directory')], shell=False, stderr=subprocess.PIPE)
        output = mkdirProcess.communicate()

        self.namesToParticles = atomicDominoUtilities.makeNamesToParticles(self.protein)

        self.times = []
        self.rawTimes = []

    #Helper method; create dictionary where the keys are names of all particles in the system and the values are themselves dictionaries
    def initializeParticleNameDict(self, leaves):
        particleDict = {}

        for leaf in leaves:
            name = self.quickParticleName(leaf)
            particleDict[name] = {}
        return particleDict

    def getModel(self):
        return self.model

    def getProtein(self):
        return self.protein

    def createModel(self):

        pdbName = self.getParam("native_pdb_input_file")

        self.model = IMP.Model()

        self.protein = IMP.atom.read_pdb(pdbName, self.model, IMP.atom.ATOMPDBSelector())

        #atom radii, forcefield topology, etc. -- no restraints created here

        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top_heav.lib"),
                                   IMP.atom.get_data_path("par.lib"))

        topology = ff.create_topology(self.protein)
        topology.apply_default_patches()

        topology.add_atom_types(self.protein)

        ff.add_radii(self.protein)
        ff.add_well_depths(self.protein)
        self.ff = ff
        self.topology = topology

    #Get parameter value. Throws dictionary exception if not found
    def getParam(self, paramName):
        paramValue = self.parameters[paramName]
        return paramValue

    #Peptide-docking friendly way to log which restraint we're dealing with
    def quickRestraintName(self, r):
        nameList = []
        ps = r.get_input_particles()

        for p in ps:
            if (self.isAtomParticle(p) == 0):
                continue
            isFlexible = 0
            if (self.flexibleAtoms.has_key(self.quickParticleName(p)) == 1):
                isFlexible = 1
            nameList.append("%s_%s" % (self.quickParticleName(p), isFlexible))

        return "%s acting on %s" % (r.get_name(), " , ".join(nameList))

    def quickParticleName(self, particle):
        return atomicDominoUtilities.quickParticleName(particle)

    #output current time
    def logTime(self, label):
        nextTime =  time.asctime(time.localtime(time.time()))
        print "%s:\t %s" % (label, nextTime)
        nextTimeEntry = [label, nextTime]
        self.times.append(nextTimeEntry)
        self.rawTimes.append(time.time())

    #output all times that were saved by logTime()
    def outputTimes(self):
        print "Time for each step:"
        for timeEntry in self.times:
            label = timeEntry[0]
            value = timeEntry[1]
            print "%s:\t%s" % (label, value)


    def getFlexibleAtoms(self):
        return self.flexibleAtoms

    #Helper method to quickly get the chain identifier for a particle
    def getChain(self, particle):
        atomDecorator = IMP.atom.Atom.decorate_particle(particle)
        residue = IMP.atom.get_residue(atomDecorator)
        chain = IMP.atom.get_chain(residue)
        chainId = chain.get_id()
        return chainId

    #Helper method to quickly get the residue index for a particle
    def getResidue(self, particle):
        atomDecorator = IMP.atom.Atom.decorate_particle(particle)
        residue = IMP.atom.get_residue(atomDecorator)
        residueNumber = residue.get_index()
        return residueNumber

    def isAtomParticle(self, p):
        return atomicDominoUtilities.isAtomParticle(p)

    #Read all particles in restraint; return 1 if all are flexible and 0 if at least one atom is fixed
    def allFlexibleAtoms(self, r):
        ps = r.get_input_particles()
        for p in ps:
            if (self.isAtomParticle(p) == 0):
                continue
            if (self.flexibleAtoms.has_key(self.quickParticleName(p)) == 0):
                return 0
        return 1

    #Check if we should skip processing of this pair of particles. Don't add restraints between particles that aren't optimized,
    #and don't add restraints between particles on the same residue (for now assume stereochemistry is good enough for this)
    def skipNonBondedPair(self, pair):

        #skip if same residue
        if (self.getResidue(pair[0]) == self.getResidue(pair[1])):
            return 1

        #skip if neither is flexible
        if (self.flexibleAtoms.has_key(self.quickParticleName(pair[0])) == 0 and self.flexibleAtoms.has_key(self.quickParticleName(pair[1])) == 0):
            return 1
        return 0

    #Set up degrees of freedom for the system. Either we are dealing with a fixed protein or a flexible one.
    #Determine which atoms are flexible and save them in self.flexibleAtoms
    def initDof(self):

        peptideChain = self.getParam("peptide_chain")
        self.flexibleAtoms = {}

        if (self.getParam("flexible_protein") == "yes"):
            #All atoms within range of the peptide are flexible; range defined by close_pair_distance param
            particlePairs = self.getClosePairs()

            for pair in particlePairs:
                p0 = pair[0]
                p1 = pair[1]
                p0Chain = self.getChain(p0)
                p1Chain = self.getChain(p1)
                if (p0Chain == peptideChain or p1Chain == peptideChain):
                    self.flexibleAtoms[self.quickParticleName(p0)] = 1
                    self.flexibleAtoms[self.quickParticleName(p1)] = 1

        else:
            #only peptide atoms are flexible, determined by the chain id
            leaves = IMP.atom.get_leaves(self.protein)
            for leaf in leaves:
                if (self.getChain(leaf) == peptideChain):
                    self.flexibleAtoms[self.quickParticleName(leaf)] = 1
        self.fixNonFlexibleAtoms()

    #return a dictionary where the keys are the names of the fixed atoms in the system
    def getFixedAtoms(self):
        leaves = IMP.atom.get_leaves(self.protein)
        fixedAtoms = {}
        for leaf in leaves:
            leafName = self.quickParticleName(leaf)
            if self.flexibleAtoms.has_key(leafName) == 0:
                fixedAtoms[leafName] = 1
        return fixedAtoms


    #for each flexible atom, give it the coordinates of another flexible atom (ensures that each atom starts out in the binding site)
    #Although it does guarantee a very random initial configuration, it is cheating since we are using knowledge of the native state
    def initializeRandomExisting(self):
        leaves = IMP.atom.get_leaves(self.protein)
        particlesToVary = []
        xyzs = []
        for leaf in leaves:
            if (self.flexibleAtoms.has_key(self.quickParticleName(leaf)) == 1):
                particlesToVary.append(self.quickParticleName(leaf))
        atomToCoordinates = {}
        for pName in particlesToVary:
            randomPname = random.choice(particlesToVary)
            randomP = self.namesToParticles[randomPname]
            print "setting coordinates for particle %s to the ones currently set for particle %s" % (pName, randomPname)
            atomToCoordinates[pName] = IMP.core.XYZ.decorate_particle(randomP).get_coordinates()

        for pName in particlesToVary:
            particle = self.namesToParticles[pName]
            xyz = IMP.core.XYZ.decorate_particle(particle)
            xyz.set_coordinates(atomToCoordinates[pName])
            xyzs.append(xyz)
        return xyzs


    #Set initial positions of flexible atoms in the system. We are experimenting with the best way to do this
    def setInitialPositions(self):
        initialPositionMethod = self.getParam("initial_position_method")

        #set random coordinates for atom by creating bounding box around native coordinates, increasing it
        #by defined offset plus random number (necessary when running on cluster)
        if (initialPositionMethod == "random"):

            myRandom = random.random()
            initialOffset = float(self.getParam("initial_atom_offset")) + myRandom
            for particleName in self.flexibleAtoms.keys():
                p = self.namesToParticles[particleName]
                xyzDecorator = IMP.core.XYZ.decorate_particle(p)
                coordinates = xyzDecorator.get_coordinates()
                bb = IMP.algebra.BoundingBox3D(coordinates)
                bb += initialOffset
                bb += myRandom

                randomXyz = IMP.algebra.get_random_vector_in(bb)
                xyzDecorator.set_coordinates(randomXyz)

        #for each flexible atom, give it the coordinates of another flexible atom (ensures that each atom starts out in the binding site)
        elif (initialPositionMethod == "random_existing"):
            print "start random existing"
            xyzs = self.initializeRandomExisting()

        #same as random_existing except offset each atom a little (doesn't seem to make too much of a difference)
        elif (initialPositionMethod == "random_existing_box"):
            xyzs = self.initializeRandomExisting()
            initialOffset = float(self.getParam("initial_atom_offset"))
            for xyzDecorator in xyzs:
                coordinates = xyzDecorator.get_coordinates()
                bb = IMP.algebra.BoundingBox3D(coordinates)
                bb += initialOffset

                randomXyz = IMP.algebra.get_random_vector_in(bb)
                xyzDecorator.set_coordinates(randomXyz)

        elif (initialPositionMethod == "random_box"):

            #Get bounding box containing all particles within close pair distance of flexible atoms
            #This doesn't work perfectly; can put initial positions in the fixed atoms accidentally
            #As constructed it is specific to peptide atoms only and not flexible protein atoms
            bsParticles = []

            for firstPname in self.nonBondedPairs.keys():
                secondPnames = self.nonBondedPairs[firstPname]
                for secondPname in secondPnames.keys():
                    if (self.flexibleAtoms.has_key(secondPname) == 0):
                        fixedAtom = self.namesToParticles[firstPname]

                        bsParticles.append(IMP.core.XYZ.decorate_particle(fixedAtom))
                        print "added next close atom %s to bounding box particles" % firstPname
            #bsBoundingBox = IMP.core.get_bounding_box(bsParticles)  -- revert if I can figure out how
            peptideParticles = []
            leaves = IMP.atom.get_leaves(self.protein)
            for leaf in leaves:
                if (self.getChain(leaf) == self.getParam("peptide_chain")):
                    peptideParticles.append(IMP.core.XYZ.decorate_particle(leaf))
            bsBoundingBox = IMP.core.get_bounding_box(peptideParticles)
            #set initial position for each flexible atoms, and adjust it if it is too close to a fixed residue
            fixedAtoms = self.getFixedAtoms()
            for flexPname in self.flexibleAtoms.keys():
                goodPosition = 0
                flexParticle = self.namesToParticles[flexPname]
                flexXyzDecorator = IMP.core.XYZ.decorate_particle(flexParticle)
                print "processing position for pname %s" % flexPname
                while(goodPosition == 0):
                    goodPosition = 1
                    flexXyzDecorator.set_coordinates(IMP.algebra.get_random_vector_in(bsBoundingBox))
                    for fixedPname in fixedAtoms.keys():
                        fixedParticle = self.namesToParticles[fixedPname]
                        distance = IMP.algebra.get_distance(IMP.core.XYZ.decorate_particle(fixedParticle).get_coordinates(),
                                                            IMP.core.XYZ.decorate_particle(flexParticle).get_coordinates())
                        if (distance < 2):
                            print "finding new position for %s" % flexPname
                            goodPosition = 0
                            break    #start the while loop over which will set new coordinates


        #take a box around the whole protein and randomly initialize atoms here
        elif (initialPositionMethod == "random_full"):
            bb = IMP.atom.get_bounding_box(self.protein)
            for particleName in self.flexibleAtoms.keys():
                print "creating random position for particle %s" % particleName
                p = self.namesToParticles[particleName]
                xyzDecorator = IMP.core.XYZ.decorate_particle(p)
                randomXyz = IMP.algebra.get_random_vector_in(bb)
                myRandom = random.random()
                bb += myRandom
                xyzDecorator.set_coordinates(randomXyz)


        #Read in a file generated from a previous run (fastest way is to create a new model and copy corresponding XYZ Decorators)
        elif (initialPositionMethod == "file"):

            initialPositionFile = self.getParam("saved_initial_atom_positions")
            print "reading initial positions from %s" % initialPositionFile
            initialModel = IMP.Model()
            initialProtein = IMP.atom.read_pdb(initialPositionFile, initialModel, IMP.atom.ATOMPDBSelector())
            initialLeaves = IMP.atom.get_leaves(initialProtein)
            for initialLeaf in initialLeaves:
                name = self.quickParticleName(initialLeaf)
                if (self.namesToParticles.has_key(name) == 1):
                    existingLeaf = self.namesToParticles[name]
                    existingLeafXyz = IMP.core.XYZ.decorate_particle(existingLeaf)
                    initialLeafXyz = IMP.core.XYZ.decorate_particle(initialLeaf)
                    existingLeafXyz.set_coordinates(initialLeafXyz.get_coordinates())
                else:
                    print "Read in initial positions from file %s but this file contained particle %s which is not in current model" % (initialPositionFile, name)
        else:
            print "Please specify valid initial position method (random or file)\n"
            sys.exit()

        #output initial positions
        outputDir = self.getParam("output_directory")
        initialAtomPositionFile = self.getParam("initial_atom_positions_output_file")
        fullOutputFile = os.path.join(outputDir, initialAtomPositionFile)
        print "writing output to file %s" % fullOutputFile
        IMP.atom.write_pdb(self.protein, fullOutputFile)

        #get initial score for model
        initialScore = self.model.evaluate(False)
        print "initial score for model is %s" % initialScore


    #Get non-bonded pairs in the system in preparation for restraining them
    def getNonBondedPairs(self):
        nonBondedDefinition = self.getParam("non_bonded_definition")
        particlePairs = []
        if (nonBondedDefinition == "close_pairs"):
            particlePairs = self.getClosePairs()
        elif (nonBondedDefinition == "random"):
            particlePairs = self.getRandomParticlePairs()
        elif (nonBondedDefinition == "manual"):
            particlePairs = self.readManualRestraints(nativeProtein, 6)
        else:
            print "Please specify valid non bonded pair definition"
            sys.exit()
        return particlePairs

    #Get 3D distance between the particles in the pair
    def getXyzDistance(self, pair):
        d0 = IMP.core.XYZ.decorate_particle(pair[0])
        d1 = IMP.core.XYZ.decorate_particle(pair[1])
        distance = IMP.core.get_distance(d0, d1)
        return distance

    #Get all pairs of particles within the distance defined by close_pair_distance parameter
    def getClosePairs(self):

        # Get a list of all atoms in the protein, and put it in a container
        leaves = IMP.atom.get_leaves(self.protein)
        cont = IMP.container.ListSingletonContainer(leaves)
        closePairDistance = float(self.getParam("close_pair_distance"))
        useHardCutoff = self.getParam("use_hard_distance_cutoff")  #sometimes closePairContainer returns particles > distanceCutoff, even with slack = 0

        nbl = IMP.container.ClosePairContainer(cont, closePairDistance, 0.0)

        self.model.evaluate(False)

        particlePairs = nbl.get_particle_pairs()
        finalPairs = []
        for pair in particlePairs:
            distance = self.getXyzDistance(pair)
            q0 = self.quickParticleName(pair[0])
            q1 = self.quickParticleName(pair[1])

            if (useHardCutoff == "0" or (useHardCutoff == "1" and distance < closePairDistance)):
                finalPairs.append([pair[0], pair[1]])

        return finalPairs

    #Add restraints for bond lengths, angles, dihedrals, and impropers
    def addForceFieldRestraints(self):

        useForcefieldRestraints = self.getParam("use_forcefield_restraints")
        if (useForcefieldRestraints == "1"):

            ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top_heav.lib"),
                                       IMP.atom.get_data_path("par.lib"))

            # Set up and evaluate the stereochemical part (bonds, angles, dihedrals,
            # impropers) of the CHARMM forcefield
            bonds = self.topology.add_bonds(self.protein)
            angles = ff.create_angles(bonds)
            dihedrals = ff.create_dihedrals(bonds)
            impropers = self.topology.add_impropers(self.protein)

            #tracks which particles are together in forcefield restraints (useful for filtering excluded volume)
            self.ffParticleGroups = self.initializeParticleNameDict(IMP.atom.get_leaves(self.protein))

            self.createForceFieldRestraintsForModel(bonds, "bonds", "Bond Restraints", IMP.atom.BondSingletonScore(IMP.core.Harmonic(0,1)))
            self.createForceFieldRestraintsForModel(angles, "angles", "Angle Restraints", IMP.atom.AngleSingletonScore(IMP.core.Harmonic(0,1)))
            self.createForceFieldRestraintsForModel(dihedrals, "dihedrals", "Dihedral Restraints", IMP.atom.DihedralSingletonScore())
            self.createForceFieldRestraintsForModel(impropers, "impropers", "Improper Restraints", IMP.atom.ImproperSingletonScore(IMP.core.Harmonic(0,1)))

        else:
            print ("Skipping forcefield restraints")

    #Create one type of forcefield restraint. The restraint is initially across the full model and needs to be decomposed
    #so it can be used in the restraint graph.
    def createForceFieldRestraintsForModel(self, ffParticles, containerName, restraintName, singletonScore):

        maxForceFieldScore = float(self.getParam("max_forcefield_score"))
        cont = IMP.container.ListSingletonContainer(self.model, containerName)
        cont.add_particles(ffParticles)
        listBondRestraint = IMP.container.SingletonsRestraint(singletonScore, cont, restraintName)
        self.model.add_restraint(listBondRestraint)

        #Restraint decomposition has been a little touchy; a couple workarounds contained
        decomposedRestraintTemp = listBondRestraint.create_decomposition()
        decomposedRestraints = IMP.kernel.RestraintSet.get_from(decomposedRestraintTemp)
        rs_restraints= decomposedRestraints.get_restraints()

        #manually remove the full restraint after getting the decomposition
        self.model.remove_restraint(listBondRestraint)

        count = decomposedRestraints.get_number_of_restraints()
        for i in range(count):

            r = decomposedRestraints.get_restraint(i)

            if (self.allFlexibleAtoms(r) == 1):
                #add the decomposed restraint to the model
                self.model.add_restraint(r)
                self.model.set_maximum_score(r, maxForceFieldScore)
                self.addParticlesToGroups(r)
            else:
                t = 1
                #self.model.remove_restraint(r)

    #Add all atoms in r to ffParticleGroups; tracks which atoms are restrained by forcefield terms so
    #we don't restrain them in other methods (e.g. excluded volume) -- essentially a manual pair-filter
    def addParticlesToGroups(self, r):
        inputParticles = []
        ps = r.get_input_particles()

        for p in ps:

            if (self.isAtomParticle(p) == 0):  #skip particles that aren't actually atoms (e.g. bond particles)
                continue
            inputParticles.append(p.get_name())

        for firstName in inputParticles:
            for secondName in inputParticles:
                if (firstName == secondName):
                    continue
                self.ffParticleGroups[firstName][secondName] = 1

    #Add excluded volume restraints across close pairs. Still a work in progress as they have been slowing down
    #the code quite a bit and we'll probably want to turn them on only at a defined time
    def addExcludedVolumeRestraints(self):

        restrainedParticles = atomicDominoUtilities.getRestrainedParticles(self.protein, self.model, self.namesToParticles)
        lsc = IMP.container.ListSingletonContainer(restrainedParticles)

        evr = IMP.core.ExcludedVolumeRestraint(lsc, 1, 1)
        self.model.add_restraint(evr)


    #Add lennard jones restraints between flexible atoms if they haven't been added already, and then scale radii of flexible atoms by input scaling factor
    def addFlexFlexLjRestraints(self, scalingFactor):
        if (self.initializedFlexFlexLjRestraints == 0):
            print "adding initial lennard jones restraints between pairs of flexible atoms"
            self.initializedFlexFlexLjRestraints = 1
            leaves = IMP.atom.get_leaves(self.protein)
            counter = 0

            sf = IMP.atom.ForceSwitch(6.0, 7.0)
            ps = IMP.atom.LennardJonesPairScore(sf)
            for i in range(len(leaves)):
                iLeaf = leaves[i]
                for j in range(i + 1, len(leaves)):
                    jLeaf = leaves[j]
                    if (self.flexibleAtoms.has_key(self.quickParticleName(iLeaf)) == 1 and self.flexibleAtoms.has_key(self.quickParticleName(jLeaf)) == 1):
                        ljpr = IMP.core.PairRestraint(ps, [iLeaf, jLeaf], "LennardJones %s_%s" % (i, j))
                        self.model.add_restraint(ljpr)
        chainHierarchies = IMP.atom.get_by_type(self.protein, IMP.atom.CHAIN_TYPE)
        peptideChainHierarchy = None
        print "scaling atomic radii by scaling factor %s" % scalingFactor
        self.ff.add_radii(self.protein)  #reset radii back to original in preparation for rescaling
        for pName in self.flexibleAtoms.keys():
            particle = self.namesToParticles[pName]
            xyzr = IMP.core.XYZR.decorate_particle(particle)
            radius = xyzr.get_radius()
            radius *= scalingFactor
            xyzr.set_radius(radius)

    #add lennard-jones restraints between all fixed atoms and all flexible atoms
    def addFixedFlexibleLjRestraints(self):
        print "adding initial fixed flexible lj restraints"
        fixedAtoms = self.getFixedAtoms()
        sf = IMP.atom.ForceSwitch(6.0, 7.0)
        ps = IMP.atom.LennardJonesPairScore(sf)

        for flexPname in self.flexibleAtoms.keys():
            flexParticle = self.namesToParticles[flexPname]
            for fixedPname in fixedAtoms.keys():
                fixedParticle = self.namesToParticles[fixedPname]
                ljpr = IMP.core.PairRestraint(ps, [fixedParticle, flexParticle], "LennardJones %s_%s" % (flexPname, fixedPname))
                self.model.add_restraint(ljpr)

    #actually do the work to create the DOPE restraint
    def createDopeRestraint(self, pair):
        q0 = self.quickParticleName(pair[0])
        q1 = self.quickParticleName(pair[1])
        #create restraint around this distance, add to model
        prName = "NonBonded %s_%s" % (q0, q1)
        pairRestraint = IMP.core.PairRestraint(self.dopePairScore, pair, prName)

        #print "Added non-bonded restraint between %s and %s" % (q0, q1)
        self.model.add_restraint(pairRestraint)
        maxNonBondedScore = float(self.getParam("max_non_bonded_score"))
        self.model.set_maximum_score(pairRestraint, maxNonBondedScore)
        #initialScore = pairRestraint.evaluate(0)


    #add non-bonded restraints across close pairs. At least one atom in each pair should be flexible
    def addClosePairNonBondedRestraints(self):
        particlePairs = self.getNonBondedPairs()

        nonBondedRestraintType = self.getParam("non_bonded_restraint_type")
        dopeCutoff = float(self.getParam("dope_cutoff"))

        k = IMP.core.Harmonic.get_k_from_standard_deviation(1)

        ps = 0
        if (nonBondedRestraintType == "dope"):
            IMP.atom.add_dope_score_data(self.protein)
            ps = IMP.atom.DopePairScore(dopeCutoff)
        self.dopePairScore = ps
        nonBondedCount = 0
        self.nonBondedPairs = self.initializeParticleNameDict(IMP.atom.get_leaves(self.protein))
        for pair in particlePairs:

            if (self.skipNonBondedPair(pair) == 1):
                continue

            self.createDopeRestraint(pair)

            nonBondedCount += 1
            self.nonBondedPairs[self.quickParticleName(pair[0])][self.quickParticleName(pair[0])] = 1
            self.nonBondedPairs[self.quickParticleName(pair[1])][self.quickParticleName(pair[1])] = 1

        print "Added %s non-bonded restraints" % nonBondedCount
        self.model.evaluate(False)


    def addCompleteNonBondedRestraints(self):
        leaves = IMP.atom.get_leaves(self.protein)
        counter = 0

        for i in range(len(leaves)):
            iLeaf = leaves[i]
            for j in range(i + 1, len(leaves)):
                jLeaf = leaves[j]

                if (self.nonBondedPairs[self.quickParticleName(iLeaf)].has_key(self.quickParticleName(jLeaf)) == 1):
                    continue
                if (self.skipNonBondedPair([iLeaf, jLeaf]) == 1):
                    continue

                self.createDopeRestraint([iLeaf, jLeaf])
                counter += 1
                #print "added complete restraint between %s and %s" % (self.quickParticleName(iLeaf), self.quickParticleName(jLeaf))
        print "added %s restraints for %s particles" % (counter, len(leaves))
        self.model.evaluate(False)


    #lots of output on all restraints
    def writeAllRestraints(self):
        print "write all restraints:"
        for r in IMP.get_restraints([self.model.get_root_restraint_set()]):
            print self.quickRestraintName(r)



    #Read parameterss with md temperature / step schedule and save them
    def initializeMdSchedule(self):
        schedule = self.getParam("md_schedule")
        stages = schedule.split()
        totalSteps = 0
        for stage in stages:
            print "unpacking stage %s" % stage
            [steps, temperature, excludedVolume] = stage.split('_')
            totalSteps += int(steps)
        self.totalMdSteps = totalSteps
        self.mdStages = stages


    #Creates roothandle for hdf5 objects interacting with md trajectory
    #Either we create a new trajectory in this file and read it back in the same run, or we are reading a saved trajectory
    def createMdRootHandle(self):

        mdFileName = ""
        outputDir = self.getParam("output_directory")

        if (self.getParam("read_assignments_from_file") == "no"):
            #create new hdf5 file to which md output will be written
            mdFileName = self.getParam("md_trajectory_output_file")
            fileName = os.path.join(outputDir, mdFileName)
            print "creating rmf file with filename %s" % fileName
            rh = RMF.create_rmf_file(fileName)
            my_kc= rh.add_category("my data");
            IMP.rmf.add_hierarchy(rh, self.protein)

            self.rootHandle = rh
        else:
            #use existing hdf5 file
            mdFileName = self.getParam("saved_md_filename")

            rh = RMF.open_rmf_file(mdFileName)
            IMP.rmf.set_hierarchies(rh, [self.protein])

            self.rootHandle = rh

    def evaluateModel(self):
        return self.model.evaluate(False)

    def loadMdHelpers(self):

        self.createMdRootHandle()

        self.createMdOptimizer()

        self.addMdOptimizerStates()

        self.initializeMdSchedule()

        self.createdMdOptimizer = 1


    #Create the optimizer object for running MD
    def createMdOptimizer(self):
        print "creating md object"
        md = IMP.atom.MolecularDynamics(self.model)
        md.optimize(1)  # hack to get around bug where vx attributes in non-optimized particles are not being set
        md.set_velocity_cap(100.0)
        self.mdOptimizer = md

    def addMdOptimizerStates(self):

        #VelocityScalingOptimizerState
        vsos = IMP.atom.VelocityScalingOptimizerState(IMP.atom.get_leaves(self.protein), 0, 0) #temperature is set later
        vsIndex = self.mdOptimizer.add_optimizer_state(vsos)

        #SaveHierarchyConfigurationOptimizerState
        hdos = IMP.rmf.SaveHierarchyConfigurationOptimizerState([self.protein], self.rootHandle)
        hdos.set_skip_steps(0)
        self.mdOptimizer.add_optimizer_state(hdos)

        self.hdos = hdos
        self.vsos = vsos

    def readTrajectories(self):

        cgFileName = self.getParam("cg_output_file")
        bestCgScore = 10000000
        bestCgScoreFile = ""
        bestCgRmsd = 10000000
        bestCgRmsdFile = ""

        outputDir = self.getParam("output_directory")
        trajectoryFile = self.getParam("md_trajectory_output_file")
        fullFile = os.path.join(outputDir, trajectoryFile)
        rh = RMF.open_rmf_file(fullFile)
        IMP.rmf.set_hierarchies(rh, [self.protein])
        framesToRead = atomicDominoUtilities.getMdIntervalFrames(rh, int(self.getParam("cg_interval")), self.protein)

        if (len(framesToRead) > 0):
            for cgNumber in framesToRead:
                #Open next cg trajectory
                outputDir = self.getParam("output_directory")
                fullCgFileName = os.path.join(outputDir, "%s%s" % (cgFileName, cgNumber))
                rh = RMF.open_rmf_file(fullCgFileName)
                IMP.rmf.set_hierarchies(rh, [self.protein])

                frameCount = IMP.rmf.get_number_of_frames(rh, self.protein)
                IMP.rmf.load_frame(rh, frameCount - 1, self.protein)
                score = self.model.evaluate(False)
                rmsd = self.calculateNativeRmsd(self.flexibleAtoms)
                print "cg number %s has score %s rmsd %s" % (cgNumber, score, rmsd)
                if (score < bestCgScore):
                    bestCgScore = score
                    bestCgScoreFile = fullCgFileName
                if (rmsd < bestCgRmsd):
                    bestCgRmsd = rmsd
                    bestCgRmsdFile = fullCgFileName

            #output best score information
            self.singlePdbResults(bestCgScoreFile, -1, self.getParam("best_cg_score_output_file"))
            self.singlePdbResults(bestCgRmsdFile, -1, self.getParam("best_cg_rmsd_output_file"))
            self.singlePdbResults("%s%s" % (cgFileName, framesToRead[-1]), -1, self.getParam("final_cg_frame_output_file"))
            finalCgRmsd = self.calculateNativeRmsd(self.flexibleAtoms)
            self.bestCgScore = bestCgScore
            self.bestCgRmsd = bestCgRmsd
            self.finalCgRmsd = finalCgRmsd


    def singlePdbResults(self, trajectoryFile, frame, outputPdbFile):

        fullTrajectoryFile = os.path.join(self.getParam("output_directory"), trajectoryFile)
        fullOutputFile = os.path.join(self.getParam("output_directory"), outputPdbFile)
        rh = RMF.open_rmf_file(fullTrajectoryFile)
        IMP.rmf.set_hierarchies(rh, [self.protein])
        if (frame == -1):
            frame = IMP.rmf.get_number_of_frames(rh, self.protein) - 1
        IMP.rmf.load_frame(rh, frame, self.protein)
        IMP.atom.write_pdb(self.protein, fullOutputFile)

    def calculateRmsd(self, otherProtein, flexibleAtoms):
        otherNamesToParticles = atomicDominoUtilities.makeNamesToParticles(otherProtein)
        otherVector = []
        modelVector = []
        for pName in otherNamesToParticles.keys():
            if (flexibleAtoms.has_key(pName) == 0):
                continue
            otherParticle = otherNamesToParticles[pName]
            modelParticle = self.namesToParticles[pName]
            otherVector.append(IMP.core.XYZ.decorate_particle(otherParticle).get_coordinates())
            modelVector.append(IMP.core.XYZ.decorate_particle(modelParticle).get_coordinates())
        rmsd = IMP.atom.get_rmsd(otherVector, modelVector)
        return rmsd

    def calculateNativeRmsd(self, flexibleAtoms):

        if (self.wroteNativeProtein == 0):
            pdbName = self.getParam("native_pdb_input_file")
            self.nativeModel = IMP.Model()
            self.nativeProtein = IMP.atom.read_pdb(pdbName, self.nativeModel, IMP.atom.ATOMPDBSelector())
            self.wroteNativeProtein = 1

        return self.calculateRmsd(self.nativeProtein, flexibleAtoms)


    #Run MD according to the temperature schedule read in from parameters
    def runMolecularDynamics(self):
        self.initializedFlexFlexLjRestraints = 0
        if (self.fixedNonFlexibleAtoms == 0):
            print "error: before running md, must fix non flexible atoms"
            sys.exit()
        stepsSoFar = 0
        for stage in self.mdStages:
            [steps, temperature, excludedVolume] = stage.split('_')
            stepsSoFar += int(steps)
            self.vsos.set_temperature(int(temperature))
            if (excludedVolume == "1"): #add lennard jones restraints between flexible atoms and fixed atoms (don't add between flex atoms)
                self.addFixedFlexibleLjRestraints()
            elif (excludedVolume != "0"):
                if (self.getParam("excluded_volume_type") == "lj"): #add lennard jones restraints between flexible atoms
                    self.addFlexFlexLjRestraints(float(excludedVolume))
                elif (self.getParam("excluded_volume_type") == "ev"):
                    self.addExcludedVolumeRestraints()  # add full excluded volume restraints (just take everything in the protein)
                else:
                    "please set excluded_volume_type to either lj or ev"
                    sys.exit()
            print "running md at temperature %s for %s steps" % (temperature, steps)
            self.mdOptimizer.optimize(int(steps))

            IMP.atom.write_pdb(self.protein, os.path.join(self.getParam("output_directory"), "md_after_%s.pdb" % stepsSoFar))
            print "model score after stage %s is %s" % (stage, self.model.evaluate(False))
        print "done running md"



    def runAllCg(self):
        cgInterval = int(self.getParam("cg_interval"))
        outputDir = self.getParam("output_directory")
        trajectoryFile = self.getParam("md_trajectory_output_file")
        fullFile = os.path.join(outputDir, trajectoryFile)
        print "open rmf %s" % fullFile
        rh = RMF.open_rmf_file(fullFile)
        IMP.rmf.set_hierarchies(rh, [self.protein])
        framesToRead = atomicDominoUtilities.getMdIntervalFrames(rh, cgInterval, self.protein)

        for i in framesToRead:
            cgFileName = "%s%s" % (self.getParam("cg_output_file"), i)
            self.applyCg(i, cgFileName)

    #Apply CG to each step of the saved MD trajectory. Writes results to same file storing the MD trajectory
    def applyCg(self, mdStepStart, cgFileName):

        cgSteps = int(self.getParam("cg_steps"))

        #load md step specified by calling object
        print "apply cg: loading md frame for cg step start %s" % mdStepStart
        IMP.rmf.load_frame(self.rootHandle, mdStepStart, self.protein)

        #create new hdf5 file to which cg output will be written
        outputDir = self.getParam("output_directory")
        fileName = os.path.join(outputDir, cgFileName)
        print "creating cg hdf5 file %s" % fileName
        rh = RMF.create_rmf_file(fileName)
        my_kc= rh.add_category("my data");
        IMP.rmf.add_hierarchy(rh, self.protein)

        #apply cg
        cg = IMP.core.ConjugateGradients(self.model)
        firstScore = self.model.evaluate(False)

        print "creating optimizer state"
        hdos = IMP.rmf.SaveHierarchyConfigurationOptimizerState([self.protein], rh)
        hdos.set_skip_steps(0)
        cg.add_optimizer_state(hdos)  #hdos is the optimizer state writing configurations to disk

        print "running cg"
        cg.optimize(cgSteps)
        secondScore = self.model.evaluate(False)
        print "cg score after md step %s before %s after %s" % (mdStepStart, firstScore, secondScore)
        return secondScore


    #Tell the optimizer not to move atoms we have determined ar fixed
    def fixNonFlexibleAtoms(self):
        if (self.createdMdOptimizer == 0):
            self.createMdOptimizer()
            #print "ERROR: must create md optimizer and optimize once before set_particles_are_optimized is called"
            #sys.exit()

        self.fixedNonFlexibleAtoms = 1
        leaves = IMP.atom.get_leaves(self.protein)
        for leaf in leaves:
            if (self.flexibleAtoms.has_key(self.quickParticleName(leaf)) == 0):
                xyzDecorator = IMP.core.XYZ.decorate_particle(leaf)
                xyzDecorator.set_coordinates_are_optimized(0)


    def writeOutput(self):
        restraintCount = self.model.get_number_of_restraints()
        leaves = IMP.atom.get_leaves(self.protein)
        flexiblePeptideAtoms = 0
        flexibleProteinAtoms = 0
        fixedPeptideAtoms = 0
        fixedProteinAtoms = 0

        for leaf in leaves:
            if (self.flexibleAtoms.has_key(self.quickParticleName(leaf)) == 0):
                if (self.getChain(leaf) == self.getParam("peptide_chain")):
                    fixedPeptideAtoms += 1
                else:
                    fixedProteinAtoms += 1
            else:
                if (self.getChain(leaf) == self.getParam("peptide_chain")):
                    flexiblePeptideAtoms += 1
                else:
                    flexibleProteinAtoms += 1
        print "Flexible peptide atoms: %s" % flexiblePeptideAtoms
        print "Fixed peptide atoms: %s" % fixedPeptideAtoms
        print "Flexible protein atoms: %s" % flexibleProteinAtoms
        print "Fixed protein atoms: %s" % fixedProteinAtoms
        print "Total number of restraints: %s" % restraintCount

    def writeOsOutput(self):

        print "Best cg score: %s" % self.bestCgScore
        print "best cg rmsd: %s" % self.bestCgRmsd
        print "final cg rmsd: %s" % self.finalCgRmsd
