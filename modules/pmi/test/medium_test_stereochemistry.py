import IMP
import IMP.atom
import IMP.test
import IMP.pmi
import IMP.pmi.io
import IMP.pmi.dof
import IMP.pmi.topology
import IMP.pmi.restraints.stereochemistry


class Tests(IMP.test.TestCase):

    def test_stereochemistry_basic(self):
        """ test PMI2 connectivity restraint on basic system"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        mol = st1.create_molecule("test", sequence="CHARLES", chain_id="A")
        mol.add_representation(mol.get_residues(),resolutions=[1])
        hier = s.build()

        cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol)

        self.assertEqual(len(mol.get_residues()) - 1, cr.get_num_restraints() )

    def test_stereochemistry_different_input(self):
        """ test PMI2 connectivity restraint on basic system
        different inputs"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        mol = st1.create_molecule("test", sequence="CHARLES", chain_id="A")
        mol.add_representation(mol.get_residues(),resolutions=[1])
        hier = s.build()
        # check the molecule input
        cr1 = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol)
        # check the molecule root hierarchy
        # cr2 = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(IMP.atom.get_leaves(hier))
        # check the molecules
        molecules=IMP.atom.get_by_type(hier,IMP.atom.MOLECULE_TYPE)
        for ml in molecules:
            cr3 = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(IMP.atom.get_leaves(ml))

        self.assertEqual(len(mol.get_residues()) - 1, cr1.get_num_restraints() )

    def test_stereochemistry_basic_rb(self):
        """ test PMI2 connectivity restraint on basic system with rigid body"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        mol = st1.create_molecule("test", sequence="CHARLES", chain_id="A")
        mol.add_representation(mol.get_residues(),resolutions=[1])
        hier = s.build()

        # Add rigid body
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        rbres = mol[0:4]
        nrparts = mol[0:1]
        dof.create_rigid_body(rbres, nonrigid_parts=nrparts)

        cr1 = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol)
        self.assertEqual( 4, cr1.get_num_restraints() )


    def test_stereochemistry_basic_two_rbs(self):
        """ test PMI2 connectivity restraint on basic system with two rigid bodies"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        mol = st1.create_molecule("test", sequence="CHARLES", chain_id="A")
        mol.add_representation(mol.get_residues(),resolutions=[1])
        hier = s.build()

        # Add rigid body
        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        rbres1 = mol[0:4]
        rbres2 = mol[4:7]
        dof.create_rigid_body(rbres1)
        dof.create_rigid_body(rbres2)

        cr1 = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol)
        self.assertEqual( 1, cr1.get_num_restraints() )

    def test_stereochemistry_system(self):
        """ test PMI2 connectivity restraint with coarse-grained real system"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('chainA.fasta'))
        st1 = s.create_state()
        mol = st1.create_molecule("GCP2_YEAST",sequence=seqs["GCP2_YEAST"][:100],chain_id='A')
        atomic_res = mol.add_structure(self.get_input_file_name('chainA.pdb'),
                                       chain_id='A',
                                       res_range=(1,100))
        mol.add_representation(mol.get_atomic_residues(),resolutions=[1,10])
        mol.add_representation(mol.get_non_atomic_residues(), resolutions=[10])
        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        dof.create_rigid_body(mol, nonrigid_parts=mol.get_non_atomic_residues())

        cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol)

        self.assertEqual(cr.get_num_restraints(), 8)


    def test_parse_dssp(self):
        """Test reading DSSP files"""
        sses = IMP.pmi.io.parse_dssp(self.get_input_file_name('chainA.dssp'),'A')
        self.assertEqual(sorted(sses.keys()),sorted(['helix','beta','loop']))
        self.assertEqual(sses['helix'][1][0],[100,126,'A'])
        self.assertEqual(sses['beta'][0],[[76,78,'A'],[91,93,'A']])
        self.assertEqual(len(sses['helix']),20)
        self.assertEqual(len(sses['beta']),3)
        self.assertEqual(len(sses['loop']),32)

    def test_elastic_network(self):
        """ test PMI setup of elastic nets """
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('chainA.fasta'))
        st1 = s.create_state()
        m1 = st1.create_molecule("GCP2_YEAST",sequence=seqs["GCP2_YEAST"][:100],chain_id='A')
        atomic_res = m1.add_structure(self.get_input_file_name('chainA.pdb'),
                                      chain_id='A',
                                      res_range=(1,100))
        m1.add_representation(atomic_res,resolutions=[0])
        hier = s.build()

        # create elastic network from some SSEs
        sses = IMP.pmi.io.parse_dssp(self.get_input_file_name('chainA.dssp'),'A',name_map={'A':'GCP2_YEAST'})
        er = IMP.pmi.restraints.stereochemistry.ElasticNetworkRestraint(
            selection_tuples=[sses['helix'][0][0]],
            strength=10.0,
            dist_cutoff=5.0,
            ca_only=True,
            hierarchy=hier)
        self.assertEqual(er.get_restraint().get_number_of_restraints(),12)

        lhelix = sses['helix'][0][0][1] - sses['helix'][0][0][0]+1
        hr = IMP.pmi.restraints.stereochemistry.HelixRestraint(hier,sses['helix'][0][0])
        self.assertEqual(hr.get_number_of_dihedrals(),lhelix-2)
        self.assertEqual(hr.get_number_of_bonds(),lhelix-4)


    def test_excluded_volume_sphere(self):
        """Test excluded volume in PMI1"""
        pdbfile = self.get_input_file_name("mini.pdb")
        fastafile = self.get_input_file_name("mini.fasta")

        comps = ["P1", "P2", "P3"]
        chains = "ABB"
        colors = [0., 0.5, 1.0]
        beadsize = 20
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)

        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)

        simo.create_component(comps[0], color=colors[0])
        simo.add_component_sequence(comps[0], fastafile, id=fastids[0])
        simo.autobuild_model(comps[0], pdbfile, chains[0],
                             resolutions=[1], missingbeadsize=beadsize)

        simo.create_component(comps[1], color=colors[1])
        simo.add_component_sequence(comps[1], fastafile, id=fastids[1])
        simo.autobuild_model(comps[1], pdbfile, chains[1],
                             resolutions=[10], missingbeadsize=beadsize)

        simo.create_component(comps[2], color=colors[2])
        simo.add_component_sequence(comps[2], fastafile, id=fastids[1])
        simo.autobuild_model(comps[2], pdbfile, chains[2],
                             resolutions=[1,10], missingbeadsize=2)

        ev1 = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(simo)
        ev1.add_to_model()
        print(ev1.get_output())
        sf = IMP.core.RestraintsScoringFunction(
                                   IMP.pmi.tools.get_restraint_set(m))
        print(sf.evaluate(False))
        self.assertEqual(len(ev1.cpc.get_all_possible_indexes()),16)

        ev2 = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
            simo,
            [simo.hier_dict["P2"]],
            [simo.hier_dict["P3"]])
        ev2.add_to_model()
        print(ev2.get_output())
        sf = IMP.core.RestraintsScoringFunction(
            IMP.pmi.tools.get_restraint_set(m))
        print(sf.evaluate(False))
        self.assertEqual(len(ev2.cpc.get_all_possible_indexes()),8)

    def test_excluded_volume_sphere_pmi2(self):
        """ Tests excluded volume restraint in PMI2 """

        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('chainA.fasta'))
        mol = st1.create_molecule("Test", seqs["GCP2_YEAST"][:100])
        mol.add_structure(self.get_input_file_name('chainA.pdb'), chain_id="A", offset=0,res_range=(1,100))
        mol.add_representation(mol.get_atomic_residues(), resolutions=[1,10])
        mol.add_representation(mol.get_non_atomic_residues(), resolutions=[10])
        hier = s.build()

        # Test that the correct number of particles are included
        #  (note, res1 is going to include the res10 beads because they're the only ones around)
        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
            included_objects=mol, resolution=1)
        self.assertEqual(len(ev.cpc.get_all_possible_indexes()), 44)

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
            included_objects=mol, resolution=10)
        self.assertEqual(len(ev.cpc.get_all_possible_indexes()), 12)

        # Test that default returns lowest resolution
        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
            included_objects=mol)
        self.assertEqual(len(ev.cpc.get_all_possible_indexes()), 12)

        # test just picking a few residues - this one picks two beads
        resis = mol.residue_range(10,29)
        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
            included_objects=resis, resolution=1)
        self.assertEqual(len(ev.cpc.get_all_possible_indexes()), 2)


    def test_charmm(self):
        """ test PMI setup of CHARMM"""
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('chainA.fasta'))
        st1 = s.create_state()
        m1 = st1.create_molecule("GCP2_YEAST",sequence=seqs["GCP2_YEAST"][:100],chain_id='A')
        atomic_res = m1.add_structure(self.get_input_file_name('chainA.pdb'),
                                      chain_id='A',
                                      res_range=(1,100))
        m1.add_representation(atomic_res,resolutions=[0])
        hier = s.build()

        # create elastic network from some SSEs
        charmm = IMP.pmi.restraints.stereochemistry.CharmmForceFieldRestraint(hier)

if __name__ == '__main__':
    IMP.test.main()
