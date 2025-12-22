import functools
import math
import jax
import jax.numpy as jnp
from typing import NamedTuple
import IMP

class _MCStats(NamedTuple):
    last_score: float
    best_score: float
    downward_steps_taken: int
    upward_steps_taken: int
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

        def init_func(X):
            score = score_func(X)
            X["mc"] = X["mc"]._replace(last_score=score, best_score=score)
            return X

        def apply_func(k, X):
            old_score = X["mc"].last_score
            newX = X.copy()
            proposal_ratio = 1.0
            for propose in propose_funcs:
                k, subkey = jax.random.split(k)
                newX, ratio = propose(subkey, newX)
                proposal_ratio *= ratio
            new_score = score_func(newX)

            def downward_step():
                newX["mc"] = X["mc"]._replace(
                    last_score=new_score,
                    best_score=jax.lax.min(new_score, X["mc"].best_score),
                    downward_steps_taken=X["mc"].downward_steps_taken + 1)
                return newX

            def upward_step():
                newX["mc"] = X["mc"]._replace(
                    last_score=new_score,
                    upward_steps_taken=X["mc"].upward_steps_taken + 1)
                return newX

            def reject_step():
                X["mc"] = X["mc"]._replace(
                    rejected_steps=X["mc"].rejected_steps + 1)
                return X

            def metrop_step():
                diff = new_score - old_score
                e = jnp.exp(-diff / temperature)
                prob = jax.random.uniform(k, minval=0.0, maxval=1.0)
                return jax.lax.cond(e * proposal_ratio > prob,
                                    upward_step, reject_step)

            return jax.lax.cond(new_score < old_score,
                                downward_step, metrop_step)

        self.init_func = init_func
        self.score_func = ji.score_func
        self.apply_func = apply_func

    def get_model_state(self):
        m = self._mc.get_model()
        ji = self._sf._get_jax()
        X = ji.get_model_state()
        X['xyz'] = jax.numpy.array(X['xyz'])
        X['mc'] = _MCStats(last_score=math.inf, best_score=math.inf,
                           downward_steps_taken=0, upward_steps_taken=0,
                           rejected_steps=0)
        return X


def _sync_stats(imp_mc, jax_mc):
    """Update IMP MonteCarlo object with stats from JAX run"""
    imp_mc.set_number_of_downward_steps(jax_mc.downward_steps_taken)
    imp_mc.set_number_of_upward_steps(jax_mc.upward_steps_taken)
    imp_mc.set_number_of_rejected_steps(jax_mc.rejected_steps)
    imp_mc.set_best_accepted_energy(jax_mc.best_score)
    imp_mc.set_last_accepted_energy(jax_mc.last_score)


def _mc_optimize(mc, max_steps):
    # Get the number of steps that we can run in JAX, before having to
    # copy JAX arrays back to the IMP Model
    inner_steps = functools.reduce(
        math.gcd, [x.get_period() for x in mc.optimizer_states], max_steps)
    n_loops = max_steps // inner_steps

    ji = mc._get_jax()
    init_func = jax.jit(ji.init_func)

    def run_n_mc_steps(k, X):
        def mc_step_with_key(i, kX):
            k, X = kX
            k, subkey = jax.random.split(k)
            return (k, ji.apply_func(subkey, X))
        return jax.lax.fori_loop(0, inner_steps, mc_step_with_key, (k, X))[1]
    apply_func = jax.jit(run_n_mc_steps)

    X = init_func(ji.get_model_state())

    m = mc.get_model()
    xyz = m.get_spheres_numpy()[0]

    k = jax.random.key(IMP.random_number_generator())
    n_step = 0
    for i in range(n_loops):
        k, subkey = jax.random.split(k)
        X = apply_func(subkey, X)
        # Resync IMP Model arrays with JAX
        xyz[:] = X['xyz']
        # Update any necessary OptimizerStates
        n_step += inner_steps
        for s in mc.optimizer_states:
            if n_step % s.get_period() == 0:
                s.update_always()

    # Update IMP MonteCarlo object with stats from JAX run
    _sync_stats(mc, X['mc'])
    if mc.get_return_best():
        # todo: we must also return best X
        return mc.get_best_accepted_energy()
    else:
        return mc.get_last_accepted_energy()
