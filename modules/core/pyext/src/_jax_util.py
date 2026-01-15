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
    ji = score._get_jax()
    score_jax = ji.score_func
    indexes = jnp.array([r.get_index()])

    def jax_restraint(X):
        return jnp.sum(score_jax(X, indexes))
    return r._wrap_jax(jax_restraint, keys=ji._keys)


class JAXMoverInfo:
    """Information about a JAX implementation of a MonteCarloMover."""
    def __init__(self, init_func, propose_func):
        self.init_func = init_func
        self.propose_func = propose_func


@jax.tree_util.register_dataclass
@dataclass
class _MonteCarloState:
    """Track the state of a MonteCarlo optimization using JAX"""

    # Current model state
    X: dict
    # Score of the current model state
    score: float
    # Best model state seen (if return_best is turned on)
    best_X: dict
    # Score of the best model state seen
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
    # Any state used by Movers
    mover_state: list
    # Any state used by OptimizerStates. Each OptimizerState's _get_jax()
    # method is given a unique index into this list.
    optimizer_states: list


class _MCJAXInfo(IMP._jax_util.JAXOptimizerInfo):
    def __init__(self, mc):
        super().__init__(mc)
        score_func = self.score_func
        movers = [mover.get_derived_object()._get_jax() for mover in mc.movers]
        temperature = mc.get_kt()
        return_best = mc.get_return_best()
        state_index = 0
        jax_optstates = []
        for s in mc.optimizer_states:
            j = s.get_derived_object()._get_jax(state_index)
            if j is not None:
                state_index += 1
                jax_optstates.append(j)

        def init_func(X, key):
            score, X = score_func(X)
            mover_state = []
            for mover in movers:
                key, subkey = jax.random.split(key)
                mover_state.append(mover.init_func(subkey))
            ms = _MonteCarloState(
                score=score, best_score=score, X=X, best_X=X,
                accepted_steps=0, downward_steps_taken=0,
                upward_steps_taken=0, rejected_steps=0,
                optimizer_states=[None] * len(jax_optstates),
                rkey=key, mover_state=mover_state)
            for js in jax_optstates:
                ms = js.init_func(ms)
            return ms

        def apply_func(ms):
            new_X = ms.X.copy()
            proposal_ratio = 1.0
            for i in range(len(movers)):
                new_X, ms.mover_state[i], ratio = movers[i].propose_func(
                    new_X, ms.mover_state[i])
                proposal_ratio *= ratio
            new_score, new_X = score_func(new_X)

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
                # new (score,X) should replace best
                ms.score = ms.best_score = new_score
                ms.X = ms.best_X = new_X
                return update_states(ms)

            def downward_step_not_best(ms):
                ms.score = new_score
                ms.X = new_X
                return update_states(ms)

            def upward_step(ms):
                ms.upward_steps_taken += 1
                ms.accepted_steps += 1
                ms.score = new_score
                ms.X = new_X
                return update_states(ms)

            def reject_step(ms):
                # Keep X and score from previous step
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
        lambda X: jax.lax.fori_loop(0, inner_steps,
                                    lambda i, X: ji.apply_func(X), X))

    mc_state = init_func(ji.get_model_state(),
                         key=IMP._jax_util.get_random_key())

    m = mc.get_model()
    xyz = m.get_spheres_numpy()[0]

    for _ in jopt.loop():
        mc_state = apply_func(mc_state)
        # Resync IMP Model arrays with JAX
        xyz[:] = mc_state.X['xyz']

    # Update IMP MonteCarlo object with stats from JAX run
    _sync_stats(mc, mc_state)

    if mc.get_return_best():
        # Resync IMP Model arrays with JAX best model state
        xyz[:] = mc_state.best_X['xyz']
        return mc.get_best_accepted_energy()
    else:
        return mc.get_last_accepted_energy()


@jax.tree_util.register_dataclass
@dataclass
class _SerialMoverState:
    """Persistent state for a JAX SerialMover"""

    # Mover index to delegate to
    imov: int
    # Any state used by Movers
    mover_state: list
