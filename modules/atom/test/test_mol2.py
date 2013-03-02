import IMP
import IMP.test
import IMP.atom
import IMP.core
import StringIO

class Tests(IMP.test.TestCase):

    def test_read(self):
        """Check reading a mol2 file containing small molecules"""
        m = IMP.Model()

        #! read Mol2
        root_d= IMP.atom.read_mol2(self.get_input_file_name("1d3d-ligands.mol2"),
                                   m, IMP.atom.AllMol2Selector())

        print "number of particles"
        print m.get_number_of_particles()
        print len(IMP.atom.get_by_type(root_d, IMP.atom.ATOM_TYPE))
        self.assertEqual(146, len(IMP.atom.get_by_type(root_d, IMP.atom.ATOM_TYPE)))
        m2= StringIO.StringIO()
        IMP.atom.write_mol2(root_d,m2)

        pdb= StringIO.StringIO()
        IMP.atom.write_pdb(root_d,pdb)
        #print m2.getvalue()
        #print pdb
        return
    def test_write(self):
        """Check that a mol2-created hierarchy can be written to a PDB"""
        m = IMP.Model()

        #! read Mol2
        root_d= IMP.atom.read_mol2(self.get_input_file_name("1d3d-ligands.mol2"),
                                   m, IMP.atom.AllMol2Selector())
        print "number of particles"
        print m.get_number_of_particles()
        print len(IMP.atom.get_by_type(root_d, IMP.atom.ATOM_TYPE))
        self.assertEqual(146, len(IMP.atom.get_by_type(root_d, IMP.atom.ATOM_TYPE)))

        pdb= StringIO.StringIO()
        IMP.atom.write_pdb(root_d,pdb)
        print pdb.getvalue()
        return

        m2 = IMP.Model()
        root_d2 = IMP.atom.read_mol2(self.get_input_file_name("1d3d-ligands.mol2"),
                                       m2, IMP.atom.NonhydrogenMol2Selector())

        print "number of particles"
        print m2.get_number_of_particles()

        IMP.atom.write_file(root_d2, self.get_tmp_file_name("2.mol2"))
        IMP.atom.write_file(root_d2, self.get_tmp_file_name("2.pdb"))

    def test_hao(self):
        """Check that written mol2s look right"""
        m = IMP.Model()

        #! read Mol2
        root_d= IMP.atom.read_mol2(self.get_input_file_name("1d3d-ligands.mol2"),
                                   m, IMP.atom.AllMol2Selector())
        out= StringIO.StringIO()
        IMP.atom.write_mol2(root_d, out)
        testout= open(self.get_input_file_name("1d3d-ligands.out1.mol2"), "r").read()
        #print out.getvalue()
        #print " vs "
        #print testout
        open(self.get_tmp_file_name("out1.mol2"), "w").write(out.getvalue())
        self.assertEqual(out.getvalue(), testout)

        m = IMP.Model()
        root_d = IMP.atom.read_mol2(self.get_input_file_name("1d3d-ligands.mol2"),
                                       m, IMP.atom.NonHydrogenMol2Selector())
        print "test 2"
        print "number of particles"
        print m.get_number_of_particles()

        out= StringIO.StringIO()
        IMP.atom.write_mol2(root_d, out)
        testout= open(self.get_input_file_name("1d3d-ligands.out2.mol2"), "r").read()
        open(self.get_tmp_file_name("out2.mol2"), "w").write(out.getvalue())
        #print out.getvalue()
        #print " vs "
        #print testout
        self.assertEqual(out.getvalue(), testout)

if __name__ == '__main__':
    IMP.test.main()
