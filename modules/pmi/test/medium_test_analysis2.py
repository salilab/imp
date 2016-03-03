import IMP
import IMP.pmi
import IMP.test
import RMF
import IMP.rmf
from math import sqrt
import itertools
import os
import glob
try:
    import scipy
except ImportError:
    scipy = None
if scipy is not None:
    import IMP.pmi.analysis
    import IMP.pmi.io
    import IMP.pmi.dof
    import IMP.pmi.macros
    import IMP.pmi.restraints.basic

class AnalysisTest(IMP.test.TestCase):
    def init_topology(self,mdl):
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
        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        dof.create_rigid_body(m1,nonrigid_parts = m1.get_non_atomic_residues(),
                              max_trans=0.1)
        dof.create_rigid_body(m2,nonrigid_parts = m2.get_non_atomic_residues(),
                              max_trans=0.1)

        dr = IMP.pmi.restraints.basic.DistanceRestraint(root_hier = hier,
                                                        tuple_selection1=("Prot1",1,1),
                                                        tuple_selection2=("Prot2",1,1),
                                                        distancemin=5,
                                                        distancemax=15)
        dr.add_to_model()

        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=hier,
                                              output_objects=[dr],
                                              monte_carlo_sample_objects=dof.get_movers(),
                                              number_of_frames=10,
                                              number_of_best_scoring_models=0,
                                              global_output_directory = \
                                              self.get_input_file_name("pmi2_sample_0/"))
        rex.execute_macro()

        trans = IMP.algebra.Transformation3D(IMP.algebra.Vector3D(10,10,10))
        for rb in dof.rigid_bodies:
            IMP.core.transform(rb,trans)

        rex2 = IMP.pmi.macros.ReplicaExchange0(mdl,
                                               root_hier=hier,
                                               output_objects=[dr],
                                               monte_carlo_sample_objects=dof.get_movers(),
                                               number_of_frames=10,
                                               number_of_best_scoring_models=0,
                                               global_output_directory = \
                                               self.get_input_file_name("pmi2_sample_1/"),
                                               replica_exchange_object = rex.get_replica_exchange_object())
        rex2.execute_macro()

    def init_res5(self,mdl):
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
        m1.add_representation(m1,resolutions=[5])
        m2.add_representation(m2,resolutions=[5])
        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        dof.create_rigid_body(m1,nonrigid_parts = m1.get_non_atomic_residues(),
                              max_trans=0.1)
        dof.create_rigid_body(m2,nonrigid_parts = m2.get_non_atomic_residues(),
                              max_trans=0.1)

        dr = IMP.pmi.restraints.basic.DistanceRestraint(root_hier = hier,
                                                        tuple_selection1=("Prot1",1,1),
                                                        tuple_selection2=("Prot2",1,1),
                                                        distancemin=5,
                                                        distancemax=15)
        dr.add_to_model()

        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=hier,
                                              output_objects=[dr],
                                              monte_carlo_sample_objects=dof.get_movers(),
                                              number_of_frames=10,
                                              number_of_best_scoring_models=0,
                                              global_output_directory = \
                                              self.get_input_file_name("pmi2_sample_res5/"))
        rex.execute_macro()
    def test_get_model_density(self):
        """Test creation of density correctly averages frames for res=5"""
        mdl = IMP.Model()
        #self.init_res5(mdl)
        density_ranges = {"Prot1":["Prot1"],
                          "Prot2":["Prot2"]}
        mdens = IMP.pmi.analysis.GetModelDensity(density_ranges)
        rmf_file = self.get_input_file_name('pmi2_sample_res5/rmfs/0.rmf3')
        rh = RMF.open_rmf_file_read_only(rmf_file)
        hier = IMP.rmf.create_hierarchies(rh,mdl)[0]
        sel1 = IMP.atom.Selection(hier,molecule='Prot1').get_selected_particles()

        IMP.rmf.load_frame(rh,RMF.FrameID(0))
        mdens.add_subunits_density(hier)
        self.assertEqual(sorted(mdens.get_density_keys()),['Prot1','Prot2'])

        sel1_coords=[]
        sel2_coords=[]
        for i in range(4):
            IMP.rmf.load_frame(rh,RMF.FrameID(i))
            sel1 = IMP.atom.Selection(hier,molecule='Prot1').get_selected_particles()
            sel1_coords += [IMP.core.XYZ(p).get_coordinates() for p in sel1]
            sel2 = IMP.atom.Selection(hier,molecule='Prot2').get_selected_particles()
            sel2_coords += [IMP.core.XYZ(p).get_coordinates() for p in sel2]
            mdens.add_subunits_density(hier)

        bbox1=IMP.algebra.BoundingBox3D(sel1_coords)
        bbox2=IMP.algebra.BoundingBox3D(sel2_coords)
        self.assertTrue(IMP.em.get_bounding_box(mdens.get_density('Prot1')).get_contains(bbox1))
        self.assertTrue(IMP.em.get_bounding_box(mdens.get_density('Prot2')).get_contains(bbox2))

    def test_clustering(self):
        """Test create correct clusters"""

    def test_precision(self):
        """Test correct calcluation of precision"""

    def test_analysis_macro(self):
        """Test you can organize files correctly with macro"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")

        mdl = IMP.Model()
        #self.init_topology(mdl)
        rmsd_names = {"Prot1":"Prot1",
                      "Prot2":"Prot2"}
        density_ranges = {"Prot1":["Prot1"],
                          "Prot2":["Prot2"]}
        am = IMP.pmi.macros.AnalysisReplicaExchange0(
            mdl,
            merge_directories=[self.get_input_file_name("pmi2_sample_0/"),
                               self.get_input_file_name("pmi2_sample_1/")],
            global_output_directory="./")

        out_dir = IMP.test.TempDir(None).tmpdir
        am.clustering(score_key="Total_Score",
                      rmsd_calculation_components=rmsd_names,
                      number_of_clusters=2,
                      number_of_best_scoring_models=20,
                      outputdir=out_dir,
                      density_custom_ranges=density_ranges)

        cl0 = os.path.join(out_dir,'cluster.0')
        cl1 = os.path.join(out_dir,'cluster.1')
        self.assertEqual(len(glob.glob(os.path.join(cl0,'*.pdb'))),10)
        self.assertEqual(len(glob.glob(os.path.join(cl1,'*.pdb'))),10)

        # get initial coords
        rh0 = RMF.open_rmf_file_read_only(self.get_input_file_name("pmi2_sample_0/rmfs/0.rmf3"))
        prot0 = IMP.rmf.create_hierarchies(rh0,mdl)[0]
        rh1 = RMF.open_rmf_file_read_only(self.get_input_file_name("pmi2_sample_1/rmfs/0.rmf3"))
        prot1 = IMP.rmf.create_hierarchies(rh1,mdl)[0]

        icoords0 = []
        icoords1 = []
        p0 = IMP.atom.Selection(prot0,atom_type=IMP.atom.AtomType("CA")).get_selected_particles()[0]
        p1 = IMP.atom.Selection(prot1,atom_type=IMP.atom.AtomType("CA")).get_selected_particles()[0]
        for i in range(10):
            IMP.rmf.load_frame(rh0,i)
            IMP.rmf.load_frame(rh1,i)
            icoords0.append(IMP.core.XYZ(p0).get_coordinates())
            icoords1.append(IMP.core.XYZ(p1).get_coordinates())

        # get clustered coords
        coords0 = [] # all first coordinates in cluster 0
        coords1 = [] # all first coordinates in cluster 1
        c_prot01 = [] # all coords for cluster 0, protein 1
        c_prot02 = [] # all coords for cluster 0, protein 2
        for i in range(10):
            mh0 = IMP.atom.read_pdb(os.path.join(cl0,str(i)+'.pdb'),mdl,IMP.atom.CAlphaPDBSelector())
            coords0.append(IMP.core.XYZ(IMP.core.get_leaves(mh0)[0]).get_coordinates())
            mh1 = IMP.atom.read_pdb(os.path.join(cl1,str(i)+'.pdb'),mdl,IMP.atom.CAlphaPDBSelector())
            coords1.append(IMP.core.XYZ(IMP.core.get_leaves(mh1)[0]).get_coordinates())

            sel_prot1 = IMP.atom.Selection(mh0,chain_id='A').get_selected_particles()
            sel_prot2 = IMP.atom.Selection(mh0,chain_id='B').get_selected_particles()
            c_prot01 += [IMP.core.XYZ(p).get_coordinates() for p in sel_prot1]
            c_prot02 += [IMP.core.XYZ(p).get_coordinates() for p in sel_prot2]

            IMP.atom.destroy(mh0)
            del mh0
            IMP.atom.destroy(mh1)
            del mh1

        # dumb check for at least one close distance
        n00 = 0
        n01 = 0
        n10 = 0
        n11 = 0
        for c0 in coords0:
            n00 += min(IMP.algebra.get_distance(c0,i0) for i0 in icoords0)<0.01
            n01 += min(IMP.algebra.get_distance(c0,i1) for i1 in icoords1)<0.01
        for c1 in coords1:
            n10 += min(IMP.algebra.get_distance(c1,i0) for i0 in icoords0)<0.01
            n11 += min(IMP.algebra.get_distance(c1,i1) for i1 in icoords1)<0.01

        self.assertTrue((n00==10 and n11==10) or (n01==10 and n10==10))

        # check density
        bbox1 = IMP.algebra.BoundingBox3D(c_prot01)
        bbox2 = IMP.algebra.BoundingBox3D(c_prot02)
        dmap1 = IMP.em.read_map(os.path.join(cl0,'Prot1.mrc'))
        dmap2 = IMP.em.read_map(os.path.join(cl0,'Prot2.mrc'))
        self.assertTrue(IMP.em.get_bounding_box(dmap1).get_contains(bbox1))
        self.assertTrue(IMP.em.get_bounding_box(dmap2).get_contains(bbox2))

if __name__ == '__main__':
    IMP.test.main()
