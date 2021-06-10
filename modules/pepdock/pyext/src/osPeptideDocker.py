import sys
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
