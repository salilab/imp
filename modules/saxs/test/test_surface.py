import unittest
import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.saxs
import os
import time

class SurfaceTest(IMP.test.TestCase):

    def test_surface_area(self):
        """Check protein profile computation"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'), m,
                              IMP.atom.NonWaterNonHydrogenPDBSelector())
        IMP.atom.add_radii(mp)

        #! select atom particles from the model
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

        #! calculate surface aceesability
        s = IMP.saxs.SolventAccessibleSurface();
        surface_area = s.get_solvent_accessibility(IMP.core.XYZRs(particles));

        #! sum up
        total_area = 0.0
        for area in surface_area:
            total_area += area
        print 'Area = ' + str(total_area)
        self.assertInTolerance(total_area, 37.728, 0.1)

if __name__ == '__main__':
    unittest.main()
