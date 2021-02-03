import os
import modeller
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.modeller


def assertSimilarModellerIMPScores(tst, sf, modeller_model, imp_atoms):
    """Assert that Modeller and IMP give the same score and derivatives"""
    modeller_energy = modeller.Selection(modeller_model).energy()[0]
    imp_score = sf.evaluate(True)
    tst.assertAlmostEqual(modeller_energy, imp_score, delta=0.001)
    for imp_atom, modeller_atom in zip(IMP.atom.get_leaves(imp_atoms),
                                       modeller_model.atoms):
        imp_deriv = IMP.core.XYZ(imp_atom).get_derivatives()
        tst.assertAlmostEqual(imp_deriv[0], modeller_atom.dvx, delta=1e-2,
                              msg="x derivative for atom %s differs between "
                                  "Modeller (%f) and IMP (%f)"
                                  % (str(modeller_atom), modeller_atom.dvx,
                                     imp_deriv[0]))
        tst.assertAlmostEqual(imp_deriv[1], modeller_atom.dvy, delta=1e-2,
                              msg="y derivative for atom %s differs between "
                                  "Modeller (%f) and IMP (%f)"
                                  % (str(modeller_atom), modeller_atom.dvy,
                                     imp_deriv[1]))
        tst.assertAlmostEqual(imp_deriv[2], modeller_atom.dvz, delta=1e-2,
                              msg="z derivative for atom %s differs between "
                                  "Modeller (%f) and IMP (%f)"
                                  % (str(modeller_atom), modeller_atom.dvz,
                                     imp_deriv[2]))


