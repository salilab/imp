import unittest
import IMP
import IMP.test
import IMP.atom
import IMP.core
import os
import time

class ScoreTest(IMP.test.TestCase):

    def test_score(self):
        """Test protein ligand restraint"""
        m= IMP.Model()
        IMP.set_log_level(IMP.SILENT)
        p= IMP.atom.read_pdb(self.get_input_file_name("1d3d-protein.pdb"),
                             m)
        l= IMP.atom.read_mol2(self.get_input_file_name("1d3d-ligands.mol2"),
                              m)
        #print "protein"
        #IMP.atom.show_molecular_hierarchy(p)
        #print "ligand"
        #IMP.atom.show_molecular_hierarchy(l)
        print "rigid bodies"
        rbp= IMP.atom.rigid_body_setup_hierarchy(p)
        rbl= IMP.atom.rigid_body_setup_hierarchy(l)
        it=IMP.algebra.Transformation3D(IMP.algebra.identity_rotation(),
                                        IMP.algebra.Vector3D(0,0,0))
        rbp.set_transformation(it)
        rbl.set_transformation(it)
        IMP.atom.write_mol2(l, self.get_tmp_file_name("transformed_ligand.mol2"))
        IMP.atom.write_pdb(p, self.get_tmp_file_name("transformed_protein.pdb"))

        print "evaluate"
        #IMP.set_log_level(IMP.VERBOSE)
        r= IMP.atom.ProteinLigandRestraint(p,l, 15.0)
        m.add_restraint(r)
        raw=m.evaluate(False)
        deriv= m.evaluate(True)
        self.assertInTolerance(raw, deriv, .1*(raw+deriv))
        self.assertInTolerance(raw, 2200, 100)
        print r.evaluate(False)


if __name__ == '__main__':
    unittest.main()
