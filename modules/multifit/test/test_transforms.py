import IMP
import os
import IMP.test
import IMP.multifit
import IMP.algebra
from IMP.multifit import transforms


class Tests(IMP.test.TestCase):

    def test_transforms_help(self):
        """Test transforms module help"""
        self.check_runnable_python_module("IMP.multifit.transforms")

    def test_transforms_usage(self):
        """Test transforms module incorrect usage"""
        r = self.run_python_module("IMP.multifit.transforms", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_transforms_run_chimera(self):
        """Test transforms module run with Chimera output"""
        self.run_python_module(transforms,
                               ['-f', 'chimera',
                                self.get_input_file_name(
                                    'transforms.asmb.input'),
                                self.get_input_file_name('transforms.scores'),
                                'transforms.out.chimera'])
        lines = open('transforms.out.chimera').readlines()
        lines = lines[3]
        spl = lines.split('\t')
        self.assertEqual(int(spl[0]), 3)
        self.assertAlmostEqual(float(spl[1]), 0.60, delta=1e-3)
        spl = spl[2].split('|')
        self.assertEqual(len(spl), 3)
        spl = spl[0].split(' ')
        r = IMP.algebra.get_rotation_from_matrix(*[float(x) for x in spl[:9]])
        t = IMP.algebra.Vector3D(*[float(x) for x in spl[9:12]])
        self.assert_transformation(r, t)
        os.unlink('transforms.out.chimera')

    def test_transforms_run_dockref(self):
        """Test transforms module run with dockref output"""
        self.run_python_module(transforms,
                               ['--format', 'dockref',
                                self.get_input_file_name(
                                    'transforms.asmb.input'),
                                self.get_input_file_name('transforms.scores'),
                                'transforms.out.dockref'])
        lines = open('transforms.out.dockref').readlines()
        self.assertEqual(lines[0].rstrip('\r\n'), 'A|B|')
        spl = lines[3].split('|')
        spl = spl[0].split(' ')
        r = IMP.algebra.get_rotation_from_fixed_xyz(*[float(x)
                                                      for x in spl[:3]])
        t = IMP.algebra.Vector3D(*[float(x) for x in spl[3:6]])
        self.assert_transformation(r, t)
        os.unlink('transforms.out.dockref')

    def assert_transformation(self, r, t):
        # Compare transformation against known good value
        q = r.get_quaternion()
        self.assertAlmostEqual(q[0], 0.612, delta=1e-3)
        self.assertAlmostEqual(q[1], 0.707, delta=1e-3)
        self.assertAlmostEqual(q[2], 0., delta=1e-3)
        self.assertAlmostEqual(q[3], -0.354, delta=1e-3)
        self.assertAlmostEqual(t[0], -58.1836, delta=1e-4)
        self.assertAlmostEqual(t[1], 96.2797, delta=1e-4)
        self.assertAlmostEqual(t[2], -58.7221, delta=1e-4)

if __name__ == '__main__':
    IMP.test.main()
