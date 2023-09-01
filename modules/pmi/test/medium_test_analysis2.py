import IMP
import IMP.pmi
import IMP.test
import RMF
import IMP.rmf
from math import sqrt
import itertools
import os
import glob
import shutil
import tempfile
nicemodules = True
try:
    import numpy as np
    import scipy
    import sklearn
    import IMP.pmi.analysis
    import IMP.pmi.io
    import IMP.pmi.dof
    import IMP.pmi.macros
    import IMP.pmi.restraints.basic
    import matplotlib
except ImportError:
    nicemodules = False

def get_drms(coords0,coords1):
    total = 0.0
    ct = 0
    ncoords = len(coords1)
    for p0 in range(ncoords):
        for p1 in range(p0+1,ncoords):
            d0 = sqrt(sum([(c1-c2)**2 for c1,c2 in zip(coords0[p0],coords0[p1])]))
            d1 = sqrt(sum([(c1-c2)**2 for c1,c2 in zip(coords1[p0],coords1[p1])]))
            total += (d0-d1)**2
            ct += 1
    return sqrt(total/ct)

def coords_from_rmf(filename,nframes,mdl,molecules):
    rh0 = RMF.open_rmf_file_read_only(filename)
    hier0 = IMP.rmf.create_hierarchies(rh0,mdl)[0]
    coords = []
    for i in range(nframes):
        IMP.rmf.load_frame(rh0,RMF.FrameID(i))
        sel = IMP.atom.Selection(hier0,resolution=1,molecules=molecules)
        coords.append([IMP.core.XYZ(p).get_coordinates() for p in sel.get_selected_particles()])
    del rh0
    return coords

def get_dist(cl0,cl1):
    dist = 0.0
    for x0,x1 in itertools.product(cl0,cl1):
        dist += get_drms(x0,x1)
    dist /= float(len(cl0)*len(cl1))
    return dist

