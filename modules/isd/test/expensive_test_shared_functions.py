#!/usr/bin/env python

import IMP.test
import IMP.isd.shared_functions as sf
import math
import tempfile,os

class Tests(IMP.test.TestCase):

    kB= (1.381 * 6.02214) / 4184.0

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.sfo=sf.sfo_common()

    def writetofile(self, seq):
        handle, name = tempfile.mkstemp(text=True)
        for line in seq:
            os.write(handle, line+'\n')
        os.close(handle)
        return name

    def init_protein(self, name):
        self.sfo.init_model_base('./')
        pdb=self.get_input_file_name(name)
        top=self.get_input_file_name('top.lib')
        par=self.get_input_file_name('par.lib')
        selector=IMP.atom.NonWaterPDBSelector()
        pairscore=IMP.isd.RepulsiveDistancePairScore(0,1)
        prot,ff,rsb,rs = \
                self.sfo.init_model_charmm_protein_and_ff(pdb,top,par,selector,pairscore)
        return prot,ff,rsb,rs

    def test_hello(self):
        self.assertEqual(self.sfo.hello(), 'hello world')

    def test_set_checklevel(self):
        self.fail()
    test_set_checklevel = IMP.test.expectedFailure(test_set_checklevel)

    def test_set_loglevel(self):
        self.fail()
    test_set_loglevel = IMP.test.expectedFailure(test_set_loglevel)

    def test_m(self):
        self.fail()
    test_m = IMP.test.expectedFailure(test_m)

    def test_init_model_base(self):
        self.sfo.init_model_base('./')
        self.assertIsInstance(self._m,IMP.Model)
    test_init_model_base = IMP.test.expectedFailure(test_init_model_base)

    def test_init_model_charmm_protein_and_ff(self):
        "rudimentary test of things"
        self.init_protein('1G6J_MODEL1.pdb')
        self.assertIsInstance(ff, IMP.atom.CHARMMParameters)
        self.assertIsInstance(prot,IMP.atom.Hierarchy)
        self.assertIsInstance(rsb, IMP.RestraintSet)
        #self.assertEqual(rsb.get_type_name(), 'phys_bonded')
        self.assertTrue(prot.get_is_valid(False))
    test_init_model_charmm_protein_and_ff = \
             IMP.test.expectedFailure(test_init_model_charmm_protein_and_ff)

    def test_init_model_setup_scale(self):
        self.sfo.init_model_base('./')
        s=self.sfo.init_model_setup_scale(3.0,1.0, 5.0)
        self.assertAlmostEqual(s.get_scale(),3.0,delta=1e-6)
        self.assertAlmostEqual(s.get_lower(),1.0,delta=1e-6)
        self.assertAlmostEqual(s.get_upper(),5.0,delta=1e-6)
        sc=s.get_score_state(0)
        self.assertIsInstance(sc, IMP.core.SingletonConstraint)
    test_init_model_setup_scale = \
             IMP.test.expectedFailure(test_init_model_setup_scale)

    def test_init_model_Jeffreys(self):
        self.sfo.init_model_base('./')
        s=self.sfo.init_model_setup_scale(3.0,1.0,5.0)
        rs=self.sfo.init_model_jeffreys([s])
        self.assertIsInstance(rs, IMP.RestraintSet)
        #self.assertEqual(rs.get_type_name(), 'prior')
        self.assertEqual(rs.get_number_of_restraints(), 1)
        #self.assertIsInstance(rs.get_restraint(0), IMP.isd.JeffreysRestraint)
        self.assertAlmostEqual(rs.get_weight(), 1.0)
        self.assertTrue(rs.get_is_part_of_model())
        rs=IMP.RestraintSet('test')
        rs.add_restraint(IMP.isd.JeffreysRestraint(s))
        rs=self.sfo.init_model_jeffreys([s], rs)
        self.assertIsInstance(rs, IMP.RestraintSet)
        #self.assertEqual(rs.get_type_name(), 'test')
        self.assertEqual(rs.get_number_of_restraints(), 2)
        #self.assertIsInstance(rs.get_restraint(0), IMP.isd.JeffreysRestraint)
        #self.assertIsInstance(rs.get_restraint(1), IMP.isd.JeffreysRestraint)
        self.assertAlmostEqual(rs.get_weight(), 1.0)
        self.assertFalse(rs.get_is_part_of_model())
    test_init_model_Jeffreys = \
             IMP.test.expectedFailure(test_init_model_Jeffreys)

    def test_find_atom(self):
        prot,ff,rsb,rs = self.init_protein('1G6J_MODEL1.pdb')
        p0=self.sfo.find_atom((1, 'HE22'), prot)
        self.assertIsInstance(p0,IMP.core.XYZ)
        self.assertAlmostEqual(p0.get_x(), -12.982, delta=1e-3)
        self.assertAlmostEqual(p0.get_y(), 2.380, delta=1e-3)
        self.assertAlmostEqual(p0.get_z(), 9.723, delta=1e-3)
    test_find_atom = IMP.test.expectedFailure(test_find_atom)

    def test_init_model_NOE_restraint(self):
        prot,ff,rsb,rs = self.init_protein('1G6J_MODEL1.pdb')
        sigma=IMP.isd.Scale.setup_particle(IMP.Particle(self.sfo._m),1.0)
        gamma=IMP.isd.Scale.setup_particle(IMP.Particle(self.sfo._m),1.0)
        ln=self.sfo.init_model_NOE_restraint(prot, ((1, 'HE22'), (2, 'O')),
                1.0, sigma, gamma)
        self.assertIsInstance(ln, IMP.isd.NOERestraint)

    def test_init_model_ambiguous_NOE_restraint(self):
        prot,ff,rsb,rs = self.init_protein('1G6J_MODEL1.pdb')
        sigma=IMP.isd.Scale(1.0)
        gamma=IMP.isd.Scale(1.0)
        ln=self.sfo.init_model_ambiguous_NOE_restraint(prot,
                (((1, 'HE22'), (2, 'O')),((1, 'HE21'), (2, 'O'))),
                1.0, sigma, gamma)
        self.assertIsInstance(ln, IMP.isd.AmbiguousNOERestraint)
    test_init_model_ambiguous_NOE_restraint = \
           IMP.test.expectedFailure(test_init_model_ambiguous_NOE_restraint)

    def test_init_model_NOEs(self):
        prot,ff,rsb,rs = self.init_protein('1G6J_MODEL1.pdb')
        seqfile=self.get_input_file_name('sequence.dat')
        tbl=['assign (resid 1 and name CA) (resid 2 and name H) 1.0 0.0 0.0',
             'assign (resid 1 and name HA) (resid 2 and name OE1) 2.0 0.0 0.0']
        tblfile=self.writetofile(tbl)
        data_rs, prior_rs, sigma, gamma = \
                self.sfo.init_model_NOEs(prot, seqfile,
                tblfile, name='test')
        #check data restraintset
        self.assertIsInstance(data_rs, IMP.RestraintSet)
        #self.assertEqual(data_rs.get_type_name(), 'test')
        self.assertEqual(data_rs.get_number_of_restraints(), 2)
        #self.assertIsInstance(data_rs.get_restraint(0),
        #        IMP.isd.NOERestraint)
        #self.assertIsInstance(data_rs.get_restraint(1),
        #        IMP.isd.NOERestraint)
        self.assertAlmostEqual(data_rs.get_weight(), 1.0)
        self.assertTrue(data_rs.get_is_part_of_model())
        #check prior restraintset
        self.assertIsInstance(prior_rs, IMP.RestraintSet)
        #self.assertEqual(prior_rs.get_type_name(), 'prior')
        self.assertEqual(prior_rs.get_number_of_restraints(), 2)
        #self.assertIsInstance(prior_rs.get_restraint(0),
        #        IMP.isd.JeffreysRestraint)
        #self.assertIsInstance(prior_rs.get_restraint(1),
        #        IMP.isd.JeffreysRestraint)
        self.assertAlmostEqual(prior_rs.get_weight(), 1.0)
        self.assertTrue(prior_rs.get_is_part_of_model())
        #check particles
        self.assertIsInstance(sigma, IMP.isd.Scale)
        self.assertIsInstance(gamma, IMP.isd.Scale)
    test_init_model_NOEs = IMP.test.expectedFailure(test_init_model_NOEs)

    def test_init_model_standard_SAXS_restraint(self):
        prot,ff,rsb,rs = self.init_protein('6lyz.pdb')
        profile=self.get_input_file_name('lyzexp.dat')
        rs=self.init_model_standard_SAXS_restraint(prot, profile, name='test')
        self.assertIsInstance(rs, IMP.RestraintSet)
        #self.assertEqual(rs.get_type_name(), 'test')
        self.assertEqual(rs.get_number_of_restraints(), 1)
        self.assertIsInstance(prior_rs.get_restraint(0),
                IMP.saxs.Restraint)
        self.assertAlmostEqual(rs.get_weight(), 1.0)
    test_init_model_standard_SAXS_restraint = \
           IMP.test.expectedFailure(test_init_model_standard_SAXS_restraint)

    def test_init_model__setup_md_berendsen(self):
        prot, ff, rsb, rs = self.init_protein('1G6J_MODEL1.pdb')
        md, os = self.sfo._setup_md(prot, temperature=400.0, thermostat='berendsen',
                coupling=600, md_restraints=None, momentum=10)
        self.assertIsInstance(md, IMP.atom.MolecularDynamics)
        self.assertTrue(md.get_has_optimizer_states())
        self.assertEqual(md.get_number_of_optimizer_states(), 2)
        self.assertIsInstance(os, IMP.atom.BerendsenThermostatOptimizerState)
        st1=md.get_optimizer_state(0)
        st2=md.get_optimizer_state(1)
        exp=[IMP.atom.BerendsenThermostatOptimizerState,
            IMP.atom.RemoveRigidMotionOptimizerState]
        self.assertTrue(isinstance(st1,exp[0]) or isinstance(st1,exp[1]))
        self.assertTrue(isinstance(st2,exp[0]) or isinstance(st2,exp[1]))
        if isinstance(st1,exp[0]):
            self.assertEqual(st1.get_tau(), 600)
            self.assertAlmostEqual(st1.get_temperature(), 400.0, delta=1e-6)
            self.assertEqual(st2.get_skip_steps(), 10)
        else:
            self.assertEqual(st2.get_tau(), 600)
            self.assertAlmostEqual(st2.get_temperature(), 400.0, delta=1e-6)
            self.assertEqual(st1.get_skip_steps(), 10)
    test_init_model__setup_md_berendsen = \
            IMP.test.expectedFailure(test_init_model__setup_md_berendsen)

    def test_init_model__setup_md_langevin(self):
        prot, ff, rsb, rs = self.init_protein('1G6J_MODEL1.pdb')
        md, os = self.sfo._setup_md(prot, temperature=400.0, thermostat='langevin',
                coupling=600, md_restraints=None, momentum=10)
        self.assertIsInstance(md, IMP.atom.MolecularDynamics)
        self.assertTrue(md.get_has_optimizer_states())
        self.assertEqual(md.get_number_of_optimizer_states(), 2)
        self.assertIsInstance(os, IMP.atom.LangevinThermostatOptimizerState)
        st1=md.get_optimizer_state(0)
        st2=md.get_optimizer_state(1)
        exp=[IMP.atom.LangevinThermostatOptimizerState,
            IMP.atom.RemoveTranslationOptimizerState]
        self.assertTrue(isinstance(st1,exp[0]) or isinstance(st1,exp[1]))
        self.assertTrue(isinstance(st2,exp[0]) or isinstance(st2,exp[1]))
        if isinstance(st1,exp[0]):
            self.assertEqual(st1.get_gamma(), 600)
            self.assertAlmostEqual(st1.get_temperature(), 400.0, delta=1e-6)
            self.assertEqual(st2.get_skip_steps(), 10)
        else:
            self.assertEqual(st2.get_gamma(), 600)
            self.assertAlmostEqual(st2.get_temperature(), 400.0, delta=1e-6)
            self.assertEqual(st1.get_skip_steps(), 10)
    test_init_model__setup_md_langevin = \
            IMP.test.expectedFailure(test_init_model__setup_md_langevin)

    def test_init_model__setup_md_NVE(self):
        prot, ff, rsb, rs = self.init_protein('1G6J_MODEL1.pdb')
        md, os = self.sfo._setup_md(prot, thermostat='NVE', md_restraints=None)
        self.assertIsInstance(md, IMP.atom.MolecularDynamics)
        self.assertFalse(md.get_has_optimizer_states())
        self.assertIsNone(os)

    def test_init_model__setup_md_rescale(self):
        prot, ff, rsb, rs = self.init_protein('1G6J_MODEL1.pdb')
        md, os = self.sfo._setup_md(prot, temperature=400.0,
                thermostat='rescale_velocities',md_restraints=None)
        self.assertIsInstance(md, IMP.atom.MolecularDynamics)
        self.assertTrue(md.get_has_optimizer_states())
        self.assertEqual(md.get_number_of_optimizer_states(), 1)
        self.assertIsInstance(os, IMP.atom.VelocityScalingOptimizerState)
        self.assertEqual(os.get_skip_steps(), 0)
    test_init_model__setup_md_rescale = \
           IMP.test.expectedFailure(test_init_model__setup_md_rescale)

    def test_init_model__setup_md_restraints(self):
        prot, ff, rsb, rs = self.init_protein('1G6J_MODEL1.pdb')
        md, os = self.sfo._setup_md(prot, md_restraints=[rsb])
        self.assertIsInstance(md, IMP.atom.MolecularDynamics)
        self.assertEqual(md.get_restraint_sets()[0], rsb)
    test_init_model__setup_md_restraints = \
           IMP.test.expectedFailure(test_init_model__setup_md_restraints)

    def test__setup_normal_mover(self):
        self.sfo.init_model_base('./')
        p0=IMP.isd.Scale.setup_particle(IMP.Particle(self.sfo._m), 1.0)
        nm = self.sfo._setup_normal_mover(p0, IMP.FloatKey("scale"), 0.1)
        self.assertIsInstance(nm, IMP.core.NormalMover)
        self.assertTrue(nm.get_container().get_contains_particle(p0))
        #self.assertEqual(nm.get_number_of_float_keys(), 1)
        #self.assertEqual(nm.get_float_key(0), IMP.FloatKey("scale"))
        self.assertAlmostEqual(nm.get_sigma(),0.1)
    test__setup_normal_mover = \
            IMP.test.expectedFailure(test__setup_normal_mover)

    def test__setup_md_mover(self):
        prot,ff,rsb,rs = self.init_protein('1G6J_MODEL1.pdb')
        p0=IMP.isd.Scale.setup_particle(IMP.Particle(self.sfo._m), 1.0)
        md, os = self.sfo._setup_md(prot)
        mdmv = self.sfo._setup_md_mover(md, IMP.atom.get_leaves(prot),
                100.0, n_md_steps = 50)
        self.assertIsInstance(mdmv, IMP.atom.MDMover)
        self.assertEqual(mdmv.get_nsteps(),50)
        self.assertAlmostEqual(mdmv.get_temperature(),100.0)
    test__setup_md_mover = IMP.test.expectedFailure(test__setup_md_mover)

    def test__setup_mc(self):
        self.sfo.init_model_base('./')
        p0=IMP.isd.Scale.setup_particle(IMP.Particle(self.sfo._m), 1.0)
        nm = self.sfo._setup_normal_mover(p0, IMP.FloatKey("scale"), 0.1)
        mc=self.sfo._setup_mc(nm, temperature=200.0)
        self.assertAlmostEqual(mc.get_temperature(), self.kB*200.0, delta=1e-6)
        self.assertTrue(mc.get_has_movers())
        self.assertEqual(mc.get_mover(0), nm)
    test__setup_mc = IMP.test.expectedFailure(test__setup_mc)

    def test__setup_mc_restraints(self):
        self.sfo.init_model_base('./')
        p0=IMP.isd.Scale.setup_particle(IMP.Particle(self.sfo._m), 1.0)
        prior_rs = self.sfo.init_model_jeffreys([p0])
        nm = self.sfo._setup_normal_mover(p0, IMP.FloatKey("scale"), 0.1)
        mc=self.sfo._setup_mc(nm, mc_restraints=[prior_rs])
        self.assertEqual(mc.get_restraint_sets()[0], prior_rs)
    test__setup_mc_restraints = \
              IMP.test.expectedFailure(test__setup_mc_restraints)


if __name__ == '__main__':
    IMP.test.main()
