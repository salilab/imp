import IMP
import IMP.test
import IMP.pmi.restraints.stereochemistry
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.output
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import os,shutil

try:
    import IMP.mpi
    rem = IMP.mpi.ReplicaExchange()
except ImportError:
    rem = None

class Tests(IMP.test.TestCase):
    def test_pdb_writing(self):
        """Test writing of PDB files"""

        # input parameter
        pdbfile = self.get_input_file_name("mini.pdb")
        fastafile = self.get_input_file_name("mini.fasta")

        comps = ["P1", "P2", "P3"]
        chains = "ABB"
        colors = [0., 0.5, 1.0]
        beadsize = 20
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)


        m = IMP.Model()
        with IMP.allow_deprecated():
            simo = IMP.pmi.representation.Representation(m)

        simo.create_component(comps[0], color=colors[0])
        simo.add_component_sequence(comps[0], fastafile, id=fastids[0])
        simo.autobuild_model(comps[0], pdbfile, chains[0],
                             resolutions=[1], missingbeadsize=beadsize)
        simo.setup_component_sequence_connectivity(comps[0], 1)

        simo.create_component(comps[1], color=colors[1])
        simo.add_component_sequence(comps[1], fastafile, id=fastids[1])
        simo.autobuild_model(comps[1], pdbfile, chains[1],
                             resolutions=[10], missingbeadsize=beadsize)
        simo.setup_component_sequence_connectivity(comps[1], 1)

        simo.create_component(comps[2], color=colors[2])
        simo.add_component_sequence(comps[2], fastafile, id=fastids[1])
        simo.autobuild_model(comps[2], pdbfile, chains[2],
                             resolutions=[1,10], missingbeadsize=2)
        simo.setup_component_sequence_connectivity(comps[2], 1)

        output = IMP.pmi.output.Output()
        output.init_pdb("test_pdb_writing.pdb", simo.prot)
        output.write_pdbs()
        output.init_pdb_best_scoring("test_pdb_writing", simo.prot, 10)
        for i in range(20):
            score = -float(i)
            output.write_pdb_best_scoring(score)

        pdb_content='''ATOM      1  CA  MET A   1     114.370  27.980 -26.088  1.00  3.07           C
ATOM      2  CA  VAL A   2     114.370  27.980 -26.088  1.00  2.89           C
ATOM      3  CA  GLY A   3     111.506  26.368 -28.075  1.00  2.27           C
ATOM      4  CA  GLN A   4     113.468  23.113 -28.639  1.00  3.01           C
ATOM      5  CA  GLN A   5     113.808  21.534 -32.168  1.00  3.01           C
ATOM      6  CA  TYR A   6     116.743  22.770 -34.259  1.00  3.32           C
ATOM      7  CA  SER A   7     116.626  25.161 -37.229  1.00  2.61           C
ATOM      8  CA  SER A   8     119.165  25.590 -40.036  1.00  2.61           C
ATOM      9  CA  BEA B   2      70.427  58.819  51.717  1.00  3.50           C
ATOM     10  CA  BEA B   4      68.799  58.791  50.131  1.00  3.69           C
ATOM     11  CA  BEA B   7      68.799  58.791  50.131  1.00  4.76           C
ATOM     12  CA  BEA C   2      70.427  58.819  51.717  1.00  3.50           C
ATOM     13  CA  ASP C   3      70.427  58.819  51.717  1.00  2.87           C
ATOM     14  CA  GLU C   4      68.584  58.274  48.425  1.00  3.00           C
ATOM     15  CA  BEA C   6      68.584  58.274  48.425  1.00  3.50           C
ATOM     16  CA  BEA C   8      68.584  58.274  48.425  1.00  3.50           C
ATOM     17  CA  THR C   9      68.584  58.274  48.425  1.00  2.80           C
ENDMDL'''.split("\n")

        with open("test_pdb_writing.pdb") as f:
            for nl, l in enumerate(f):
                self.assertEqual(pdb_content[nl],l.rstrip('\r\n '))

        with open("test_pdb_writing.0.pdb") as f:
            for nl, l in enumerate(f):
                self.assertEqual(pdb_content[nl],l.rstrip('\r\n '))

        for i in range(10):
            os.unlink('test_pdb_writing.'+str(i)+'.pdb')
        os.unlink('test_pdb_writing.pdb')

    def test_pdb_pmi2(self):
        """Test PDB writing in PMI2"""
        mdl = IMP.Model()
        pdb_file = self.get_input_file_name("mini.pdb")
        fasta_file = self.get_input_file_name("mini.fasta")

        seqs = IMP.pmi.topology.Sequences(fasta_file)
        s = IMP.pmi.topology.System(mdl)
        st = s.create_state()

        molA = st.create_molecule("P1",seqs[0],chain_id='A')
        aresA = molA.add_structure(pdb_file,chain_id='A',soft_check=True)
        molA.add_representation(aresA,[1])
        molA.add_representation(molA[:]-aresA,20)

        molB = st.create_molecule("P2",seqs[1],chain_id='B')
        aresB = molB.add_structure(pdb_file,chain_id='B',soft_check=True)
        molB.add_representation(aresB,[10])
        molB.add_representation(molB[:]-aresB,20)

        molC = molB.create_copy(chain_id='C')
        aresC = molC.add_structure(pdb_file,chain_id='B',soft_check=True)
        molC.add_representation(aresC,[1,10])
        molC.add_representation(molC[:]-aresC,2)
        root_hier = s.build()

        # write PDB and check it's ok
        output = IMP.pmi.output.Output(atomistic=True)
        output.init_pdb("test_pdb_writing.pdb", root_hier)
        output.write_pdbs()
        print('init best scoring')
        output.init_pdb_best_scoring("test_pdb_writing", root_hier, 10)
        print('scoring')
        for i in range(20):
            score = -float(i)
            output.write_pdb_best_scoring(score)

        pdb_content='''ATOM      1  CA  MET A   1     114.370  27.980 -26.088  1.00  3.07           C
ATOM      2  CA  VAL A   2     114.370  27.980 -26.088  1.00  2.89           C
ATOM      3  CA  GLY A   3     111.506  26.368 -28.075  1.00  2.27           C
ATOM      4  CA  GLN A   4     113.468  23.113 -28.639  1.00  3.01           C
ATOM      5  CA  GLN A   5     113.808  21.534 -32.168  1.00  3.01           C
ATOM      6  CA  TYR A   6     116.743  22.770 -34.259  1.00  3.32           C
ATOM      7  CA  SER A   7     116.626  25.161 -37.229  1.00  2.61           C
ATOM      8  CA  SER A   8     119.165  25.590 -40.036  1.00  2.61           C
ATOM      9  CA  BEA B   2      70.427  58.819  51.717  1.00  3.50           C
ATOM     10  CA  BEA B   4      68.799  58.791  50.131  1.00  3.69           C
ATOM     11  CA  BEA B   7      68.584  58.274  48.425  1.00  4.76           C
ATOM     12  CA  BEA C   2      70.427  58.819  51.717  1.00  3.50           C
ATOM     13  CA  ASP C   3      70.427  58.819  51.717  1.00  2.87           C
ATOM     14  CA  GLU C   4      68.584  58.274  48.425  1.00  3.00           C
ATOM     15  CA  BEA C   6      68.584  58.274  48.425  1.00  3.50           C
ATOM     16  CA  BEA C   8      68.584  58.274  48.425  1.00  3.50           C
ATOM     17  CA  THR C   9      68.584  58.274  48.425  1.00  2.80           C
ENDMDL'''.split("\n")

        with open("test_pdb_writing.pdb") as f:
            for nl, l in enumerate(f):
                self.assertEqual(pdb_content[nl],l.rstrip('\r\n '))

        with open("test_pdb_writing.0.pdb") as f:
            for nl, l in enumerate(f):
                self.assertEqual(pdb_content[nl],l.rstrip('\r\n '))

        for i in range(10):
            os.unlink('test_pdb_writing.'+str(i)+'.pdb')
        os.unlink('test_pdb_writing.pdb')

    def test_pdb_from_rex(self):
        """Test PDB writing in PMI2 from replica exchange"""
        mdl = IMP.Model()
        pdb_file = self.get_input_file_name("mini.pdb")
        fasta_file = self.get_input_file_name("mini.fasta")

        seqs = IMP.pmi.topology.Sequences(fasta_file)
        s = IMP.pmi.topology.System(mdl)
        st = s.create_state()

        molA = st.create_molecule("P1",seqs[0],chain_id='A')
        aresA = molA.add_structure(pdb_file,chain_id='A',soft_check=True)
        molA.add_representation(aresA,[1])
        molA.add_representation(molA[:]-aresA,20)

        molB = st.create_molecule("P2",seqs[1],chain_id='B')
        aresB = molB.add_structure(pdb_file,chain_id='B',soft_check=True)
        molB.add_representation(aresB,[10])
        molB.add_representation(molB[:]-aresB,20)

        molC = molB.create_copy(chain_id='C')
        aresC = molC.add_structure(pdb_file,chain_id='B',soft_check=True)
        molC.add_representation(aresC,[1,10])
        molC.add_representation(molC[:]-aresC,2)
        root_hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        rb_movers = dof.create_rigid_body([molA,molB,molC],
                                          name="test RB")
        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=root_hier,
                                              monte_carlo_sample_objects = dof.get_movers(),
                                              number_of_frames=3,
                                              monte_carlo_steps=10,
                                              number_of_best_scoring_models=3,
                                              global_output_directory='pdb_test/',
                                              replica_exchange_object = rem)
        rex.execute_macro()
        for i in range(3):
            self.assertTrue(os.path.isfile('pdb_test/pdbs/model.%i.pdb'%i))
        mhtest = IMP.atom.read_pdb('pdb_test/pdbs/model.0.pdb',mdl)
        self.assertEqual(len(IMP.core.get_leaves(mhtest)),17)
        shutil.rmtree("pdb_test/")

    def test_pdb_multistate(self):
        """Test PDB writing in PMI2 from replica exchange"""
        mdl = IMP.Model()
        pdb_file = self.get_input_file_name("mini.pdb")
        fasta_file = self.get_input_file_name("mini.fasta")

        seqs = IMP.pmi.topology.Sequences(fasta_file)
        s = IMP.pmi.topology.System(mdl)
        st = s.create_state()

        molA = st.create_molecule("P1",seqs[0],chain_id='A')
        aresA = molA.add_structure(pdb_file,chain_id='A',soft_check=True)
        molA.add_representation(aresA,[0])

        st2 = s.create_state()
        molB = st2.create_molecule("P1",seqs[0],chain_id='A')
        aresB = molB.add_structure(pdb_file,chain_id='A',soft_check=True)
        molB.add_representation(molB,[1])

        root_hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        dof.create_rigid_body(molA,name="rbA")
        dof.create_rigid_body(molB,name="rbB")
        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=root_hier,
                                              monte_carlo_sample_objects = dof.get_movers(),
                                              number_of_frames=3,
                                              monte_carlo_steps=10,
                                              number_of_best_scoring_models=3,
                                              global_output_directory='pdb_test/',
                                              replica_exchange_object = rem)
        rex.execute_macro()
        for i in range(3):
            self.assertTrue(os.path.isfile('pdb_test/pdbs/0/model.%i.pdb'%i))
            self.assertTrue(os.path.isfile('pdb_test/pdbs/1/model.%i.pdb'%i))
        testA = IMP.atom.read_pdb('pdb_test/pdbs/0/model.0.pdb',mdl)
        self.assertEqual(len(IMP.core.get_leaves(testA)),53)
        testB = IMP.atom.read_pdb('pdb_test/pdbs/1/model.0.pdb',mdl)
        self.assertEqual(len(IMP.core.get_leaves(testB)),8)
        shutil.rmtree("pdb_test/")

if __name__ == '__main__':
    IMP.test.main()
