import IMP
import IMP.test
import IMP.kinematics
import pickle


class Test(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of DOFValues"""
        dof = IMP.kinematics.DOF(42., 1., 2., 0.1)
        dofs = IMP.kinematics.DOFValues([dof])
        dump = pickle.dumps(dofs)
        newdofs = pickle.loads(dump)
        self.assertLess(newdofs.get_distance2(dofs), 1e-4)


if __name__ == '__main__':
    IMP.test.main()
