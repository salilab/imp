"""JAX implementations of IMP.core local optimizers
   (currently IMP.core.SteepestDescent).

   These are ports of the C++ algorithms, written as state machines driven
   by jax.lax.while_loop so that an entire minimization (or a block of it,
   between calls to any IMP OptimizerStates) runs on the device without
   returning to Python. Derivatives are obtained with jax.grad rather than
   from the restraints' C++ derivative code.

   Each optimizer state object carries an `n_updates` counter, which counts
   the number of times the C++ optimizer would have called
   Optimizer::update_states(), and a `done` flag. IMP (non-JAX)
   OptimizerStates are called from Python whenever `n_updates` reaches a
   multiple of their period.
"""

import functools
import math
from dataclasses import dataclass
import numpy as np
import jax
import jax.numpy as jnp
import jax.tree_util
import IMP
import IMP._jax_util
import IMP.jax
import IMP.core


# Sphere attributes are stored in jm['xyz'] and jm['r'] rather than
# under their own key names
_SPHERE_KEYS = [IMP.FloatKey(i) for i in range(4)]

# Large enough to never be reached by n_updates
_MAX_INT = 2**31 - 1


def _get_float_dtype():
    """Get the default JAX floating point type (float32 unless JAX's
       x64 mode is enabled)"""
    return jnp.result_type(float)


def _to_float_range(v):
    """Clamp a C++ double (which may be huge, e.g. DBL_MAX) to the range
       of the JAX floating point type, to avoid overflow warnings"""
    fmax = float(jnp.finfo(_get_float_dtype()).max)
    return min(max(v, -fmax), fmax)


class _OptimizedAttributes:
    """Map between a JAX Model and a flat vector of the values of all of the
       IMP Model's optimized attributes, as per IMP::AttributeOptimizer.

       Only attributes that are present in the JAX Model (i.e. that are used
       by the scoring function or ScoreStates) are included; any other
       optimized attribute cannot affect the score, so its derivative
       is zero and neither optimizer would change its value.

       @param m The IMP Model.
       @param jm A JAX Model (only its keys are used).
       @param scaled If True, values are divided by the width of the
              attribute's range (as for
              AttributeOptimizer::get_scaled_value()) so that the vector,
              and derivatives with respect to it, are scaled.
    """

    def __init__(self, m, jm, scaled):
        if 'rigid_bodies' in jm:
            raise NotImplementedError(
                "JAX local optimizers do not yet support rigid bodies")
        if jm.multi_attr_keys:
            raise NotImplementedError(
                "JAX local optimizers do not yet support multiple "
                "attribute sets")
        pis = [IMP.ParticleIndex(pi) for pi in m.get_particle_indexes()]
        particles = [m.get_particle(pi) for pi in pis]

        def get_optimized(k):
            return np.array([int(pi) for pi, p in zip(pis, particles)
                             if p.has_attribute(k) and p.get_is_optimized(k)],
                            dtype=np.int32)

        # Each part is (JAX Model key, index into that array, FloatKey)
        self._parts = []
        for i, k in enumerate(_SPHERE_KEYS[:3]):
            ind = get_optimized(k)
            if len(ind) > 0:
                self._parts.append(('xyz', (ind, i), k))
        ind = get_optimized(_SPHERE_KEYS[3])
        if len(ind) > 0:
            self._parts.append(('r', ind, _SPHERE_KEYS[3]))
        for name in sorted(jm.keys()):
            if name in ('xyz', 'r') or not IMP.FloatKey.get_key_exists(name):
                continue
            k = IMP.FloatKey(name)
            ind = get_optimized(k)
            if len(ind) > 0:
                self._parts.append((name, ind, k))

        self.size = sum(_part_size(p) for p in self._parts)
        widths = []
        for part in self._parts:
            w = _get_width(m, part[2]) if scaled else 1.0
            widths.append(np.full(_part_size(part), w))
        self._widths = (jnp.asarray(np.concatenate(widths)) if widths
                        else jnp.zeros(0))

    def get(self, jm):
        """Get a flat vector of the optimized (scaled) values in `jm`"""
        if not self._parts:
            return jnp.zeros(0, dtype=_get_float_dtype())
        vals = [jnp.asarray(jm[name][ind]) for name, ind, _ in self._parts]
        return jnp.concatenate(vals) / self._widths

    def set(self, jm, x):
        """Return a copy of `jm` with the optimized values set from the flat
           (scaled) vector `x`"""
        jm = jm.copy()
        v = x * self._widths
        offset = 0
        for part in self._parts:
            name, ind, _ = part
            n = _part_size(part)
            arr = jm[name]
            if isinstance(arr, np.ndarray):
                arr = jnp.asarray(arr)
            jm[name] = arr.at[ind].set(v[offset:offset + n])
            offset += n
        return jm


