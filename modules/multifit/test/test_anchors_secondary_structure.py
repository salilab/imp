import IMP
import IMP.test
import IMP.atom
import IMP.multifit

class Tests(IMP.test.TestCase):
    """Testing methods for adding secondary structure to anchor points"""
    def test_set_ssres(self):
        """Test if you can assign ssres to anchors"""
        m=IMP.Model()

        ### read in anchors data and set the secondary structure
        anchors_data=IMP.multifit.read_anchors_data(self.get_input_file_name("anchors.input"))
        anchors_data.setup_secondary_structure(m)
        anchor_ssres_ps=[IMP.Particle(m),IMP.Particle(m)]
        IMP.atom.SecondaryStructureResidue.setup_particle(anchor_ssres_ps[0],1.0,0.0,0.0)
        IMP.atom.SecondaryStructureResidue.setup_particle(anchor_ssres_ps[1],0.0,1.0,0.0)
        anchors_data.set_secondary_structure_probabilities(anchor_ssres_ps,[0,2])
        out_ssres=map(IMP.atom.SecondaryStructureResidue,
                      anchors_data.get_secondary_structure_particles())
        self.assertEqual(len(out_ssres),3)
        self.assertAlmostEqual(out_ssres[0].get_prob_helix(),1.0,delta=1e-6)
        self.assertAlmostEqual(out_ssres[0].get_prob_strand(),0.0,delta=1e-6)
        self.assertAlmostEqual(out_ssres[0].get_prob_coil(),0.0,delta=1e-6)
        self.assertAlmostEqual(out_ssres[1].get_prob_helix(),1.0/3,0,delta=1e-6)
        self.assertAlmostEqual(out_ssres[1].get_prob_strand(),1.0/3.0,delta=1e-6)
        self.assertAlmostEqual(out_ssres[1].get_prob_coil(),1.0/3.0,delta=1e-6)
        self.assertAlmostEqual(out_ssres[2].get_prob_helix(),0.0,delta=1e-6)
        self.assertAlmostEqual(out_ssres[2].get_prob_strand(),1.0,delta=1e-6)
        self.assertAlmostEqual(out_ssres[2].get_prob_coil(),0.0,delta=1e-6)

    def test_query_ssres(self):
        """Test if you can query anchors ssres"""
        m=IMP.Model()

        ### read in anchors data and set the secondary structure
        anchors_data=IMP.multifit.read_anchors_data(self.get_input_file_name("anchors.input"))
        anchors_data.setup_secondary_structure(m)
        anchor_ssres_ps=[IMP.Particle(m),IMP.Particle(m)]
        IMP.atom.SecondaryStructureResidue.setup_particle(anchor_ssres_ps[0],1.0,0.0,0.0)
        IMP.atom.SecondaryStructureResidue.setup_particle(anchor_ssres_ps[1],0.0,1.0,0.0)
        anchors_data.set_secondary_structure_probabilities(anchor_ssres_ps,[0,2])
        out_ssres=map(IMP.atom.SecondaryStructureResidue,
                      anchors_data.get_secondary_structure_particles())

        ### make query ssres and check match
        query_ps=[IMP.Particle(m),IMP.Particle(m)]
        IMP.atom.SecondaryStructureResidue.setup_particle(query_ps[0],1.0,0.0,0.0)
        IMP.atom.SecondaryStructureResidue.setup_particle(query_ps[1],0.0,1.0,0.0)
        matches=IMP.multifit.get_anchor_indices_matching_secondary_structure(
                            anchors_data,
                            query_ps,
                            0.9)
        self.assertEqual(set(matches[0]),set([0,1]))
        self.assertEqual(set(matches[1]),set([1,2]))
if __name__ == '__main__':
    IMP.test.main()
