import IMP
import IMP.test
import os

class Tests(IMP.test.ApplicationTestCase):

    def test_param_run(self):
        """Test run of cnmultifit param command"""
        p = self.run_python_application('cnmultifit',
                               ['param', '-o', 'test.output', '-i', 'test.int',
                                '--params', 'test.params',
                                '--model', 'test.model',
                                '--numsols', '42', '--',
                                '7', 'testmonomer.pdb', 'test.mrc', '8.0',
                                '4.0', '5.0', '-10.0', '-20.0', '-30.0'])
        stdout, stderr = p.communicate()
        self.assertApplicationExitedCleanly(p.returncode, stderr)
        contents = open('test.params').read()
        self.assertIn('output = test.output', contents)
        self.assertIn('intermediate = test.int', contents)
        self.assertIn('model = test.model', contents)
        self.assertIn('solutions = 42', contents)
        self.assertIn('cn = 7', contents)
        self.assertIn('monomer = testmonomer.pdb', contents)
        self.assertIn('map = test.mrc', contents)
        self.assertIn('resolution = 8.0', contents)
        self.assertIn('spacing = 4.0', contents)
        self.assertIn('threshold = 5.0', contents)
        self.assertIn('origin_x = -10.0', contents)
        self.assertIn('origin_y = -20.0', contents)
        self.assertIn('origin_z = -30.0', contents)
        os.unlink('test.params')

if __name__ == '__main__':
    IMP.test.main()
