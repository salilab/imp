# coding=utf-8

import ihm
from ihm.model import _text_choice_property

"""Classes for handling connected/ordered schemes formed by multiple state
    together with information on kinetic schemes"""


class MultiStateScheme:
    """MultiStateScheme collects information about a collection of
       multiple states, that can form a connected/ordered scheme.
       A special case is a kinetic scheme, for which kinetic rates and
       relaxation times are available.

       :param str name: The name of the multi-state scheme.
       :param str details: Details on the scheme.
       :param connectivities: A list of connectivities that belong to
        the scheme.
       :type connectivities: List of :class:`Connectivity`
       :param relaxation_times: A list of relaxation times not assigned
        to specific connectivities, but to the scheme
       :type relaxation_times: List of :class:`RelaxationTime`
    """
    def __init__(self, name, details=None, connectivities=None,
                 relaxation_times=None):
        self.name = name
        self.details = details
        self._connectivity_list = []
        self._relaxation_time_list = []
        # states is filled automatically based on connectivity_list
        self._states = []

        if connectivities is not None:
            for c in connectivities:
                if c not in self._connectivity_list:
                    self.add_connectivity(c)
        if relaxation_times is not None:
            for r in relaxation_times:
                if r not in self._relaxation_time_list:
                    self.add_relaxation_time(r)

    def add_connectivity(self, connectivity):
        """Add a connectivity to the scheme.

        :param connectivity: The connectivity to add to the scheme
        :type connectivity: :class:`Connectivity`
        """
        if connectivity is None:
            return
        if connectivity not in self._connectivity_list:
            # Make sure that the connectivity has not been assigned to
            # another scheme
            if not connectivity._assigned_to_scheme:
                connectivity.set_assigned_to_scheme()
                self._connectivity_list.append(connectivity)
            # If the connectivity has been assigned to another scheme,
            # create a copy of the connectivity and use that
            else:
                old_connectivity = connectivity
                connectivity = \
                    ihm.multi_state_scheme.Connectivity(
                        begin_state=old_connectivity.begin_state,
                        end_state=old_connectivity.end_state,
                        details=old_connectivity.details,
                        dataset_group=old_connectivity.dataset_group,
                        kinetic_rate=old_connectivity.kinetic_rate,
                        relaxation_time=old_connectivity.relaxation_time
                    )
                connectivity.set_assigned_to_scheme()
                self._connectivity_list.append(connectivity)

        # Add the states that belong to the connectivity
        self._add_state(connectivity.begin_state)
        self._add_state(connectivity.end_state)

    def _add_state(self, state):
        """Add a state to the self._states list if it is not present yet.
        This function checks whether the state has optional properties,
        such as a name. If this is the case, the name is compared to the names
        already in the list. If the state does not have a name, it might only
        be a list of elements. Then only the contents of the list are checked
        This is important for empty states, i.e. those that do not have
        models associated.

        :param state: The state to add.
        :type state: :class:`ihm.model.State`
        """
        if state is None:
            return
        for tmp_state in self._states:
            # Check whether both states have the name attributes
            if hasattr(state, 'name') and hasattr(tmp_state, 'name'):
                # compare the properties of the two states and the elements of
                # the lists
                if state.__dict__ == tmp_state.__dict__ \
                        and state == tmp_state:
                    # state found
                    return
            # If neither of the two states has the name attribute, only compare
            # the elements of the lists
            if not hasattr(state, 'name') and not hasattr(tmp_state, 'name'):
                # If the two states have the same elements
                if state == tmp_state:
                    # state found
                    return
        # If the state was not found in the list yet, add it
        self._states.append(state)

    def add_relaxation_time(self, relaxation_time):
        """Add a relaxation time to the scheme. This relaxation time is not
        assigned to a connectivity.

        :param relaxation_time: The relaxation time to add to the scheme.
        :type relaxation_time: :class:`RelaxationTime`
        """
        if relaxation_time is not None:
            self._relaxation_time_list.append(relaxation_time)

    def get_connectivities(self):
        """Return the connectivities assigned to a scheme"""
        return self._connectivity_list

    def get_relaxation_times(self):
        """Return the relaxation times assigned to a scheme"""
        return self._relaxation_time_list

    def get_states(self):
        """Return the states involved in a scheme"""
        return self._states

    def __eq__(self, other):
        return ((self.__dict__ == other.__dict__)
                and (self._connectivity_list ==
                     other._connectivity_list)
                and (self._relaxation_time_list ==
                     other._relaxation_time_list))


