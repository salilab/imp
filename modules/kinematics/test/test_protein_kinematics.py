import IMP
import IMP.core
import IMP.test
import IMP.kinematics

class Test(IMP.test.TestCase):
    # Failing test for importing ProteinKinematics class
    def test_setup_kinematics(self):
        m = IMP.Model()
        hier = IMP.atom.read_pdb(self.open_input_file("three.pdb"), m)
        pk = IMP.kinematics.ProteinKinematics(hier)
        print(pk)

if __name__ == '__main__':
    IMP.test.main()
