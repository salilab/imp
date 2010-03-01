import unittest
import os
from modeller import *
import IMP
import IMP.test
import IMP.core
import IMP.modeller

class ModellerRestraintsTests(IMP.test.TestCase):

    def assertSimilarModellerIMPScores(self, modeller_model, imp_atoms):
        """Assert that Modeller and IMP give the same score and derivatives"""
        modeller_energy = selection(modeller_model).energy()[0]
        imp_score = imp_atoms.get_model().evaluate(True)
        self.assertInTolerance(modeller_energy, imp_score, 0.001)
        for imp_atom, modeller_atom in zip(imp_atoms.get_leaves(),
                                           modeller_model.atoms):
            imp_deriv = IMP.core.XYZ(imp_atom).get_derivatives()
            self.assertInTolerance(imp_deriv[0], modeller_atom.dvx, 1e-4,
                                   "x derivative for atom %s differs between "
                                   "Modeller (%f) and IMP (%f)" \
                                   % (str(modeller_atom), modeller_atom.dvx,
                                      imp_deriv[0]))
            self.assertInTolerance(imp_deriv[1], modeller_atom.dvy, 1e-4,
                                   "y derivative for atom %s differs between "
                                   "Modeller (%f) and IMP (%f)" \
                                   % (str(modeller_atom), modeller_atom.dvy,
                                      imp_deriv[1]))
            self.assertInTolerance(imp_deriv[2], modeller_atom.dvz, 1e-4,
                                   "z derivative for atom %s differs between "
                                   "Modeller (%f) and IMP (%f)" \
                                   % (str(modeller_atom), modeller_atom.dvz,
                                      imp_deriv[2]))

    def test_read_static_restraints(self):
        """Check loading of Modeller static restraints"""
        e = environ()
        e.edat.dynamic_sphere = False
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = model(e)
        modmodel.build_sequence('GGCC')

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)

        at = modmodel.atoms
        restraints = []
        # Typical distance restraints or stereochemical bonds:
        r = forms.gaussian(feature=features.distance(at[0], at[1]),
                           mean=1.54, stdev=0.1, group=physical.xy_distance)
        restraints.append(r)
        r = forms.lower_bound(feature=features.distance(at[0], at[1]),
                              mean=10.0, stdev=0.1, group=physical.xy_distance)
        restraints.append(r)
        r = forms.upper_bound(feature=features.distance(at[0], at[1]),
                              mean=10.0, stdev=0.1, group=physical.xy_distance)
        restraints.append(r)

        # Typical stereochemical angle restraint:
        r = forms.gaussian(feature=features.angle(at[0], at[1], at[2]),
                           mean=1.92, stdev=0.07, group=physical.xy_distance)
        restraints.append(r)

        # Typical stereochemical improper dihedral restraint:
        r = forms.gaussian(feature=features.dihedral(at[0], at[1], at[2],
                                                     at[3]),
                           mean=3.14, stdev=0.1, group=physical.xy_distance)
        restraints.append(r)

        # Typical stereochemical dihedral restraint:
        r = forms.cosine(feature=features.dihedral(at[0], at[1], at[2],
                                                   at[3]),
                         group=physical.xy_distance,
                         phase=0.0, force=2.5, period=2)
        restraints.append(r)

        # Typical splined restraint:
        r = forms.spline(feature=features.distance(at[0], at[1]), open=True,
                         low=1.0, high=5.0, delta=1.0,
                         group=physical.xy_distance,
                         lowderiv=0.0, highderiv=0.0,
                         values=[100.0, 200.0, 300.0, 200.0, 100.0])
        restraints.append(r)

        # Test forms.factor
        r = forms.factor(feature=features.angle(at[0], at[1], at[2]),
                          factor=100.0, group=physical.xy_distance)
        restraints.append(r)

        # Test periodic splined restraint:
        r = forms.spline(feature=features.dihedral(at[0], at[1], at[2], at[3]),
                         open=False, low=0.0, high=6.2832, delta=1.2566,
                         group=physical.xy_distance,
                         lowderiv=0.0, highderiv=0.0,
                         values=[100.0, 200.0, 300.0, 400.0, 300.0])
        restraints.append(r)

        # Test multiple binormal restraint
        r = forms.multi_binormal(features=(features.dihedral(*at[0:4]),
                                           features.dihedral(*at[4:8])),
                                 group=physical.xy_distance,
                                 weights=[0.2,0.8,0.3],
                                 means=[[0.1,0.2],[0.3,0.4],[0.5,0.6]],
                                 stdevs=[[0.1,0.2],[0.3,0.4],[0.1,0.3]],
                                 correls=[0.3,0.6,0.9])
        restraints.append(r)

        for r in restraints:
            modmodel.restraints.clear()
            modmodel.restraints.add(r)

            rset = IMP.RestraintSet()
            m.add_restraint(rset)
            for rsr in loader.load_static_restraints():
                rset.add_restraint(rsr)
            self.assertSimilarModellerIMPScores(modmodel, protein)
            rset.set_weight(0)

    def test_rsr_file_read(self):
        """Check reading of arbitrary Modeller restraint files"""
        e = environ()
        e.edat.dynamic_sphere = False
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = model(e)
        modmodel.build_sequence('GGCC')
        open('test.rsr', 'w').write('MODELLER5 VERSION: MODELLER FORMAT\n'
                                    'R    3   1   1   1   2   2   0     3'
                                    '     2       1.5380    0.0364')
        modmodel.restraints.append('test.rsr')
        # Deprecated interface
        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)
        r = IMP.modeller.load_restraints_file('test.rsr', protein)
        self.assert_(isinstance(r, list))
        for rsr in r:
            m.add_restraint(rsr)
        self.assertSimilarModellerIMPScores(modmodel, protein)

        # Need atoms before loading restraints
        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        self.assertRaises(ValueError, loader.load_static_restraints_file,
                          'test.rsr')
        # New interface
        protein = loader.load_atoms(m)
        for rsr in loader.load_static_restraints_file('test.rsr'):
            m.add_restraint(rsr)
        self.assertSimilarModellerIMPScores(modmodel, protein)
        os.unlink('test.rsr')

    def test_bond_restraints(self):
        """Check bond restraints against Modeller"""
        e = environ()
        e.edat.dynamic_sphere = False
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = model(e)
        modmodel.build_sequence('G')
        modmodel.restraints.make(selection(modmodel), restraint_type='BOND',
                                 spline_on_site=False,
                                 residue_span_range=(0, 99999))

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path('top.lib'),
                                       IMP.atom.get_data_path('par.lib'))
        topology = ff.create_topology(protein)
        topology.apply_default_patches(ff)
        topology.add_atom_types(protein)
        bonds = topology.add_bonds(protein, ff)
        cont = IMP.container.ListSingletonContainer("bonds")
        cont.add_particles(bonds)
        bss = IMP.atom.BondSingletonScore(IMP.core.Harmonic(0,1))
        m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))
        self.assertSimilarModellerIMPScores(modmodel, protein)

    def test_improper_restraints(self):
        """Check improper restraints against Modeller"""
        e = environ()
        e.edat.dynamic_sphere = False
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = model(e)
        modmodel.build_sequence('A')
        modmodel.restraints.make(selection(modmodel), restraint_type='IMPROPER',
                                 spline_on_site=False,
                                 residue_span_range=(0, 99999))

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path('top.lib'),
                                       IMP.atom.get_data_path('par.lib'))
        topology = ff.create_topology(protein)
        topology.apply_default_patches(ff)
        topology.add_atom_types(protein)
        bonds = topology.add_impropers(protein, ff)
        cont = IMP.container.ListSingletonContainer("bonds")
        cont.add_particles(bonds)
        bss = IMP.atom.ImproperSingletonScore(IMP.core.Harmonic(0,1))
        m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))
        self.assertSimilarModellerIMPScores(modmodel, protein)

if __name__ == '__main__':
    unittest.main()
