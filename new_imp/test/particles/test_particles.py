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
            p1.add_float("radius", 1.5 * i, False)
            p1.add_int("id", i)
            p1.add_int("six", 6)
            p1.add_string("id_str", "name_"+str(i))
            p1.add_string("six", "b:0110")

        # add additional attributes to particle 11
        for i in range(0,6):
            p1.add_float("attr_" + str(i), 3.5 * i, False)

    def test_bad_attributes(self):
        """Asking for non-existent attributes should cause an error"""
        p1 = self.particles[0]
        self.assertRaises(IndexError, p1.get_float_index, "notexist")
        self.assertRaises(IndexError, p1.get_int_index, "notexist")
        self.assertRaises(IndexError, p1.get_string_index, "notexist")

    def test_attribute_iterator(self):
        """Iterate over all attributes within a particle"""
        float_attr_iter = IMP.FloatAttributeIterator()
        float_attr_iter.reset(self.particles[0])
        cnt = 0
        while float_attr_iter.next():
            cnt = cnt+1
            self.assert_(self.particles[0].get_float(float_attr_iter.get_key()) == float_attr_iter.get_value(), "Value should match the one accessed through the key.")
        self.assert_(cnt == 4, "Particle 0 should have 4 float attributes.")

        float_attr_iter.reset(self.particles[11])
        cnt = 0
        while float_attr_iter.next():
            cnt = cnt+1
            self.assert_(self.particles[11].get_float(float_attr_iter.get_key()) == float_attr_iter.get_value(), "Value should match the one accessed through the key.")
        self.assert_(cnt == 10, "Particle 11 should have 10 float attributes.")

        int_attr_iter = IMP.IntAttributeIterator()
        int_attr_iter.reset(self.particles[0])
        cnt = 0
        while int_attr_iter.next():
            cnt = cnt+1
            self.assert_(self.particles[0].get_int(int_attr_iter.get_key()) == int_attr_iter.get_value(), "Value should match the one accessed through the key.")
        self.assert_(cnt == 2, "Particle 0 should have 2 int attributes.")

        int_attr_iter .reset(self.particles[11])
        cnt = 0
        while int_attr_iter.next():
            cnt = cnt+1
            self.assert_(self.particles[11].get_int(int_attr_iter.get_key()) == int_attr_iter.get_value(), "Value should match the one accessed through the key.")
        self.assert_(cnt == 2, "Particle 11 should have 2 int attributes.")

        string_attr_iter = IMP.StringAttributeIterator()
        string_attr_iter.reset(self.particles[0])
        cnt = 0
        while string_attr_iter.next():
            cnt = cnt+1
            self.assert_(self.particles[0].get_string(string_attr_iter.get_key()) == string_attr_iter.get_value(), "Value should match the one accessed through the key.")
        self.assert_(cnt == 2, "Particle 0 should have 2 string attributes.")

        string_attr_iter .reset(self.particles[11])
        cnt = 0
        while string_attr_iter.next():
            cnt = cnt+1
            self.assert_(self.particles[11].get_string(string_attr_iter.get_key()) == string_attr_iter.get_value(), "Value should match the one accessed through the key.")
        self.assert_(cnt == 2, "Particle 11 should have 2 string attributes.")

        IMP.utils.show_particles(self.particles)

    def test_particles(self):
        """Test that particle attributes are available and correct"""

        model_data = self.imp_model.get_model_data()

         # check particles 0 - 11
        for i in range(0,12):
            p1 = self.particles[i]
            fidx = p1.get_float_index("radius")
            self.assert_(model_data.get_float(fidx) == 1.5 * i, "expecting particle "+str(i)+" radius to be "+str(1.5*i) + " not " + str(model_data.get_float(fidx)))
            iidx = p1.get_int_index("id")
            self.assert_(model_data.get_int(iidx) == i, "expecting particle "+str(i)+" id to be "+str(i) + " not " + str(model_data.get_int(iidx)))
            sidx = p1.get_string_index("id_str")
            self.assert_(model_data.get_string(sidx) == "name_"+str(i), "expecting particle "+str(i)+" id_str to be name_"+str(i) + " not " + model_data.get_string(sidx))

        # test additional attributes in particle 11
        p1 = self.particles[11]
        for i in range(0,6):
            fidx = p1.get_float_index("attr_" + str(i))
            self.assert_(model_data.get_float(fidx) == 3.5 * i, "expecting particle "+str(i)+" radius to be "+str(3.2*i) + " not " + str(model_data.get_float(fidx)))


if __name__ == '__main__':
    unittest.main()
