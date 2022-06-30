from __future__ import print_function, division
import IMP
import IMP.test
import IMP.core
import IMP.pmi
import IMP.pmi.restraints
import IMP.pmi.restraints.residue_proximity_restraint
import math

class Tests(IMP.test.TestCase):

    def setup_system(self, ):

        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        
        pA = st1.create_molecule('ProtA', 'A', chain_id='A')
        pA.add_representation(pA, resolutions=[1], color='red')

        pB = st1.create_molecule('ProtB', 'A', chain_id='A')
        pB.add_representation(pB, resolutions=[1], color='blue')
        
        root_hier = s.build()
        
        return root_hier, pA, pB

    def get_score(self, distance, sigma, xi):
        prior_prob = math.exp(-xi*distance)
        prob = math.exp(-(distance**2)/(2*sigma**2))/math.sqrt(2*math.pi*sigma*sigma)
        score = -math.log(prob*prior_prob)
        return score
        

    def test_ResidueProteinProximityRestraint_min_score(self):
        root_hier, pA, pB = self.setup_system()
        mdl = root_hier.get_model()
        
        # Translate one bead
        trans = IMP.algebra.Transformation3D([2, 0, 0])
        for fb in IMP.core.get_leaves(pB.get_hierarchy()):
            IMP.core.transform(IMP.core.XYZ(fb), trans)

        xi = 0.01
        br_rest = IMP.pmi.restraints.residue_proximity_restraint.ResidueProteinProximityRestraint(root_hier,
                                                                                                  selection  = ('ProtA',1,1,'ProtB'))
        br_rest.add_to_model()
        mdl.update()

        min_score = -math.log(math.exp(-xi*2.))
        self.assertAlmostEqual(br_rest.evaluate(), min_score, delta = 0.01)

        

    def test_ResidueProteinProximityRestraint_continuity1(self):
        root_hier, pA, pB = self.setup_system()
        mdl = root_hier.get_model()

        cutoff = 6.
        xi = 0.01
        
        # Translate one bead
        trans = IMP.algebra.Transformation3D([cutoff/2., 0, 0])
        for fb in IMP.core.get_leaves(pB.get_hierarchy()):
            IMP.core.transform(IMP.core.XYZ(fb), trans)

        br_rest = IMP.pmi.restraints.residue_proximity_restraint.ResidueProteinProximityRestraint(root_hier,
                                                                                                  selection  = ('ProtA',1,2,'ProtB'))
        br_rest.add_to_model()
        mdl.update()

        score_1 = br_rest.evaluate()
        
        trans = IMP.algebra.Transformation3D([0.01, 0, 0])
        for fb in IMP.core.get_leaves(pB.get_hierarchy()):
            IMP.core.transform(IMP.core.XYZ(fb), trans)

        score_2 = br_rest.evaluate()

        print(score_1, score_2)
        self.assertGreater(score_2, score_1)
        self.assertAlmostEqual(score_1, score_2, delta = 0.02)

    def test_ResidueProteinProximityRestraint_continuity2(self):
        root_hier, pA, pB = self.setup_system()
        mdl = root_hier.get_model()

        cutoff = 6.
        xi = 0.01
        
        # Translate one bead
        trans = IMP.algebra.Transformation3D([cutoff, 0, 0])
        for fb in IMP.core.get_leaves(pB.get_hierarchy()):
            IMP.core.transform(IMP.core.XYZ(fb), trans)

        br_rest = IMP.pmi.restraints.residue_proximity_restraint.ResidueProteinProximityRestraint(root_hier,
                                                                                                  selection  = ('ProtA',1,2,'ProtB'))
        br_rest.add_to_model()
        mdl.update()

        score_1 = br_rest.evaluate()
        
        trans = IMP.algebra.Transformation3D([0.01, 0, 0])
        for fb in IMP.core.get_leaves(pB.get_hierarchy()):
            IMP.core.transform(IMP.core.XYZ(fb), trans)

        score_2 = br_rest.evaluate()

        self.assertGreater(score_2, score_1)
        self.assertAlmostEqual(score_1, score_2, delta = 0.02)

    def test_ResidueProteinProximityRestraint_score(self):
        root_hier, pA, pB = self.setup_system()
        mdl = root_hier.get_model()

        distance = 8.
        xi = 0.01
        sigma = 3.0

        # Translate one bead
        trans = IMP.algebra.Transformation3D([distance, 0, 0])
        for fb in IMP.core.get_leaves(pB.get_hierarchy()):
            IMP.core.transform(IMP.core.XYZ(fb), trans)

        br_rest = IMP.pmi.restraints.residue_proximity_restraint.ResidueProteinProximityRestraint(root_hier,
                                                                                                  selection  = ('ProtA',1,2,'ProtB'))
        br_rest.add_to_model()
        mdl.update()

        score = self.get_score(distance, sigma, xi)
        
        self.assertAlmostEqual(br_rest.evaluate(), score, delta = 0.02)

    def test_ResidueProteinProximityRestraint_pairs(self):
        root_hier, pA, pB = self.setup_system()
        mdl = root_hier.get_model()

        distance = 8.
        xi = 0.01
        sigma = 3.0

        # Translate one bead
        trans = IMP.algebra.Transformation3D([3., 0, 0])
        for fb in IMP.core.get_leaves(pB.get_hierarchy()):
            IMP.core.transform(IMP.core.XYZ(fb), trans)

        br_rest = IMP.pmi.restraints.residue_proximity_restraint.ResidueProteinProximityRestraint(root_hier,
                                                                                                  selection  = ('ProtA',1,2,'ProtB'))
        br_rest.add_to_model()
        mdl.update()
        
        self.assertEqual(len(br_rest.get_container_pairs()), 1)
        self.assertEqual(len(br_rest.get_container_pairs()[0]), 2)


if __name__ == '__main__':
    IMP.test.main()
