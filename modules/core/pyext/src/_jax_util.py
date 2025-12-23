import functools
import math
import jax
import jax.numpy as jnp
from typing import NamedTuple
import IMP

class _MCState(NamedTuple):
    """Track the state of a MonteCarlo optimization using JAX"""

    # Current model state
    X: dict
    # Score of the current model state
    score: float
    # Best model state seen (if return_best is turned on)
    best_X: dict
    # Score of the best model state seen
    best_score: float
    # Number of accepted steps that reduced the score
    downward_steps_taken: int
    # Number of accepted steps that increased the score
    upward_steps_taken: int
    # Number of rejected steps
    rejected_steps: int


class _MCJaxInfo:
    def __init__(self, mc):
        self._mc = mc
        self._sf = mc.get_scoring_function().get_derived_object()
        ji = self._sf._get_jax()
        score_func = ji.score_func
        propose_funcs = [mover.get_derived_object()._get_jax()
                         for mover in mc.movers]
        temperature = mc.get_kt()
        return_best = mc.get_return_best()

        def init_func(X):
            score = score_func(X)
            ms = _MCState(score=score, best_score=score, X=X, best_X=X,
                          downward_steps_taken=0, upward_steps_taken=0,
                          rejected_steps=0)
            return ms

        def apply_func(k, ms):
            new_X = ms.X.copy()
            proposal_ratio = 1.0
            for propose in propose_funcs:
                k, subkey = jax.random.split(k)
                new_X, ratio = propose(subkey, new_X)
                proposal_ratio *= ratio
            new_score = score_func(new_X)

            def downward_step():
                if return_best:
                    return jax.lax.cond(new_score < ms.best_score,
                                        downward_step_new_best,
                                        downward_step_not_best)
                else:
                    return downward_step_not_best()

            def downward_step_new_best():
                return ms._replace(
                    downward_steps_taken=ms.downward_steps_taken + 1,
                    # new (score,X) should replace best
                    score=new_score, best_score=new_score,
                    X=new_X, best_X=new_X)

            def downward_step_not_best():
                return ms._replace(
                    score=new_score, X=new_X,
                    downward_steps_taken=ms.downward_steps_taken + 1)

            def upward_step():
                return ms._replace(
                    score=new_score, X=new_X,
                    upward_steps_taken=ms.upward_steps_taken + 1)

            def reject_step():
                # Keep X and score from previous step
                return ms._replace(rejected_steps=ms.rejected_steps + 1)

            def metrop_step():
                diff = new_score - ms.score
                e = jnp.exp(-diff / temperature)
                prob = jax.random.uniform(k, minval=0.0, maxval=1.0)
                return jax.lax.cond(e * proposal_ratio > prob,
                                    upward_step, reject_step)

            return jax.lax.cond(new_score < ms.score,
                                downward_step, metrop_step)

        self.init_func = init_func
        self.score_func = ji.score_func
        self.apply_func = apply_func

    def get_model_state(self):
        ji = self._sf._get_jax()
        return ji.get_model_state()


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
        # Get the number of steps that we can run in JAX, before having to
        # copy JAX arrays back to the IMP Model
        self.inner_steps = functools.reduce(
            math.gcd, [x.get_period() for x in opt.optimizer_states],
            max_steps)
        self.n_loops = max_steps // self.inner_steps

    def loop(self):
        """Run the outer loop (in Python) of the Optimizer. On each yield,
           inner_steps of JAX optimization should be run."""
        n_step = 0
        for i in range(self.n_loops):
            yield i
            # Update any necessary OptimizerStates
            n_step += self.inner_steps
            for s in self.opt.optimizer_states:
                if n_step % s.get_period() == 0:
                    s.update_always()


def _mc_optimize(mc, max_steps):
    jopt = _JAXOptimizer(mc, max_steps)
    inner_steps = jopt.inner_steps
    ji = mc._get_jax()
    init_func = jax.jit(ji.init_func)

    def run_n_mc_steps(k, mc_state):
        def mc_step_with_key(i, kms):
            k, mc_state = kms
            k, subkey = jax.random.split(k)
            return (k, ji.apply_func(subkey, mc_state))
        return jax.lax.fori_loop(0, inner_steps, mc_step_with_key,
                                 (k, mc_state))
    apply_func = jax.jit(run_n_mc_steps)

    mc_state = init_func(ji.get_model_state())
    m = mc.get_model()
    xyz = m.get_spheres_numpy()[0]

    k = jax.random.key(IMP.random_number_generator())
    for _ in jopt.loop():
        k, mc_state = apply_func(k, mc_state)
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
