import sys
import unittest
import IMP.utils
import IMP.test, IMP
import IMP.domino as domino
import IMP.core
import IMP.atom
import IMP.algebra
import math
class DOMINOTests(IMP.test.TestCase):
    def __set_particles(self):
        #after the sampler you need to check that the centroids are rotated the same
        self.m = IMP.Model()
        self.ps = IMP.Particles()
        self.prots=[]
        #create three MolecularHierarchy particels:
        self.prots.append(IMP.atom.read_pdb(self.get_input_file_name("symm1.pdb"),
                                    self.m))
        self.prots[0].get_particle().set_value(IMP.StringKey("name"),str(0))
        self.ps.append(self.prots[0].get_particle())
        self.ref = IMP.atom.read_pdb(self.get_input_file_name("symm1.pdb"),
                                     self.m)

        self.prots.append(IMP.atom.read_pdb(self.get_input_file_name("symm2.pdb"),
                                         self.m))
        self.prots[1].get_particle().set_value(IMP.StringKey("name"),str(1))
        self.ps.append(self.prots[1].get_particle())
        self.prots.append(IMP.atom.read_pdb(self.get_input_file_name("symm3.pdb"),
                                    self.m))
        self.prots[2].get_particle().set_value(IMP.StringKey("name"),str(2))
        self.ps.append(self.prots[2].get_particle())

    def __set_sampling_space__(self):
        print "================"
        #set 10 transformations on a patch of a sphere
        self.rt = IMP.domino.TransformationDiscreteSet()
        self.rt.set_model(self.m)
        max_d1 = IMP.atom.diameter(self.ref)
        max_d=IMP.algebra.Segment3D(max_d1.get_point(1),max_d1.get_point(0))
        #print ".dot " + str(max_d.get_point(0)[0]) + " " + str(max_d.get_point(0)[1]) + " " + str(max_d.get_point(0)[2])
        #print ".dot " +str(max_d.get_point(1)[0]) + " " + str(max_d.get_point(1)[1]) + " " + str(max_d.get_point(1)[2])
        cone = IMP.algebra.Cone3D(max_d,max_d.lenght())
        print ".cone " + str(max_d.get_point(1)[0]) + " " + str(max_d.get_point(1)[1]) + " " + str(max_d.get_point(1)[2]) +" " + str(max_d.get_point(0)[0]) + " " + str(max_d.get_point(0)[1]) + " " + str(max_d.get_point(0)[2]) + " " + str(max_d.lenght())
        pln = cone.get_intersecting_plane()
        patch = IMP.algebra.Sphere3DPatch(cone.get_bounding_sphere(),pln)
        print ".dot " +str(pln.get_point_on_plane()[0]) + " " + str(pln.get_point_on_plane()[1]) + " " + str(pln.get_point_on_plane()[2])
        sss = cone.get_bounding_sphere()
        print ".sphere " + str(sss.get_center()[0]) + " " + str(sss.get_center()[1]) + " "+ str(sss.get_center()[2]) + " " + str(sss.get_radius())
        #print ".sphere " + str(patch.get_sphere().get_center()[0]) + " " + str(patch.get_sphere().get_center()[1]) + " "+ str(patch.get_sphere().get_center()[2]) + " " + str(patch.get_sphere().get_radius())
        zero_vec = IMP.algebra.Vector3D(0.0,0.0,0.0)
        rots = IMP.algebra.uniform_cover(patch,10)
        for rot in rots:
            print ".dot " + str(rot[0]) + " " + str(rot[1]) + " " + str(rot[2])
            r = IMP.algebra.rotation_between_two_vectors(max_d.get_point(1),rot)
            t = IMP.algebra.Transformation3D(r,zero_vec)
            self.rt.add_transformation(t)
        #write a function to find a bounding cylinder
#         self.cyl = IMP.algebra.Cylinder3D(IMP.algebra.Vector3D(0.0,0.0,-50.0),
#                                           IMP.algebra.Vector3D(0.0,0.0,-90.0),5.0)
        self.cyl = IMP.algebra.Cylinder3D(IMP.algebra.Vector3D(0.0,0.0,0.0),
                                          IMP.algebra.Vector3D(0.0,0.0,1.0),5.0)

        self.sampler = IMP.domino.SymmetrySampler(self.ps,self.rt,self.cyl)
        self.sampler.set_ref(self.ref.get_particle())
        print "11"
    def set_restraint_graph(self):
        jt_filename = self.get_input_file_name("simple_jt3.txt")
        self.d_opt = IMP.domino.DominoOptimizer(jt_filename,self.m)
        self.d_opt.set_sampling_space(self.sampler)

    def setUp(self):
        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)
        self.__set_particles()
        self.__set_sampling_space__()
        self.set_restraint_graph()

    def test_global_min(self):
        """
        Test that the sampler created symmetric sampling space
        """
        r = self.d_opt.get_graph()
        n = r.get_node(self.ps)
        cendtroids=[IMP.algebra.Vector3D(0.0,0.0,0.0),
                    IMP.algebra.Vector3D(0.0,0.0,0.0),
                    IMP.algebra.Vector3D(0.0,0.0,0.0),]
        for i in xrange(5):
            state = n.get_state(i)
            self.sampler.move2state(state)
            for j,p in enumerate(self.ps):
                cendtroids[j] = cendtroids[j]+IMP.atom.centroid(self.prots[j])
            state.show()
        rot120 = IMP.algebra.rotate(self.cyl,2.*math.pi/3)
        for j in xrange(2):
            cendtroids[j].show()
            cendtroids[j+1].show()
            rot120.transform(cendtroids[j]).show()
            self.assertAlmostEqual(IMP.algebra.distance(rot120.transform(cendtroids[j]),cendtroids[j+1]),0.0,2)
if __name__ == '__main__':
    unittest.main()
