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
import resource
import atomicDominoUtilities

class AtomicDomino:

    def __init__(self, model, protein, parameterFileName):
        self.model = model
        self.protein = protein
        self.namesToParticles = atomicDominoUtilities.makeNamesToParticles(protein)
        self.readParameters(parameterFileName)
        self.wroteNativeProtein = 0
        self.maxMem = 0

    def readParameters(self, parameterFileName):
        self.parameters = atomicDominoUtilities.readParameters(parameterFileName)

    def logMemory(self):
        currentMem = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
        currentMem *= 10**-6
        print "log memory: %s" % currentMem
        if (currentMem > self.maxMem):
            self.maxMem = currentMem

    #Get parameter value. Throws dictionary exception if not found
    def getParam(self, paramName):
        paramValue = self.parameters[paramName]
        return paramValue

    def loadDominoHelpers(self):
        self.subsetStateScores = {}
        self.subsetRestraintScores = {}
        self.subsetStateFailures = {}
        self.nodesToAssignmentCount = {}

        self.totalAssignments = 0
        outputDir = self.getParam("output_directory")
        mtreeCytoscapeAssignmentFile = self.getParam("mtree_cytoscape_assignment_file")
        self.mtreeCytoscapeAssignmentFh = open(os.path.join(outputDir, mtreeCytoscapeAssignmentFile), 'w')
        self.mtreeCytoscapeAssignmentFh.write("Assignments\n")

        self.mtNamesToIndices = {}

        self.subsetNamesToAssignmentFiles = {}

    #Simple way to calculate run-time for certain methods and procedures.
    #When reset is 0, compares the previous saved time to the current one
    #and returns the difference (saving the current time too).
    #When reset is 1, just saves the current time.
    def logTimePoint(self, reset):
        newTime = time.time()
        if (reset == 0):
            timeDifference = newTime - self.timePoint
            timeDifference = round(timeDifference, 0)
            self.timePoint = newTime
            return timeDifference
        else:

            self.timePoint = newTime
            return newTime

    def createParticleStatesTable(self):

        for particleName in self.particleInfo.keys():

            statesToCenters = {}
            dataArray = self.particleInfo[particleName]
            for i in range(len(dataArray)):
                [state, center] = dataArray[i]
                statesToCenters[state] = center

            statesList = []
            for stateIndex in sorted(statesToCenters.keys()):
                vector3d = IMP.algebra.Vector3D(statesToCenters[stateIndex])
                statesList.append(vector3d)
                #print "appending particle %s state %s center %s" %  (particleName, stateIndex, vector3d)
            xyzStates = IMP.domino.XYZStates(statesList)
            self.dominoPst.set_particle_states(self.namesToParticles[particleName], xyzStates)

    def quickParticleName(self, particle):
        return atomicDominoUtilities.quickParticleName(particle)

    def filterAssignments(self, assignments, subset, nodeIndex, rssft):

        filteredSubsets = []
        restraintList = []

        #make dependency graph for stats
        for r in IMP.get_restraints([self.model.get_root_restraint_set()]):
            restraintList.append(r)
        dg = IMP.get_dependency_graph(restraintList)

        stateCounter = 0
        passedCounter = 0

        #create hdf5AssignmentContainer

        sFilter = rssft.get_subset_filter(subset, filteredSubsets)
        #check each unique state to see if passes filter
        filteredAssignments = []
        for assignment in assignments:

            if (sFilter == None or sFilter.get_is_ok(assignment)):
                #add to assignment container if it passes
                passedCounter += 1
                filteredAssignments.append(assignment)

            stateCounter += 1
        fraction = (passedCounter * 1.0) / (stateCounter * 1.0)
        print "%s states passed out of %s total for this subset (fraction %s)" % (passedCounter, stateCounter, fraction)
        if (passedCounter == 0):
            print "subset %s had 0 assignments (out of %s) pass. Exiting..." % (subset, stateCounter)
            sys.exit()

        return filteredAssignments

    #Convert the name of the subset to something more readable. Currently returning name as sorted list of atoms
    def quickSubsetName(self, subset):
        cleanName = self.cleanVertexName(subset)
        atomNameList = cleanName.split(" ")
        sortedList = sorted(atomNameList)

        name = " ".join(sortedList)
        return name

    def getNodeIndexList(self):
        return self.mt.get_vertices()

    def getLeafNodeIndexList(self):
        leafNodeIndexList = []
        for subset in self.subsets:
            index = self.getMtIndexForSubset(self.quickSubsetName(subset))
            leafNodeIndexList.append(index)
        return leafNodeIndexList



    #Create Domino sampler and give it all the other domino objects it needs
    def createSampler(self):
        s=IMP.domino.DominoSampler(self.model, self.dominoPst)

        s.set_merge_tree(self.mt)
        filterTables = []
        filterTables.append(self.rssft)
        s.set_subset_filter_tables(filterTables)

        s.set_assignments_table(self.lat)

        if (self.getParam("cross_subset_filtering") == 1):
            s.set_use_cross_subset_filtering(1)

        self.sampler = s

    #Use the model restraint set to get the interaction graph, junction tree, and merge tree, and also get subsets
    #from the junction tree and return them
    def createSubsets(self):
        self.initializeParticleStatesTable()
        ig = IMP.domino.get_interaction_graph([self.model.get_root_restraint_set()], self.dominoPst)
        print "interaction graph:"
        ig.show()
        jt = IMP.domino.get_junction_tree(ig)
        print "junction tree:"
        jt.show()
        self.subsetNamesToSubsets = {}
        mt = IMP.domino.get_balanced_merge_tree(jt)


        #make map of vertex indices to atoms in subsets
        for index in mt.get_vertices():
            subset = mt.get_vertex_name(index)
            subsetName = self.cleanVertexName(subset)
            self.mtNamesToIndices[subsetName] = index

        subsets = IMP.domino.get_subsets(jt)
        for subset in subsets:
            print "created subset %s" % subset
            subsetName = self.quickSubsetName(subset)
            self.subsetNamesToSubsets[subsetName] = subset


        print "merge tree:"
        mt.show()

        self.ig = ig
        self.jt = jt
        self.mt = mt
        self.subsets = subsets

        self.parentSiblingMap = {}
        self.parentSiblingMap[self.mt.get_vertices()[-1]] = {}
        self.createSiblingMap(self.mt.get_vertices()[-1])


    def getMtIndexForSubset(self, subsetName):
        for index in self.mt.get_vertices():
            mtNode = self.mt.get_vertex_name(index)
            mtName = self.cleanVertexName(mtNode)
            mtName = mtName.rstrip()
            mtName = mtName.lstrip()

            mtNameList = mtName.split(" ")
            subsetNameList = subsetName.split(" ")
            mtNameListSorted = sorted(mtNameList)
            subsetNameListSorted = sorted(subsetNameList)
            if (" ".join(subsetNameListSorted) == " ".join(mtNameListSorted)):

                return index
        print "did not find merge tree index for subset name %s" % subsetName
        sys.exit()


    def getDominoParticleNames(self):
        particles = self.dominoPst.get_particles()
        particleNameList = []
        for particle in particles:
            pName = self.quickParticleName(particle)
            particleNameList.append(pName)

        return particleNameList

    def getSubsetNameList(self):
        subsetNameList = []
        for subset in self.subsets:
            name = self.quickSubsetName(subset)
            subsetNameList.append(name)
        return subsetNameList

    def getMtIndexToParticles(self):

        mtIndexToParticles = {}
        allVertices = self.mt.get_vertices()
        for nodeIndex in allVertices:
            subset = self.mt.get_vertex_name(nodeIndex)
            particleList = self.quickSubsetName(subset)
            mtIndexToParticles[nodeIndex] = particleList
            print "createtMtIndexToParticles: index %s is particleList %s " % (nodeIndex, particleList)

        return mtIndexToParticles


    def readTrajectoryFile(self, atomList, rh, frames, scoreOutputFile, skipDomino, flexibleAtoms):

        bestScore = 100000
        bestRmsd = 10000
        bestScoreFrame = 0
        bestRmsdFrame = 0
        scoreOutputFh = open(scoreOutputFile, 'w')
        for i in frames:
            try:
                IMP.rmf.load_frame(rh, i, self.protein)
            except Exception:
                print "execption in loading frame %s" % i
                continue
            score = self.model.evaluate(False)
            leaves = IMP.atom.get_leaves(self.protein)
            #for leaf in leaves:
            #    xyzD = IMP.core.XYZ.decorate_particle(leaf)
            #    print "read trajectory file: coordinates for frame %s particle %s are %s" % (i, self.quickParticleName(leaf), xyzD.get_coordinates())

            rmsd = self.calculateNativeRmsd(flexibleAtoms)
            scoreOutputFh.write("%s\t%s\t%s\n" % (i, score, rmsd))
            if (score < bestScore):
                bestScore = score
                bestScoreFrame = i

            if (rmsd < bestRmsd):
                bestRmsd = rmsd
                bestRmsdFrame = i
            if (skipDomino == 0):
                for atomName in atomList:

                    particle = self.namesToParticles[atomName]

                    #get grid index and coordinates
                    gridIndex = self.snapToGrid(particle)
                    center = self.grid.get_center(gridIndex)
                    pythonCenter = []
                    for coordinate in center:
                        pythonCenter.append(coordinate)

                    #grid indices are mapped to coordinate states. Check if we've seen this grid index
                    if (self.particleStatesSeen[atomName].has_key(gridIndex) == 0):
                        #set this particle state index to size of the dictionary, which effectively increments the index with each new state
                        currentSize = len(self.particleStatesSeen[atomName].keys())
                        self.particleStatesSeen[atomName][gridIndex] = currentSize
                    state = self.particleStatesSeen[atomName][gridIndex]
                    self.particleInfo[atomName].append([state, pythonCenter])
            else:
                print "didn't add domino states due to skip parameters"
        return [bestScore, bestScoreFrame, bestRmsd, bestRmsdFrame]


    #Get the grid index for the given particle. Returns an integer that can be used to get the center
    #of the grid space for discretizing the particle
    def getParticleGridIndex(self, leaf):
        xyzDecorator = IMP.core.XYZ.decorate_particle(leaf)
        coordinates = xyzDecorator.get_coordinates()
        extendedIndex = 0
        extendedIndex = self.grid.get_extended_index(coordinates)
        if (self.grid.get_has_index(extendedIndex) == 0):
            if (self.getParam("grid_type") == "sparse"):  #mostly working with sparse grids now
                self.grid.add_voxel(extendedIndex, 1)
            else:
                self.grid.add_voxel(extendedIndex)

        index = self.grid.get_index(extendedIndex)
        return index


    #Set coordinates of this atom to be the center of the grid space containing it (effectiely discretizing the system)
    def snapToGrid(self, particle):

        index  = self.getParticleGridIndex(particle)
        center = self.grid.get_center(index)
        xyzDecorator = IMP.core.XYZ.decorate_particle(particle)
        xyzDecorator.set_coordinates(center)

        return index

    #Take initial protein and snap every atom to the center of its gridpoint
    def discretizeNativeProtein(self):

        outputDir = self.getParam("output_directory")
        nativeSnappedFile = self.getParam("native_protein_snapped_output_file")

        leaves = IMP.atom.get_leaves(self.protein)
        for leaf in leaves:
            self.snapToGrid(leaf)

        IMP.atom.write_pdb(self.protein, os.path.join(outputDir, nativeSnappedFile))

    #Get particle representing the alpha carbon at the center of the peptide
    def getPeptideCa(self):

        #get all residue indices in the peptide
        residues = IMP.atom.get_by_type(self.protein, IMP.atom.RESIDUE_TYPE)
        peptideIndicesToResidues = {}
        for residueH in residues:
            chain = IMP.atom.get_chain(residueH)
            chainId = chain.get_id()
            residue = residueH.get_as_residue()
            if (chainId == self.getParam("peptide_chain")):
                peptideIndicesToResidues[residue.get_index()] = residue

        #use the min and max residue indices to get the residue in the middle (rounding down)
        minPeptide = min(sorted(peptideIndicesToResidues.keys()))
        maxPeptide = max(sorted(peptideIndicesToResidues.keys()))
        centerPeptide = round(((maxPeptide - minPeptide) / 2 + minPeptide), 0)

        #get the particle corresponding to the ca atom at the middle residue and return it,
        centerName = atomicDominoUtilities.makeParticleName(self.getParam("peptide_chain"), int(centerPeptide), "CA")
        centerParticle = self.namesToParticles[centerName]
        return centerParticle


    #Create grid object that will be used to create discrete states for each particle
    def createGrid(self):

        protBb = IMP.atom.get_bounding_box(self.protein)

        gridSpacing = float(self.getParam("grid_spacing"))
        bufferSpace = float(self.getParam("grid_buffer_space"))

        protBb += bufferSpace #add buffer around grid
        g = 0
        if (self.getParam("grid_type") == "sparse"):
            ca = self.getPeptideCa()
            xyzCa = IMP.core.XYZ.decorate_particle(ca)
            g= IMP.algebra.SparseUnboundedIntGrid3D(gridSpacing, xyzCa.get_coordinates())
        else:
            g = IMP.algebra.DenseDoubleGrid3D(gridSpacing, protBb)

        self.grid = g

    #Create Particle States Table and for each particle in the system, add XYZStates with that particle's
    #initial location
    def initializeParticleStatesTable(self):

        dominoPst = IMP.domino.ParticleStatesTable()
        restrainedParticles = atomicDominoUtilities.getRestrainedParticles(self.protein, self.model, self.namesToParticles)

        for p in restrainedParticles:

            xyzD = IMP.core.XYZ.decorate_particle(p)
            xyz = IMP.core.XYZ(p).get_coordinates()
            xyzStates = IMP.domino.XYZStates([xyz])
            dominoPst.set_particle_states(p, xyzStates)

        self.dominoPst = dominoPst

    def createUniqueLeafAssignments(self, particleNameList, particleInfo):

        size =  len(particleInfo[particleNameList[0]])
        allAssignments = []

        for i in range(size):
            nextAssignment = []
            for particleName in particleNameList:
                dataArray = particleInfo[particleName]
                [state, center] = dataArray[i]
                nextAssignment.append(state)
            allAssignments.append(nextAssignment)

        #make unique assignments to avoid duplicates
        uniqueAssignments = {}
        for assignment in allAssignments:

            stateString = ""
            for index in assignment:
                stateString = stateString + str(index) + "_"
            uniqueAssignments[stateString] = assignment
        finalAssignments = []

        #add all assignments to final list
        for stateString in uniqueAssignments.keys():
            assignmentList = uniqueAssignments[stateString]
            assignment = IMP.domino.Assignment(assignmentList)
            finalAssignments.append(assignment)
        return finalAssignments


    #Read MD trajectory; for each particle, save all unique states, and for each subset, save all assignments
    def readMdTrajectory(self, atomList, flexibleAtoms):

        #open trajectory file
        outputDir = self.getParam("output_directory")
        trajectoryFile = self.getParam("md_trajectory_output_file")
        fullFile = os.path.join(outputDir, trajectoryFile)
        rh = RMF.open_rmf_file(fullFile)
        IMP.rmf.set_hierarchies(rh, [self.protein])
        framesToRead = atomicDominoUtilities.getMdIntervalFrames(rh, int(self.getParam("md_interval")), self.protein)
        print "preparing to read md frames %s" % framesToRead
        #prepare data structures for tracking
        particleInfo = {} #for each particle, list where each entry corresponds to an md step, and its value [domino state, coordinates]
        particleStatesSeen = {} #for each particle, dictionary where the key is the grid index and value is domino state
        for atomName in atomList:
            particle = self.namesToParticles[atomName]
            particleInfo[atomName] = []
            particleStatesSeen[atomName] = {}

        self.particleStatesSeen = particleStatesSeen
        self.particleInfo = particleInfo

        #read trajectory file
        mdScoreOutputFile = os.path.join(outputDir, "%s" % self.getParam("md_score_output_file"))
        [bestMdScore, bestScoreFrame, bestRmsd, bestRmsdFrame] = self.readTrajectoryFile(atomList, rh, framesToRead, mdScoreOutputFile, 0, flexibleAtoms)

        #output best score information
        #print "try loading bad frame"
        self.singlePdbResults(fullFile, bestScoreFrame, self.getParam("best_md_score_output_file"))
        #self.singlePdbResults(fullFile, 10000, self.getParam("best_md_score_output_file"))

        self.singlePdbResults(fullFile, bestRmsdFrame, self.getParam("best_md_rmsd_output_file"))
        self.singlePdbResults(fullFile, -1, self.getParam("final_md_frame_output_file"))

        self.bestMdScore = round(bestMdScore, 2)
        self.bestMdRmsd = round(bestRmsd, 2)
        self.bestMdScoreFrame = bestScoreFrame
        self.bestMdRmsdFrame = bestRmsdFrame

    def readCgTrajectories(self, atomList, flexibleAtoms):

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

        skipCgDomino = int(self.getParam("skip_cg_domino"))

        if (len(framesToRead) > 0):
            for cgNumber in framesToRead:
                #Open next cg trajectory
                outputDir = self.getParam("output_directory")
                fullCgFileName = os.path.join(outputDir, "%s%s" % (cgFileName, cgNumber))
                rh = RMF.open_rmf_file(fullCgFileName)
                IMP.rmf.set_hierarchies(rh, [self.protein])

                #Only look at the bottom 20 frames
                frameCount = IMP.rmf.get_number_of_frames(rh, self.protein)
                cgFrames = []
                startFrameCount = 0
                if (frameCount > 20):
                    startFrameCount = frameCount - 20

                for i in range(startFrameCount, frameCount):
                    cgFrames.append(i)

                #Process trajectory
                cgScoreOutputFile = os.path.join(outputDir, "%s%s" % (self.getParam("cg_score_output_file"), cgNumber))
                [cgScore, cgScoreFrame, cgRmsd, cgRmsdFrame] = self.readTrajectoryFile(atomList, rh, cgFrames, cgScoreOutputFile, skipCgDomino, flexibleAtoms)
                print "cg number %s rmsd %s score %s" % (cgNumber, cgRmsd, cgScore)
                #Update best score
                if (cgScore < bestCgScore):
                    bestCgScore = cgScore
                    bestCgScoreFile = fullCgFileName
                if (cgRmsd < bestCgRmsd):
                    bestCgRmsd = cgRmsd
                    bestCgRmsdFile = fullCgFileName

            #output best score information
            self.singlePdbResults(bestCgScoreFile, -1, self.getParam("best_cg_score_output_file"))
            self.singlePdbResults(bestCgRmsdFile, -1, self.getParam("best_cg_rmsd_output_file"))
            self.singlePdbResults("%s%s" % (cgFileName, framesToRead[-1]), -1, self.getParam("final_cg_frame_output_file"))
            finalCgRmsd = self.calculateNativeRmsd(flexibleAtoms)
            print "final cg rmsd is %s " % finalCgRmsd
        self.bestCgScore = round(bestCgScore, 2)
        self.bestCgRmsd = round(bestCgRmsd, 2)
        self.bestCgScoreFile = bestCgScoreFile
        self.bestCgRmsdFile = bestCgRmsdFile


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

    def calculateTrajectoryRmsd(self, trajectoryFile, trajectoryFrame, flexibleAtoms):
        pdbName = self.getParam("native_pdb_input_file")
        otherModel = IMP.Model()
        otherProtein = IMP.atom.read_pdb(pdbName, self.nativeModel, IMP.atom.ATOMPDBSelector())
        outputDir = self.getParam("output_directory")
        fullFile = os.path.join(outputDir, trajectoryFile)
        print "open calculate traj rmf %s" % fullFile
        rh = RMF.open_rmf_file(fullFile)
        IMP.rmf.set_hierarchies(rh, [otherProtein])
        if (trajectoryFrame == -1):
            trajectoryFrame = IMP.rmf.get_number_of_frames(rh, otherProtein) - 1
        IMP.rmf.load_frame(rh, trajectoryFrame, otherProtein)
        return self.calculateRmsd(otherProtein, flexibleAtoms)


    def createAllSubsetAssignments(self):

        lat = IMP.domino.ListAssignmentsTable()
        rssft = IMP.domino.RestraintScoreSubsetFilterTable(self.model.get_root_restraint_set(), self.dominoPst)

        leafNodeIndexList = self.getLeafNodeIndexList()

        for nodeIndex in leafNodeIndexList:
            #get subset for this leaf
            subset = self.mt.get_vertex_name(nodeIndex)
            particleNameList = []
            for particle in subset:
                particleNameList.append(self.quickParticleName(particle))
            print "creating initial assignments for leaf %s" % nodeIndex
            #use particleInfo to create assignments and filter them
            assignments = self.createUniqueLeafAssignments(particleNameList, self.particleInfo)
            filteredAssignments = self.filterAssignments(assignments, subset, nodeIndex, rssft)

            #add assignemtns to container and listAssignmentTable
            packedAssignmentContainer = IMP.domino.PackedAssignmentContainer()
            for assignment in filteredAssignments:
                packedAssignmentContainer.add_assignment(assignment)
            lat.set_assignments(subset, packedAssignmentContainer)

        self.lat = lat
        self.rssft = rssft

    def runDomino(self):
        root = self.mt.get_vertices()[-1]
        completeAc = self.loadAssignments(root)
        self.completeAc = completeAc

    def loadAssignments(self, nodeIndex):

        children = self.mt.get_out_neighbors(nodeIndex)
        subset = self.mt.get_vertex_name(nodeIndex)
        heapCount = int(self.getParam("heap_count"))
        mine= IMP.domino.HeapAssignmentContainer(heapCount, self.rssft.get_subset_filter(subset, []))
        if len(children)==0:
            print "computing assignments for leaf %s" % nodeIndex

            self.sampler.load_vertex_assignments(nodeIndex, mine)
            print "leaf node %s has %s leaf assignments" % (nodeIndex, mine.get_number_of_assignments())
        else:
            if (children[0] > children[1]):
                children = [children[1], children[0]]
            # recurse on the two children
            firstAc = self.loadAssignments(children[0])
            secondAc = self.loadAssignments(children[1])
            self.logTimePoint(1)
            self.sampler.load_vertex_assignments(nodeIndex, firstAc, secondAc, mine)

            timeDifference = self.logTimePoint(0)
            print "Done Parent %s Assignments %s first child %s second child %s time %s" % (nodeIndex, mine.get_number_of_assignments(), firstAc.get_number_of_assignments(),
                                                                                       secondAc.get_number_of_assignments(), timeDifference)
        self.totalAssignments += mine.get_number_of_assignments()
        self.logMemory()
        return mine




    def writeOutput(self, flexibleAtoms, startTime):
        bestAssignment = -1
        bestDominoScore = 100000
        bestAssignment = 0
        finalAssignments = self.completeAc.get_assignments()
        for assignment in finalAssignments:
            IMP.domino.load_particle_states(self.dominoPst.get_subset(), assignment, self.dominoPst)
            score = self.model.evaluate(False)
            if (score < bestDominoScore):
                bestAssignment = assignment
                bestDominoScore = round(score, 2)
        print "best domino score is %s " % bestDominoScore
        print "best md score is %s" % self.bestMdScore
        print "best md rmsd is %s" % self.bestMdRmsd
        print "best cg score is %s" % self.bestCgScore
        print "best cg rmsd is %s" % self.bestCgRmsd
        print "merge tree contained %s total assignments" % self.totalAssignments

        IMP.domino.load_particle_states(self.dominoPst.get_subset(), bestAssignment, self.dominoPst)
        dominoVsMdRmsd = round(self.calculateTrajectoryRmsd(self.getParam("md_trajectory_output_file"), self.bestMdScoreFrame, flexibleAtoms), 2)
        cg = IMP.core.ConjugateGradients(self.model)
        cg.optimize(100)
        IMP.atom.write_pdb(self.protein, os.path.join(self.getParam("output_directory"), self.getParam("minimum_domino_score_pdb")))

        dominoVsCgRmsd = round(self.calculateTrajectoryRmsd(self.bestCgScoreFile, -1, flexibleAtoms), 2)
        dominoMinimizedScore = round(self.model.evaluate(False), 2)
        dominoRmsd = round(self.calculateNativeRmsd(flexibleAtoms), 2)

        runTime = round(time.time() - startTime, 2)
        print "final domino score (after cg): %s" % dominoMinimizedScore
        print "final domino rmsd: %s" % dominoRmsd
        print "best domino rmsd with best md score: %s" % dominoVsMdRmsd
        print "domino rmsd with best cg score: %s" % dominoVsCgRmsd
        print "Final Results\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s" % (self.bestMdScore, self.bestMdRmsd, self.bestCgScore, self.bestCgRmsd, bestDominoScore, dominoRmsd, dominoMinimizedScore, dominoVsCgRmsd, self.totalAssignments, self.maxMem, runTime)

    ####################
    # Parallel methods
    ####################
    def createSubsetFromParticles(self, particleNames):
        particleNameList = particleNames.split(" ")
        particleList = []
        for particleName in particleNameList:
            particle = self.namesToParticles[particleName]
            particleList.append(particle)

        subset = IMP.domino.Subset(particleList)
        return [subset, particleList]

    def createHdf5AssignmentContainer(self, index, particleNames, read):
        root = self.getAssignmentContainerRoot(index, read)
        print "got root for index %s" % index
        dataset = 0
        if (read == 1):
            dataset = root.get_child_index_data_set_2d(str(index))
        else:
            dataset = root.add_child_index_data_set_2d(str(index))
        print "added child index dataset"
        #firstDataset.set_size([
        [subset, particleOrder] = self.createSubsetFromParticles(particleNames)
        print "created subset for index %s" % index
        hdf5Ac = IMP.domino.create_assignments_container(dataset, subset, particleOrder)
        print "returning from create"
        order = IMP.domino.get_order(subset, particleOrder)
        for nextInt in order:
            print "next int is %s" % nextInt
        return [subset, hdf5Ac]

    def loadAssignmentsParallel(self, nodeIndex, particleInfo, mtIndexToNodeInfo, mtIndexToSubsetOrder, mtIndexToParticles):
        IMP.base.set_log_level(IMP.WARNING)

        if (mtIndexToNodeInfo[nodeIndex].has_key("firstChild") == 0):
            print "writing file for leaf index %s" % nodeIndex
            return self.createAssignmentsParallel(particleInfo, nodeIndex, mtIndexToParticles)

        else:
            beginTime = self.logTimePoint(1)
            firstChildIndex = mtIndexToNodeInfo[nodeIndex]["firstChild"]
            [firstSubset, firstAc] = self.createHdf5AssignmentContainer(firstChildIndex, mtIndexToSubsetOrder[firstChildIndex], 1)
            firstAcCreateTime = self.logTimePoint(0)

            secondChildIndex = mtIndexToNodeInfo[nodeIndex]["secondChild"]
            [secondSubset, secondAc] = self.createHdf5AssignmentContainer(secondChildIndex, mtIndexToSubsetOrder[secondChildIndex], 1)
            secondAcCreateTime = self.logTimePoint(0)

            print "getting assignments for nodeIndex %s first child %s second child %s" % (nodeIndex, firstChildIndex, secondChildIndex)
            firstChildParticles = mtIndexToSubsetOrder[firstChildIndex]
            secondChildParticles = mtIndexToSubsetOrder[secondChildIndex]
            myParticles = mtIndexToParticles[nodeIndex]

            print "first child particles %s\nsecond child particles %s\nmy particles; %s\n" % (firstChildParticles, secondChildParticles, myParticles)
            for p in firstSubset:
                print "next particle in first subset: %s" % self.quickParticleName(p)


            for p in secondSubset:
                print "next particle in second subset: %s" % self.quickParticleName(p)

            for assignment in firstAc.get_assignments():
                print "next assignment for first child %s: %s" % (firstChildIndex, assignment)


            for assignment in secondAc.get_assignments():
                print "next assignment for second child %s: %s" % (secondChildIndex, assignment)

            [mySubset, myAc] = self.createHdf5AssignmentContainer(nodeIndex, mtIndexToParticles[nodeIndex], 0)
            print "done creating hdf5"
            prepTime = self.logTimePoint(0)
            rssft = IMP.domino.RestraintScoreSubsetFilterTable(self.model.get_root_restraint_set(), self.dominoPst)
            rssf = rssft.get_subset_filter(mySubset, [])

            #heapAc = IMP.domino.HeapAssignmentContainer(1000, rssf)

            IMP.domino.load_merged_assignments(firstSubset, firstAc, secondSubset, secondAc, [rssft], myAc)

            heapTime = self.logTimePoint(0)
            #myAc.add_assignments(heapAc.get_assignments())
            addTime = self.logTimePoint(0)
            for assignment in myAc.get_assignments():
                print "loadAssignmentsParallel next assignment for %s: %s" % (nodeIndex, assignment)
            doneTime = self.logTimePoint(0)
            firstChildCount = firstAc.get_number_of_assignments()
            secondChildCount = secondAc.get_number_of_assignments()
            print "first count: %s second count: %s begin: %s firstAc: %s secondAc: %s  prep: %s heap: %s add: %s done: %s" % (firstChildCount, secondChildCount, beginTime, firstAcCreateTime, secondAcCreateTime,  prepTime, heapTime, addTime, doneTime)
            subsetOrder = self.getSubsetOrderList(mySubset)
            return subsetOrder


    def createAssignmentsParallel(self, particleInfo, nodeIndex, mtIndexToParticles):


        subsetName = mtIndexToParticles[nodeIndex]
        print "starting assignments parallel leaf index %s subset name %s" % (nodeIndex, subsetName)
        [subset, particleList] = self.createSubsetFromParticles(subsetName)


        particleNameList = subsetName.split(" ")


        #create assignment by reading states
        finalAssignments = self.createUniqueLeafAssignments(particleNameList, particleInfo)


        #lat = IMP.domino.ListAssignmentsTable()
        #lat.set_assignments(subset, finalAssignmentContainer)
        #self.sampler.set_assignments_table(lat)

        #hdf5AssignmentContainer = IMP.domino.HDF5AssignmentContainer(dataset, subset, self.dominoPst.get_particles(), subsetName)
        rssft = IMP.domino.RestraintScoreSubsetFilterTable(self.model.get_root_restraint_set(), self.dominoPst)
        filteredAssignments = self.filterAssignments(finalAssignments, subset, nodeIndex, rssft)
        root = self.getAssignmentContainerRoot(nodeIndex, 0)
        dataset= root.add_child_index_data_set_2d(str(nodeIndex))
        dataset.set_size([0, len(subset)])

        hdf5AssignmentContainer = IMP.domino.create_assignments_container(dataset, subset, particleOrder)
        for assignment in filteredAssignments:
            hdf5AssignmentContainer.add_assignment(assignment)
        for assignment in hdf5AssignmentContainer.get_assignments():
            print "hdf5 assignment container node %s next assignmet %s" % (nodeIndex, assignment)

        #self.checkAssignments(subset, nodeIndex, particleOrder)
        subsetOrder = self.getSubsetOrderList(subset)
        print "leaf node returning with order %s" % subsetOrder
        return subsetOrder






    #Write pymol session files for the interactions across atoms and all subsets
    def writePymolData(self):

        outputDir = self.getParam("output_directory")
        geometry = IMP.domino.get_interaction_graph_geometry(self.ig)
        pymolInteractions = self.getParam("pymol_interactions_file")
        w= IMP.display.PymolWriter(os.path.join(outputDir, pymolInteractions))

        for gg in geometry:
            w.add_geometry(gg)

        pymolSubsets = self.getParam("pymol_subsets_file")
        geometry = IMP.domino.get_subset_graph_geometry(self.jt)
        w= IMP.display.PymolWriter(os.path.join(outputDir, pymolSubsets))
        for gg in geometry:
            w.add_geometry(gg)


    #Clean the default name of the vertex (in brackets and with each atom contained in quotes) and return a string where [] and " are removed
    def cleanVertexName(self, vertexName):

        nodeRe = re.compile('Subset\(\[(.*?)\s*\]')   # not sure if any vertices still have a Subset prefix but keeping anyway
        secondNodeRe = re.compile('\[(.*?)\s*\]')   #atom name
        node = nodeRe.search(str(vertexName))
        secondNode = secondNodeRe.search(str(vertexName))
        vertexNameFinal = ""
        foundName = 0
        if node:
            foundName = node.group(1)
        if secondNode:
            foundName = secondNode.group(1)
        vertexNameFinal = foundName.replace('"', '')
        return vertexNameFinal

    def getSubsetOrderList(self, subset):
        subsetOrderList = []
        for particle in subset:
            name = self.quickParticleName(particle)
            subsetOrderList.append(name)
        subsetOrder = " ".join(subsetOrderList)
        return subsetOrder

    def checkAssignments(self, subset, nodeIndex, particleOrder):
        print "reading back in assignments for leaf index %s" % nodeIndex
        root = self.getAssignmentContainerRoot(nodeIndex, 1)
        dataset = root.get_child_index_data_set_2d(str(nodeIndex))
        hdf5 = IMP.domino.create_assignments_container(dataset, subset, particleOrder)
        for assignment in hdf5.get_assignments():
            print "leaf index %s read back in %s" % (nodeIndex, assignment)

    def createSamplerLite(self):
        s=IMP.domino.DominoSampler(self.model, self.dominoPst)

        if (self.getParam("cross_subset_filtering") == 1):
            s.set_use_cross_subset_filtering(1)

        self.sampler = s

    def createSiblingMap(self, parentIndex):

        children = self.mt.get_out_neighbors(parentIndex)
        if (len(children) > 0):
            firstChild = children[0]
            secondChild = children[1]
            self.parentSiblingMap[firstChild] = {}
            self.parentSiblingMap[firstChild]["sibling"] = secondChild
            self.parentSiblingMap[firstChild]["parent"] = parentIndex

            self.parentSiblingMap[secondChild] = {}
            self.parentSiblingMap[secondChild]["sibling"] = firstChild
            self.parentSiblingMap[secondChild]["parent"] = parentIndex
            print "created map for parent %s first child %s second child %s" % (parentIndex, firstChild, secondChild)

            self.parentSiblingMap[parentIndex]["firstChild"] = firstChild
            self.parentSiblingMap[parentIndex]["secondChild"] = secondChild

            self.createSiblingMap(firstChild)
            self.createSiblingMap(secondChild)

    def getMtIndexToNodeInfo(self):
        return self.parentSiblingMap

    def getLeafParentNodeIndexList(self):
        leafParentNodeIndexList = {}
        leafNodeIndexList = self.getLeafNodeIndexList()
        for leafIndex in leafNodeIndexList:
            parent = self.parentSiblingMap[leafIndex]["parent"]
            leafParentNodeIndexList[parent] = 1
        return leafParentNodeIndexList

    def getMtIndexToNameList(selt):
        mtIndexToNames = {}
        for index in self.mt.get_vertices():
            name = self.mt.get_vertex_name(index)
            mtIndexToNames[index] = name
        return mtIndexToNames


    def getAssignmentContainerRoot(self, subsetIndex, read):
        outputDir = self.getParam("output_directory")
        filePrefix = self.getParam("subset_assignment_output_file")
        assignmentFileName = os.path.join(outputDir, "%s%s" % (filePrefix, subsetIndex))
        print "creating hdf5 file with name %s" % assignmentFileName
        root = 0
        if (read == 1):
            root = RMF.open_hdf5_file(assignmentFileName)
        else:
            root= RMF.create_hdf5_file(assignmentFileName)
        return root

    ##########
    #Begin Cytoscape Methods
    ##########

    def writeVisualization(self):

        self.writeCytoscapeIgInput()
        self.writeCytoscapeJtInput()
        self.writeCytoscapeMtInput()
        self.writeCytoscapeScripts()
        self.writePymolData()

    def writeCytoscapeScripts(self):
        outputDir = self.getParam("output_directory")
        mTreeCytoscapeInput = self.getParam("mtree_cytoscape_input_file")
        mTreeCytoscapeAssignments = self.getParam("mtree_cytoscape_assignment_file")
        mTreeCytoscapeAtomChains = self.getParam("mtree_cytoscape_atom_chain_file")
        mTreeCytoscapeAtomSummary = self.getParam("mtree_cytoscape_atom_summary_file")

        mTreeCytoscapeScript = self.getParam("mtree_cytoscape_script")
        mTreeFh = open(os.path.join(outputDir, mTreeCytoscapeScript), 'w')
        mTreeFh.write("network import file=%s\n" % os.path.join(outputDir, mTreeCytoscapeInput))
        mTreeFh.write("node import attributes file=\"%s\"\n" % os.path.join(outputDir, mTreeCytoscapeAssignments))
        mTreeFh.write("node import attributes file=\"%s\"\n" % os.path.join(outputDir, mTreeCytoscapeAtomSummary))
        mTreeFh.write("node import attributes file=\"%s\"\n" % os.path.join(outputDir, mTreeCytoscapeAtomChains))
        mTreeFh.write("layout jgraph-tree\n")


    def getGraphStructure(self, graph, fileName, separator):
        #write junction tree to file
        graphLogWrite = open(fileName, 'w')
        graph.show(graphLogWrite)
        graphLogWrite.close()

        #read file
        graphLogRead = open(fileName, 'r')

        nodeRe = re.compile('^(\d+)\[label\=\"\[*(.*?)\s*\]*\"')   #atom name
        separatorEscape = "\\" + separator
        edgeString = "^(\d+)\-%s(\d+)" % separatorEscape
        edgeRe = re.compile(edgeString)

        nodesToNodes = {} #keys: source node, value: target node (track edges)
        nodesToNames = {} # keys: node number, value; string parsed from file

        for line in graphLogRead:

            #search for nodes
            node = nodeRe.search(line)
            if node:
                nodeNumber = node.group(1)
                atomString = node.group(2)
                nodesToNames[nodeNumber] = atomString
                continue

            #search for edges
            edge = edgeRe.search(line)
            if edge:
                firstNode = edge.group(1)
                secondNode = edge.group(2)
                firstNodeDict = {}
                if (nodesToNodes.has_key(firstNode)):
                    firstNodeDict = nodesToNodes[firstNode]
                firstNodeDict[secondNode] = 1
                nodesToNodes[firstNode] = firstNodeDict

        return [nodesToNames, nodesToNodes]

    def writeEdgeFile(self, nodesToNodes, edgeFileName):
        #write edge file
        outputDir = self.getParam("output_directory")
        graphInputFile = open(os.path.join(outputDir, edgeFileName), 'w')
        for firstNode in nodesToNodes.keys():
            nodeDict = nodesToNodes[firstNode]
            for secondNode in nodeDict.keys():
                graphInputFile.write("%s ttt %s\n" % (firstNode, secondNode))
        graphInputFile.close()

    def writeCytoscapeIgInput(self):
        outputDir = self.getParam("output_directory")
        igOutputFile = self.getParam("ig_output_file")
        [nodesToNames, nodesToNodes] = self.getGraphStructure(self.ig, os.path.join(outputDir, igOutputFile), "-")

        self.writeEdgeFile(nodesToNodes, self.getParam("ig_cytoscape_input_file"))

        #write residue numbers for each node
        igResiduesFile = self.getParam("ig_cytoscape_residues_file")
        peptideChain = self.getParam("peptide_chain")

        subsetResidueFile = open(os.path.join(outputDir, igResiduesFile), 'w')
        subsetResidueFile.write("ResidueNumber\n")
        for nodeNumber in nodesToNames.keys():
            nodeName = nodesToNames[nodeNumber]

            [nodeChain, residueNumber, nodeAtom] = atomicDominoUtilities.getAtomInfoFromName(nodeName)
            if (nodeChain == peptideChain): #peptideAtom
                subsetResidueFile.write("%s = %s\n" % (nodeNumber, residueNumber))
            else:
                subsetResidueFile.write("%s = 100\n" % nodeNumber)  #for now just write arbitrary number to designate as protein atom


    def writeCytoscapeMtInput(self):
        outputDir = self.getParam("output_directory")
        mTreeOutputFile = self.getParam("mtree_output_file")
        [nodesToNames, nodesToNodes] = self.getGraphStructure(self.mt, os.path.join(outputDir, mTreeOutputFile), ">")

        self.writeEdgeFile(nodesToNodes, self.getParam("mtree_cytoscape_input_file"))
        self.writeNodeNameAttributes(nodesToNames, self.getParam("mtree_cytoscape_atom_name_file"), self.getParam("mtree_cytoscape_atom_summary_file"),
                                     self.getParam("mtree_cytoscape_atom_chain_file"))

    def writeCytoscapeJtInput(self):

        outputDir = self.getParam("output_directory")
        jTreeOutputFile = self.getParam("jtree_output_file")
        [nodesToNames, nodesToNodes] = self.getGraphStructure(self.jt, os.path.join(outputDir, jTreeOutputFile), "-")

        self.writeEdgeFile(nodesToNodes, self.getParam("jtree_cytoscape_input_file"))

        self.writeNodeNameAttributes(nodesToNames, self.getParam("jtree_cytoscape_atom_name_file"), self.getParam("jtree_cytoscape_atom_summary_file"),
                                     self.getParam("jtree_cytoscape_atom_chain_file"))

        #write edge weight file -- weights are number of shared particles across nodes
        jtreeCytoscapeEdgeFile = self.getParam("jtree_cytoscape_edge_file")
        edgeWeightFile = open(os.path.join(outputDir, jtreeCytoscapeEdgeFile), 'w')
        edgeWeightFile.write("SubsetOverlap (class=Integer)\n")
        for firstNode in nodesToNodes.keys():
            nodeDict = nodesToNodes[firstNode]
            for secondNode in nodeDict.keys():
                firstNodeAtoms = nodesToNames[firstNode].split(" ")
                secondNodeAtoms = nodesToNames[secondNode].split(" ")
                intersection = [val for val in firstNodeAtoms if val in secondNodeAtoms]
                edgeWeightFile.write("%s (pp) %s = %s\n" % (firstNode, secondNode, len(intersection)))
        edgeWeightFile.close()

    def getAtomTypeCounts(self, atomNames):

        atomNames = atomNames.lstrip('[')
        atomNames = atomNames.rstrip(']')
        atomNames = atomNames.rstrip()

        atomList = atomNames.split(" ") #atom names
        peptideAtomCount = 0
        proteinAtomCount = 0
        for atom in atomList:
            [chain, residue, atom] = atomicDominoUtilities.getAtomInfoFromName(atom)
            if (chain == self.getParam("peptide_chain")):
                peptideAtomCount += 1
            else:
                proteinAtomCount += 1
        return [peptideAtomCount, proteinAtomCount]


    def writeNodeNameAttributes(self, nodesToNames, atomNameFile, atomSummaryFile, atomChainFile):
        #write attribute file (atom names for each node)
        outputDir = self.getParam("output_directory")
        subsetAtomNameFile = open(os.path.join(outputDir, atomNameFile), 'w')
        subsetAtomSummaryFile = open(os.path.join(outputDir, atomSummaryFile), 'w')
        subsetAtomChainFile = open(os.path.join(outputDir, atomChainFile), 'w')

        subsetAtomNameFile.write("Atom names\n")
        subsetAtomSummaryFile.write("Atom Summary\n")
        subsetAtomChainFile.write("Atom chain\n")
        for node in nodesToNames.keys():
            atomNames =  nodesToNames[node]
            subsetAtomNameFile.write("%s = %s\n" % (node, atomNames))
            [peptideAtomCount, proteinAtomCount] = self.getAtomTypeCounts(atomNames)
            #Number of protein and peptide atoms in each subset
            subsetAtomSummaryFile.write("%s = %sp %sl\n" % (node, proteinAtomCount, peptideAtomCount))

            #whether each subset has protein, peptide, or a mix of atoms
            if (proteinAtomCount == 0):
                subsetAtomChainFile.write("%s = 1\n" % node)
            elif (peptideAtomCount == 0):
                subsetAtomChainFile.write("%s = 2\n" % node)
            else:
                subsetAtomChainFile.write("%s = 3\n" % node)

        subsetAtomChainFile.close()
        subsetAtomSummaryFile.close()
        subsetAtomNameFile.close()


    ##########
    #End Cytoscape Methods
    ##########
