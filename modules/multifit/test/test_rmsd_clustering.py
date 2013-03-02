import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em
import IMP.multifit


class Tests(IMP.test.TestCase):
    def test_clustering(self):
        """Testing proper clustering"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m = IMP.Model()
        # read PDB
        mh= IMP.atom.read_pdb(self.open_input_file("1z5s_A.pdb"),
                              m, IMP.atom.CAlphaPDBSelector())
        xyz=IMP.core.XYZs(IMP.atom.get_leaves(mh))
        #create 100 transformation that should be clustered into 4 clusters
        ts=IMP.em.FittingSolutions()
        rot=IMP.algebra.get_identity_rotation_3d()
        for i in range(4):
            cen=IMP.algebra.Vector3D(100*i,100*i,100*i)
            for j in range(25):
                t=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),
                                               cen+IMP.algebra.Vector3D(j,j,j))

                ts.add_solution(t,1.)
        out_ts=IMP.multifit.fitting_clustering(mh,ts,3,100,30)
        self.assertEqual(out_ts.get_number_of_solutions(),4)

if __name__ == '__main__':
    IMP.test.main()