def _part_size(part):
    ind = part[1]
    return len(ind[0]) if isinstance(ind, tuple) else len(ind)


def _get_width(m, k):
    """Equivalent to AttributeOptimizer::get_width()"""
    lo, hi = IMP._get_range(m, k)
    wid = hi - lo
    return wid if wid > .0001 else 1.0


def _sync_imp_model(m, jm):
    """Copy all attributes that a local optimizer could change (directly or
       via ScoreStates) from the JAX Model back to the IMP Model"""
    xyz, r = m.get_spheres_numpy()
    xyz[:], r[:] = jax.device_get((jm['xyz'], jm['r']))
    for name, v in jm.items():
        if isinstance(v, IMP.jax._CompactArray):
            v.sync()
        elif (name not in ('xyz', 'r') and isinstance(v, jax.Array)
              and IMP.FloatKey.get_key_exists(name)
              and jnp.issubdtype(v.dtype, jnp.floating)):
            m.get_numpy(IMP.FloatKey(name))[:] = jax.device_get(v)


class _LocalOptimizerJAXInfo(IMP._jax_util.JAXOptimizerInfo):
    """Common setup for JAX local optimizers."""

    def __init__(self, opt, space, scaled):
        super().__init__(opt, space)
        if self._setup_jax_optimizer_states():
            raise NotImplementedError(
                "JAX local optimizers do not support OptimizerStates with "
                "a JAX implementation")
        self.attrs = _OptimizedAttributes(
            opt.get_model(), self.get_jax_model(attributes=False), scaled)
        attrs = self.attrs
        score_func = self.score_func
        dtype = _get_float_dtype()

        def score_x(x, jm):
            """Score the model with optimized values `x`. Return the score
               and the new (constrained) JAX Model"""
            score, jm = score_func(attrs.set(jm, x))
            return jnp.asarray(score, dtype=dtype), jm

        self.score_x = score_x
        self.value_and_grad_x = jax.value_and_grad(score_x, has_aux=True)


@jax.tree_util.register_dataclass
@dataclass
class _SteepestDescent:
    """Track the state of a SteepestDescent optimization using JAX"""

    # Current JAX Model
    jm: IMP.jax._JAXModel
    # Current values of the optimized attributes
    x: jax.Array
    # Score to return (the most recently evaluated score)
    score: jax.Array
    # Current step size
    step_size: jax.Array
    # Number of (outer) steps taken, and the maximum number allowed
    steps: jax.Array
    max_steps: jax.Array
    # Number of times update_states() would have been called
    n_updates: jax.Array
    # True once the optimization has terminated
    done: jax.Array


@jax.tree_util.register_dataclass
@dataclass
class _SDInner:
    """State of the SteepestDescent inner (step size search) loop"""
    cnt: jax.Array
    step_size: jax.Array
    constant_score: jax.Array
    new_score: jax.Array
    new_x: jax.Array
    new_jm: IMP.jax._JAXModel
    status: jax.Array


# Outcomes of the SteepestDescent inner loop
(_SD_CONTINUE, _SD_ACCEPT, _SD_BELOW_THRESHOLD, _SD_CONSTANT, _SD_STUCK,
 _SD_NO_PROGRESS) = range(6)