class Connectivity:
    """A connectivity between states. Used to describe the directed
    edge of graph.
    If no end_state is given, the state is not connected to another state.
    This could be the case for states where no connection to other states
    could be resolved.

    :param begin_state: The start state of the connectivity.
    :type begin_state: :class:`ihm.model.State`
    :param end_state: The end state of the connectivity. Can be None in case
     of states that are not connected to others.
    :type end_state: :class:`ihm.model.State`
    :param details: Details to the connectivity.
    :param dataset_group: The DatasetGroup that was used to obtain information
     on the connectivity.
    :type dataset_group: :class:`ihm.dataset.DatasetGroup`
    :param kinetic_rate: A kinetic rate assigned to the connectivity.
    :type kinetic_rate: :class:`KineticRate`
    :param relaxation_time: A relaxation time assigned to the connectivity.
    :type relaxation_time: :class:`RelaxationTime`
    """
    def __init__(self, begin_state, end_state=None, details=None,
                 dataset_group=None, kinetic_rate=None, relaxation_time=None):
        self.begin_state = begin_state
        self.end_state = end_state
        self.details = details
        self.dataset_group = dataset_group
        self.kinetic_rate = kinetic_rate
        self.relaxation_time = relaxation_time
        # The _assigned_to_scheme variable tracks whether the connectivity
        # has been assigned to a scheme. This is to ensure that each
        # connectivity is only assigned to a single scheme.
        self._assigned_to_scheme = False

    def set_assigned_to_scheme(self):
        self._assigned_to_scheme = True

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class KineticRate:
    """A base class for a kinetic rate that can be assigned to a connectivity.
    The kinetic rate could be a transition_rate_constant or
    an equilibrium_constant. Alternatively, both could be provided.

    :param float transition_rate_constant: A transition rate constant
     describing the exchange between two states. Unit: per second.
    :param equilibrium_constant: An equilibrium constant describing the
     exchange between two states
    :type equilibrium_constant: :class:`EquilibriumConstant` or
     :class:`PopulationEquilibriumConstant` or
     :class:`KineticRateEquilibriumConstant`
    :param str details: Details on the kinetic rate.
    :param dataset_group: The DatasetGroup used to determine the kinetic rate.
    :type dataset_group: :class:`ihm.dataset.DatasetGroup`
    :param file: External file containing measurement data for the kinetic
     rate.
    :type file: :class:`ihm.location.OutputFileLocation`

    """
    def __init__(self,
                 transition_rate_constant=None,
                 equilibrium_constant=None,
                 details=None,
                 dataset_group=None,
                 file=None):
        self.transition_rate_constant = transition_rate_constant
        self.equilibrium_constant = equilibrium_constant
        self.details = details
        self.dataset_group = dataset_group
        self.external_file = file

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class EquilibriumConstant:
    """Base class for an equilibrium constant.
    This class handles the case that none of the derived classes is applicable.

    :param float value: The value of the equilibrium constant
    :param str unit: Unit of the equilibrium constant. Depending on what
     the process describes, a unit might be applicable or not"""
    def __init__(self, value, unit=None):
        self.method = 'equilibrium constant is determined from another ' \
                      'method not listed'
        self.value = value
        self.unit = unit

    def __eq__(self, other):
        if other is None:
            return False
        return self.__dict__ == other.__dict__


class PopulationEquilibriumConstant(EquilibriumConstant):
    """An equilibrium constant determined from population"""
    def __init__(self, value, unit=None):
        super(PopulationEquilibriumConstant, self).__init__(value, unit)
        self.method = 'equilibrium constant is determined from population'


class KineticRateEquilibriumConstant(EquilibriumConstant):
    """An equilibrium constant determined from kinetic rates as kAB/kBA"""
    def __init__(self, value, unit=None):
        super(KineticRateEquilibriumConstant, self).__init__(value, unit)
        self.method = 'equilibrium constant is determined from kinetic ' \
                      'rates, kAB/kBA'


class RelaxationTime:
    """A relaxation time determined for a scheme.
    The relaxation time can either be connected to a specific connectivity
    in the scheme or to the scheme in general if no assignment is possible.

    :param float value: The relaxation time.
    :param str unit: The unit of the relaxation time. Options are
     ['seconds','milliseconds', microseconds']
    :param float amplitude: The amplitude of the relaxation time if determined.
    :param str details: Details on the relaxation time.
    :param dataset_group: DatasetGroup used to determine the relaxation time.
    :type dataset_group: :class:`ihm.dataset.DatasetGroup`
    :param file: An external file containing measurement data for
     the relaxation time.
    :type file: :class:`ihm.location.OutputFileLocation`

    """
    def __init__(self, value, unit, amplitude=None,
                 details=None, dataset_group=None, file=None):

        self.value = value
        self.unit = unit
        self.amplitude = amplitude
        self.details = details
        self.dataset_group = dataset_group
        self.external_file = file

    def __eq__(self, other):
        return self.__dict__ == other.__dict__

    # Check whether the given unit is within the allowed options
    allowed_relaxation_time_units = ['seconds',
                                     'milliseconds',
                                     'microseconds']
    unit = _text_choice_property(
        "unit",
        allowed_relaxation_time_units,
        doc="The unit of the relaxation time.")
