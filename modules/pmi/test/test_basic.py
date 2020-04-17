import math
import IMP
import IMP.core
import IMP.pmi.topology
import IMP.pmi.restraints.basic
import IMP.test


def _harmonic_prob(x, x0, sig):
    return math.exp(-(x-x0)**2 / 2. / sig**2)


class Tests(IMP.test.TestCase):
    def test_distance(self):
        """Test setup of distance restraint"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        m1 = st1.create_molecule("Prot1",chain_id='A',sequence=seqs["Prot1"])
        m2 = st1.create_molecule("Prot2",chain_id='B',sequence=seqs["Prot2"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(55,64),offset=-54)
        a2 = m2.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='B',res_range=(180,192),offset=-179)
        m1.add_representation(a1,resolutions=[0,1])
        m1.add_representation(m1[:]-a1,resolutions=[1])
        m2.add_representation(a2,resolutions=[0,1])
        m3 = m2.create_clone('C')
        hier = s.build()


        dr = IMP.pmi.restraints.basic.DistanceRestraint(root_hier = hier,
                                                        tuple_selection1=(1,1,"Prot1",0),
                                                        tuple_selection2=(1,1,"Prot2",1),
                                                        distancemin=5,
                                                        distancemax=15)
        dr.add_to_model()
        rs = dr.get_restraint().get_restraints()
        self.assertEqual(len(rs),2)
        p1 = IMP.atom.Selection(hier,molecule="Prot1",resolution=1,
                                residue_index=1).get_selected_particles()[0]
        p2 = IMP.atom.Selection(hier,molecule="Prot2",resolution=1,
                                residue_index=1,copy_index=1).get_selected_particles()[0]
        self.assertEqual(set(rs[0].get_inputs()),set([p1,p2]))

    def test_bistable_distance(self):
        m = IMP.Model()
        p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m))
        p2 = IMP.core.XYZ.setup_particle(IMP.Particle(m))
        p1.set_coordinates([0., 0., 0.])
        p2.set_coordinates([0., 0., 2.])
        dists = [1., 4.]
        weights = [.5, .5]
        sigmas = [1., 1.]
        r = IMP.pmi.restraints.basic.BiStableDistanceRestraint(
            m, p1, p2, dists[0], dists[1], sigmas[0], sigmas[1], weights[0],
            weights[1])
        r.set_was_used(True)
        rscore = r.unprotected_evaluate(None)

        tprob = (.5 * _harmonic_prob(2., dists[0], sigmas[0]) +
                 .5 * _harmonic_prob(2., dists[1], sigmas[1]))
        tscore = -math.log(tprob)
        self.assertAlmostEqual(rscore, tscore, delta=1e-6)
        self.assertEqual(len(r.do_get_inputs()), 2)
        self.assertListEqual(r.do_get_inputs(), [p1, p2])
        self.assertRaises(
            ValueError, IMP.pmi.restraints.basic.BiStableDistanceRestraint, m,
            p1, p2, dists[0], dists[1], sigmas[0], sigmas[1], weights[0],
            weights[1] + 1e-5)

    def test_distance_to_point(self):
        """Test DistanceToPointRestraint"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})
        m1 = st1.create_molecule("Prot1",chain_id='A',sequence=seqs["Prot1"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(55,64),offset=-54)
        m1.add_representation(a1,resolutions=[0,1])
        hier = s.build()

        # Invalid anchor point
        self.assertRaises(TypeError,
                          IMP.pmi.restraints.basic.DistanceToPointRestraint,
                          root_hier=hier, tuple_selection=(1,1,"Prot1",0),
                          anchor_point='foo')

        # anchor point explicitly set to None
        dr = IMP.pmi.restraints.basic.DistanceToPointRestraint(
                                   root_hier=hier,
                                   tuple_selection=(1,1,"Prot1",0),
                                   anchor_point=None)

        # default anchor point
        dr = IMP.pmi.restraints.basic.DistanceToPointRestraint(
                                   root_hier=hier,
                                   tuple_selection=(1,1,"Prot1",0))


if __name__ == '__main__':
    IMP.test.main()