class _SDJAXInfo(_LocalOptimizerJAXInfo):
    def __init__(self, sd, space):
        super().__init__(sd, space, scaled=False)
        dtype = _get_float_dtype()
        threshold = _to_float_range(sd.get_threshold())
        start_step_size = _to_float_range(sd.get_step_size())
        max_step_size = _to_float_range(sd.get_maximum_step_size())
        score_x = self.score_x
        value_and_grad_x = self.value_and_grad_x

        def init_func(jm, key, max_steps):
            return _SteepestDescent(
                jm=jm, x=self.attrs.get(jm),
                # C++ returns 0. if no steps are taken
                score=jnp.asarray(0., dtype=dtype),
                step_size=jnp.asarray(start_step_size, dtype=dtype),
                steps=jnp.asarray(0), max_steps=jnp.asarray(max_steps),
                n_updates=jnp.asarray(0),
                done=jnp.asarray(max_steps <= 0))

        def apply_func(s):
            """Do one step of steepest descent; this is one iteration of the
               outer loop in SteepestDescent::do_optimize()"""
            (last_score, jm0), derivs = value_and_grad_x(s.x, s.jm)
            x0 = s.x

            def inner_body(c):
                cnt = c.cnt + 1
                # try new values based on moving down the gradient at the
                # current step size
                new_x = x0 - derivs * c.step_size
                new_score, new_jm = score_x(new_x, jm0)
                accepted = new_score < last_score
                below = new_score <= threshold
                same = jnp.abs(new_score - last_score) < .0000001
                # score is the same twice in a row; give up on this step
                constant = ~accepted & same & c.constant_score
                step_size = jnp.where(
                    accepted, jnp.minimum(c.step_size * 1.4, max_step_size),
                    jnp.where(constant, c.step_size,
                              c.step_size * jnp.where(same, 0.9, 0.7)))
                rejected = ~accepted & ~constant
                stuck = rejected & (cnt > 200)
                no_progress = rejected & ~stuck & (step_size < .00000001)
                status = jnp.select(
                    [accepted & below, accepted, constant, stuck,
                     no_progress],
                    [_SD_BELOW_THRESHOLD, _SD_ACCEPT, _SD_CONSTANT, _SD_STUCK,
                     _SD_NO_PROGRESS], _SD_CONTINUE)
                return _SDInner(cnt=cnt, step_size=step_size,
                                constant_score=same, new_score=new_score,
                                new_x=new_x, new_jm=new_jm, status=status)

            c = jax.lax.while_loop(
                lambda c: c.status == _SD_CONTINUE, inner_body,
                _SDInner(cnt=jnp.asarray(0), step_size=s.step_size,
                         constant_score=jnp.asarray(False),
                         new_score=last_score, new_x=x0, new_jm=jm0,
                         status=jnp.asarray(_SD_CONTINUE)))
            stuck = c.status == _SD_STUCK
            # If stuck, restore the old values and return the old score.
            # Otherwise, keep the new values (even if the step was not
            # accepted, as the C++ code does)
            x, jm, score = jax.lax.cond(
                stuck, lambda: (x0, jm0, last_score),
                lambda: (c.new_x, c.new_jm, c.new_score))
            updated = ((c.status == _SD_ACCEPT)
                       | (c.status == _SD_BELOW_THRESHOLD)
                       | (c.status == _SD_NO_PROGRESS))
            steps = s.steps + 1
            done = ((c.status == _SD_BELOW_THRESHOLD) | stuck
                    | (c.status == _SD_NO_PROGRESS) | (steps >= s.max_steps))
            return _SteepestDescent(
                jm=jm, x=x, score=score, step_size=c.step_size, steps=steps,
                max_steps=s.max_steps,
                n_updates=s.n_updates + updated.astype(s.n_updates.dtype),
                done=done)

        self.init_func = init_func
        self.apply_func = apply_func


class _LocalJAXOptimizer:
    """Run a local optimizer (e.g. SteepestDescent) with
       JAX, and update the IMP Model with the result.

       The optimization runs entirely in JAX, except that if any
       OptimizerStates without a JAX implementation are present, it pauses
       each time one of them is due to be updated, syncs the IMP Model,
       and calls the OptimizerState."""

    def __init__(self, opt, max_steps, space):
        self.opt = opt
        self.max_steps = max_steps
        self._space = space or IMP.jax.FreeSpace
        self._jax_info = ji = opt._get_jax(space=self._space)
        self._imp_opt_states = [s for s in opt.optimizer_states
                                if s.get_derived_object()._get_jax(0) is None]
        # Run in blocks between OptimizerState updates
        if self._imp_opt_states:
            self.block_size = functools.reduce(
                math.gcd, [s.get_period() for s in self._imp_opt_states])
        else:
            self.block_size = None
        self.init_func = jax.jit(ji.init_func)
        self.score_func = jax.jit(ji.score_func)

        def run_block(state, target):
            return jax.lax.while_loop(
                lambda s: ~s.done & (s.n_updates < target), ji.apply_func,
                state)
        self._run_block = jax.jit(run_block)

    def get_initial_state(self):
        """Get the JAX optimizer object for the current IMP Model"""
        jm = self._jax_info.get_jax_model()
        return self.init_func(jm, key=IMP.jax.get_random_key(),
                              max_steps=self.max_steps)

    def optimize(self, state):
        """Run the optimizer with JAX and update the IMP Model with the
           result. Return the final score and the new JAX optimizer state."""
        m = self.opt.get_model()
        handled = int(state.n_updates)
        while True:
            target = (handled + self.block_size if self.block_size
                      else _MAX_INT)
            state = self._run_block(state, target)
            n_updates = int(state.n_updates)
            if n_updates > handled and self._imp_opt_states:
                _sync_imp_model(m, state.jm)
                for s in self._imp_opt_states:
                    if n_updates % s.get_period() == 0:
                        s.update_always()
            handled = n_updates
            if bool(state.done):
                break
        _sync_imp_model(m, state.jm)
        return float(self._get_score(state)), state

    def _get_score(self, state):
        return state.score


class _SDJAXOptimizer(_LocalJAXOptimizer):
    pass
