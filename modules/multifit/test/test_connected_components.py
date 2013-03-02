import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit

class Tests(IMP.test.TestCase):
    """Test connected components """

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.VERBOSE)#SILENT)

    def test_connected_components(self):
        """test connected components"""
        #create a map of X connected components
        dmap=IMP.em.create_density_map(IMP.algebra.BoundingBox3D(
            IMP.algebra.Vector3D(-50,-50,-50),
            IMP.algebra.Vector3D(50,50,50)),2.)

        for i in range(5):
            #sample i populations
            mdl=IMP.Model()
            ps=[]
            for j in range(i+1):
                bb=IMP.algebra.BoundingBox3D(
                    IMP.algebra.Vector3D(-1*(j+1),-1*(j+1),-1*(j+1)),
                    IMP.algebra.Vector3D(1*(j+1),1*(j+1),1*(j+1)))
                for k in range(10):
                    p=IMP.Particle(mdl)
                    IMP.core.XYZR.setup_particle(p,
                                   IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb)+IMP.algebra.Vector3D(j*20,j*20,j*20),2))
                    IMP.atom.Mass.setup_particle(p,1)
                    ps.append(p)
            dmap=IMP.em.particles2density(ps,1,1)
            con_comp=IMP.multifit.get_connected_components(dmap,0.001,0.5)
            for c in con_comp:
                for ind in c:
                    self.assertLess(ind,dmap.get_number_of_voxels())

            self.assertEqual(len(con_comp),i+1)

if __name__ == '__main__':
    IMP.test.main()
