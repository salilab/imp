import IMP.test
import IMP
import IMP.statistics
import IMP.algebra

class Tests(IMP.test.TestCase):
    IMP.set_log_level(IMP.VERBOSE)
    def test_gmm(self):
        vs= []
        centers=(IMP.algebra.Vector3D(0,0,0),
                 IMP.algebra.Vector3D(10,15,20),
                 IMP.algebra.Vector3D(60,30,12))
        for i in range(0,3):
            for j in range(0,100):
                vs.append(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(centers[i], 10 )))
        e= IMP.statistics.VectorDEmbedding(vs)
        gmm=IMP.statistics.create_gaussian_mixture_model(e,3)
        #print gmm
        #for i in range(3):
        #    print gmm.get_gaussian_component(i).show_single_line()

if __name__ == '__main__':
    IMP.test.main()
