import unittest
import os
import IMP.test, IMP
import IMP.xml_loader

# Class to test XML file loading
class test_xml(IMP.test.IMPTestCase):
    """test test XML file loading"""

    def setUp(self):
        """set up IMP model by loading the xml document """
        self.imp_model = IMP.Model()
        self.doc = IMP.xml_loader.load_imp_model(self.imp_model,
                                                     "xml/model.xml")

    def test_xml(self):
        """ check the xml """
        model_data = self.imp_model.get_model_data()

        # test particles
        p_iter = IMP.ParticleIterator()
        p_iter.reset(self.imp_model)
        num_particles = 0
        while p_iter.next():
            p = p_iter.get()
            fidx = p.float_index("radius")
            radius = model_data.get_float(fidx)
            iidx = p.int_index("protein")
            protein = model_data.get_int(iidx)
            num_particles = num_particles + 1
            if protein == 1:
                self.assert_(radius == 2.0, "protein1 beads should have radius 2.0")
            elif protein == 2:
                self.assert_(radius == 1.5, "protein2 beads should have radius 1.5")
            elif protein == 3:
                self.assert_(radius == 1.0, "protein3 beads should have radius 1.0")

        self.assert_(num_particles == 12, "number of particles is incorrect")

        # test restraint sets
        restraint_sets = self.imp_model.restraint_sets
        self.assert_(len(restraint_sets) == 3, "xml file contains three restraint sets")
        self.assert_(restraint_sets[0].name() == 'exclusion_volumes', "not expecting restraint set name: " + restraint_sets[0].name())
        self.assert_(restraint_sets[1].name() == 'torus', "not expecting restraint set name: " + restraint_sets[1].name())
        self.assert_(restraint_sets[2].name() == 'connections', "not expecting restraint set name: " + restraint_sets[2].name())

        # test restraints
        score = restraint_sets[0].evaluate(False)
        self.assert_((score > 16000) and (score < 17000), "expecting a score of 16198.2236328 not : " + str(score))
        score = restraint_sets[1].evaluate(False)
        self.assert_((score > 624000) and (score < 625000), "expecting a score of 624531.0625 not : " + str(score))
        score = restraint_sets[2].evaluate(False)
        self.assert_((score > -100) and (score < 100), "expecting a score of 0.0 not : " + str(score))

if __name__ == '__main__':
    unittest.main()
