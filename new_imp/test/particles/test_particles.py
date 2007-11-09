import unittest
import IMP
import IMP.utils
import IMP.test

class ParticleTests(IMP.test.IMPTestCase):
    """Test particles"""

    def setUp(self):
        """set up Modeller with the torus restraints """

        self.imp_model = IMP.Model()
        self.particles = []
        self.restraint_sets = []
        self.rsrs = []

        # create particles 0 - 11
        for i in range(0,12):
            self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                        i*2, i*3, i*4))
            p1 = self.particles[i]
            p1.add_attribute(IMP.FloatKey("radius"), 1.5 * i, False)
            p1.add_attribute(IMP.IntKey("id"), i)
            p1.add_attribute(IMP.IntKey("six"), 6)
            p1.add_attribute(IMP.StringKey("id_str"), "name_"+str(i))
            p1.add_attribute(IMP.StringKey("six"), "b:0110")

        # add additional attributes to particle 11
        for i in range(0,6):
            p1.add_attribute(IMP.FloatKey("attr_" + str(i)), 3.5 * i, False)

    def test_bad_attributes(self):
        """Asking for non-existent attributes should cause an error"""
        p1 = self.particles[0]
        self.assertRaises(IndexError, p1.get_attribute, IMP.FloatKey("notexist"))
        self.assertRaises(IndexError, p1.get_attribute, IMP.IntKey("notexist"))
        self.assertRaises(IndexError, p1.get_attribute, IMP.StringKey("notexist"))


    def test_particles(self):
        """Test that particle attributes are available and correct"""

        model_data = self.imp_model.get_model_data()

         # check particles 0 - 11
        for i in range(0,12):
            p1 = self.particles[i]
            fidx = p1.get_attribute(IMP.FloatKey("radius"))
            self.assert_(model_data.get_value(fidx) == 1.5 * i, "expecting particle "+str(i)+" radius to be "+str(1.5*i) + " not " + str(model_data.get_value(fidx)))
            iidx = p1.get_attribute(IMP.IntKey("id"))
            self.assert_(model_data.get_value(iidx) == i, "expecting particle "+str(i)+" id to be "+str(i) + " not " + str(model_data.get_value(iidx)))
            sidx = p1.get_attribute(IMP.StringKey("id_str"))
            self.assert_(model_data.get_value(sidx) == "name_"+str(i), "expecting particle "+str(i)+" id_str to be name_"+str(i) + " not " + model_data.get_value(sidx))

        # test additional attributes in particle 11
        p1 = self.particles[11]
        for i in range(0,6):
            fidx = p1.get_attribute(IMP.FloatKey("attr_" + str(i)))
            self.assert_(model_data.get_value(fidx) == 3.5 * i, "expecting particle "+str(i)+" radius to be "+str(3.2*i) + " not " + str(model_data.get_value(fidx)))


if __name__ == '__main__':
    unittest.main()
