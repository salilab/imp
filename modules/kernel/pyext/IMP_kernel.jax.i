%pythoncode %{
class JAXWarning(UserWarning):
    """Warning for incomplete JAX support or different behavior of JAX code"""
    pass
%}

%extend IMP::Restraint {
  %pythoncode %{
    def _wrap_jax(self, score_func, keys=None):
        """Create the return value for _get_jax.
            Use this method in _get_jax() to wrap the JAX scoring function
            with other model- and restraint-specific information.

            @param score_func A function implemented using JAX that takes
                   a single argument (the current JAX Model) and returns
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
           that scores the current JAX Model. See also _wrap_jax.
        """
        raise NotImplementedError(
            f"No JAX implementation for {self} ({type(self)})")

    def _evaluate_jax(self):
        """Similar to evaluate(False), but using JAX.
           This is intended to be useful for testing purposes. It will likely
           not be particularly fast as it will copy the IMP Model and
           jax.jit-compile the scoring function each time."""
        import jax
        ji = self._get_jax()
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        return j(jm)
  %}
}

%extend IMP::internal::_ConstRestraint {
  %pythoncode %{
    def _get_jax(self):
        value = self.get_value()
        # We always return `value` regardless of the JAX Model
        return self._wrap_jax(lambda jm: value)
  %}
}

%extend IMP::PairScore {
  %pythoncode %{
    def _wrap_jax(self, m, score_func, keys=None):
        """Create the return value for _get_jax.
           Use this method in _get_jax() to wrap the JAX scoring function
           with other score-specific information.

           @param m The IMP.Model that the score will act on.
           @param score_func A function implemented using JAX that takes
                  one argument (the current JAX Model) and returns the total
                  score.
           @param keys Model attributes used by the PairScore.
                  See IMP::Restraint::_wrap_jax.
        """
        from IMP._jax_util import JAXScoreInfo
        return JAXScoreInfo(m, score_func=score_func, keys=keys)

    def _get_jax(self, m, indexes):
        """Return a JAX implementation of this PairScore.
           Implement this method in a PairScore subclass to provide
           an equivalent function using [JAX](https://docs.jax.dev/)
           that scores the current JAX Model with a given set of
           ParticlePairIndexes. See also _wrap_jax.

           @param m The IMP.Model that the score will act on.
           @param indexes The ParticlePairIndexes that the score will act on.
        """
        raise NotImplementedError(
            f"No JAX implementation for {self} ({type(self)})")
  %}
}

%extend IMP::SingletonScore {
  %pythoncode %{
    def _wrap_jax(self, m, score_func, keys=None):
        """See IMP::PairScore::_wrap_jax"""
        from IMP._jax_util import JAXScoreInfo
        return JAXScoreInfo(m, score_func=score_func, keys=keys)

    def _get_jax(self, m, indexes):
        """See IMP::PairScore::_get_jax"""
        raise NotImplementedError(
            f"No JAX implementation for {self} ({type(self)})")
  %}
}

%extend IMP::internal::_ConstSingletonScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        value = self.get_value()
        # We always return `value` regardless of the JAX Model or indexes
        return self._wrap_jax(m, lambda jm: value)
  %}
}

%extend IMP::internal::_ConstPairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        value = self.get_value()
        # We always return `value` regardless of the JAX Model or indexes
        return self._wrap_jax(m, lambda jm: value)
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
        raise NotImplementedError(
            f"No JAX implementation for {self} ({type(self)})")
  %}
}

%extend IMP::ScoringFunction {
  %pythoncode %{
    def _get_jax(self):
        r = self.get_single_restraint()
        if r is None:
            raise NotImplementedError(
                f"No JAX implementation for {self} ({type(self)})")
        else:
            return r.get_derived_object()._get_jax()

    def _evaluate_jax(self):
        """Similar to evaluate(False), but using JAX.
           This is intended to be useful for testing purposes. It will likely
           not be particularly fast as it will copy the IMP Model and
           jax.jit-compile the scoring function each time."""
        import jax
        ji = self._get_jax()
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        return j(jm)
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

           Alternatively, a pure JAX implementation can be provided.
           See _wrap_jax for more information.
        """
        raise NotImplementedError(
            f"No JAX implementation for {self} ({type(self)})")

    def _wrap_jax(self, init_func, apply_func):
        """Create the return value for _get_jax.
           Use this method in _get_jax() to wrap the JAX function
           with other OptimizerState-specific information.

           @param init_func a JAX function which, given a JAX Optimizer,
                  does any necessary setup and returns a (possibly modified)
                  JAX Optimizer. If any OptimizerState-specific
                  persistent state is needed, it can be stored in
                  `opt.optimizer_states[state_index]` and later retrieved or
                  modified in apply_func.
           @param apply_func a JAX function which, given a JAX Optimizer,
                  does the JAX equivalent of do_update() and returns a new
                  JAX Optimizer.
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
        def jax_sf(jm):
            if funcs:
                return sum(f(jm) for f in funcs)
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
                   a single argument (the current JAX Model) and returns
                   a new JAX Model with the ScoreState's transformation
                   applied.
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
           that modifies the JAX Model. See also _wrap_jax.
        """
        raise NotImplementedError(
            f"No JAX implementation for {self} ({type(self)})")
  %}
}

%extend IMP::SingletonModifier {
  %pythoncode %{
    def _wrap_jax(self, apply_func, keys=None):
        """Create the return value for _get_jax.
           Use this method in _get_jax() to wrap the JAX function
           with other modifier-specific information.

           @param apply_func A function implemented using JAX that takes
                  one argument (the current JAX Model) and returns a new
                  modified JAX Model.
           @param keys Model attributes used by the SingletonModifier.
                  See IMP::Restraint::_wrap_jax.
        """
        from IMP._jax_util import JAXModifierInfo
        return JAXModifierInfo(apply_func=apply_func, keys=keys)

    def _get_jax(self, m, indexes):
        """Return a JAX implementation of this SingletonModifier.
           Implement this method in a SingletonModifier subclass to provide
           an equivalent function using [JAX](https://docs.jax.dev/)
           that modifies the current JAX Model. See also _wrap_jax.

           @param m The IMP.Model that the modifier will act on.
           @param indexes The ParticleIndexes that the modifier will act on.
        """
        raise NotImplementedError(
            f"No JAX implementation for {self} ({type(self)})")
  %}
}

%extend IMP::internal::GenericRestraintsScoringFunction<::IMP::Restraints> {
  %pythoncode %{
    def _get_jax(self):
        import IMP._jax_util
        jis = [r.get_derived_object()._get_jax() for r in self.restraints]
        funcs = [j.score_func for j in jis]
        keys = frozenset(x for j in jis for x in j._keys)
        def jax_sf(jm):
            return sum(f(jm) for f in funcs)
        return IMP._jax_util.JAXRestraintInfo(
            m=self.get_model(), score_func=jax_sf, weight=1.0, keys=keys)
  %}
}
