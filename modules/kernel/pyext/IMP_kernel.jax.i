%extend IMP::Restraint {
  %pythoncode %{
    def _wrap_jax(self, score_func, keys=None):
        """Create the return value for _get_jax.
            Use this method in _get_jax() to wrap the JAX scoring function
            with other model- and restraint-specific information.

            @param score_func A function implemented using JAX that takes
                   a single argument (the current model state) and returns
                   the score of the restraint.
            @param keys If given, a set of IMP::Key objects describing Model
                   attributes (other than xyz and radius) that the restraint
                   uses. For example, a restraint that uses electrostatic charge
                   would pass IMP::atom::Charged::get_charge_key() here.
        """
        from IMP._jax_util import JAXRestraintInfo
        return JAXRestraintInfo(m=self.get_model(), score_func=score_func,
                                weight=self.get_weight(), keys=keys)

    def _get_jax(self):
        """Return a JAX implementation of this Restraint.
           Implement this method in a Restraint subclass to provide
           an equivalent function using [JAX](https://docs.jax.dev/)
           that scores the current model state. See also _wrap_jax.
        """
        raise NotImplementedError(f"No JAX implementation for {self}")
  %}
}

%extend IMP::internal::_ConstRestraint {
  %pythoncode %{
    def _get_jax(self):
        value = self.get_value()
        # We always return `value` regardless of the model state
        return self._wrap_jax(lambda X: value)
  %}
}

%extend IMP::PairScore {
  %pythoncode %{
    def _wrap_jax(self, score_func, keys=None):
        """Create the return value for _get_jax.
           Use this method in _get_jax() to wrap the JAX scoring function
           with other score-specific information.

           @param score_func A function implemented using JAX that takes
                  two arguments (the current model state, and the
                  ParticlePairIndexes to act on) and returns the total
                  score (for all indexes).
           @param keys Model attributes used by the PairScore.
                  See IMP::Restraint::_wrap_jax.
        """
        from IMP._jax_util import JAXScoreInfo
        return JAXScoreInfo(score_func=score_func, keys=keys)

    def _get_jax(self):
        """Return a JAX implementation of this PairScore.
           Implement this method in a PairScore subclass to provide
           an equivalent function using [JAX](https://docs.jax.dev/)
           that scores the current model state with a given set of
           ParticlePairIndexes. See also _wrap_jax.
        """
        raise NotImplementedError(f"No JAX implementation for {self}")
  %}
}

%extend IMP::SingletonScore {
  %pythoncode %{
    def _wrap_jax(self, score_func, keys=None):
        """See IMP::PairScore::_wrap_jax"""
        from IMP._jax_util import JAXScoreInfo
        return JAXScoreInfo(score_func=score_func, keys=keys)

    def _get_jax(self):
        """See IMP::PairScore::_get_jax"""
        raise NotImplementedError(f"No JAX implementation for {self}")
  %}
}

%extend IMP::UnaryFunction {
  %pythoncode %{
    def _get_jax(self):
        """Return a JAX implementation of this UnaryFunction.
           Implement this method in a UnaryFunction subclass to provide
           an equivalent function using [JAX](https://docs.jax.dev/)
           that scores the provided feature value.
        """
        raise NotImplementedError(f"No JAX implementation for {self}")
  %}
}

%extend IMP::ScoringFunction {
  %pythoncode %{
    def _get_jax(self):
        r = self.get_single_restraint()
        if r is None:
            raise NotImplementedError(f"No JAX implementation for {self}")
        else:
            return r.get_derived_object()._get_jax()
  %}
}

%extend IMP::OptimizerState {
  %pythoncode %{
    def _get_jax(self, state_index):
        """Add JAX support for this OptimizerState.

           Each OptimizerState must explicitly support how it is going to
           be called when the Optimizer is run via JAX.

           OptimizerStates can run as traditional IMP CPU code
           on the IMP Model object, in which case None should be returned here.
           The Optimizer will sync any necessary information from JAX back
           with the IMP Model before calling the OptimizerState. However,
           the OptimizerState is not permitted to modify the Model; any
           changes will not be propagated back to the Optimizer.
           (For example, a thermostat which tries to change particle
           velocities will not function correctly.)

           Alternatively, a pure JAX implememtation can be provided.
           See _wrap_jax for more information.
        """
        raise NotImplementedError(f"No JAX implementation for {self}")

    def _wrap_jax(self, init_func, apply_func):
        """Create the return value for _get_jax.
           Use this method in _get_jax() to wrap the JAX function
           with other OptimizerState-specific information.

           @param init_func a JAX function which, given an optimizer state,
                  does any necessary setup and returns a (possibly modified)
                  optimizer state. If any OptimizerState-specific
                  persistent state is needed, it can be stored in
                  `state.optimizer_states[state_index]` and later retrieved or
                  modified in apply_func.
           @param apply_func a JAX function which, given an optimizer state,
                  does the JAX equivalent of do_update() and returns a new
                  optimizer state.
        """
        from IMP._jax_util import JAXOptimizerStateInfo
        return JAXOptimizerStateInfo(self, init_func, apply_func)
  %}
}

%extend IMP::RestraintSet {
  %pythoncode %{
    def _get_jax(self):
        jis = [r.get_derived_object()._get_jax() for r in self.restraints]
        funcs = [j.score_func for j in jis]
        keys = frozenset(x for j in jis for x in j._keys)
        def jax_sf(X):
            if funcs:
                return sum(f(X) for f in funcs)
            else:
                # sum([]) returns int, but we must return float
                return 0.
        return self._wrap_jax(jax_sf, keys=keys)
  %}
}

%extend IMP::ScoreState {
  %pythoncode %{
    def _wrap_jax(self, apply_func, keys=None):
        """Create the return value for _get_jax.
            Use this method in _get_jax() to wrap the JAX apply function
            with other model- and ScoreState-specific information.

            @param apply_func A function implemented using JAX that takes
                   a single argument (the current model state) and returns
                   a new model state.
            @param keys If given, a set of IMP::Key objects describing Model
                   attributes (other than xyz and radius) that the ScoreState
                   uses.
        """
        from IMP._jax_util import JAXScoreStateInfo
        return JAXScoreStateInfo(m=self.get_model(), apply_func=apply_func,
                                 keys=keys)

    def _get_jax(self):
        """Return a JAX implementation of this ScoreState.
           Implement this method in a ScoreState subclass to provide
           an equivalent function using [JAX](https://docs.jax.dev/)
           that modifies the model state. See also _wrap_jax.
        """
        raise NotImplementedError(f"No JAX implementation for {self}")
  %}
}

%extend IMP::SingletonModifier {
  %pythoncode %{
    def _wrap_jax(self, apply_func, keys=None):
        from IMP._jax_util import JAXModifierInfo
        return JAXModifierInfo(apply_func=apply_func, keys=keys)

    def _get_jax(self, m, index=None):
        raise NotImplementedError(f"No JAX implementation for {self}")
  %}
}
