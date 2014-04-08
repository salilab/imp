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
import peptideDocker
import atomicDomino

parameterFileName = sys.argv[1]

startTime = time.time()

p = peptideDocker.PeptideDocker(parameterFileName)

p.createModel()

p.loadHelpers()

p.initDof()

p.addForceFieldRestraints()
p.setInitialPositions()
p.addClosePairNonBondedRestraints()

d = atomicDomino.AtomicDomino(p.getModel(), p.getProtein(), parameterFileName)

d.loadDominoHelpers()

d.createSubsets()
d.writeVisualization()
p.logTime("Created subsets")
p.addCompleteNonBondedRestraints()

p.logTime("Added complete nonbonded restraints")


# p.addExcludedVolume()
p.logTime("Setup")

p.runMolecularDynamics()

p.logTime("Run MD")

p.runAllCg()

p.logTime("Run CG")

d.createGrid()

d.discretizeNativeProtein()

particleNameList = d.getDominoParticleNames()

flexibleAtoms = p.getFlexibleAtoms()

d.readMdTrajectory(particleNameList, flexibleAtoms)

d.readCgTrajectories(particleNameList, flexibleAtoms)

p.logTime("Read Trajectory")

d.createParticleStatesTable()

p.logTime("create particle states table")

d.createAllSubsetAssignments()

p.logTime("Create Leaf assignments")

d.createSampler()

d.runDomino()

p.logTime("Ran Domino")

p.writeOutput()

p.outputTimes()

d.writeOutput(flexibleAtoms, startTime)
