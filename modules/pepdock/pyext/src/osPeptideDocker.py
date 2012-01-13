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

parameterFileName = sys.argv[1]

p = peptideDocker.PeptideDocker(parameterFileName)

p.createModel()

p.loadHelpers()

p.initDof()

p.addForceFieldRestraints()

p.addClosePairNonBondedRestraints()

p.addCompleteNonBondedRestraints()

p.setInitialPositions()

p.logTime("Setup")

p.runMolecularDynamics()

p.logTime("Run MD")

p.runAllCg()

p.logTime("Run CG")

p.readTrajectories()

p.writeOutput()

p.writeOsOutput()

p.outputTimes()
