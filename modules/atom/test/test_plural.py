import unittest
import IMP.atom
import IMP.test

plural_exceptions=['Ac', 'Ag', 'Al', 'AllMol2Selector', 'AllPDBSelector', 'Am', 'Ar', 'At', 'AtomTypeBase', 'Au', 'Ba', 'Be', 'Bi', 'Bk', 'Br', 'Ca', 'Cd', 'Ce', 'Cf', 'ChainPDBSelector', 'Cl', 'Cm', 'Co', 'Cr', 'Cu', 'Db',  'Dy', 'ElementTable', 'Er', 'Eu', 'Fe', 'Fm', 'ForceSwitch', 'Fr', 'Ga', 'Gd', 'Ge', 'He', 'Hf', 'Hg', 'Hierarchy', 'HierarchyVector', 'Ho', 'HydrogenPDBSelector', 'In', 'Ir', 'Jl', 'Kr', 'La', 'Li', 'Lr', 'Lu', 'Md', 'Mg', 'Mn', 'Mo', 'Mol2Selector', 'Na', 'Nb', 'Nd', 'Ne', 'Ni', 'No', 'NonAlternativePDBSelector', 'NonWaterNonHydrogenPDBSelector', 'NonWaterPDBSelector', 'NonhydrogenMol2Selector', 'Np', 'Pa', 'Pb', 'Pd', 'Pm', 'Po', 'Pr', 'Pt', 'Pu', 'Ra', 'Rb', 'Re', 'ResidueType', 'ResidueTypeBase', 'Rf', 'Rh', 'Rn', 'Ru', 'Sb', 'Sc', 'Se', 'Si', 'Sm', 'SmoothingFunction', 'Sn', 'Sr', 'StereochemistryPairFilter', 'Ta', 'Tb', 'Tc', 'Te', 'Th', 'Ti', 'Tl', 'Tm', 'WaterPDBSelector', 'Xe', 'Yb', 'Zn', 'Zr']

show_exceptions=['Ac', 'Ag', 'Al', 'AllMol2Selector', 'AllPDBSelector', 'Am', 'Ar', 'At', 'Au', 'Ba', 'Be', 'Bi', 'Bk', 'Br', 'Ca', 'Cd', 'Ce', 'Cf', 'ChainPDBSelector', 'Cl', 'Cm', 'Co', 'Cr', 'Cu', 'Db', 'Dy', 'ElementTable', 'Er', 'Eu', 'Fe', 'Fm', 'Fr', 'Ga', 'Gd', 'Ge', 'He', 'Hf', 'Hg', 'HierarchyVector', 'Ho', 'HydrogenPDBSelector', 'In', 'Ir', 'Jl', 'Kr', 'La', 'Li', 'Lr', 'Lu', 'Md', 'Mg', 'Mn', 'Mo', 'Mol2Selector', 'Na', 'Nb', 'Nd', 'Ne', 'Ni', 'No', 'NonAlternativePDBSelector', 'NonWaterNonHydrogenPDBSelector', 'NonWaterPDBSelector', 'NonhydrogenMol2Selector', 'Np', 'Pa', 'Pb', 'Pd', 'Pm', 'Po', 'Pr', 'Pt', 'Pu', 'Ra', 'Rb', 'Re', 'Rf', 'Rh', 'Rn', 'Ru', 'Sb', 'Sc', 'Se', 'Si', 'Sm', 'Sn', 'Sr', 'Ta', 'Tb', 'Tc', 'Te', 'Th', 'Ti', 'Tl', 'Tm', 'WaterPDBSelector', 'Xe', 'Yb', 'Zn', 'Zr']

class DirectoriesTests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test all atom objects have plural version"""
        return self.assertPlural(IMP.atom, plural_exceptions)

    def test_2(self):
        """Test all atom objects have show"""
        return self.assertShow(IMP.atom, show_exceptions)

if __name__ == '__main__':
    unittest.main()
