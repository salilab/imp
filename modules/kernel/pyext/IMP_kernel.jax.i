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
        from IMP._jax_util import JaxRestraintInfo
        return JaxRestraintInfo(m=self.get_model(), score_func=score_func,
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
        from IMP._jax_util import JaxScoreInfo
        return JaxScoreInfo(score_func=score_func, keys=keys)

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
        from IMP._jax_util import JaxScoreInfo
        return JaxScoreInfo(score_func=score_func, keys=keys)

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
      raise NotImplementedError(f"No JAX implementation for {self}")
  %}
}
