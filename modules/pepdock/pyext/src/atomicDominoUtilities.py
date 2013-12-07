import IMP
import re
import sys
import os


def readParameters(parameterFileName):

    parameters = {}
    parameterFh = open(parameterFileName)
    blankRe = re.compile('^\s*$')
    skipRe = re.compile('^\#')
    for line in parameterFh:

        blankLine = blankRe.search(line)
        if blankLine:
            continue
        skipLine = skipRe.search(line)
        if skipLine:
            continue

        line = line.rstrip("\n\r")
        [paramName, paramValue] = line.split('\t')
        print "parameter %s:\t%s" % (paramName, paramValue)
        parameters[paramName] = paramValue

    parameterFh.close()
    return parameters

# Represents an atom particle as a string (contains its chain ID, residue ID, and atom name)
# Other methods parse the name so if the name format changes they need to be updated
#getPeptideCa(); writeCytoscapeIgInput(); getAtomTypeCounts()


def quickParticleName(particle):
    atomDecorator = IMP.atom.Atom.decorate_particle(particle)
    atomName = atomDecorator.get_atom_type()

    atomDecorator = IMP.atom.Atom.decorate_particle(particle)
    residue = IMP.atom.get_residue(atomDecorator)
    residueNumber = residue.get_index()

    chain = IMP.atom.get_chain(residue)
    chainId = chain.get_id()

    name = "%s_%s_%s" % (chainId, residueNumber, atomName)
    nameFinal = name.replace('"', '')

    return nameFinal

# make dictionary mapping particle name to the object it represents


def makeNamesToParticles(protein):

    leaves = IMP.atom.get_leaves(protein)
    namesToParticles = {}
    for leaf in leaves:
        name = quickParticleName(leaf)
        namesToParticles[name] = leaf
        leaf.set_name(name)
    return namesToParticles


# get the chain id, residue id, and atom name from the particle name -- slightly cleaner in that if we change the name
# format, we don't have to change all the methods that rely on that format
def getAtomInfoFromName(particleName):
    [chain, residue, atom] = particleName.split("_")
    return [chain, residue, atom]

# quick way to get formatted name


def makeParticleName(chain, residueNumber, atomName):
    return "%s_%s_%s" % (chain, residueNumber, atomName)


# Check if this particle is an atom particle or a restraint
def isAtomParticle(p):
    # hack; need to distinguish from non-atom particle
    if (p.get_name().find('_') == -1):
        return 0
    else:
        return 1


# Get particles in model that are contained in model's restraints
def getRestrainedParticles(protein, model, namesToParticles):
    leaves = IMP.atom.get_leaves(protein)

    particleDict = {}
    count = 0
    for r in IMP.get_restraints([model.get_root_restraint_set()]):
        count += 1
        ps = r.get_input_particles()

        for p in ps:
            if (isAtomParticle(p) == 0):
                continue
            name = quickParticleName(p)
            # use dictionary keyed on names to avoid duplication
            particleDict[name] = 1
        score = r.evaluate(0)

    restrainedParticles = []
    for name in particleDict.keys():
        p = namesToParticles[name]
        restrainedParticles.append(p)

    return restrainedParticles


def getMdIntervalFrames(rh, interval, protein):
    frames = []
    if (interval > -1):
        frameCount = IMP.rmf.get_number_of_frames(rh, protein)
        for i in range(1, frameCount, interval):
            frames.append(i)
    return frames
