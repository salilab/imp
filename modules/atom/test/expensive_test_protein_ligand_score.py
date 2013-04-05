import IMP
import IMP.test
import IMP.atom
import IMP.core
import os
import time

class Tests(IMP.test.TestCase):

    def _test_score(self):
        """Test protein ligand restraint 1"""
        m= IMP.Model()
        IMP.base.set_log_level(IMP.base.SILENT)
        p= IMP.atom.read_pdb(self.get_input_file_name("1d3d-protein.pdb"),
                             m)
        l= IMP.atom.read_mol2(self.get_input_file_name("1d3d-ligands.mol2"),
                              m)
        #print "protein"
        #IMP.atom.show_molecular_hierarchy(p)
        #print "ligand"
        #IMP.atom.show_molecular_hierarchy(l)
        print "rigid bodies"
        rbp= IMP.atom.create_rigid_body(p)
        rbl= IMP.atom.create_rigid_body(l)
        it=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                        IMP.algebra.Vector3D(0,0,0))
        rbp.set_transformation(it)
        rbl.set_transformation(it)
        IMP.atom.write_mol2(l, self.get_tmp_file_name("transformed_ligand.mol2"))
        IMP.atom.write_pdb(p, self.get_tmp_file_name("transformed_protein.pdb"))

        print "evaluate"
        #IMP.base.set_log_level(IMP.base.VERBOSE)
        r= IMP.atom.ProteinLigandRestraint(p,l, 15.0)
        m.add_restraint(r)
        raw=m.evaluate(False)
        deriv= m.evaluate(True)
        self.assertAlmostEqual(raw, deriv, delta=.1*(raw+deriv))
        print r.evaluate(False)
    def test_score_2(self):
        """Test protein ligand restraint 2"""
        m= IMP.Model()
        IMP.base.set_log_level(IMP.base.SILENT)
        p= IMP.atom.read_pdb(self.get_input_file_name("1d3d-protein.pdb"),
                             m, IMP.atom.NonWaterNonHydrogenPDBSelector())
        l= IMP.atom.read_mol2(self.get_input_file_name("1d3d-ligands.mol2"),
                              m, IMP.atom.NonHydrogenMol2Selector())
        #patms= IMP.atom.get_by_type(p, IMP.atom.ATOM_TYPE)
        #for cp in patms:
        #    print "Read patom "+str(IMP.core.XYZ(cp.get_particle()).get_coordinates())+":Type: \""+str(IMP.atom.Atom(cp.get_particle()).get_atom_type())+"\" atom number: "+str(IMP.atom.Atom(cp.get_particle()).get_input_index())
        #latms= IMP.atom.get_by_type(l, IMP.atom.ATOM_TYPE)
        #for cp in latms:
        #    print "Read latom "+str(IMP.core.XYZ(cp.get_particle()).get_coordinates())+":Type: \""+str(IMP.atom.Atom(cp.get_particle()).get_atom_type())+"\" atom number: "+str(IMP.atom.Atom(cp.get_particle()).get_input_index())
        print "evaluate"
        ls= IMP.atom.get_by_type(l, IMP.atom.RESIDUE_TYPE)
        self.assertEqual(len(ls), 2)
        IMP.base.set_log_level(IMP.base.SILENT)
        r0= IMP.atom.ProteinLigandRestraint(p,ls[0], 6.0)
        m.add_restraint(r0)
        r1= IMP.atom.ProteinLigandRestraint(p,ls[1], 6.0)
        m.add_restraint(r1)
        print r0.evaluate(False)
        print r1.evaluate(False)
        self.assertAlmostEqual(r0.evaluate(False), 8.3, delta=1)
        self.assertAlmostEqual(r1.evaluate(False), 6.5, delta=1)


if __name__ == '__main__':
    IMP.test.main()
