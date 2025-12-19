import math
import jax
import jax.numpy as jnp
from typing import NamedTuple

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
            for propose in propose_funcs:
                k, subkey = jax.random.split(k)
                newX = propose(subkey, newX)
            new_score = score_func(newX)

            def downward_step():
                newX["mc"] = X["mc"]._replace(
                    last_score=new_score,
                    best_score=jax.lax.min(new_score, X["mc"].best_score),
                    downward_steps_taken=X["mc"].downward_steps_taken + 1)
                return newX

            def upward_step():
                newX["mc"] = X["mc"]._replace(
                    upward_steps_taken=X["mc"].upward_steps_taken + 1)
                return newX

            def reject_step():
                X["mc"] = X["mc"]._replace(
                    rejected_steps=X["mc"].rejected_steps + 1)
                return X

            def metrop_step():
                diff = new_score - old_score
                e = jnp.exp(-diff / temperature)
                # todo: use proposal_ratio from movers
                prob = jax.random.uniform(k, minval=0.0, maxval=1.0)
                return jax.lax.cond(e > prob, upward_step, reject_step)

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
