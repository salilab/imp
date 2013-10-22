"""@namespace IMP.em2d.restraints
   Utility functions to handle restraints.
"""

import IMP
import IMP.core as core
import IMP.domino as domino
import IMP.container as container
import IMP.em2d as em2d
import IMP.em2d.imp_general.representation as representation
import IMP.atom as atom
import IMP.base as base
import os

import logging

log = logging.getLogger("restraints")

def get_connectivity_restraint(particles, distance=10.,
                                    n_pairs=1, spring_constant=1):
    """
        Set a connectivity restraint for the leaves of a set of particles

        The intended use is that the each particle is a hierarchy. Each
        hierarchy contains leaves that are atoms, or particles
        that are a coarse representation of a molecule
    """
    score = core.HarmonicUpperBoundSphereDistancePairScore(distance,
                                                           spring_constant)
    refiner = IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
    # score based on the one pair (the closest leaves)
    pair_score = IMP.core.KClosePairsPairScore(score, refiner, n_pairs)
    cr = IMP.core.ConnectivityRestraint(pair_score)
    cr.set_particles(particles)
    return cr


def get_em2d_restraint( assembly,
                        images_selection_file,
                        restraint_params,
                        mode="fast",
                        n_optimized=1):
    """ Sets a restraint for comparing the model to a set of EM images
    """
    model = assembly.get_model()
    # Setup the restraint
    sc = em2d.EM2DScore()
    r = em2d.Em2DRestraint()
    r.setup(sc, restraint_params)
    names = em2d.read_selection_file(images_selection_file)
    names = [base.get_relative_path(images_selection_file, x) for x in names]
    log.debug("names of the images %s", names)
    srw = em2d.SpiderImageReaderWriter()
    imgs = em2d.read_images(names, srw)
    r.set_images(imgs)

    ps = atom.get_leaves(assembly)
    lsc = container.ListSingletonContainer(ps)
    r.set_particles(lsc)

    if (mode == "coarse"):
        r.set_coarse_registration_mode(True)
    elif (mode == "fast"):
        r.set_fast_mode(n_optimized)
    elif(mode == "complete"):
        pass
    else:
        raise ValueError("Em2DRestraint mode not recognized")
    return r
