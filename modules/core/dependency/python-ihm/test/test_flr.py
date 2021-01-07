import utils
import os
import unittest

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.flr


class Tests(unittest.TestCase):

    def test_Probe_Init(self):
        """Test initialization of probe_list_entry and probe_descriptor."""
        p = ihm.flr.Probe(probe_list_entry='foo', probe_descriptor='bar')
        self.assertEqual(p.probe_list_entry, 'foo')
        self.assertEqual(p.probe_descriptor, 'bar')

    def test_Probe_Eq(self):
        """Test equality and inequality of Probe objects"""
        p_ref = ihm.flr.Probe(probe_list_entry='foo', probe_descriptor='bar')
        p_equal = ihm.flr.Probe(probe_list_entry='foo', probe_descriptor='bar')
        p_unequal = ihm.flr.Probe(
            probe_list_entry='foo2', probe_descriptor='bar')
        self.assertTrue(p_ref == p_equal)
        self.assertFalse(p_ref == p_unequal)
        self.assertTrue(p_ref != p_unequal)

    def test_probe_descriptor_init(self):
        """ Test initalization of ProbeDescriptor """
        p = ihm.flr.ProbeDescriptor(reactive_probe_chem_descriptor='foo',
                                    chromophore_chem_descriptor='bar',
                                    chromophore_center_atom='foo2')
        self.assertEqual(p.reactive_probe_chem_descriptor, 'foo')
        self.assertEqual(p.chromophore_chem_descriptor, 'bar')
        self.assertEqual(p.chromophore_center_atom, 'foo2')

    def test_probe_descriptor_eq(self):
        """Test equality and inequality of ProbeDescriptor objects."""
        p_ref = ihm.flr.ProbeDescriptor(reactive_probe_chem_descriptor='foo',
                                        chromophore_chem_descriptor='bar',
                                        chromophore_center_atom='foo2')
        p_equal = ihm.flr.ProbeDescriptor(reactive_probe_chem_descriptor='foo',
                                          chromophore_chem_descriptor='bar',
                                          chromophore_center_atom='foo2')
        p_unequal = ihm.flr.ProbeDescriptor(
            reactive_probe_chem_descriptor='foo',
            chromophore_chem_descriptor='bar2', chromophore_center_atom='foo2')
        self.assertTrue(p_ref == p_equal)
        self.assertFalse(p_ref == p_unequal)
        self.assertTrue(p_ref != p_unequal)

    def test_probe_list_init(self):
        """ Test initialization of ProbeList. """
        p = ihm.flr.ProbeList(chromophore_name='foo',
                              reactive_probe_flag=False,
                              reactive_probe_name='bar',
                              probe_origin='foo2',
                              probe_link_type='bar2')
        self.assertEqual(p.chromophore_name, 'foo')
        self.assertEqual(p.reactive_probe_flag, False)
        self.assertEqual(p.reactive_probe_name, 'bar')
        self.assertEqual(p.probe_origin, 'foo2')
        self.assertEqual(p.probe_link_type, 'bar2')

    def test_probe_list_eq(self):
        """Test equality and inequality of ProbeList objects."""
        p_ref = ihm.flr.ProbeList(chromophore_name='foo',
                                  reactive_probe_flag=False,
                                  reactive_probe_name='bar',
                                  probe_origin='foo2',
                                  probe_link_type='bar2')
        p_equal = ihm.flr.ProbeList(chromophore_name='foo',
                                    reactive_probe_flag=False,
                                    reactive_probe_name='bar',
                                    probe_origin='foo2',
                                    probe_link_type='bar2')
        p_unequal = ihm.flr.ProbeList(chromophore_name='foo2',
                                      reactive_probe_flag=True,
                                      reactive_probe_name='bar',
                                      probe_origin='foo2',
                                      probe_link_type='bar2')
        self.assertTrue(p_ref == p_equal)
        self.assertFalse(p_ref == p_unequal)
        self.assertTrue(p_ref != p_unequal)

    def test_sample_probe_details_init(self):
        """ Test initialization of SampleProbeDetails. """
        s = ihm.flr.SampleProbeDetails(sample='foo',
                                       probe='bar',
                                       description='foo2',
                                       poly_probe_position='bar2')
        self.assertEqual(s.sample, 'foo')
        self.assertEqual(s.probe, 'bar')
        self.assertEqual(s.fluorophore_type, 'unspecified')
        self.assertEqual(s.description, 'foo2')
        self.assertEqual(s.poly_probe_position, 'bar2')

    def test_sample_probe_details_eq(self):
        """Test equality and inequality of SampleProbeDetails objects."""
        s_ref = ihm.flr.SampleProbeDetails(
            sample='foo', probe='bar', description='foo2',
            poly_probe_position='bar2')
        s_equal = ihm.flr.SampleProbeDetails(
            sample='foo', probe='bar', description='foo2',
            poly_probe_position='bar2')
        s_unequal = ihm.flr.SampleProbeDetails(
            sample='foo', probe='bar3', description='foo2',
            poly_probe_position='bar2')

        self.assertTrue(s_ref == s_equal)
        self.assertFalse(s_ref == s_unequal)
        self.assertTrue(s_ref != s_unequal)

    def test_poly_probe_conjugate_init(self):
        """ Test initialization of PolyProbeConjugate. """
        p = ihm.flr.PolyProbeConjugate(sample_probe='foo',
                                       chem_descriptor='bar',
                                       ambiguous_stoichiometry=True,
                                       probe_stoichiometry=0.5)
        self.assertEqual(p.sample_probe, 'foo')
        self.assertEqual(p.chem_descriptor, 'bar')
        self.assertEqual(p.ambiguous_stoichiometry, True)
        self.assertEqual(p.probe_stoichiometry, 0.5)

    def test_poly_probe_conjugate_eq(self):
        """ Test equality and inequality of PolyProbeConjugate objects. """
        p_ref = ihm.flr.PolyProbeConjugate(sample_probe='foo',
                                           chem_descriptor='bar',
                                           ambiguous_stoichiometry=True,
                                           probe_stoichiometry=0.5)
        p_equal = ihm.flr.PolyProbeConjugate(sample_probe='foo',
                                             chem_descriptor='bar',
                                             ambiguous_stoichiometry=True,
                                             probe_stoichiometry=0.5)
        p_unequal = ihm.flr.PolyProbeConjugate(
            sample_probe='foo2', chem_descriptor='bar',
            ambiguous_stoichiometry=True, probe_stoichiometry=0.5)
        self.assertTrue(p_ref == p_equal)
        self.assertFalse(p_ref == p_unequal)
        self.assertTrue(p_ref != p_unequal)

    def test_poly_probe_position_init(self):
        """ Test initialization of PolyProbePosition. """
        p = ihm.flr.PolyProbePosition(resatom='foo',
                                      mutation_flag=True,
                                      modification_flag=True,
                                      auth_name='foo3',
                                      mutated_chem_comp_id='foobar',
                                      modified_chem_descriptor='foobar2')
        self.assertEqual(p.resatom, 'foo')
        self.assertEqual(p.mutation_flag, True)
        self.assertEqual(p.modification_flag, True)
        self.assertEqual(p.auth_name, 'foo3')
        self.assertEqual(p.mutated_chem_comp_id, 'foobar')
        self.assertEqual(p.modified_chem_descriptor, 'foobar2')

    def test_poly_probe_position_eq(self):
        """Test equality and inequality of PolyProbePosition objects."""
        p_ref = ihm.flr.PolyProbePosition(
            resatom='foo', mutation_flag=True, modification_flag=True,
            auth_name='foo3', mutated_chem_comp_id='foobar',
            modified_chem_descriptor='foobar2')
        p_equal = ihm.flr.PolyProbePosition(
            resatom='foo', mutation_flag=True, modification_flag=True,
            auth_name='foo3', mutated_chem_comp_id='foobar',
            modified_chem_descriptor='foobar2')
        p_unequal = ihm.flr.PolyProbePosition(
            resatom='bar', mutation_flag=True, modification_flag=True,
            auth_name='foo3', mutated_chem_comp_id='foobar',
            modified_chem_descriptor='foobar2')

        self.assertTrue(p_ref == p_equal)
        self.assertFalse(p_ref == p_unequal)
        self.assertTrue(p_ref != p_unequal)

    def test_sample_init(self):
        """Test initialization of Sample."""
        s = ihm.flr.Sample(entity_assembly='foo', num_of_probes='bar',
                           condition='foo2', description='foo3',
                           details='foo4', solvent_phase='foobar')
        self.assertEqual(s.entity_assembly, 'foo')
        self.assertEqual(s.num_of_probes, 'bar')
        self.assertEqual(s.condition, 'foo2')
        self.assertEqual(s.description, 'foo3')
        self.assertEqual(s.details, 'foo4')
        self.assertEqual(s.solvent_phase, 'foobar')

    def test_sample_eq(self):
        """Test equality and inequality of Sample objects."""
        s_ref = ihm.flr.Sample(entity_assembly='foo', num_of_probes='foo2',
                               condition='foo3', description='foo4',
                               details='foo5', solvent_phase='foo6')
        s_equal = ihm.flr.Sample(entity_assembly='foo', num_of_probes='foo2',
                                 condition='foo3', description='foo4',
                                 details='foo5', solvent_phase='foo6')
        s_unequal = ihm.flr.Sample(entity_assembly='bar', num_of_probes='bar2',
                                   condition='bar3', description='bar4',
                                   details='bar5', solvent_phase='bar6')
        self.assertTrue(s_ref == s_equal)
        self.assertFalse(s_ref == s_unequal)
        self.assertTrue(s_ref != s_unequal)

    def test_entity_assembly_init(self):
        """ Test initialization of EntityAssembly. """
        e = ihm.flr.EntityAssembly(entity='foo', num_copies=1)
        self.assertEqual(len(e.entity_list), 1)
        self.assertEqual(e.entity_list[0], 'foo')
        self.assertEqual(e.num_copies_list[0], 1)

    def test_entity_assembly_add_entity(self):
        """Test addition of entities to the assembly."""
        e = ihm.flr.EntityAssembly(entity='foo', num_copies=1)
        e.add_entity(entity='foo2', num_copies=2)
        self.assertRaises(ValueError, e.add_entity,
                          entity='foo2', num_copies=-1)
        self.assertEqual(len(e.entity_list), 2)
        self.assertEqual(e.entity_list, ['foo', 'foo2'])
        self.assertEqual(e.num_copies_list, [1, 2])

    def test_entity_assembly_eq(self):
        """ Test equality and inequality of EntityAssembly objects. """
        e_ref = ihm.flr.EntityAssembly(entity='foo', num_copies=1)
        e_equal = ihm.flr.EntityAssembly(entity='foo', num_copies=1)
        e_unequal = ihm.flr.EntityAssembly(entity='foo2', num_copies=1)
        self.assertTrue(e_ref == e_equal)
        self.assertFalse(e_ref == e_unequal)
        self.assertTrue(e_ref != e_unequal)

    def test_sample_condition_init(self):
        """ Test initialization of SampleCondition. """
        s = ihm.flr.SampleCondition(details='foo')
        self.assertEqual(s.details, 'foo')

    def test_sample_condition_eq(self):
        """ Test equality and inequality of SampleCondition objects. """
        s_ref = ihm.flr.SampleCondition(details='foo')
        s_equal = ihm.flr.SampleCondition(details='foo')
        s_unequal = ihm.flr.SampleCondition(details='bar')
        self.assertTrue(s_ref == s_equal)
        self.assertFalse(s_ref == s_unequal)
        self.assertTrue(s_ref != s_unequal)

    def test_Experiment_Init(self):
        """Test initialization of Experiment."""
        # Initialization with only one parameter given should not add an entry
        e1 = ihm.flr.Experiment(instrument='foo')
        self.assertEqual(len(e1.instrument_list), 0)
        self.assertEqual(len(e1.inst_setting_list), 0)
        self.assertEqual(len(e1.exp_condition_list), 0)
        self.assertEqual(len(e1.sample_list), 0)

        # Correct initialization should fill the lists
        e2 = ihm.flr.Experiment(instrument='foo',
                                inst_setting='bar',
                                exp_condition='foo2',
                                sample='foo3',
                                details='bar2')
        self.assertEqual(len(e2.instrument_list), 1)
        self.assertEqual(e2.instrument_list[0], 'foo')
        self.assertEqual(e2.inst_setting_list[0], 'bar')
        self.assertEqual(e2.exp_condition_list[0], 'foo2')
        self.assertEqual(e2.sample_list[0], 'foo3')
        self.assertEqual(e2.details_list[0], 'bar2')

        # Initialization without details given should still have an entry
        # in the list
        e3 = ihm.flr.Experiment(instrument='foo',
                                inst_setting='bar',
                                exp_condition='bar2',
                                sample='foo2')
        self.assertEqual(len(e3.details_list), 1)
        self.assertIsNone(e3.details_list[0])

    def test_Experiment_Add_entry(self):
        """ Test addition of an entry to the experiment. """
        # Adding to an empty Experiment
        e1 = ihm.flr.Experiment()
        e1.add_entry(instrument='foo',
                     inst_setting='bar',
                     exp_condition='foo2',
                     sample='foo3',
                     details='bar2')
        self.assertEqual(e1.instrument_list[0], 'foo')
        self.assertEqual(e1.inst_setting_list[0], 'bar')
        self.assertEqual(e1.exp_condition_list[0], 'foo2')
        self.assertEqual(e1.sample_list[0], 'foo3')
        self.assertEqual(e1.details_list[0], 'bar2')

        # adding to an existing Experiment
        e2 = ihm.flr.Experiment(instrument='foo',
                                inst_setting='foo2',
                                exp_condition='foo3',
                                sample='foo4',
                                details='foo5')
        e2.add_entry(instrument='bar',
                     inst_setting='bar2',
                     exp_condition='bar3',
                     sample='bar4',
                     details='bar5')
        self.assertEqual(e2.instrument_list, ['foo', 'bar'])
        self.assertEqual(e2.inst_setting_list, ['foo2', 'bar2'])
        self.assertEqual(e2.exp_condition_list, ['foo3', 'bar3'])
        self.assertEqual(e2.sample_list, ['foo4', 'bar4'])
        self.assertEqual(e2.details_list, ['foo5', 'bar5'])

    def test_Experiment_Get_entry_by_index(self):
        """ Test access to entries by index. """
        e = ihm.flr.Experiment()
        e.add_entry(instrument='foo', inst_setting='foo2',
                    exp_condition='foo3', sample='foo4', details='foo5')
        e.add_entry(instrument='bar', inst_setting='bar2',
                    exp_condition='bar3', sample='bar4', details='bar5')
        e.add_entry(instrument='foobar', inst_setting='foobar2',
                    exp_condition='foobar3', sample='foobar4',
                    details='foobar5')
        return_value_index0 = e.get_entry_by_index(0)
        return_value_index1 = e.get_entry_by_index(1)
        return_value_index2 = e.get_entry_by_index(2)
        self.assertEqual(return_value_index0,
                         ('foo', 'foo2', 'foo3', 'foo4', 'foo5'))
        self.assertEqual(return_value_index1,
                         ('bar', 'bar2', 'bar3', 'bar4', 'bar5'))
        self.assertEqual(return_value_index2,
                         ('foobar', 'foobar2', 'foobar3', 'foobar4',
                          'foobar5'))

    def test_Experiment_Contains(self):
        """Test whether experiment contains a combination of instrument,
           exp_setting, and sample."""
        # An empty experiment should not contain anything
        e1 = ihm.flr.Experiment()
        self.assertFalse(e1.contains('foo', 'foo2', 'foo3', 'foo4'))
        # After addition, the entry should be contained
        e1.add_entry(instrument='foo', inst_setting='foo2',
                     exp_condition='foo3', sample='foo4')
        e1.add_entry(instrument='bar', inst_setting='bar2',
                     exp_condition='bar3', sample='bar4')
        self.assertTrue(e1.contains('foo', 'foo2', 'foo3', 'foo4'))
        # If one of the entries is not contained, then False
        self.assertFalse(e1.contains('foo2', 'foo2', 'foo4', 'foo5'))
        self.assertFalse(e1.contains('foobar', 'foobar2', 'foobar3',
                                     'foobar4'))

    def test_Experiment_Eq(self):
        """ Test equality and inequality of Experiment objects. """
        e_ref = ihm.flr.Experiment()
        e_ref.add_entry(instrument='foo', inst_setting='foo2',
                        exp_condition='foo3', sample='foo4')
        e_equal = ihm.flr.Experiment()
        e_equal.add_entry(instrument='foo', inst_setting='foo2',
                          exp_condition='foo3', sample='foo4')
        e_unequal = ihm.flr.Experiment()
        e_unequal.add_entry(instrument='bar', inst_setting='bar2',
                            exp_condition='bar3', sample='bar4')
        self.assertTrue(e_ref == e_equal)
        self.assertFalse(e_ref == e_unequal)
        self.assertTrue(e_ref != e_unequal)

    def test_Instrument_Init(self):
        """ Test initialization of Instrument. """
        i = ihm.flr.Instrument(details='foo')
        self.assertEqual(i.details, 'foo')

    def test_Instrument_Eq(self):
        """Test equality and inequality of Instrument objects."""
        i_ref = ihm.flr.Instrument(details='foo')
        i_equal = ihm.flr.Instrument(details='foo')
        i_unequal = ihm.flr.Instrument(details='bar')
        self.assertTrue(i_ref == i_equal)
        self.assertFalse(i_ref == i_unequal)
        self.assertTrue(i_ref != i_unequal)

    def test_inst_setting_init(self):
        """Test initialization of InstSetting."""
        e = ihm.flr.InstSetting(details='foo')
        self.assertEqual(e.details, 'foo')

    def test_inst_setting_eq(self):
        """Test equality and inequality of InstSetting objects."""
        e_ref = ihm.flr.InstSetting(details='foo')
        e_equal = ihm.flr.InstSetting(details='foo')
        e_unequal = ihm.flr.InstSetting(details='bar')
        self.assertTrue(e_ref == e_equal)
        self.assertFalse(e_ref == e_unequal)
        self.assertTrue(e_ref != e_unequal)

    def test_exp_condition_init(self):
        """Test initialization of ExpCondition."""
        e = ihm.flr.ExpCondition(details='foo')
        self.assertEqual(e.details, 'foo')

    def test_exp_condition_eq(self):
        """Test equality and inequality of ExpCondition objects."""
        e_ref = ihm.flr.ExpCondition(details='foo')
        e_equal = ihm.flr.ExpCondition(details='foo')
        e_unequal = ihm.flr.ExpCondition(details='bar')
        self.assertTrue(e_ref == e_equal)
        self.assertFalse(e_ref == e_unequal)
        self.assertTrue(e_ref != e_unequal)

    def test_fret_analysis_init(self):
        """Test initalization of FRETAnalysis."""
        f = ihm.flr.FRETAnalysis(
            experiment='this_experiment',
            sample_probe_1='this_sample_probe_1',
            sample_probe_2='this_sample_probe_2',
            forster_radius='this_forster_radius',
            type='intensity-based',
            calibration_parameters='this_calibration_parameters',
            lifetime_fit_model='this_lifetime_fit_model',
            ref_measurement_group='this_ref_measurement_group',
            method_name='this_method_name',
            chi_square_reduced='this_chi_square_reduced',
            donor_only_fraction='this_donly_fraction',
            dataset='this_dataset_list_id',
            external_file='this_external_file',
            software='this_software')
        self.assertEqual(f.experiment, 'this_experiment')
        self.assertEqual(f.sample_probe_1, 'this_sample_probe_1')
        self.assertEqual(f.sample_probe_2, 'this_sample_probe_2')
        self.assertEqual(f.forster_radius, 'this_forster_radius')
        self.assertEqual(f.type, 'intensity-based')
        self.assertEqual(f.calibration_parameters,
                         'this_calibration_parameters')
        self.assertEqual(f.lifetime_fit_model, 'this_lifetime_fit_model')
        self.assertEqual(f.ref_measurement_group, 'this_ref_measurement_group')
        self.assertEqual(f.method_name, 'this_method_name')
        self.assertEqual(f.chi_square_reduced, 'this_chi_square_reduced')
        self.assertEqual(f.donor_only_fraction, 'this_donly_fraction')
        self.assertEqual(f.dataset, 'this_dataset_list_id')
        self.assertEqual(f.external_file, 'this_external_file')
        self.assertEqual(f.software, 'this_software')

        self.assertRaises(
            ValueError,
            ihm.flr.FRETAnalysis, experiment='this_experiment',
            sample_probe_1='this_sample_probe_1',
            sample_probe_2='this_sample_probe_2',
            forster_radius='this_forster_radius', type='garbage')

    def test_fret_analysis_eq(self):
        """Test equality and inequality of FRETAnalysis objects."""
        f_ref = ihm.flr.FRETAnalysis(
            experiment='this_experiment',
            sample_probe_1='this_sample_probe_1',
            sample_probe_2='this_sample_probe_2',
            forster_radius='this_forster_radius',
            type='intensity-based',
            calibration_parameters='this_calibration_parameters',
            method_name='this_method_name',
            chi_square_reduced='this_chi_square_reduced',
            donor_only_fraction='this_donly_fraction',
            dataset='this_dataset_list_id',
            external_file='this_external_file',
            software='this_software')
        f_equal = ihm.flr.FRETAnalysis(
            experiment='this_experiment',
            sample_probe_1='this_sample_probe_1',
            sample_probe_2='this_sample_probe_2',
            forster_radius='this_forster_radius',
            type='intensity-based',
            calibration_parameters='this_calibration_parameters',
            method_name='this_method_name',
            chi_square_reduced='this_chi_square_reduced',
            donor_only_fraction='this_donly_fraction',
            dataset='this_dataset_list_id',
            external_file='this_external_file',
            software='this_software')
        f_unequal = ihm.flr.FRETAnalysis(
            experiment='this_experiment',
            sample_probe_1='foo',
            sample_probe_2='this_sample_probe_2',
            forster_radius='this_forster_radius',
            type='intensity-based',
            calibration_parameters='this_calibration_parameters',
            method_name='this_method_name',
            chi_square_reduced='this_chi_square_reduced',
            donor_only_fraction='this_donly_fraction',
            dataset='this_dataset_list_id',
            external_file='this_external_file',
            software='this_software')
        f_unequal_type = ihm.flr.FRETAnalysis(
            experiment='this_experiment',
            sample_probe_1='this_sample_probe_1',
            sample_probe_2='this_sample_probe_2',
            forster_radius='this_forster_radius',
            type='lifetime-based',
            calibration_parameters='this_calibration_parameters',
            method_name='this_method_name',
            chi_square_reduced='this_chi_square_reduced',
            donor_only_fraction='this_donly',
            dataset='this_dataset_list_id',
            external_file='this_external_file',
            software='this_software')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)
        self.assertFalse(f_ref == f_unequal_type)
        self.assertTrue(f_ref != f_unequal_type)

    def test_lifetime_fit_model_init(self):
        """ Test initialization of LifetimeFitModel."""
        f = ihm.flr.LifetimeFitModel(name='this_name',
                                     description='this_description',
                                     external_file='this_ext_file',
                                     citation='this_citation')
        self.assertEqual(f.name, 'this_name')
        self.assertEqual(f.description, 'this_description')
        self.assertEqual(f.external_file, 'this_ext_file')
        self.assertEqual(f.citation, 'this_citation')

    def test_lifetime_fit_model_eq(self):
        """ Test equality and inequality of LifeTimeFitModel objects."""
        f_ref = ihm.flr.LifetimeFitModel(
            name='this_name', description='this_desc')
        f_equal = ihm.flr.LifetimeFitModel(
            name='this_name', description='this_desc')
        f_unequal = ihm.flr.LifetimeFitModel(
            name='other_name', description='this_desc')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_ref_measurement_group_init(self):
        """ Test initialization of RefMeasurementGroup."""
        r = ihm.flr.RefMeasurementGroup()
        self.assertEqual(r.ref_measurement_list, [])

    def test_ref_measurement_group_add_ref_measurement(self):
        """ Test the addition of a RefMeasurement to the group."""
        r = ihm.flr.RefMeasurementGroup()
        r.add_ref_measurement('foo')
        r.add_ref_measurement('bar')
        self.assertEqual(r.ref_measurement_list, ['foo', 'bar'])

    def test_ref_measurement_group_get_info(self):
        """ Test the retrieval of the ref_measurement_list."""
        r = ihm.flr.RefMeasurementGroup()
        r.add_ref_measurement('foo')
        r.add_ref_measurement('bar')
        return_value = r.get_info()
        self.assertEqual(return_value, ['foo', 'bar'])

    def test_ref_measurement_group_eq(self):
        """ Test equality and inequality of RefMeasurementGroup objects."""
        r_ref = ihm.flr.RefMeasurementGroup()
        r_ref.add_ref_measurement('foo')
        r_equal = ihm.flr.RefMeasurementGroup()
        r_equal.add_ref_measurement('foo')
        r_unequal = ihm.flr.RefMeasurementGroup()
        r_unequal.add_ref_measurement('foo2')
        self.assertTrue(r_ref == r_equal)
        self.assertFalse(r_ref == r_unequal)
        self.assertTrue(r_ref != r_unequal)

    def test_ref_measurement_init(self):
        """Test initialization of RefMeasurement."""
        r1 = ihm.flr.RefMeasurement(ref_sample_probe='this_ref_sample_probe',
                                    details='this_details')
        self.assertEqual(r1.ref_sample_probe, 'this_ref_sample_probe')
        self.assertEqual(r1.details, 'this_details')
        self.assertEqual(r1.list_of_lifetimes, [])

        r2 = ihm.flr.RefMeasurement(ref_sample_probe='this_ref_sample_probe_2',
                                    details='this_details_2',
                                    list_of_lifetimes=['foo', 'bar'])
        self.assertEqual(r2.ref_sample_probe, 'this_ref_sample_probe_2')
        self.assertEqual(r2.details, 'this_details_2')
        self.assertEqual(r2.list_of_lifetimes, ['foo', 'bar'])

    def test_ref_measurement_add_lifetime(self):
        """ Test addition of to the list_of_lifetimes."""
        r = ihm.flr.RefMeasurement(ref_sample_probe='this_ref_sample_probe',
                                   details='this_details')
        r.add_lifetime('foo')
        r.add_lifetime('bar')
        self.assertEqual(r.list_of_lifetimes, ['foo', 'bar'])

    def test_ref_measurement_eq(self):
        """ Test equality and inequality of RefMeasurement objects."""
        r_ref = ihm.flr.RefMeasurement(
            ref_sample_probe='this_ref_sample_probe_1',
            details='this_details_1')
        r_equal = ihm.flr.RefMeasurement(
            ref_sample_probe='this_ref_sample_probe_1',
            details='this_details_1')
        r_unequal = ihm.flr.RefMeasurement(
            ref_sample_probe='this_ref_sample_probe_2',
            details='this_details_2')
        r_unequal_list = ihm.flr.RefMeasurement(
            ref_sample_probe='this_ref_sample_probe_1',
            details='this_details_1', list_of_lifetimes=['foo'])
        self.assertTrue(r_ref == r_equal)
        self.assertTrue(r_ref != r_unequal)
        self.assertTrue(r_ref != r_unequal_list)
        self.assertFalse(r_ref == r_unequal)
        self.assertFalse(r_ref == r_unequal_list)

    def test_ref_measurement_lifetime_init(self):
        """ Test initialization of RefMeasuremenLifetime objects."""
        f = ihm.flr.RefMeasurementLifetime(species_fraction='this_frac',
                                           lifetime='this_lifetime',
                                           species_name='foo')
        self.assertEqual(f.species_fraction, 'this_frac')
        self.assertEqual(f.lifetime, 'this_lifetime')
        self.assertEqual(f.species_name, 'foo')

    def test_ref_measurement_lifetime_eq(self):
        """ Test equality and inequality of RefMeasurementLifetime objects."""
        f_ref = ihm.flr.RefMeasurementLifetime(species_fraction='this_frac_1',
                                               lifetime='this_lifetime_1')
        f_equal = ihm.flr.RefMeasurementLifetime(
            species_fraction='this_frac_1', lifetime='this_lifetime_1')
        f_unequal = ihm.flr.RefMeasurementLifetime(
            species_fraction='this_frac_2', lifetime='this_lifetime_1')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fret_distance_restraint_group_init(self):
        """ Test initialization of FRETDistanceRestraintGroup. """
        f = ihm.flr.FRETDistanceRestraintGroup()
        self.assertEqual(f.distance_restraint_list, [])

    def test_fret_distance_restraint_group_add_distance_restraint(self):
        """ Test the addition of a Fret_distance restraint to the group. """
        f = ihm.flr.FRETDistanceRestraintGroup()
        f.add_distance_restraint('foo')
        f.add_distance_restraint('bar')
        self.assertEqual(f.distance_restraint_list, ['foo', 'bar'])

    def test_fret_distance_restraint_group_get_info(self):
        """ Test the retrieval of the distance_restraint_list. """
        f = ihm.flr.FRETDistanceRestraintGroup()
        f.add_distance_restraint('foo')
        f.add_distance_restraint('bar')
        return_value = f.get_info()
        self.assertEqual(return_value, ['foo', 'bar'])

    def test_fret_distance_restraint_group_eq(self):
        """Test equality and inequality of FRETDistanceRestraintGroup
           objects."""
        f_ref = ihm.flr.FRETDistanceRestraintGroup()
        f_ref.add_distance_restraint('foo')
        f_equal = ihm.flr.FRETDistanceRestraintGroup()
        f_equal.add_distance_restraint('foo')
        f_unequal = ihm.flr.FRETDistanceRestraintGroup()
        f_unequal.add_distance_restraint('bar')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fret_distance_restraint_init(self):
        """Test initialization of FRETDistanceRestraint."""
        f = ihm.flr.FRETDistanceRestraint(
            sample_probe_1='this_sample_probe_1',
            sample_probe_2='this_sample_probe_2',
            state='this_state',
            analysis='this_analysis',
            distance='this_distance',
            distance_error_plus='this_distance_error_plus',
            distance_error_minus='this_distance_error_minus',
            distance_type='this_distance_type',
            population_fraction='this_population_fraction',
            peak_assignment='this_peak_assignment')
        self.assertEqual(f.sample_probe_1, 'this_sample_probe_1')
        self.assertEqual(f.sample_probe_2, 'this_sample_probe_2')
        self.assertEqual(f.state, 'this_state')
        self.assertEqual(f.analysis, 'this_analysis')
        self.assertEqual(f.distance, 'this_distance')
        self.assertEqual(f.distance_error_plus, 'this_distance_error_plus')
        self.assertEqual(f.distance_error_minus, 'this_distance_error_minus')
        self.assertEqual(f.distance_type, 'this_distance_type')
        self.assertEqual(f.population_fraction, 'this_population_fraction')
        self.assertEqual(f.peak_assignment, 'this_peak_assignment')

    def test_fret_distance_restraint_eq(self):
        """Test equality and inequality of FRETDistanceRestraint objects."""
        f_ref = ihm.flr.FRETDistanceRestraint(
            sample_probe_1='this_sample_probe_1',
            sample_probe_2='this_sample_probe_2',
            state='this_state', analysis='this_analysis',
            distance='this_distance',
            distance_error_plus='this_distance_error_plus',
            distance_error_minus='this_distance_error_minus',
            distance_type='this_distance_type',
            population_fraction='this_population_fraction',
            peak_assignment='this_peak_assignment')

        f_equal = ihm.flr.FRETDistanceRestraint(
            sample_probe_1='this_sample_probe_1',
            sample_probe_2='this_sample_probe_2',
            state='this_state', analysis='this_analysis',
            distance='this_distance',
            distance_error_plus='this_distance_error_plus',
            distance_error_minus='this_distance_error_minus',
            distance_type='this_distance_type',
            population_fraction='this_population_fraction',
            peak_assignment='this_peak_assignment')

        f_unequal = ihm.flr.FRETDistanceRestraint(
            sample_probe_1='this_sample_probe_1',
            sample_probe_2='this_sample_probe_2', state='foo',
            analysis='this_analysis', distance='this_distance',
            distance_error_plus='this_distance_error_plus',
            distance_error_minus='this_distance_error_minus',
            distance_type='this_distance_type',
            population_fraction='this_population_fraction',
            peak_assignment='this_peak_assignment')

        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fret_forster_radius_init(self):
        """ Test initialization of FRETForsterRadius. """
        f = ihm.flr.FRETForsterRadius(
            donor_probe='foo', acceptor_probe='bar', forster_radius='foo2',
            reduced_forster_radius='bar2')
        self.assertEqual(f.donor_probe, 'foo')
        self.assertEqual(f.acceptor_probe, 'bar')
        self.assertEqual(f.forster_radius, 'foo2')
        self.assertEqual(f.reduced_forster_radius, 'bar2')

    def test_fret_forster_radius_eq(self):
        """ Test equality and inequality of FRETForsterRadius objects. """
        f_ref = ihm.flr.FRETForsterRadius(
            donor_probe='foo', acceptor_probe='bar', forster_radius='foo2',
            reduced_forster_radius='bar2')
        f_equal = ihm.flr.FRETForsterRadius(
            donor_probe='foo', acceptor_probe='bar', forster_radius='foo2',
            reduced_forster_radius='bar2')
        f_unequal = ihm.flr.FRETForsterRadius(
            donor_probe='foobar', acceptor_probe='bar', forster_radius='foo2',
            reduced_forster_radius='bar2')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fret_calibration_parameters_init(self):
        """Test initialization of FRETCalibrationParameters."""
        f = ihm.flr.FRETCalibrationParameters(phi_acceptor='this_phi_acceptor',
                                              alpha='this_alpha',
                                              alpha_sd='this_alpha_sd',
                                              gg_gr_ratio='this_gG_gR_ratio',
                                              beta='this_beta',
                                              gamma='this_gamma',
                                              delta='this_delta',
                                              a_b='this_a_b')
        self.assertEqual(f.phi_acceptor, 'this_phi_acceptor')
        self.assertEqual(f.alpha, 'this_alpha')
        self.assertEqual(f.alpha_sd, 'this_alpha_sd')
        self.assertEqual(f.gg_gr_ratio, 'this_gG_gR_ratio')
        self.assertEqual(f.beta, 'this_beta')
        self.assertEqual(f.gamma, 'this_gamma')
        self.assertEqual(f.delta, 'this_delta')
        self.assertEqual(f.a_b, 'this_a_b')

    def test_fret_calibration_parameters_eq(self):
        """Test equality and inequality of
           FRETCalibrationParameters objects."""
        f_ref = ihm.flr.FRETCalibrationParameters(
            phi_acceptor='this_phi_acceptor', alpha='this_alpha',
            alpha_sd='this_alpha_sd', gg_gr_ratio='this_gG_gR_ratio',
            beta='this_beta', gamma='this_gamma', delta='this_delta',
            a_b='this_a_b')
        f_equal = ihm.flr.FRETCalibrationParameters(
            phi_acceptor='this_phi_acceptor', alpha='this_alpha',
            alpha_sd='this_alpha_sd', gg_gr_ratio='this_gG_gR_ratio',
            beta='this_beta', gamma='this_gamma', delta='this_delta',
            a_b='this_a_b')
        f_unequal = ihm.flr.FRETCalibrationParameters(
            phi_acceptor='foo', alpha='this_alpha', alpha_sd='this_alpha_sd',
            gg_gr_ratio='this_gG_gR_ratio', beta='this_beta',
            gamma='this_gamma', delta='this_delta', a_b='this_a_b')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_peak_assignment_init(self):
        """ Test initialization of PeakAssignment. """
        p = ihm.flr.PeakAssignment(method_name='foo', details='bar')
        self.assertEqual(p.method_name, 'foo')
        self.assertEqual(p.details, 'bar')

    def test_peak_assignment_eq(self):
        """ Test equality and inequality of PeakAssignment objects. """
        p_ref = ihm.flr.PeakAssignment(method_name='foo', details='bar')
        p_equal = ihm.flr.PeakAssignment(method_name='foo', details='bar')
        p_unequal = ihm.flr.PeakAssignment(method_name='foobar', details='bar')
        self.assertTrue(p_ref == p_equal)
        self.assertFalse(p_ref == p_unequal)
        self.assertTrue(p_ref != p_unequal)

    def test_fret_model_quality_init(self):
        """ Test initialization of FRETModelQuality. """
        f = ihm.flr.FRETModelQuality(
            model='this_model_id',
            chi_square_reduced='this_chi_square_reduced',
            dataset_group='this_dataset_group_id', method='this_method',
            details='this_details')
        self.assertEqual(f.model, 'this_model_id')
        self.assertEqual(f.chi_square_reduced, 'this_chi_square_reduced')
        self.assertEqual(f.dataset_group, 'this_dataset_group_id')
        self.assertEqual(f.method, 'this_method')
        self.assertEqual(f.details, 'this_details')

    def test_fret_model_quality_eq(self):
        """ Test equality and inequality of FRETModelQuality objects. """
        f_ref = ihm.flr.FRETModelQuality(
            model='this_model_id',
            chi_square_reduced='this_chi_square_reduced',
            dataset_group='this_dataset_group_id', method='this_method',
            details='this_details')
        f_equal = ihm.flr.FRETModelQuality(
            model='this_model_id',
            chi_square_reduced='this_chi_square_reduced',
            dataset_group='this_dataset_group_id', method='this_method',
            details='this_details')
        f_unequal = ihm.flr.FRETModelQuality(
            model='foo', chi_square_reduced='this_chi_square_reduced',
            dataset_group='this_dataset_group_id', method='this_method',
            details='this_details')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fret_model_distance_init(self):
        """Test initialization of FRETModelDistance.
           Also test the implicit calculation of the distance deviation"""
        # Initialization with explicit setting of distance deviation
        f1 = ihm.flr.FRETModelDistance(restraint='foo', model='bar',
                                       distance=50, distance_deviation=4.0)
        self.assertEqual(f1.restraint, 'foo')
        self.assertEqual(f1.model, 'bar')
        self.assertEqual(f1.distance, 50)
        self.assertEqual(f1.distance_deviation, 4.0)

        # Initialization with calculation of distance deviation
        class Dummy_Restraint():
            def __init__(self, distance):
                self.distance = distance
        f2 = ihm.flr.FRETModelDistance(restraint=Dummy_Restraint(40),
                                       model='bar2', distance=30)
        self.assertEqual(f2.model, 'bar2')
        self.assertEqual(f2.distance, 30)
        self.assertEqual(f2.distance_deviation, 10.0)

    def test_fret_model_distance_calculate_deviation(self):
        """Test FRETModelDistance.calculate_deviation()"""
        class Dummy_Restraint():
            def __init__(self, distance):
                self.distance = distance
        f1 = ihm.flr.FRETModelDistance(restraint=Dummy_Restraint(40),
                                       model='foo', distance=30)
        self.assertEqual(f1.distance_deviation, 10.0)
        # Directly changing the distance should not change the deviation
        f1.distance = 25
        self.assertEqual(f1.distance_deviation, 10.0)
        # Calculation of the distance deviation should update the deviation
        f1.update_deviation()
        self.assertEqual(f1.distance_deviation, 15.0)
        # update_deviation() is a noop if restraint is None
        f2 = ihm.flr.FRETModelDistance(restraint=None, model='foo',
                                       distance=30)
        f2.update_deviation()
        self.assertIsNone(f2.distance_deviation)

    def test_fret_model_distance_eq(self):
        """ Test equality and inequality of FRETModelDistance objects. """
        f_ref = ihm.flr.FRETModelDistance(restraint='foo', model='bar',
                                          distance=50,
                                          distance_deviation=4.0)
        f_equal = ihm.flr.FRETModelDistance(restraint='foo', model='bar',
                                            distance=50,
                                            distance_deviation=4.0)
        f_unequal = ihm.flr.FRETModelDistance(restraint='foo2',
                                              model='bar', distance=50,
                                              distance_deviation=4.0)
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fps_modeling_init(self):
        """ Test initialization of FPSModeling. """
        f = ihm.flr.FPSModeling(protocol='foo',
                                restraint_group='bar',
                                global_parameter='foo2',
                                probe_modeling_method='foo3',
                                details='bar2')
        self.assertEqual(f.protocol, 'foo')
        self.assertEqual(f.restraint_group, 'bar')
        self.assertEqual(f.global_parameter, 'foo2')
        self.assertEqual(f.probe_modeling_method, 'foo3')
        self.assertEqual(f.details, 'bar2')

    def test_fps_modeling_eq(self):
        """Test equality and inequality of FPSModeling objects."""
        f_ref = ihm.flr.FPSModeling(protocol='foo', restraint_group='bar',
                                    global_parameter='foo2',
                                    probe_modeling_method='foo3',
                                    details='bar2')
        f_equal = ihm.flr.FPSModeling(protocol='foo', restraint_group='bar',
                                      global_parameter='foo2',
                                      probe_modeling_method='foo3',
                                      details='bar2')
        f_unequal = ihm.flr.FPSModeling(protocol='foo',
                                        restraint_group='foobar',
                                        global_parameter='foo2',
                                        probe_modeling_method='foo3',
                                        details='bar2')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fps_global_parameters_init(self):
        """Test initialization of FPSGlobalParameters."""
        f = ihm.flr.FPSGlobalParameters(
            forster_radius='this_forster_radius',
            conversion_function_polynom_order='this_cfpo',
            repetition='this_repetition',
            av_grid_rel='this_AV_grid_rel',
            av_min_grid_a='this_AV_min_grid_A',
            av_allowed_sphere='this_AV_allowed_sphere',
            av_search_nodes='this_AV_search_nodes',
            av_e_samples_k='this_AV_E_samples_k',
            sim_viscosity_adjustment='this_sim_viscosity_adjustment',
            sim_dt_adjustment='this_sim_dt_adjustment',
            sim_max_iter_k='this_sim_max_iter_k',
            sim_max_force='this_sim_max_force',
            sim_clash_tolerance_a='this_sim_clash_tolerance_A',
            sim_reciprocal_kt='this_sim_reciprocal_kT',
            sim_clash_potential='this_sim_clash_potential',
            convergence_e='this_convergence_E',
            convergence_k='this_convergence_K',
            convergence_f='this_convergence_F',
            convergence_t='this_convergence_T',
            optimized_distances='this_optimized_distances')

        self.assertEqual(f.forster_radius, 'this_forster_radius')
        self.assertEqual(f.conversion_function_polynom_order, 'this_cfpo')
        self.assertEqual(f.repetition, 'this_repetition')
        self.assertEqual(f.av_grid_rel, 'this_AV_grid_rel')
        self.assertEqual(f.av_min_grid_a, 'this_AV_min_grid_A')
        self.assertEqual(f.av_allowed_sphere, 'this_AV_allowed_sphere')
        self.assertEqual(f.av_search_nodes, 'this_AV_search_nodes')
        self.assertEqual(f.av_e_samples_k, 'this_AV_E_samples_k')
        self.assertEqual(f.sim_viscosity_adjustment,
                         'this_sim_viscosity_adjustment')
        self.assertEqual(f.sim_dt_adjustment, 'this_sim_dt_adjustment')
        self.assertEqual(f.sim_max_iter_k, 'this_sim_max_iter_k')
        self.assertEqual(f.sim_max_force, 'this_sim_max_force')
        self.assertEqual(f.sim_clash_tolerance_a, 'this_sim_clash_tolerance_A')
        self.assertEqual(f.sim_reciprocal_kt, 'this_sim_reciprocal_kT')
        self.assertEqual(f.sim_clash_potential, 'this_sim_clash_potential')
        self.assertEqual(f.convergence_e, 'this_convergence_E')
        self.assertEqual(f.convergence_k, 'this_convergence_K')
        self.assertEqual(f.convergence_f, 'this_convergence_F')
        self.assertEqual(f.convergence_t, 'this_convergence_T')
        self.assertEqual(f.optimized_distances, 'this_optimized_distances')

    def test_fps_global_parameters_eq(self):
        """Test equality and inequality of FPSGlobalParameters objects."""
        f_ref = ihm.flr.FPSGlobalParameters(
            forster_radius='this_forster_radius',
            conversion_function_polynom_order='this_cfpo',
            repetition='this_repetition',
            av_grid_rel='this_AV_grid_rel',
            av_min_grid_a='this_AV_min_grid_A',
            av_allowed_sphere='this_AV_allowed_sphere',
            av_search_nodes='this_AV_search_nodes',
            av_e_samples_k='this_AV_E_samples_k',
            sim_viscosity_adjustment='this_sim_viscosity_adjustment',
            sim_dt_adjustment='this_sim_dt_adjustment',
            sim_max_iter_k='this_sim_max_iter_k',
            sim_max_force='this_sim_max_force',
            sim_clash_tolerance_a='this_sim_clash_tolerance_A',
            sim_reciprocal_kt='this_sim_reciprocal_kT',
            sim_clash_potential='this_sim_clash_potential',
            convergence_e='this_convergence_E',
            convergence_k='this_convergence_K',
            convergence_f='this_convergence_F',
            convergence_t='this_convergence_T',
            optimized_distances='this_optimized_distances')
        f_equal = ihm.flr.FPSGlobalParameters(
            forster_radius='this_forster_radius',
            conversion_function_polynom_order='this_cfpo',
            repetition='this_repetition',
            av_grid_rel='this_AV_grid_rel',
            av_min_grid_a='this_AV_min_grid_A',
            av_allowed_sphere='this_AV_allowed_sphere',
            av_search_nodes='this_AV_search_nodes',
            av_e_samples_k='this_AV_E_samples_k',
            sim_viscosity_adjustment='this_sim_viscosity_adjustment',
            sim_dt_adjustment='this_sim_dt_adjustment',
            sim_max_iter_k='this_sim_max_iter_k',
            sim_max_force='this_sim_max_force',
            sim_clash_tolerance_a='this_sim_clash_tolerance_A',
            sim_reciprocal_kt='this_sim_reciprocal_kT',
            sim_clash_potential='this_sim_clash_potential',
            convergence_e='this_convergence_E',
            convergence_k='this_convergence_K',
            convergence_f='this_convergence_F',
            convergence_t='this_convergence_T',
            optimized_distances='this_optimized_distances')
        f_unequal = ihm.flr.FPSGlobalParameters(
            forster_radius='foo',
            conversion_function_polynom_order='this_cfpo',
            repetition='this_repetition', av_grid_rel='this_AV_grid_rel',
            av_min_grid_a='this_AV_min_grid_A',
            av_allowed_sphere='this_AV_allowed_sphere',
            av_search_nodes='this_AV_search_nodes',
            av_e_samples_k='this_AV_E_samples_k',
            sim_viscosity_adjustment='this_sim_viscosity_adjustment',
            sim_dt_adjustment='this_sim_dt_adjustment',
            sim_max_iter_k='this_sim_max_iter_k',
            sim_max_force='this_sim_max_force',
            sim_clash_tolerance_a='this_sim_clash_tolerance_A',
            sim_reciprocal_kt='this_sim_reciprocal_kT',
            sim_clash_potential='this_sim_clash_potential',
            convergence_e='this_convergence_E',
            convergence_k='this_convergence_K',
            convergence_f='this_convergence_F',
            convergence_t='this_convergence_T',
            optimized_distances='this_optimized_distances')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fps_av_modeling_init(self):
        """ Test initialization of FPSAVModeling. """
        f = ihm.flr.FPSAVModeling(fps_modeling='foo',
                                  sample_probe='bar',
                                  parameter='foobar')
        self.assertEqual(f.fps_modeling, 'foo')
        self.assertEqual(f.sample_probe, 'bar')
        self.assertEqual(f.parameter, 'foobar')

    def test_fps_av_modeling_eq(self):
        """Test equality and inequality of FPSAVModeling objects."""
        f_ref = ihm.flr.FPSAVModeling(fps_modeling='foo',
                                      sample_probe='bar',
                                      parameter='foobar')
        f_equal = ihm.flr.FPSAVModeling(fps_modeling='foo',
                                        sample_probe='bar',
                                        parameter='foobar')
        f_unequal = ihm.flr.FPSAVModeling(fps_modeling='foo',
                                          sample_probe='bar2',
                                          parameter='foobar')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fps_av_parameter_init(self):
        """Test initialization of FPSAVParameter."""
        # Initialization with AV1
        f1 = ihm.flr.FPSAVParameter(num_linker_atoms='this_num_linker_atoms_1',
                                    linker_length='this_linker_length_1',
                                    linker_width='this_linker_width_1',
                                    probe_radius_1='this_probe_radius_1_1')
        self.assertEqual(f1.num_linker_atoms, 'this_num_linker_atoms_1')
        self.assertEqual(f1.linker_length, 'this_linker_length_1')
        self.assertEqual(f1.linker_width, 'this_linker_width_1')
        self.assertEqual(f1.probe_radius_1, 'this_probe_radius_1_1')
        self.assertIsNone(f1.probe_radius_2)
        self.assertIsNone(f1.probe_radius_3)
        # Initialization with AV3
        f2 = ihm.flr.FPSAVParameter(num_linker_atoms='this_num_linker_atoms_2',
                                    linker_length='this_linker_length_2',
                                    linker_width='this_linker_width_2',
                                    probe_radius_1='this_probe_radius_1_2',
                                    probe_radius_2='this_probe_radius_2_2',
                                    probe_radius_3='this_probe_radius_3_2')
        self.assertEqual(f2.num_linker_atoms, 'this_num_linker_atoms_2')
        self.assertEqual(f2.linker_length, 'this_linker_length_2')
        self.assertEqual(f2.linker_width, 'this_linker_width_2')
        self.assertEqual(f2.probe_radius_1, 'this_probe_radius_1_2')
        self.assertEqual(f2.probe_radius_2, 'this_probe_radius_2_2')
        self.assertEqual(f2.probe_radius_3, 'this_probe_radius_3_2')

    def test_fps_av_parameter_eq(self):
        """ Test equality and inequality of FPSAVParameter objects. """
        f_ref = ihm.flr.FPSAVParameter(
            num_linker_atoms='this_num_linker_atoms_1',
            linker_length='this_linker_length_1',
            linker_width='this_linker_width_1',
            probe_radius_1='this_probe_radius_1_1')
        f_equal = ihm.flr.FPSAVParameter(
            num_linker_atoms='this_num_linker_atoms_1',
            linker_length='this_linker_length_1',
            linker_width='this_linker_width_1',
            probe_radius_1='this_probe_radius_1_1')
        f_unequal = ihm.flr.FPSAVParameter(
            num_linker_atoms='this_num_linker_atoms_1', linker_length='foo',
            linker_width='this_linker_width_1',
            probe_radius_1='this_probe_radius_1_1')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fps_mpp_modeling_init(self):
        """Test initialization of FPSMPPModeling."""
        f = ihm.flr.FPSMPPModeling(fps_modeling='foo', mpp='bar',
                                   mpp_atom_position_group='foobar')
        self.assertEqual(f.fps_modeling, 'foo')
        self.assertEqual(f.mpp, 'bar')
        self.assertEqual(f.mpp_atom_position_group, 'foobar')

    def test_fps_mpp_modeling_eq(self):
        """Test equality and inequality of FPSMPPModeling objects."""
        f_ref = ihm.flr.FPSMPPModeling(fps_modeling='foo', mpp='bar',
                                       mpp_atom_position_group='foobar')
        f_equal = ihm.flr.FPSMPPModeling(fps_modeling='foo', mpp='bar',
                                         mpp_atom_position_group='foobar')
        f_unequal = ihm.flr.FPSMPPModeling(fps_modeling='foo2', mpp='bar',
                                           mpp_atom_position_group='foobar')

        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fps_mean_probe_position_init(self):
        """Test initialization of FPSMeanProbePosition."""
        f = ihm.flr.FPSMeanProbePosition(sample_probe='foo', x='bar',
                                         y='bar2', z='bar3')
        self.assertEqual(f.sample_probe, 'foo')
        self.assertEqual(f.x, 'bar')
        self.assertEqual(f.y, 'bar2')
        self.assertEqual(f.z, 'bar3')

    def test_fps_mean_probe_position_eq(self):
        """Test equality and inequality of FPSMeanProbePosition objects."""
        f_ref = ihm.flr.FPSMeanProbePosition(sample_probe='foo', x='bar',
                                             y='bar2', z='bar3')
        f_equal = ihm.flr.FPSMeanProbePosition(sample_probe='foo', x='bar',
                                               y='bar2', z='bar3')
        f_unequal = ihm.flr.FPSMeanProbePosition(
            sample_probe='foobar', x='bar', y='bar2', z='bar3')
        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fps_mpp_atom_position_group_init(self):
        """ Test initialization of FPSMPPAtomPositionGroup. """
        f = ihm.flr.FPSMPPAtomPositionGroup()
        self.assertEqual(f.mpp_atom_position_list, [])

    def test_fps_mpp_atom_position_group_add_atom_position(self):
        """ Test addition of an atom position to FPSMPPAtomPositionGroup. """
        f = ihm.flr.FPSMPPAtomPositionGroup()
        f.add_atom_position('foo')
        f.add_atom_position('bar')
        self.assertEqual(f.mpp_atom_position_list, ['foo', 'bar'])

    def test_fps_mpp_atom_position_group_eq(self):
        """Test equality and inequality of FPSMPPAtomPositionGroup objects."""
        f_ref = ihm.flr.FPSMPPAtomPositionGroup()
        f_ref.add_atom_position('foo')
        f_equal = ihm.flr.FPSMPPAtomPositionGroup()
        f_equal.add_atom_position('foo')
        f_unequal = ihm.flr.FPSMPPAtomPositionGroup()
        f_unequal.add_atom_position('bar')

        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_fps_mpp_atom_position_init(self):
        """Test initialization of FPSMPPAtomPosition."""
        f = ihm.flr.FPSMPPAtomPosition(atom='this_atom_id', x='this_xcoord',
                                       y='this_ycoord', z='this_zcoord')
        self.assertEqual(f.atom, 'this_atom_id')
        self.assertEqual(f.x, 'this_xcoord')
        self.assertEqual(f.y, 'this_ycoord')
        self.assertEqual(f.z, 'this_zcoord')

    def test_fps_mpp_atom_position_eq(self):
        """Test equality and inequality of FPSMPPAtomPosition objects."""
        f_ref = ihm.flr.FPSMPPAtomPosition(atom='this_atom_id',
                                           x='this_xcoord', y='this_ycoord',
                                           z='this_zcoord')
        f_equal = ihm.flr.FPSMPPAtomPosition(atom='this_atom_id',
                                             x='this_xcoord', y='this_ycoord',
                                             z='this_zcoord')
        f_unequal = ihm.flr.FPSMPPAtomPosition(
            atom='other_atom_id', x='this_xcoord', y='this_ycoord',
            z='this_zcoord')

        self.assertTrue(f_ref == f_equal)
        self.assertFalse(f_ref == f_unequal)
        self.assertTrue(f_ref != f_unequal)

    def test_flr_data_init(self):
        """ Test initialization of FLRData. """
        f = ihm.flr.FLRData()
        self.assertEqual(f.distance_restraint_groups, [])
        self.assertEqual(f.poly_probe_conjugates, [])
        self.assertEqual(f.fret_model_qualities, [])
        self.assertEqual(f.fret_model_distances, [])
        self.assertEqual(f.fps_modeling, [])

    def test_flr_data_add_distance_restraint_group(self):
        """Test addition of a distance restraint group."""
        f = ihm.flr.FLRData()
        f.distance_restraint_groups.append('foo')
        f.distance_restraint_groups.append('bar')
        self.assertEqual(f.distance_restraint_groups, ['foo', 'bar'])

    def test_flr_data_add_poly_probe_conjugate(self):
        """Test addition of a poly_probe_conjugate."""
        f = ihm.flr.FLRData()
        f.poly_probe_conjugates.extend(('foo', 'bar'))
        self.assertEqual(f.poly_probe_conjugates, ['foo', 'bar'])

    def test_flr_data_add_fret_model_quality(self):
        """Test addition of a fret_model_quality."""
        f = ihm.flr.FLRData()
        f.fret_model_qualities.extend(('foo', 'bar'))
        self.assertEqual(f.fret_model_qualities, ['foo', 'bar'])

    def test_flr_data_add_fret_model_distance(self):
        """Test addition of a fret_model_distance."""
        f = ihm.flr.FLRData()
        f.fret_model_distances.append('foo')
        f.fret_model_distances.append('bar')
        self.assertEqual(f.fret_model_distances, ['foo', 'bar'])

    def test_flr_data_add_flr_fps_modeling(self):
        """Test addition of flr_FPS_modeling."""
        f = ihm.flr.FLRData()
        f.fps_modeling.append('foo')
        f.fps_modeling.append('bar')
        self.assertEqual(f.fps_modeling, ['foo', 'bar'])

    def test_flr_data_all_chemical_descriptors(self):
        """Test for collection of all chemical descriptors."""
        f = ihm.flr.FLRData()
        # Define probe descriptors
        this_probe_descriptor_1 = ihm.flr.ProbeDescriptor(
            reactive_probe_chem_descriptor='This_reactive_probe_desc_1',
            chromophore_chem_descriptor='This_chromophore_desc_1')
        this_probe_descriptor_2 = ihm.flr.ProbeDescriptor(
            reactive_probe_chem_descriptor='This_reactive_probe_desc_2',
            chromophore_chem_descriptor='This_chromophore_desc_2')
        this_probe_descriptor_3 = ihm.flr.ProbeDescriptor(
            reactive_probe_chem_descriptor='This_reactive_probe_desc_3',
            chromophore_chem_descriptor='This_chromophore_desc_3')
        this_probe_descriptor_4 = ihm.flr.ProbeDescriptor(
            reactive_probe_chem_descriptor='Ref_reactive_probe_desc',
            chromophore_chem_descriptor='Ref_chromophore_desc')

        # Define probes
        this_probe_1 = ihm.flr.Probe(probe_list_entry='foo',
                                     probe_descriptor=this_probe_descriptor_1)
        this_probe_2 = ihm.flr.Probe(probe_list_entry='foo',
                                     probe_descriptor=this_probe_descriptor_2)
        this_probe_3 = ihm.flr.Probe(probe_list_entry='foo',
                                     probe_descriptor=this_probe_descriptor_3)
        this_probe_4 = ihm.flr.Probe(probe_list_entry='foo',
                                     probe_descriptor=this_probe_descriptor_4)

        # Define poly probe positions
        this_poly_probe_position_1 = ihm.flr.PolyProbePosition(
            resatom='foo', mutation_flag=True,
            mutated_chem_comp_id='Mutated_Chem_comp_id_1',
            modification_flag=False)
        this_poly_probe_position_2 = ihm.flr.PolyProbePosition(
            resatom='foo', mutation_flag=False, modification_flag=True,
            modified_chem_descriptor='Modified_Chem_descriptor_1')
        this_poly_probe_position_3 = ihm.flr.PolyProbePosition(
            resatom='foo', mutation_flag=False, modification_flag=False)
        this_poly_probe_position_4 = ihm.flr.PolyProbePosition(
            resatom='foo', mutation_flag=True,
            mutated_chem_comp_id='Mutated_Chem_comp_id_2')
        this_poly_probe_position_5 = ihm.flr.PolyProbePosition(
            resatom='foo', modification_flag=True,
            modified_chem_descriptor='Modified_Chem_descriptor_2')

        this_sample_probe_1 = ihm.flr.SampleProbeDetails(
            sample='foo', probe=this_probe_1, fluorophore_type='donor',
            poly_probe_position=this_poly_probe_position_1)
        this_sample_probe_2 = ihm.flr.SampleProbeDetails(
            sample='foo2', probe=this_probe_2, fluorophore_type='donor',
            poly_probe_position=this_poly_probe_position_2)
        this_sample_probe_3 = ihm.flr.SampleProbeDetails(
            sample='foo3', probe=this_probe_3, fluorophore_type='donor',
            poly_probe_position=this_poly_probe_position_3)
        this_sample_probe_4 = ihm.flr.SampleProbeDetails(
            sample='foo4', probe=this_probe_4, fluorophore_type='donor',
            poly_probe_position=this_poly_probe_position_4)
        this_sample_probe_5 = ihm.flr.SampleProbeDetails(
            sample='foo5', probe=this_probe_4, fluorophore_type='donor',
            poly_probe_position=this_poly_probe_position_5)

        this_reference_measurement_1 = ihm.flr.RefMeasurement(
            ref_sample_probe=this_sample_probe_4)
        this_reference_measurement_2 = ihm.flr.RefMeasurement(
            ref_sample_probe=this_sample_probe_5)
        this_reference_measurement_group_1 = ihm.flr.RefMeasurementGroup()
        this_reference_measurement_group_1.add_ref_measurement(
            this_reference_measurement_1)
        this_reference_measurement_group_1.add_ref_measurement(
            this_reference_measurement_2)

        this_analysis_1 = ihm.flr.FRETAnalysis(
            experiment='foo', sample_probe_1=this_sample_probe_1,
            sample_probe_2=this_sample_probe_2, forster_radius='foo',
            type='lifetime-based',
            ref_measurement_group=this_reference_measurement_group_1)
        this_analysis_2 = ihm.flr.FRETAnalysis(
            experiment='bar', sample_probe_1=this_sample_probe_1,
            sample_probe_2=this_sample_probe_2, forster_radius='bar',
            type='intensity-based')

        this_distance_restraint_1 = ihm.flr.FRETDistanceRestraint(
            sample_probe_1=this_sample_probe_1,
            sample_probe_2=this_sample_probe_2,
            analysis=this_analysis_1, distance=50)
        this_distance_restraint_2 = ihm.flr.FRETDistanceRestraint(
            sample_probe_1=this_sample_probe_1,
            sample_probe_2=this_sample_probe_3,
            analysis=this_analysis_2, distance=50)

        this_distance_restraint_group = ihm.flr.FRETDistanceRestraintGroup()
        this_distance_restraint_group.add_distance_restraint(
            this_distance_restraint_1)
        this_distance_restraint_group.add_distance_restraint(
            this_distance_restraint_2)
        f.distance_restraint_groups.append(this_distance_restraint_group)

        conj = ihm.flr.PolyProbeConjugate(
            sample_probe=this_sample_probe_1,
            chem_descriptor='Conjugate_probe_desc',
            ambiguous_stoichiometry=False)
        f.poly_probe_conjugates.append(conj)

        descs = list(f._all_flr_chemical_descriptors())
        self.assertEqual(
            descs,
            ['This_reactive_probe_desc_1', 'This_chromophore_desc_1',
             'This_reactive_probe_desc_2', 'This_chromophore_desc_2',
             'Modified_Chem_descriptor_1',
             'Ref_reactive_probe_desc', 'Ref_chromophore_desc',
             'Ref_reactive_probe_desc', 'Ref_chromophore_desc',
             'Modified_Chem_descriptor_2',
             'This_reactive_probe_desc_1', 'This_chromophore_desc_1',
             'This_reactive_probe_desc_3', 'This_chromophore_desc_3',
             'Conjugate_probe_desc'])


if __name__ == '__main__':
    unittest.main()