class Tests(IMP.test.TestCase):
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
                                                        tuple_selection1=(1,1,"Prot1",0),
                                                        tuple_selection2=(1,1,"Prot2",0),
                                                        distancemin=5,
                                                        distancemax=15)
        dr.add_to_model()

        with IMP.allow_deprecated():
            rex = IMP.pmi.macros.ReplicaExchange(
                mdl, root_hier=hier, output_objects=[dr],
                monte_carlo_sample_objects=dof.get_movers(),
                number_of_frames=10, number_of_best_scoring_models=0,
                global_output_directory = \
                self.get_input_file_name("pmi2_sample_0/"))
        rex.execute_macro()

        trans = IMP.algebra.Transformation3D(IMP.algebra.Vector3D(10,10,10))
        for rb in dof.rigid_bodies:
            IMP.core.transform(rb,trans)

        with IMP.allow_deprecated():
            rex2 = IMP.pmi.macros.ReplicaExchange(
                mdl, root_hier=hier, output_objects=[dr],
                monte_carlo_sample_objects=dof.get_movers(),
                number_of_frames=10, number_of_best_scoring_models=0,
                global_output_directory = \
                self.get_input_file_name("pmi2_sample_1/"),
                replica_exchange_object=rex.get_replica_exchange_object())
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

        with IMP.allow_deprecated():
            rex = IMP.pmi.macros.ReplicaExchange(
                mdl, root_hier=hier, output_objects=[dr],
                monte_carlo_sample_objects=dof.get_movers(),
                number_of_frames=10, number_of_best_scoring_models=0,
                global_output_directory = \
                self.get_input_file_name("pmi2_sample_res5/"))
        rex.execute_macro()

    def init_with_copies(self,mdl):
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('seqs.fasta'),
                         name_map={'Protein_1':'Prot1',
                                   'Protein_2':'Prot2',
                                   'Protein_3':'Prot3'})

        m1 = st1.create_molecule("Prot1",chain_id='A',sequence=seqs["Prot1"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A',res_range=(55,64),offset=-54)
        m1.add_representation(a1,resolutions=[1])
        m1.add_representation(m1[:]-a1,resolutions=[1])
        m2 = m1.create_clone('B')
        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        mv1,rb1 = dof.create_rigid_body(m1,nonrigid_parts = m1.get_non_atomic_residues(),
                                        max_trans=0.000001)
        mv2,rb2 = dof.create_rigid_body(m2,nonrigid_parts = m2.get_non_atomic_residues(),
                                        max_trans=0.000001)

        trans = IMP.algebra.Transformation3D(IMP.algebra.Vector3D(100,0,0))
        IMP.core.transform(rb2,trans)

        ps0 = IMP.atom.Selection(hier,molecule='Prot1',copy_index=0).get_selected_particles()
        ps1 = IMP.atom.Selection(hier,molecule='Prot1',copy_index=1).get_selected_particles()

        with IMP.allow_deprecated():
            rex = IMP.pmi.macros.ReplicaExchange(
                mdl, root_hier=hier,
                monte_carlo_sample_objects=dof.get_movers(),
                number_of_frames=1, number_of_best_scoring_models=0,
                global_output_directory = \
                self.get_input_file_name("pmi2_copies_0/"))
        rex.execute_macro()

        coords0A = [IMP.core.XYZ(p).get_coordinates() for p in ps0]
        coords1A = [IMP.core.XYZ(p).get_coordinates() for p in ps1]

        # swap the objects
        IMP.core.transform(rb2,trans.get_inverse())
        IMP.core.transform(rb1,trans)

        with IMP.allow_deprecated():
            rex2 = IMP.pmi.macros.ReplicaExchange(
                mdl, root_hier=hier,
                monte_carlo_sample_objects=dof.get_movers(),
                number_of_frames=1, number_of_best_scoring_models=0,
                global_output_directory = \
                self.get_input_file_name("pmi2_copies_1/"),
                replica_exchange_object=rex.get_replica_exchange_object())
        rex2.execute_macro()

        coords0B = [IMP.core.XYZ(p).get_coordinates() for p in ps0]
        coords1B = [IMP.core.XYZ(p).get_coordinates() for p in ps1]

        rmsd = IMP.algebra.get_rmsd(coords0A+coords1A,coords1B+coords0B)
        return rmsd


    def test_get_model_density(self):
        """Test creation of density correctly averages frames for res=5"""
        if not nicemodules:
            self.skipTest("missing scipy or sklearn")

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

    def test_analysis_macro(self):
        """Test you can organize files correctly with macro"""
        if not nicemodules:
            self.skipTest("missing scipy or sklearn")

        mdl = IMP.Model()
        #self.init_topology(mdl)
        rmsd_names = {"Prot1":"Prot1",
                      "Prot2":"Prot2"}
        density_ranges = {"Prot1":["Prot1"],
                          "Prot2":["Prot2"]}
        with IMP.allow_deprecated():
            am = IMP.pmi.macros.AnalysisReplicaExchange0(
                mdl,
                merge_directories=[self.get_input_file_name("pmi2_sample_0/"),
                                   self.get_input_file_name("pmi2_sample_1/")],
                global_output_directory="./")

        with IMP.test.temporary_directory() as out_dir:
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
            rh0 = RMF.open_rmf_file_read_only(
                          self.get_input_file_name("pmi2_sample_0/rmfs/0.rmf3"))
            prot0 = IMP.rmf.create_hierarchies(rh0,mdl)[0]
            rh1 = RMF.open_rmf_file_read_only(
                          self.get_input_file_name("pmi2_sample_1/rmfs/0.rmf3"))
            prot1 = IMP.rmf.create_hierarchies(rh1,mdl)[0]

            icoords0 = []
            icoords1 = []
            p0 = IMP.atom.Selection(prot0,
                  atom_type=IMP.atom.AtomType("CA")).get_selected_particles()[0]
            p1 = IMP.atom.Selection(prot1,
                  atom_type=IMP.atom.AtomType("CA")).get_selected_particles()[0]
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
                mh0 = IMP.atom.read_pdb(os.path.join(cl0,str(i)+'.pdb'),
                                        mdl,IMP.atom.CAlphaPDBSelector())
                coords0.append(IMP.core.XYZ(IMP.core.get_leaves(mh0)[0]).get_coordinates())
                mh1 = IMP.atom.read_pdb(os.path.join(cl1,str(i)+'.pdb'),mdl,
                                        IMP.atom.CAlphaPDBSelector())
                coords1.append(IMP.core.XYZ(IMP.core.get_leaves(mh1)[0]).get_coordinates())

                sel_prot1 = IMP.atom.Selection(mh0,
                                        chain_id='A').get_selected_particles()
                sel_prot2 = IMP.atom.Selection(mh0,
                                        chain_id='B').get_selected_particles()
                c_prot01 += [IMP.core.XYZ(p).get_coordinates()
                             for p in sel_prot1]
                c_prot02 += [IMP.core.XYZ(p).get_coordinates()
                             for p in sel_prot2]

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
                n00 += min(IMP.algebra.get_distance(c0,i0)
                           for i0 in icoords0)<0.01
                n01 += min(IMP.algebra.get_distance(c0,i1)
                           for i1 in icoords1)<0.01
            for c1 in coords1:
                n10 += min(IMP.algebra.get_distance(c1,i0)
                           for i0 in icoords0)<0.01
                n11 += min(IMP.algebra.get_distance(c1,i1)
                           for i1 in icoords1)<0.01

            self.assertTrue((n00==10 and n11==10) or (n01==10 and n10==10))

            rhC = RMF.open_rmf_file_read_only(os.path.join(cl0,'0.rmf3'))
            protC = IMP.rmf.create_hierarchies(rhC,mdl)[0]
            # Provenance information should be attached to the state,
            # which is one level down (under the System)
            self.check_provenance(protC.get_children()[0])

            # check density
            bbox1 = IMP.algebra.BoundingBox3D(c_prot01)
            bbox2 = IMP.algebra.BoundingBox3D(c_prot02)
            dmap1 = IMP.em.read_map(os.path.join(cl0,'Prot1.mrc'))
            dmap2 = IMP.em.read_map(os.path.join(cl0,'Prot2.mrc'))
            self.assertTrue(IMP.em.get_bounding_box(dmap1).get_contains(bbox1))
            self.assertTrue(IMP.em.get_bounding_box(dmap2).get_contains(bbox2))

    def check_provenance(self, h):
        """Make sure that appropriate provenance information was added to
           the cluster"""
        prov = list(IMP.core.get_all_provenance(h))
        self.assertEqual(len(prov), 6)
        self.assertIsInstance(prov[0], IMP.core.ClusterProvenance)
        self.assertEqual(prov[0].get_number_of_members(), 10)

        self.assertIsInstance(prov[1], IMP.core.FilterProvenance)
        self.assertEqual(prov[1].get_method(), 'Best scoring')
        self.assertEqual(prov[1].get_number_of_frames(), 20)
        self.assertAlmostEqual(prov[1].get_threshold(), 0., delta=1e-6)

        self.assertIsInstance(prov[2], IMP.core.CombineProvenance)
        self.assertEqual(prov[2].get_number_of_frames(), 20)
        self.assertEqual(prov[2].get_number_of_runs(), 2)

        self.assertIsInstance(prov[3], IMP.core.ScriptProvenance)

        self.assertEqual(prov[4].get_software_name(), "IMP PMI module")
        self.assertEqual(prov[5].get_software_name(),
                         "Integrative Modeling Platform (IMP)")

    def test_precision(self):
        """Test correct calcluation of precision and RMSF"""
        if not nicemodules:
            self.skipTest("missing scipy or sklearn")

        selections = {"Prot1":["Prot1"],
                      "Prot2":["Prot2"],
                      "Prot1_Prot2":["Prot1","Prot2"]}
        nframes = 5
        mdl = IMP.Model()
        pr = IMP.pmi.analysis.Precision(mdl,resolution=1,selection_dictionary=selections)
        #pr.set_precision_style('pairwise_rmsd')
        f0 = self.get_input_file_name("pmi2_sample_0/rmfs/0.rmf3")
        f1 = self.get_input_file_name("pmi2_sample_1/rmfs/0.rmf3")
        tuple0 = zip([f0]*nframes,range(nframes))
        tuple1 = zip([f1]*nframes,range(nframes))
        pr.add_structures(tuple0,'set0')
        pr.add_structures(tuple1,'set1')

        fn00 = self.get_tmp_file_name('f00.txt')
        fn01 = self.get_tmp_file_name('f01.txt')
        fn11 = self.get_tmp_file_name('f11.txt')
        cn00 = pr.get_precision('set0','set0',fn00)
        pr.get_precision('set0','set1',fn01)
        pr.get_precision('set1','set1',fn11)

        # check precision manually
        coordslist0 = coords_from_rmf(f0,nframes,mdl,['Prot1','Prot2'])
        coordslist1 = coords_from_rmf(f1,nframes,mdl,['Prot1','Prot2'])

        dist00 = get_dist(coordslist0,coordslist0)
        dist01 = get_dist(coordslist0,coordslist1)
        dist11 = get_dist(coordslist1,coordslist1)
        with open(fn00,'r') as inf:
            pdist00 = float(inf.readlines()[-1].strip().split()[-1])
            self.assertAlmostEqual(dist00,pdist00,places=2)
        with open(fn01,'r') as inf:
            pdist01 = float(inf.readlines()[-1].strip().split()[-1])
            self.assertAlmostEqual(dist01,pdist01,places=2)
        with open(fn11,'r') as inf:
            pdist11 = float(inf.readlines()[-1].strip().split()[-1])
            self.assertAlmostEqual(dist11,pdist11,places=2)

        # check RMSF
        rmsf_dir = tempfile.mkdtemp()
        pr.get_rmsf('set0',outdir=rmsf_dir)
        mean1 = coordslist0[cn00][0]
        rmsf1 = np.std([IMP.algebra.get_distance(c[0],mean1) for c in coordslist0])
        with open(os.path.join(rmsf_dir,'rmsf.Prot1.dat'),'r') as inf:
            prmsf1 = float(inf.readline().split()[2])
            self.assertAlmostEqual(rmsf1,prmsf1)
        shutil.rmtree(rmsf_dir)

    def test_accuracy(self):
        """Test you can compare to reference structure"""
        if not nicemodules:
            self.skipTest("missing scipy or sklearn")

        selections = {"Prot1":["Prot1"],
                      "Prot2":["Prot2"],
                      "Prot1_Prot2":["Prot1","Prot2"]}
        nframes = 5
        mdl = IMP.Model()
        pr = IMP.pmi.analysis.Precision(mdl,resolution=1,selection_dictionary=selections)
        pr.set_precision_style('pairwise_rmsd')
        f0 = self.get_input_file_name("pmi2_sample_0/rmfs/0.rmf3")
        tuple0 = zip([f0]*nframes,range(nframes))
        pr.add_structures(tuple0,'set0')
        pr.set_reference_structure(f0,0)
        dists = pr.get_average_distance_wrt_reference_structure("set0")

        # check accuracy manually
        coordslist01 = coords_from_rmf(f0,nframes,mdl,['Prot1'])
        coordslist02 = coords_from_rmf(f0,nframes,mdl,['Prot2'])
        pdist01 = []
        pdist02 = []
        for i in range(nframes):
            rmsd01 = IMP.algebra.get_rmsd(coordslist01[i],coordslist01[0])
            pdist01.append(rmsd01)
            rmsd02 = IMP.algebra.get_rmsd(coordslist02[i],coordslist02[0])
            pdist02.append(rmsd02)
        self.assertAlmostEqual(dists['Prot1']['average_distance'],sum(pdist01)/nframes,places=2)
        self.assertAlmostEqual(dists['Prot2']['average_distance'],sum(pdist02)/nframes,places=2)

    def test_ambiguity(self):
        """Test clustering with copies"""
        if not nicemodules:
            self.skipTest("missing scipy or sklearn")

        mdl = IMP.Model()
        #expected_rmsd = self.init_with_copies(mdl)
        alignment_comp = {"Prot1":"Prot1"}
        rmsd_comp = {"Prot1":"Prot1"}

        # the code should expand into:
        #alignment_comp = {"Prot1..0":(1,-1,"Prot1",0),"Prot1..1":(1,-1,"Prot1",1)}
        #rmsd_comp = {"Prot1..0":(1,-1,"Prot1",0),"Prot1..1":(1,-1,"Prot1",1)}

        with IMP.allow_deprecated():
            am = IMP.pmi.macros.AnalysisReplicaExchange0(
                mdl,
                merge_directories=[self.get_input_file_name("pmi2_copies_0/"),
                                   self.get_input_file_name("pmi2_copies_1/")],
                global_output_directory="./")

        with IMP.test.temporary_directory() as out_dir:
            am.clustering(feature_keys=["Total_Score"],
                          alignment_components=alignment_comp,
                          rmsd_calculation_components=rmsd_comp,
                          number_of_clusters=1,
                          number_of_best_scoring_models=20,
                          outputdir=out_dir)

            # check average RMSD
            av_rmsd = am.get_cluster_rmsd(0)
            self.assertLess(av_rmsd,1.5) # if swapped it'll be 100

if __name__ == '__main__':
    IMP.test.main()
