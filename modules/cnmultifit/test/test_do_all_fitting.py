import IMP
import IMP.test
import IMP.cnmultifit
import IMP.multifit
import IMP.atom
import os

class Tests(IMP.test.TestCase):

    def test_do_all_fitting(self):
        """Test do_all_fitting function"""
        fin = open(self.get_input_file_name('trimer.param.template'))
        fout = open('trimer.param', 'w')
        fout.write("""
[files]
monomer = %s
surface = %s.ms
prot_lib = %s
""" % (self.get_input_file_name('monomer.pdb'),
       self.get_input_file_name('monomer.pdb'),
       IMP.multifit.get_data_path('chem.lib')))
        for line in fin:
            if line.startswith("resolution ="):
                fout.write("map = %s\n" \
                           % self.get_input_file_name('trimer-8.0.mrc'))
            fout.write(line)
        fin.close()
        fout.close()
        IMP.cnmultifit.do_all_fitting('trimer.param')
        self.assertEqual(len(open('multifit.output').readlines()), 4)

        m = IMP.Model()
        ref = IMP.atom.read_pdb(self.get_input_file_name('trimer-ref.pdb'), m)
        ref_atoms = IMP.core.XYZs(IMP.atom.get_by_type(ref, IMP.atom.ATOM_TYPE))
        rmsds = []
        for i in range(3):
            out = IMP.atom.read_pdb('asmb.model.%d.pdb' % i, m)
            out_atoms = IMP.core.XYZs(IMP.atom.get_by_type(out,
                                                           IMP.atom.ATOM_TYPE))
            rmsds.append(IMP.atom.get_rmsd(ref_atoms, out_atoms))
            os.unlink('asmb.model.%d.pdb' % i)
        # First model should be close to reference
        self.assertLess(rmsds[0], 4.2)
        os.unlink('multifit.output')
        os.unlink('intermediate_asmb_sols.out')
        os.unlink('multifit.output.symm.ref')
        os.unlink('trimer.param')

if __name__ == '__main__':
    IMP.test.main()
