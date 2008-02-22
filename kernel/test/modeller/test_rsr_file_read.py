import unittest
import os
from modeller import *
import IMP
import IMP.test
import IMP.pdb
import IMP.modeller_intf

class ModellerRestraintsTests(IMP.test.TestCase):

    def test_rsr_file_read(self):
        """Check reading of Modeller restraint files"""
        e = environ()
        e.edat.dynamic_sphere = False
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = model(e)
        modmodel.build_sequence('GGCC')
        modmodel.write(file='test.pdb')
        # Writing to a PDB truncates the xyz coordinates, so read it back in
        # here so that the IMP model and Modeller model are the same
        modmodel.read(file='test.pdb')

        m = IMP.Model()
        protein = IMP.pdb.read_pdb('test.pdb', m)
        os.unlink('test.pdb')

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

        for r in restraints:
            modmodel.restraints.clear()
            modmodel.restraints.add(r)
            modmodel.restraints.write(file='test.rsr')
            modenergy = selection(modmodel).energy()[0]

            rsrs = IMP.modeller_intf.load_restraints_file('test.rsr', protein)
            os.unlink('test.rsr')
            rset = IMP.RestraintSet()
            m.add_restraint(rset)
            for rsr in rsrs:
                rset.add_restraint(rsr)
            score = m.evaluate(True)
            rset.set_is_active(False)
            # Modeller and IMP should give the same score:
            diff = abs(modenergy - score)
            self.assert_(diff < 0.1)

if __name__ == '__main__':
    unittest.main()
