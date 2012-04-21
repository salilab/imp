
import imp_general.representation as representation
import IMP
import IMP.core as core
import IMP.domino as domino
import IMP.container as container
import IMP.em2d as em2d
import IMP.atom as atom
import os

import logging

log = logging.getLogger("restraints")

def get_close_pairs_excluded_volume_restraint(assembly,
                               particles1,
                               particles2,
                               name="pair_score_restraint",
                               distance=2.0,
                               spring_constant=1):
    """
        Restraint that penalizes two sets of particles from interpenetrating
        see get_pair_score_restraint() for the parameters
    """
    m = assembly.get_model()
    marker1 = IMP.Particle(m, "marker1 "+name)
    marker2 = IMP.Particle(m, "marker2 "+name)

    table_refiner = core.TableRefiner()
    table_refiner.add_particle(marker1, particles1)
    table_refiner.add_particle(marker2, particles2)
    # Set a pair score based on penalizing short distances
    score = core.SphereDistancePairScore(
                       core.HarmonicLowerBound(distance, spring_constant))
    pair_score = core.ClosePairsPairScore(score, table_refiner, distance)
    cr = core.PairRestraint(pair_score, IMP.ParticlePair( marker1, marker2 ) )
    return cr


def get_pair_score_restraint(assembly,
                               particles1,
                               particles2,
                               name="pair_score_restraint",
                               distance=4.0,
                               number_of_pairs_scored = 1,
                               spring_constant=1):
    """
        Restraint that penalizes large distances between two sets particles
        The pair_score restraint is set in terms of particles, to
        be able to use coarse representations.
        @param assembly atom.Hierarchy for the assembly
        @param particles1 First set of particles
        @param particles2 Second set of particles
        @param name Name of the restraint
        @param distance Maximum distance tolerated before the function starts
            penalizing
        @param number_of_pairs_scored Number of pairs of particles
            used to compute the scoring function
        @param spring_constant Constat for the harmonic function
        used for scoring
    """
    m = assembly.get_model()
    # This particles are simply attachment points to the actual
    # associate marker 1 with the set of particles 1
    marker1 = IMP.Particle(m, "marker1 "+name)
    marker2 = IMP.Particle(m, "marker2 "+name)

    table_refiner = core.TableRefiner()
    # Add the marker1 and the associated particles to the refiner.
    # When the refiner gets a request for marker1, it returns particles1
    table_refiner.add_particle(marker1, particles1)
    table_refiner.add_particle(marker2, particles2)
    # Penalize distances larger than the distance
    score = core.HarmonicUpperBoundSphereDistancePairScore(distance,
                                                               spring_constant)
    # The score is set with the k closest pairs of particles
    pair_score = core.KClosePairsPairScore(score,
                                    table_refiner,
                                    number_of_pairs_scored)
    # The restraint applies the pair score to the pair marker1, marker2. When
    # the function calls KClosePairsPairScore, the refiner will provide the
    # sets particles1 and particles2
    cr = core.PairRestraint(pair_score, IMP.ParticlePair( marker1, marker2 ) )
    return cr

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


def set_distance_restraint(rb1, rb2, max_distance, name="distance restraint"):
    """
        distance restraint between 2 rigid bodies
        @param rb1 First rigid body
        @param rb2 Second rigid body
        @param max_distance Target distance
    """
    m = rb1.get_model()
    score_function = core.Harmonic(0,1)
    dr = core.DistanceRestraint(score_function,rb1,rb2)
    dr.set_name(name)
    dr.set_maximum_score(max_distance**2)
    m.add_restraint(dr)


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

    images_names = em2d.read_selection_file(images_selection_file)
    imgs_dir, nil = os.path.split(images_selection_file)
    names = [os.path.join(imgs_dir, n) for n in images_names]
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
