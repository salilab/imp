"""@namespace IMP.EMageFit.monte_carlo
   Classes for Monte Carlo sampling using rigid body relative moves.
"""

import IMP
import IMP.core as core
import IMP.em2d as em2d
import IMP.algebra as alg
import IMP.EMageFit.imp_general.representation as representation
import IMP.EMageFit.imp_general.io as io
import time
import logging
log = logging.getLogger("MonteCarloParticleSates")


#

# Generate possible configurations for the rigid bodies of the
# elements of a complex applying MonteCarlo

#


def set_random_seed(index):
    """
        initializing the random number generator manually is useful for the
        jobs in the cluster.
        @param index Seed for the random number generator
    """
    if index != -1:
        log.debug("Seed for the random number generator: %s", index)
        IMP.random_number_generator.seed(index)
    else:
        IMP.random_number_generator.seed(int(time.time()))


class MonteCarloRelativeMoves:

    """
        Class to do Monte Carlo sampling by using as the set of movements
        relative positions between rigid bodies
    """

    def __init__(self, model, rigid_bodies, anchored):
        log.info("Setting MonteCarloRelativeMoves")
        self.model = model
        self.rbs = rigid_bodies
        self.components = []
        self.best_models = []
        self.anchored = anchored
        self.parent_rbs = []
        # triplets with the information to build a relative mover using the
        # results from docking with HEX
        self.dock_transforms = None
        self.non_relative_move_prob = 0.1

        log.debug("Anchored components %s", self.anchored)
        T = alg.Transformation3D(alg.get_identity_rotation_3d(),
                                 alg.Vector3D(0., 0., 0.))
        origin = alg.ReferenceFrame3D(T)
        for rb in self.rbs:
            rb.set_reference_frame(origin)

    def set_temperature_pattern(self, temperatures, iterations, cycles):
        """
            Set the temperature pattern to use during the Monte Carlo
            optimizations.
            @param temperatures  List of temperature values
            @param iterations List with the iterations to do for each of the
                temperatures.
            @param cycles Number of repetitions of the
                pattern of temperatures and iterations
        """
        self.temperatures = temperatures
        self.iterations = iterations
        self.cycles = cycles
        log.debug("Temperatures %s", self.temperatures)
        log.debug("Iterations %s", self.iterations)
        log.debug("Cycles %s", self.cycles)

    def set_movers(self, max_translation, max_rotation):
        log.info("Setting movers for subunits")
        self.movers = []
        for is_anchored, rb in zip(self.anchored, self.rbs):
            if not is_anchored:
                self.movers.append(core.RigidBodyMover(rb, max_translation,
                                                       max_rotation))

    def set_relative_movers(self, max_translation, max_rotation):
        """
            Generate the relative models form the transforms. The transforms
            is a list with triplets [id1, id2, transform_file]
            @param max_translation Maximum translation distance allowed for
                the moves
            @param max_rotation Maximum rotation angle allowed for the moves
        """
        log.info("Setting relative movers")
        self.movers = []
        relative_movers = []
        relative_names = []
        for d in self.dock_transforms:
            rb_id = representation.get_rb_name(d[1])
            if rb_id not in relative_names:
                log.debug("Checking for %s", rb_id)
                rb_lig = representation.get_rigid_body(self.rbs, rb_id)
                rb_lig.set_coordinates_are_optimized(True)
                mv = em2d.RelativePositionMover(rb_lig, max_translation,
                                                max_rotation)
                relative_movers.append(mv)
                relative_names.append(rb_id)
                log.debug("added a RelativePositionMover for %s", rb_id)
            i = relative_names.index(rb_id)
            relative_movers[i].set_random_move_probability(
                self.non_relative_move_prob)
            rb_rec = representation.get_rigid_body(
                self.rbs, representation.get_rb_name(d[0]))
            rb_rec.set_coordinates_are_optimized(True)
            log.debug("Reference added for %s: %s. ref. frame %s ",
                      rb_id, rb_rec.get_name(), rb_rec)
            Tis = io.read_transforms(d[2])
            relative_movers[i].add_internal_transformations(rb_rec, Tis)
        # add regular movers for the rigid bodies that are neither moved
        # anchored nor moved relative to others
        regular_movers = []
        for is_anchored, rb in zip(self.anchored, self.rbs):
            if not is_anchored:
                name = rb.get_name()
                if name not in relative_names:
                    rb.set_coordinates_are_optimized(True)
                    log.debug("adding a RigidBodyMover for %s", name)
                    mv = core.RigidBodyMover(rb, max_translation, max_rotation)
                    regular_movers.append(mv)
        self.movers = regular_movers
        self.movers += relative_movers

    def set_moving_parameters(self, max_translations, max_rotations):
        self.max_translations = max_translations
        self.max_rotations = max_rotations
        log.debug("Maximum translations %s", self.max_translations)
        log.debug("Maximum rotations %s", self.max_rotations)

    def set_scoring_function(self, scoring_function):
        """Set the scoring function to be used by MonteCarlo."""
        self._scoring_function = scoring_function

    def run_monte_carlo(self):
        """
            Run MonteCarlo sampling to generate possible states for DOMINO
        """
        t0 = time.time()
        log.info("Running MonteCarlo")
        mc = core.MonteCarlo(self.model)
        mc.set_scoring_function(self._scoring_function)
        mc.set_return_best(True)
        for i in range(self.cycles):
            log.info("Cycle: %s", i)
            for iters, T, tr, rot in zip(self.iterations, self.temperatures,
                                         self.max_translations,
                                         self.max_rotations):
                for rb in self.rbs:
                    log.debug("%s %s", rb.get_name(), rb.get_reference_frame())
                self.set_movers(tr, rot)
                mc.add_movers(self.movers)
                mc.set_kt(T)
                mc.optimize(iters)
                for rb in self.rbs:
                    log.debug("%s %s", rb.get_name(), rb.get_reference_frame())

                mc.clear_movers()
        log.info("MonteCarlo run finished. Time %s", time.time() - t0)

    def run_monte_carlo_with_relative_movers(self):
        """
            Run MonteCarlo sampling to generate possible states for DOMINO
        """
        t0 = time.time()
        log.info("Running MonteCarlo")
        mc = core.MonteCarlo(self.model)
        mc.set_scoring_function(self._scoring_function)
        mc.set_return_best(True)

        for i in range(self.cycles):
            log.info("Cycle: %s", i)
            for iters, T, tr, rot in zip(self.iterations, self.temperatures,
                                         self.max_translations,
                                         self.max_rotations):
                log.debug("BEFORE RELATIVE MOVERS")
                self.set_relative_movers(tr, rot)
                for m in self.movers:
                    m.propose()
                    m.accept()
                mc.add_movers(self.movers)
                mc.set_kt(T)
                log.debug("Optimizing for %s iterations ...", iters)
                mc.optimize(iters)
                log.debug("Finished optimizing.")
                mc.clear_movers()
        log.info("MonteCarlo run finished. Time %s", time.time() - t0)
