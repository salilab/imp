import utils
import os
import unittest

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.multi_state_scheme


class Tests(unittest.TestCase):

    def test_multistatescheme_init(self):
        """Test the initialization of MultiStateScheme"""
        class MockObject(object):
            pass
        s1 = MockObject()
        s2 = MockObject()
        s1.name = 's1'
        s2.name = 's2'
        mssc1 = ihm.multi_state_scheme.Connectivity(
            begin_state=s1,
            end_state=s2)
        mss1 = ihm.multi_state_scheme.MultiStateScheme(
            name='n',
            details='d',
            connectivities=[mssc1],
            relaxation_times=['lr'])
        self.assertEqual(mss1.name, 'n')
        self.assertEqual(mss1.details, 'd')
        self.assertEqual(mss1._connectivity_list, [mssc1])
        self.assertEqual(mss1._relaxation_time_list, ['lr'])
        self.assertEqual(mss1._states, [s1, s2])

        mss2 = ihm.multi_state_scheme.MultiStateScheme(
            name='n2',
            details='d2',
            connectivities=[],
            relaxation_times=[])
        self.assertEqual(len(mss2._connectivity_list), 0)
        self.assertEqual(len(mss2._relaxation_time_list), 0)

        mss3 = ihm.multi_state_scheme.MultiStateScheme(
            name='n3',
            details='d3',
            connectivities=[mssc1, mssc1],
            relaxation_times=['lr', 'lr']
        )
        self.assertEqual(mss3._connectivity_list, [mssc1])
        self.assertEqual(mss3._relaxation_time_list, ['lr'])

        # Handle empty states (i.e. without models assigned)
        # Both states should still be added
        s1 = ihm.model.State(
            elements=[],
            type='conformational change',
            name='s1',
            experiment_type='Single molecule',
            population_fraction=0.4)
        s2 = ihm.model.State(
            elements=[],
            type='conformational change',
            name='s2',
            experiment_type='Single molecule',
            population_fraction=0.6)
        mssc2 = ihm.multi_state_scheme.Connectivity(begin_state=s1,
                                                    end_state=s2)
        mss4 = ihm.multi_state_scheme.MultiStateScheme(
            name='n4',
            details='d4',
            connectivities=[mssc2]
        )
        self.assertEqual(len(mss4._states), 2)
        self.assertEqual(mss4._states, [s1, s2])
        # Adding a connectivity with the same states again, should not
        # add the states again
        mssc3 = ihm.multi_state_scheme.Connectivity(begin_state=s2)
        mss4.add_connectivity(mssc3)
        self.assertEqual(len(mss4._states), 2)
        self.assertEqual(mss4._states, [s1, s2])
        # Compare a state with and without additional information
        mssc4 = ihm.multi_state_scheme.Connectivity(begin_state=s2,
                                                    end_state=[])
        mss4.add_connectivity(mssc4)
        self.assertEqual(len(mss4._states), 3)
        self.assertEqual(mss4._states, [s1, s2, []])

    def test_multistatescheme_add_connectivity(self):
        """Test addition of a connectivity to a MultiStateScheme"""
        class MockObject(object):
            pass
        mss1 = ihm.multi_state_scheme.MultiStateScheme(name='n',
                                                       details='d')
        # The connectivity_list should be empty upon initialization
        self.assertEqual(len(mss1._connectivity_list), 0)
        # Add a connectivity should add it to the connectivity_list and the
        # states should be stored as well
        s1 = MockObject()
        s2 = MockObject()
        s1.name = 's1'
        s2.name = 's2'
        mssc1 = ihm.multi_state_scheme.Connectivity(
            begin_state=s1,
            end_state=s2)
        mss1.add_connectivity(mssc1)
        self.assertEqual(len(mss1._connectivity_list), 1)
        self.assertEqual(mss1._connectivity_list, [mssc1])
        self.assertEqual(mss1._states, [s1, s2])
        # add a connectivity without end_state
        s3 = MockObject()
        s3.name = 's3'
        mssc2 = ihm.multi_state_scheme.Connectivity(
            begin_state=s3)
        mss1.add_connectivity(mssc2)
        self.assertEqual(len(mss1._connectivity_list), 2)
        self.assertEqual(mss1._connectivity_list, [mssc1,
                                                   mssc2])
        self.assertEqual(mss1._states, [s1, s2, s3])
        # add a connectivity with a previously known state should not add it
        # to the states
        s4 = MockObject()
        s4.name = 's4'
        mssc3 = ihm.multi_state_scheme.Connectivity(
            begin_state=s2,
            end_state=s4)
        mss1.add_connectivity(mssc3)
        self.assertEqual(len(mss1._connectivity_list), 3)
        self.assertEqual(mss1._connectivity_list, [mssc1, mssc2, mssc3])
        self.assertEqual(mss1._states, [s1, s2, s3, s4])
        mss1.add_connectivity(None)
        self.assertEqual(len(mss1._connectivity_list), 3)
        self.assertEqual(mss1._connectivity_list, [mssc1, mssc2, mssc3])
        self.assertEqual(mss1._states, [s1, s2, s3, s4])

    def test_multistatescheme_add_relaxation_time(self):
        """Test addition of a relaxation time to a MultiStateScheme"""
        mss1 = ihm.multi_state_scheme.MultiStateScheme(name='n')
        # The relaxation_time_list should be empty upon initialization
        self.assertEqual(len(mss1._relaxation_time_list), 0)
        # Add a relaxation time
        mss1.add_relaxation_time('r')
        self.assertEqual(mss1._relaxation_time_list, ['r'])

    def test_multistatesscheme_get_connectivities(self):
        """Test the return of connectivities from a MultiStateScheme"""
        class MockObject(object):
            pass
        s1 = MockObject()
        s2 = MockObject()
        s3 = MockObject()
        s4 = MockObject()
        s1.name = 's1'
        s2.name = 's2'
        s3.name = 's3'
        s4.name = 's4'
        mssc1 = ihm.multi_state_scheme.Connectivity(
            begin_state=s1,
            end_state=s2)
        mssc2 = ihm.multi_state_scheme.Connectivity(
            begin_state=s2,
            end_state=s3)
        mssc3 = ihm.multi_state_scheme.Connectivity(
            begin_state=s2,
            end_state=s4)
        mss1 = ihm.multi_state_scheme.MultiStateScheme(
            name='n',
            connectivities=[mssc1, mssc2])
        self.assertEqual(mss1.get_connectivities(), [mssc1, mssc2])
        mss1.add_connectivity(mssc3)
        # Test addition of the same connectivity multiple times
        self.assertEqual(mss1.get_connectivities(), [mssc1, mssc2, mssc3])
        mss1.add_connectivity(mssc3)
        self.assertEqual(mss1.get_connectivities(), [mssc1, mssc2, mssc3])
        mss1.add_connectivity(mssc1)
        self.assertEqual(mss1.get_connectivities(), [mssc1, mssc2, mssc3])

    def test_multistatescheme_get_relaxation_times(self):
        """Test the return of relaxation times from a MultiStateScheme"""
        mss1 = ihm.multi_state_scheme.MultiStateScheme(
            name='n',
            relaxation_times=['r1', 'r2']
        )
        self.assertEqual(mss1.get_relaxation_times(), ['r1', 'r2'])
        mss1.add_relaxation_time('r3')
        self.assertEqual(mss1.get_relaxation_times(), ['r1', 'r2', 'r3'])
        mss1.add_relaxation_time('r3')
        self.assertEqual(mss1.get_relaxation_times(), ['r1', 'r2', 'r3', 'r3'])

    def test_multistatescheme_get_states(self):
        """Test the return of states from a MultiStateScheme"""
        class MockObject(object):
            pass
        s1 = MockObject()
        s2 = MockObject()
        s3 = MockObject()
        s1.name = 's1'
        s2.name = 's2'
        s3.name = 's3'
        mssc1 = ihm.multi_state_scheme.Connectivity(
            begin_state=s1,
            end_state=s2)
        mssc2 = ihm.multi_state_scheme.Connectivity(
            begin_state=s1)
        mssc3 = ihm.multi_state_scheme.Connectivity(
            begin_state=s2,
            end_state=s3)

        mss1 = ihm.multi_state_scheme.MultiStateScheme(
            name='n',
            connectivities=[mssc1])
        self.assertEqual(mss1.get_states(), [s1, s2])
        mss1.add_connectivity(mssc2)
        self.assertEqual(mss1.get_states(), [s1, s2])
        mss1.add_connectivity(mssc3)
        self.assertEqual(mss1.get_states(), [s1, s2, s3])

    def test_multistatescheme_eq(self):
        """Test equality of MultiStateScheme objects"""
        class MockObject(object):
            pass
        s1 = MockObject()
        s2 = MockObject()
        s3 = MockObject()
        s4 = MockObject()
        s1.name = 's1'
        s2.name = 's2'
        s3.name = 's3'
        s4.name = 's4'
        mssc1 = ihm.multi_state_scheme.Connectivity(
            begin_state=s1)
        mssc2 = ihm.multi_state_scheme.Connectivity(
            begin_state=s2)
        mssc3 = ihm.multi_state_scheme.Connectivity(
            begin_state=s3)
        mssc4 = ihm.multi_state_scheme.Connectivity(
            begin_state=s1,
            end_state=s4)

        mss_ref = ihm.multi_state_scheme.MultiStateScheme(
            name='name1',
            details='details1',
            connectivities=[mssc1])
        mss_equal = ihm.multi_state_scheme.MultiStateScheme(
            name='name1',
            details='details1',
            connectivities=[mssc1])
        mss_unequal = ihm.multi_state_scheme.MultiStateScheme(
            name='name2',
            details='details2',
            connectivities=[mssc2])
        mss_unequal2 = ihm.multi_state_scheme.MultiStateScheme(
            name='name1',
            details='details1',
            connectivities=[mssc3])
        mss_unequal4 = ihm.multi_state_scheme.MultiStateScheme(
            name='name1',
            details='details1',
            connectivities=[mssc4])
        mss_unequal5 = ihm.multi_state_scheme.MultiStateScheme(
            name='name1',
            details='details1',
            connectivities=[mssc1],
            relaxation_times=['r1'])

        self.assertTrue(mss_ref == mss_equal)
        self.assertFalse(mss_ref == mss_unequal)
        self.assertTrue(mss_ref != mss_unequal)
        self.assertFalse(mss_ref == mss_unequal2)
        self.assertTrue(mss_ref != mss_unequal2)
        self.assertFalse(mss_ref == mss_unequal4)
        self.assertTrue(mss_ref != mss_unequal4)
        self.assertFalse(mss_ref == mss_unequal5)
        self.assertTrue(mss_ref != mss_unequal5)

    def test_multistateschemeconnectivity_init(self):
        """Test initialization of Connectivity"""
        class MockObject(object):
            pass
        s1 = MockObject()
        s2 = MockObject()
        s1.name = 's1'
        s2.name = 's2'

        mssc1 = ihm.multi_state_scheme.Connectivity(
            begin_state=s1,
            end_state=s2,
            details='details1',
            dataset_group='dataset_group1',
            kinetic_rate='kinetic_rate1',
            relaxation_time='relaxation_time1')
        self.assertEqual(mssc1.begin_state, s1)
        self.assertEqual(mssc1.end_state, s2)
        self.assertEqual(mssc1.details, 'details1')
        self.assertEqual(mssc1.dataset_group, 'dataset_group1')
        self.assertEqual(mssc1.kinetic_rate, 'kinetic_rate1')
        self.assertEqual(mssc1.relaxation_time, 'relaxation_time1')

    def test_multistateschemeconnectivity_eq(self):
        """Test equality of Connectivity objects"""
        class MockObject(object):
            pass
        s1 = MockObject()
        s2 = MockObject()
        s3 = MockObject()
        s1.name = 's1'
        s2.name = 's2'
        s3.name = 's3'
        mssc_ref = ihm.multi_state_scheme.Connectivity(
            begin_state=s1,
            end_state=s2,
            details='details1',
            dataset_group='dataset_group1',
            kinetic_rate='kinetic_rate1',
            relaxation_time='relaxation_time1')

        mssc_equal = ihm.multi_state_scheme.Connectivity(
            begin_state=s1,
            end_state=s2,
            details='details1',
            dataset_group='dataset_group1',
            kinetic_rate='kinetic_rate1',
            relaxation_time='relaxation_time1')

        mssc_unequal1 = ihm.multi_state_scheme.Connectivity(
            begin_state=s1,
            end_state=s3,
            details='details1',
            dataset_group='dataset_group1',
            kinetic_rate='kinetic_rate1',
            relaxation_time='relaxation_time1')

        mssc_unequal2 = ihm.multi_state_scheme.Connectivity(
            begin_state=s1,
            details='details1',
            dataset_group='dataset_group1',
            kinetic_rate='kinetic_rate1',
            relaxation_time='relaxation_time1')

        self.assertTrue(mssc_ref == mssc_equal)
        self.assertFalse(mssc_ref == mssc_unequal1)
        self.assertTrue(mssc_ref != mssc_unequal1)
        self.assertFalse(mssc_ref == mssc_unequal2)
        self.assertTrue(mssc_ref != mssc_unequal2)

    def test_equilibriumconstant_init(self):
        """Test initialization of EquilibriumConstant and the
        derived classes"""
        e1 = ihm.multi_state_scheme.EquilibriumConstant(value=1.0,
                                                        unit='a')
        self.assertIsInstance(e1, ihm.multi_state_scheme.EquilibriumConstant)
        self.assertEqual(e1.value, 1.0)
        self.assertEqual(e1.unit, 'a')
        self.assertEqual(
            e1.method,
            'equilibrium constant is determined from another method '
            'not listed')

        e2 = ihm.multi_state_scheme.EquilibriumConstant(value=2.0)
        self.assertIsInstance(e2, ihm.multi_state_scheme.EquilibriumConstant)
        self.assertEqual(e2.value, 2.0)
        self.assertIsNone(e2.unit)
        self.assertEqual(
            e2.method,
            'equilibrium constant is determined from another method '
            'not listed')

        e3 = ihm.multi_state_scheme.PopulationEquilibriumConstant(value=3.0,
                                                                  unit='b')
        self.assertIsInstance(
            e3,
            ihm.multi_state_scheme.PopulationEquilibriumConstant)
        self.assertEqual(e3.value, 3.0)
        self.assertEqual(e3.unit, 'b')
        self.assertEqual(
            e3.method,
            'equilibrium constant is determined from population')

        e4 = ihm.multi_state_scheme.KineticRateEquilibriumConstant(value=4.0,
                                                                   unit='c')
        self.assertIsInstance(
            e4,
            ihm.multi_state_scheme.KineticRateEquilibriumConstant)
        self.assertEqual(e4.value, 4.0)
        self.assertEqual(e4.unit, 'c')
        self.assertEqual(
            e4.method,
            'equilibrium constant is determined from kinetic rates, kAB/kBA')

    def test_equilibrium_constant_eq(self):
        """Test equality of EquilibriumConstant objetcs"""
        e_ref1 = ihm.multi_state_scheme.EquilibriumConstant(
            value='1.0',
            unit='a')
        e_equal1 = ihm.multi_state_scheme.EquilibriumConstant(
            value='1.0',
            unit='a')
        eq_unequal1 = ihm.multi_state_scheme.EquilibriumConstant(
            value='1.0',
            unit='b')
        eq_unequal2 = ihm.multi_state_scheme.EquilibriumConstant(
            value='1.0')
        eq_unequal3 = ihm.multi_state_scheme.EquilibriumConstant(
            value='2.0',
            unit='b')
        eq_unequal4 = ihm.multi_state_scheme.PopulationEquilibriumConstant(
            value='1.0',
            unit='a')
        eq_unequal5 = ihm.multi_state_scheme.KineticRateEquilibriumConstant(
            value='1.0',
            unit='a')
        self.assertTrue(e_ref1 == e_equal1)
        self.assertFalse(e_ref1 == eq_unequal1)
        self.assertFalse(e_ref1 == eq_unequal2)
        self.assertFalse(e_ref1 == eq_unequal3)
        self.assertFalse(e_ref1 == eq_unequal4)
        self.assertFalse(e_ref1 == eq_unequal5)

        e_ref2 = ihm.multi_state_scheme.PopulationEquilibriumConstant(
            value='1.0',
            unit='a')
        e_equal2 = ihm.multi_state_scheme.PopulationEquilibriumConstant(
            value='1.0',
            unit='a')
        e_unequal6 = ihm.multi_state_scheme.PopulationEquilibriumConstant(
            value='2.0',
            unit='a')
        e_unequal7 = ihm.multi_state_scheme.EquilibriumConstant(
            value='1.0',
            unit='a')
        e_unequal8 = ihm.multi_state_scheme.KineticRateEquilibriumConstant(
            value='1.0',
            unit='a')
        self.assertTrue(e_ref2 == e_equal2)
        self.assertFalse(e_ref2 == e_unequal6)
        self.assertFalse(e_ref2 == e_unequal7)
        self.assertFalse(e_ref2 == e_unequal8)

        e_ref3 = ihm.multi_state_scheme.KineticRateEquilibriumConstant(
            value='1.0',
            unit='a')
        e_equal3 = ihm.multi_state_scheme.KineticRateEquilibriumConstant(
            value='1.0',
            unit='a')
        e_unequal9 = ihm.multi_state_scheme.KineticRateEquilibriumConstant(
            value='2.0',
            unit='a')
        e_unequal10 = ihm.multi_state_scheme.EquilibriumConstant(
            value='1.0',
            unit='a')
        self.assertTrue(e_ref3 == e_equal3)
        self.assertFalse(e_ref3 == e_unequal9)
        self.assertFalse(e_ref3 == e_unequal10)

    def test_kineticrate_init(self):
        """Test initialization of KineticRate"""
        # Initialization with only transition_rate_constant given
        k1 = ihm.multi_state_scheme.KineticRate(transition_rate_constant=1.0)
        self.assertEqual(k1.transition_rate_constant, 1.0)
        e1 = ihm.multi_state_scheme.PopulationEquilibriumConstant(value=1.0,
                                                                  unit='unit')
        # Initialization with equilibrium_constant
        k2 = ihm.multi_state_scheme.KineticRate(
            equilibrium_constant=e1)
        self.assertEqual(k2.equilibrium_constant.value, 1.0)
        self.assertEqual(k2.equilibrium_constant.unit, "unit")
        self.assertEqual(k2.equilibrium_constant.method,
                         'equilibrium constant is determined from population')
        # Initialization with all values given
        k3 = ihm.multi_state_scheme.KineticRate(
            transition_rate_constant=0.5,
            equilibrium_constant=e1,
            details="details1",
            dataset_group='dataset_group1',
            file='file1')
        self.assertEqual(k3.transition_rate_constant, 0.5)
        self.assertEqual(k3.equilibrium_constant.value, 1.0)
        self.assertEqual(k3.equilibrium_constant.unit, "unit")
        self.assertEqual(k3.equilibrium_constant.method,
                         'equilibrium constant is determined from population')
        self.assertEqual(k3.details, "details1")
        self.assertEqual(k3.dataset_group, 'dataset_group1')
        self.assertEqual(k3.external_file, 'file1')

    def test_kineticrate_eq(self):
        """Test equality of KineticRate objects"""
        e1 = ihm.multi_state_scheme.PopulationEquilibriumConstant(
            value=1.5, unit='unit1')
        e2 = ihm.multi_state_scheme.KineticRateEquilibriumConstant(
            value=1.5, unit='unit1')
        k_ref = ihm.multi_state_scheme.KineticRate(
            transition_rate_constant=1.0,
            equilibrium_constant=e1,
            details="details1",
            dataset_group="dataset_group1",
            file="file1"
        )
        k_equal = ihm.multi_state_scheme.KineticRate(
            transition_rate_constant=1.0,
            equilibrium_constant=e1,
            details="details1",
            dataset_group="dataset_group1",
            file="file1"
        )
        k_unequal1 = ihm.multi_state_scheme.KineticRate(
            transition_rate_constant=2.0,
            equilibrium_constant=e1,
            details="details1",
            dataset_group="dataset_group1",
            file="file1"
        )
        k_unequal2 = ihm.multi_state_scheme.KineticRate(
            transition_rate_constant=1.0,
            equilibrium_constant=e2,
            details="details1",
            dataset_group="dataset_group1",
            file="file1"
        )
        k_unequal3 = ihm.multi_state_scheme.KineticRate(
            transition_rate_constant=1.0,
            details="details1",
            dataset_group="dataset_group1",
            file="file1"
        )
        self.assertTrue(k_ref == k_equal)
        self.assertFalse(k_ref == k_unequal1)
        self.assertTrue(k_ref != k_unequal1)
        self.assertFalse(k_ref == k_unequal2)
        self.assertTrue(k_ref != k_unequal2)
        self.assertFalse(k_ref == k_unequal3)
        self.assertTrue(k_ref != k_unequal3)

    def test_relaxationtime_init(self):
        """Test initialization of RelaxationTime"""
        r1 = ihm.multi_state_scheme.RelaxationTime(
            value=1.0,
            unit='milliseconds',
            details='details1',
            dataset_group='dataset_group1',
            file='file1'
        )
        self.assertEqual(r1.value, 1.0)
        self.assertEqual(r1.unit, 'milliseconds')
        self.assertEqual(r1.details, 'details1')
        self.assertEqual(r1.dataset_group, 'dataset_group1')
        self.assertEqual(r1.external_file, 'file1')
        # Check whether a wrong unit raises a ValueError
        self.assertRaises(
            ValueError,
            ihm.multi_state_scheme.RelaxationTime,
            value=2.0,
            unit='wrong_unit')

    def test_relaxationtime_eq(self):
        """Test equality of RelaxationTime objetcs"""
        r_ref = ihm.multi_state_scheme.RelaxationTime(
            value=1.0,
            unit='milliseconds',
            details='details1',
            dataset_group='dataset_group1',
            file='file1'
        )
        r_equal = ihm.multi_state_scheme.RelaxationTime(
            value=1.0,
            unit='milliseconds',
            details='details1',
            dataset_group='dataset_group1',
            file='file1'
        )
        r_unequal1 = ihm.multi_state_scheme.RelaxationTime(
            value=2.0,
            unit='milliseconds',
            details='details1',
            dataset_group='dataset_group1',
            file='file1'
        )
        r_unequal2 = ihm.multi_state_scheme.RelaxationTime(
            value=1.0,
            unit='seconds',
            details='details1',
            dataset_group='dataset_group1',
            file='file1'
        )
        r_unequal3 = ihm.multi_state_scheme.RelaxationTime(
            value=1.0,
            unit='milliseconds',
            details='details2',
            dataset_group='dataset_group1',
            file='file1'
        )
        r_unequal4 = ihm.multi_state_scheme.RelaxationTime(
            value=1.0,
            unit='milliseconds',
            details='details',
            dataset_group='dataset_group2',
            file='file1'
        )
        r_unequal5 = ihm.multi_state_scheme.RelaxationTime(
            value=1.0,
            unit='milliseconds',
            details='details1',
            file='file1'
        )
        self.assertTrue(r_ref == r_equal)
        self.assertFalse(r_ref == r_unequal1)
        self.assertTrue(r_ref != r_unequal1)
        self.assertFalse(r_ref == r_unequal2)
        self.assertTrue(r_ref != r_unequal2)
        self.assertFalse(r_ref == r_unequal3)
        self.assertTrue(r_ref != r_unequal3)
        self.assertFalse(r_ref == r_unequal4)
        self.assertTrue(r_ref != r_unequal4)
        self.assertFalse(r_ref == r_unequal5)
        self.assertTrue(r_ref != r_unequal5)


if __name__ == '__main__':
    unittest.main()
