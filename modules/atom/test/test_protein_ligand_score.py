import unittest
import IMP
import IMP.test
import IMP.atom
import IMP.core
import os
import time

class ScoreTest(IMP.test.TestCase):

    def _test_score(self):
        """Test protein ligand restraint 1"""
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
        it=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
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
        print r.evaluate(False)
    def test_score_2(self):
        """Test protein ligand restraint 2"""
        m= IMP.Model()
        IMP.set_log_level(IMP.SILENT)
        p= IMP.atom.read_pdb(self.get_input_file_name("1d3d-protein.pdb"),
                             m)
        l= IMP.atom.read_mol2(self.get_input_file_name("1d3d-ligands.mol2"),
                              m)
        print "evaluate"
        ls= IMP.atom.get_by_type(l, IMP.atom.RESIDUE_TYPE)
        self.assertEqual(len(ls), 2)
        #IMP.set_log_level(IMP.VERBOSE)
        r0= IMP.atom.ProteinLigandRestraint(p,ls[0], 10.0)
        m.add_restraint(r0)
        r1= IMP.atom.ProteinLigandRestraint(p,ls[1], 10.0)
        m.add_restraint(r1)
        print r0.evaluate(False)
        print r1.evaluate(False)
        self.assertInTolerance(r0.evaluate(False), 99, 1)
        self.assertInTolerance(r1.evaluate(False), 88, 1)


if __name__ == '__main__':
    unittest.main()
