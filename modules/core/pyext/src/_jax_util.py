import functools
import math
import jax
import jax.random
import jax.tree_util
import jax.numpy as jnp
from dataclasses import dataclass
import IMP._jax_util


def _get_jax_restraint(r):
    """Return a JAX implementation for SingletonRestraint,
       PairRestraint, etc."""
    score = r.get_score_object().get_derived_object()
    indexes = jnp.array([r.get_index()])
    ji = score._get_jax(r.get_model(), indexes)
    score_jax = ji.score_func

    def jax_restraint(jm):
        return jnp.sum(score_jax(jm))
    return r._wrap_jax(jax_restraint, keys=ji._keys)


class JAXMoverInfo:
    """Information about a JAX implementation of a MonteCarloMover."""
    def __init__(self, init_func, propose_func):
        self.init_func = init_func
        self.propose_func = propose_func


@jax.tree_util.register_dataclass
@dataclass
class _MonteCarlo:
    """Track the state of a MonteCarlo optimization using JAX"""

    # Current JAX Model
    jm: dict
    # Score of the current JAX Model
    score: float
    # Best JAX Model seen (if return_best is turned on)
    best_jm: dict
    # Score of the best JAX model seen
    best_score: float
    # Total number of accepted steps (upward + downward)
    accepted_steps: int
    # Number of accepted steps that reduced the score
    downward_steps_taken: int
    # Number of accepted steps that increased the score
    upward_steps_taken: int
    # Number of rejected steps
    rejected_steps: int
    # JAX random number key
    rkey: jax.Array
    # Any persistent state used by Movers
    mover_state: list
    # Any persistent state used by OptimizerStates. Each OptimizerState's
    # _get_jax() method is given a unique index into this list.
    optimizer_states: list


class _MCJAXInfo(IMP._jax_util.JAXOptimizerInfo):
    def __init__(self, mc):
        super().__init__(mc)
        score_func = self.score_func
        movers = [mover.get_derived_object()._get_jax() for mover in mc.movers]
        temperature = mc.get_kt()
        return_best = mc.get_return_best()
        jax_optstates = self._setup_jax_optimizer_states()

        def init_func(jm, key):
            score, jm = score_func(jm)
            mover_state = []
            for mover in movers:
                key, subkey = jax.random.split(key)
                mover_state.append(mover.init_func(subkey))
            ms = _MonteCarlo(
                score=score, best_score=score, jm=jm, best_jm=jm,
                accepted_steps=0, downward_steps_taken=0,
                upward_steps_taken=0, rejected_steps=0,
                optimizer_states=[None] * len(jax_optstates),
                rkey=key, mover_state=mover_state)
            for js in jax_optstates:
                ms = js.init_func(ms)
            return ms

        def apply_func(ms):
            new_jm = ms.jm.copy()
            proposal_ratio = 1.0
            for i in range(len(movers)):
                new_jm, ms.mover_state[i], ratio = movers[i].propose_func(
                    new_jm, ms.mover_state[i])
                proposal_ratio *= ratio
            new_score, new_jm = score_func(new_jm)

            def update_states(ms):
                steps = ms.accepted_steps
                for js in jax_optstates:
                    ms = jax.lax.cond(steps % js.period == 0, js.apply_func,
                                      lambda x: x, ms)
                return ms

            def downward_step(ms):
                ms.downward_steps_taken += 1
                ms.accepted_steps += 1
                if return_best:
                    return jax.lax.cond(new_score < ms.best_score,
                                        downward_step_new_best,
                                        downward_step_not_best, ms)
                else:
                    return downward_step_not_best(ms)

            def downward_step_new_best(ms):
                # new (score,jm) should replace best
                ms.score = ms.best_score = new_score
                ms.jm = ms.best_jm = new_jm
                return update_states(ms)

            def downward_step_not_best(ms):
                ms.score = new_score
                ms.jm = new_jm
                return update_states(ms)

            def upward_step(ms):
                ms.upward_steps_taken += 1
                ms.accepted_steps += 1
                ms.score = new_score
                ms.jm = new_jm
                return update_states(ms)

            def reject_step(ms):
                # Keep jm and score from previous step
                ms.rejected_steps += 1
                return ms

            def metrop_step(ms):
                diff = new_score - ms.score
                e = jnp.exp(-diff / temperature)
                ms.rkey, subkey = jax.random.split(ms.rkey)
                prob = jax.random.uniform(subkey, minval=0.0, maxval=1.0)
                return jax.lax.cond(e * proposal_ratio > prob,
                                    upward_step, reject_step, ms)

            return jax.lax.cond(new_score < ms.score,
                                downward_step, metrop_step, ms)

        self.init_func = init_func
        self.apply_func = apply_func


