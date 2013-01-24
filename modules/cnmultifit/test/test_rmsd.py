import IMP
import IMP.test
import IMP.atom
import IMP.cnmultifit
import os.path
from IMP.cnmultifit import rmsd

class Tests(IMP.test.TestCase):

    def test_rmsd_help(self):
        """Test rmsd module help"""
        self.check_runnable_python_module("IMP.cnmultifit.rmsd")

    def test_rmsd_usage(self):
        """Test rmsd module incorrect usage"""
        r = self.run_python_module("IMP.cnmultifit.rmsd", [])
        out,err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_rmsd_run(self):
        """Test rmsd module run"""
        fin = open(self.get_input_file_name('multifit.param.template'))
        fout = open('multifit.param', 'w')
        for line in fin:
            if line.startswith("monomer ="):
                fout.write("monomer = %s\n" \
                           % self.get_input_file_name('mini-ref-monomer.pdb'))
            else:
                fout.write(line)
        fin.close()
        fout.close()
        self.run_python_module(rmsd,
              ['--vec', 'test.vec',
               'multifit.param', self.get_input_file_name('multifit.output'),
               self.get_input_file_name('mini-ref-complex.pdb')])
        self.assertEqual(len(open('rmsd.output').readlines()), 10)
        self.assertEqual(len(open('test.vec').read().split(' ')), 10)
        os.unlink('rmsd.output')
        os.unlink('test.vec')

        self.run_python_module(rmsd,
              ['--start', '4', '--end', '5',
               'multifit.param', self.get_input_file_name('multifit.output'),
               self.get_input_file_name('mini-ref-complex.pdb')])
        self.assertEqual(len(open('rmsd.output').readlines()), 2)
        os.unlink('rmsd.output')
        os.unlink('multifit.param')

if __name__ == '__main__':
    IMP.test.main()