class Tests(IMP.test.TestCase):
    _environ = None

    def remove_atom_types(self, hierarchy):
        """Remove atom types as assigned by Modeller so we can set our own"""
        atoms = IMP.atom.get_by_type(hierarchy, IMP.atom.ATOM_TYPE)
        k = IMP.atom.CHARMMAtom.get_charmm_type_key()
        for a in atoms:
            a.get_particle().remove_attribute(k)

    def get_modeller_environ(self):
        # Reading Modeller libraries is expensive, so cache the environ
        if not self._environ:
            e = Tests._environ = modeller.Environ()
            e.edat.dynamic_sphere = False
            e.libs.topology.read('${LIB}/top_heav.lib')
            e.libs.parameters.read('${LIB}/par.lib')
        return self._environ

    def test_read_static_restraints(self):
        """Check loading of Modeller static restraints"""
        e = self.get_modeller_environ()
        modmodel = modeller.Model(e)
        modmodel.build_sequence('GGCC')

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)

        at = modmodel.atoms
        restraints = []
        # Typical distance restraints or stereochemical bonds:
        r = modeller.forms.Gaussian(
            feature=modeller.features.Distance(at[0], at[1]),
            mean=1.54, stdev=0.1, group=modeller.physical.xy_distance)
        restraints.append(r)
        r = modeller.forms.LowerBound(
            feature=modeller.features.Distance(at[0], at[1]),
            mean=10.0, stdev=0.1, group=modeller.physical.xy_distance)
        restraints.append(r)
        r = modeller.forms.UpperBound(
            feature=modeller.features.Distance(at[0], at[1]),
            mean=10.0, stdev=0.1, group=modeller.physical.xy_distance)
        restraints.append(r)

        # Typical stereochemical angle restraint:
        r = modeller.forms.Gaussian(
            feature=modeller.features.Angle(at[0], at[1], at[2]),
            mean=1.92, stdev=0.07, group=modeller.physical.xy_distance)
        restraints.append(r)

        # Typical stereochemical improper dihedral restraint:
        r = modeller.forms.Gaussian(
            feature=modeller.features.Dihedral(at[0], at[1], at[2], at[3]),
            mean=3.14, stdev=0.1, group=modeller.physical.xy_distance)
        restraints.append(r)

        # Typical stereochemical dihedral restraint:
        r = modeller.forms.Cosine(
            feature=modeller.features.Dihedral(at[0], at[1], at[2], at[3]),
            group=modeller.physical.xy_distance, phase=0.0, force=2.5, period=2)
        restraints.append(r)

        # Typical splined restraint:
        r = modeller.forms.Spline(
            feature=modeller.features.Distance(at[0], at[1]), open=True,
            low=1.0, high=5.0, delta=1.0,
            group=modeller.physical.xy_distance,
            lowderiv=0.0, highderiv=0.0,
            values=[100.0, 200.0, 300.0, 200.0, 100.0])
        restraints.append(r)

        # Test forms.factor
        r = modeller.forms.Factor(
            feature=modeller.features.Angle(at[0], at[1], at[2]),
            factor=100.0, group=modeller.physical.xy_distance)
        restraints.append(r)

        # Test periodic splined restraint:
        r = modeller.forms.Spline(
            feature=modeller.features.Dihedral(at[0], at[1], at[2], at[3]),
            open=False, low=0.0, high=6.2832, delta=1.2566,
            group=modeller.physical.xy_distance,
            lowderiv=0.0, highderiv=0.0,
            values=[100.0, 200.0, 300.0, 400.0, 300.0])
        restraints.append(r)

        # Test multiple binormal restraint
        r = modeller.forms.MultiBinormal(
            features=(modeller.features.Dihedral(*at[0:4]),
                      modeller.features.Dihedral(*at[4:8])),
            group=modeller.physical.xy_distance, weights=[0.2, 0.8, 0.3],
            means=[[0.1, 0.2], [0.3, 0.4], [0.5, 0.6]],
            stdevs=[[0.1, 0.2], [0.3, 0.4], [0.1, 0.3]],
            correls=[0.3, 0.6, 0.9])
        restraints.append(r)

        for r in restraints:
            modmodel.restraints.clear()
            modmodel.restraints.add(r)

            rset = IMP.RestraintSet(m, 1.0, "RS")
            for rsr in loader.load_static_restraints():
                rset.add_restraint(rsr)
            sf = IMP.core.RestraintsScoringFunction([rset])
            assertSimilarModellerIMPScores(self, sf, modmodel, protein)
            rset.set_weight(0)

    def test_rsr_file_read(self):
        """Check reading of arbitrary Modeller restraint files"""
        e = self.get_modeller_environ()
        modmodel = modeller.Model(e)
        modmodel.build_sequence('GGCC')
        with open('test.rsr', 'w') as fh:
            fh.write('MODELLER5 VERSION: MODELLER FORMAT\n'
                     'R    3   1   1   1   2   2   0     3'
                     '     2       1.5380    0.0364')
        modmodel.restraints.append('test.rsr')

        # Need atoms before loading restraints
        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        self.assertRaises(ValueError, loader.load_static_restraints_file,
                          'test.rsr')
        protein = loader.load_atoms(m)
        sf = IMP.core.RestraintsScoringFunction(list(
                                loader.load_static_restraints_file('test.rsr')))
        assertSimilarModellerIMPScores(self, sf, modmodel, protein)
        os.unlink('test.rsr')

    def test_bond_restraints(self):
        """Check bond restraints against Modeller"""
        e = self.get_modeller_environ()
        modmodel = modeller.Model(e)
        modmodel.build_sequence('G')
        modmodel.restraints.make(
            modeller.Selection(modmodel), restraint_type='BOND',
            spline_on_site=False, residue_span_range=(0, 99999))

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path('top.lib'),
                                       IMP.atom.get_data_path('par.lib'))
        topology = ff.create_topology(protein)
        topology.apply_default_patches()
        self.remove_atom_types(protein)
        topology.add_atom_types(protein)
        bonds = topology.add_bonds(protein)
        cont = IMP.container.ListSingletonContainer(m, "bonds")
        cont.add(bonds)
        bss = IMP.atom.BondSingletonScore(IMP.core.Harmonic(0, 1))
        r = IMP.container.SingletonsRestraint(bss, cont)
        sf = IMP.core.RestraintsScoringFunction([r])
        assertSimilarModellerIMPScores(self, sf, modmodel, protein)

    def test_improper_restraints(self):
        """Check improper restraints against Modeller"""
        e = self.get_modeller_environ()
        modmodel = modeller.Model(e)
        modmodel.build_sequence('A')
        modmodel.restraints.make(
            modeller.Selection(modmodel), restraint_type='IMPROPER',
            spline_on_site=False,
            residue_span_range=(0, 99999))

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path('top.lib'),
                                       IMP.atom.get_data_path('par.lib'))
        topology = ff.create_topology(protein)
        topology.apply_default_patches()
        self.remove_atom_types(protein)
        topology.add_atom_types(protein)
        bonds = topology.add_impropers(protein)
        cont = IMP.container.ListSingletonContainer(m, "bonds")
        cont.add(bonds)
        bss = IMP.atom.ImproperSingletonScore(IMP.core.Harmonic(0, 1))
        r = IMP.container.SingletonsRestraint(bss, cont)
        sf = IMP.core.RestraintsScoringFunction([r])
        assertSimilarModellerIMPScores(self, sf, modmodel, protein)

    def test_angle_restraints(self):
        """Check angle restraints against Modeller"""
        e = self.get_modeller_environ()
        modmodel = modeller.Model(e)
        modmodel.build_sequence('A')
        modmodel.restraints.make(
            modeller.Selection(modmodel), restraint_type='ANGLE',
            spline_on_site=False, residue_span_range=(0, 99999))

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path('top.lib'),
                                       IMP.atom.get_data_path('par.lib'))
        topology = ff.create_topology(protein)
        topology.apply_default_patches()
        self.remove_atom_types(protein)
        topology.add_atom_types(protein)
        bonds = topology.add_bonds(protein)
        angles = ff.create_angles(bonds)
        cont = IMP.container.ListSingletonContainer(m, "bonds")
        cont.add(angles)
        bss = IMP.atom.AngleSingletonScore(IMP.core.Harmonic(0, 1))
        r = IMP.container.SingletonsRestraint(bss, cont)
        sf = IMP.core.RestraintsScoringFunction([r])
        assertSimilarModellerIMPScores(self, sf, modmodel, protein)

    def test_dihedral_restraints(self):
        """Check dihedral restraints against Modeller"""
        e = self.get_modeller_environ()
        modmodel = modeller.Model(e)
        modmodel.build_sequence('A')
        modmodel.restraints.make(
            modeller.Selection(modmodel), restraint_type='DIHEDRAL',
            spline_on_site=False,
            residue_span_range=(0, 99999))

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path('top.lib'),
                                       IMP.atom.get_data_path('par.lib'))
        topology = ff.create_topology(protein)
        topology.apply_default_patches()
        self.remove_atom_types(protein)
        topology.add_atom_types(protein)
        bonds = topology.add_bonds(protein)
        dihedrals = ff.create_dihedrals(bonds)
        cont = IMP.container.ListSingletonContainer(m, "bonds")
        cont.add(dihedrals)
        bss = IMP.atom.DihedralSingletonScore()
        r = IMP.container.SingletonsRestraint(bss, cont)
        sf = IMP.core.RestraintsScoringFunction([r])
        assertSimilarModellerIMPScores(self, sf, modmodel, protein)

if __name__ == '__main__':
    IMP.test.main()