def _sync_stats(imp_mc, jax_mc):
    """Update IMP MonteCarlo object with stats from JAX run"""
    imp_mc.set_number_of_downward_steps(jax_mc.downward_steps_taken)
    imp_mc.set_number_of_upward_steps(jax_mc.upward_steps_taken)
    imp_mc.set_number_of_rejected_steps(jax_mc.rejected_steps)
    imp_mc.set_best_accepted_energy(jax_mc.best_score)
    imp_mc.set_last_accepted_energy(jax_mc.score)


class _JAXOptimizer:
    """Helper class to run an IMP Optimizer using JAX."""
    def __init__(self, opt, max_steps):
        self.opt = opt

        # Get all OptimizerStates that have no explicit JAX implementation
        self._imp_opt_states = [s for s in opt.optimizer_states
                                if s.get_derived_object()._get_jax(0) is None]

        # Get the number of steps that we can run in JAX, before having to
        # copy JAX arrays back to the IMP Model for OptimizerStates
        # implemented in IMP
        self.inner_steps = functools.reduce(
            math.gcd, [x.get_period() for x in self._imp_opt_states],
            max_steps)
        self.n_loops = max_steps // self.inner_steps

    def loop(self):
        """Run the outer loop (in Python) of the Optimizer. On each yield,
           inner_steps of JAX optimization should be run."""
        n_step = 0
        for i in range(self.n_loops):
            yield i
            # Update any necessary IMP OptimizerStates
            n_step += self.inner_steps
            for s in self._imp_opt_states:
                if n_step % s.get_period() == 0:
                    s.update_always()


def _mc_optimize(mc, max_steps):
    jopt = _JAXOptimizer(mc, max_steps)
    inner_steps = jopt.inner_steps
    ji = mc._get_jax()
    init_func = jax.jit(ji.init_func)
    apply_func = jax.jit(
        lambda jm: jax.lax.fori_loop(0, inner_steps,
                                     lambda i, jm: ji.apply_func(jm), jm))

    mc_state = init_func(ji.get_jax_model(),
                         key=IMP._jax_util.get_random_key())

    m = mc.get_model()
    xyz = m.get_spheres_numpy()[0]

    for _ in jopt.loop():
        mc_state = apply_func(mc_state)
        # Resync IMP Model arrays with JAX
        xyz[:] = mc_state.jm['xyz']

    # Update IMP MonteCarlo object with stats from JAX run
    _sync_stats(mc, mc_state)

    if mc.get_return_best():
        # Resync IMP Model arrays with best JAX Model
        xyz[:] = mc_state.best_jm['xyz']
        return mc.get_best_accepted_energy()
    else:
        return mc.get_last_accepted_energy()


@jax.tree_util.register_dataclass
@dataclass
class _SerialMover:
    """Persistent state for a JAX SerialMover"""

    # Mover index to delegate to
    imov: int
    # Any state used by Movers
    mover_state: list


def _spline(feature, minrange, lowbin, highbin, spacing, values,
            second_derivs):
    """Cubic spline interpolation"""
    lowfeature = minrange + lowbin * spacing
    b = (feature - lowfeature) / spacing
    a = 1. - b
    return (a * values[lowbin] + b * values[highbin] +
            ((a * (a * a - 1.)) * second_derivs[lowbin]
             + (b * (b * b - 1.)) * second_derivs[highbin])
            * (spacing * spacing) / 6.)


def _angle(rij, rkj):
    """Return the N angles (in radians) between Nx3 vectors rij and rkj."""
    # einsum here calculates the row-wise dot product. We could also
    # use vecdot but that likely requires numpy 2.
    scalar_product = jnp.einsum("ij,ij->i", rij, rkj)
    # Avoid division by zero if colinear
    mag_product = jnp.clip(jnp.linalg.norm(rij, axis=1)
                           * jnp.linalg.norm(rkj, axis=1), 1e-6)
    # Clip to valid domain for cos
    cosangle = jnp.clip(scalar_product / mag_product, -1.0, 1.0)
    return jnp.acos(cosangle)


def _dihedral(rij, rkj, rkl):
    """Return the N dihedrals (in radians) between Nx3 vectors rij,
       rkj and rkl."""
    v1 = jnp.cross(rij, rkj)
    v2 = jnp.cross(rkj, rkl)
    angle = _angle(v1, v2)
    # Get sign
    v0 = jnp.cross(v1, v2)
    sign = jnp.einsum("ij,ij->i", rkj, v0)
    return jnp.copysign(angle, sign)
