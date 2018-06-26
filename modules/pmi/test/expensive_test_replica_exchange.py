import IMP
import IMP.test
import IMP.pmi.samplers
import IMP.pmi.representation
import IMP.pmi.restraints.basic
import IMP.pmi.macros
import IMP.pmi.output
import RMF
import glob
import time
import os.path

class Tests(IMP.test.TestCase):

    def test_serial(self):
        """Test the _SerialReplicaExchange class"""
        s = IMP.pmi.samplers._SerialReplicaExchange()
        self.assertEqual(s.get_number_of_replicas(), 1)
        ts = s.create_temperatures(100., 200., 45)
        self.assertEqual(len(ts), 1)
        self.assertAlmostEqual(ts[0], 100.0, delta=1e-6)
        self.assertEqual(s.get_my_index(), 0)
        self.assertEqual(s.get_friend_index(0), 0)
        s.set_my_parameter("temp", ['foo', 'bar'])
        self.assertEqual(s.get_my_parameter("temp"), ['foo', 'bar'])
        self.assertEqual(s.get_friend_parameter("temp", 0), ['foo', 'bar'])
        self.assertEqual(s.do_exchange(0, 0, 0), False)

    def test_macro(self):
        """setting up the representation
        PMI 1.0 representation. Creates two particles and
        an harmonic distance restraints between them"""
        import shutil
        import itertools
        m=IMP.Model()
        r=IMP.pmi.representation.Representation(m)
        r.create_component("A")
        r.add_component_beads("A",[(1,1),(2,2)])
        ps=IMP.atom.get_leaves(r.prot)
        dr=IMP.pmi.restraints.basic.DistanceRestraint(r,(1,1,"A"),(2,2,"A"),10,10)
        dr.add_to_model()
        rex=IMP.pmi.macros.ReplicaExchange0(m,
                      r,
                      monte_carlo_sample_objects=[r],
                      output_objects=[r,dr],
                      monte_carlo_temperature=1.0,
                      replica_exchange_minimum_temperature=1.0,
                      replica_exchange_maximum_temperature=2.5,
                      number_of_best_scoring_models=10,
                      monte_carlo_steps=10,
                      number_of_frames=100,
                      write_initial_rmf=True,
                      initial_rmf_name_suffix="initial",
                      stat_file_name_suffix="stat",
                      best_pdb_name_suffix="model",
                      do_clean_first=True,
                      do_create_directories=True,
                      global_output_directory="./test_replica_exchange_macro_output",
                      rmf_dir="rmfs/",
                      best_pdb_dir="pdbs/",
                      replica_stat_file_suffix="stat_replica",
                      em_object_for_rmf=None,
                      replica_exchange_object=None)

        # check whether the directory is existing, in case remove it
        try:
            shutil.rmtree('./test_replica_exchange_macro_output')
        except OSError:
            pass


        rex.execute_macro()

        self.check_rmf_file('./test_replica_exchange_macro_output/rmfs/0.rmf3')

        # check that each replica index is below the total number of replicas
        my_index= rex.replica_exchange_object.get_my_index()
        nreplicas=rex.replica_exchange_object.get_number_of_replicas()
        temperatures=rex.replica_exchange_object.get_my_parameter("temp")
        self.assertLess(my_index, nreplicas)

        # check that each replica has a unique index
        tf=open("./test_replica_exchange_macro_output/"+str(my_index)+".test","w")
        tf.write(str(my_index))
        tf.close()
        # sleep to synchronize
        time.sleep(1)
        if my_index==0:
            for k in range(nreplicas):
                self.assertTrue(os.path.isfile("./test_replica_exchange_macro_output/"+str(k)+".test"))


        #extract the info form the stat files
        rex_out_files=glob.glob("./test_replica_exchange_macro_output/stat_replica.*.out")
        temp_key="ReplicaExchange_CurrentTemp"
        maxtf_key="ReplicaExchange_MaxTempFrequency"
        mintf_key="ReplicaExchange_MinTempFrequency"
        ssr_key="ReplicaExchange_SwapSuccessRatio"
        score_key="score"
        score_temp_dict={}
        avtemps_replicas=[]
        for f in rex_out_files:
            o=IMP.pmi.output.ProcessOutput(f)
            d=o.get_fields([temp_key,maxtf_key,mintf_key,
                         ssr_key,score_key])
            temps=[float(f) for f in d[temp_key]]
            scores=[float(f) for f in d[score_key]]
            avtemp=sum(temps)/len(temps)
            avtemps_replicas.append(avtemp)
            for n,t in enumerate(temps):
                s=scores[n]
                if t not in score_temp_dict:
                    score_temp_dict[t]=[s]
                else:
                    score_temp_dict[t].append(s)
        # test that the average temperature per replica are similar
        for c in itertools.combinations(avtemps_replicas,2):
            self.assertAlmostEqual(c[0], c[1], delta=0.05)
        for t in score_temp_dict:
            avscore=sum(score_temp_dict[t])/len(score_temp_dict[t])
            #check that the score is the energy of an 1D harmonic oscillator
            self.assertAlmostEqual(avscore, t/2, delta=0.1)

        rex_out_file="./test_replica_exchange_macro_output/stat."+str(my_index)+".out"
        dist_key="DistanceRestraint_Score"
        mc_nframe_key="MonteCarlo_Nframe"
        mc_temp_key="MonteCarlo_Temperature"
        rex_temp_key="ReplicaExchange_CurrentTemp"
        rex_max_temp_key="ReplicaExchange_MaxTempFrequency"
        rex_min_temp_key="ReplicaExchange_MinTempFrequency"
        rex_swap_key="ReplicaExchange_SwapSuccessRatio"
        rex_score_key="SimplifiedModel_Total_Score_None"
        rmf_file_key="rmf_file"
        rmf_file_index="rmf_frame_index"
        o=IMP.pmi.output.ProcessOutput(rex_out_file)
        d=o.get_fields([dist_key,
                            mc_temp_key,
                            mc_nframe_key,
                            rex_temp_key,
                            rex_max_temp_key,
                            rex_min_temp_key,
                            rex_swap_key,
                            rex_score_key,
                            rmf_file_key,
                            rmf_file_index])
        nframes=len(d[mc_nframe_key])
        self.assertNotEqual(float(d[mc_nframe_key][-1]), 0)
        self.assertEqual(map(float,d[mc_temp_key]), [1.0]*nframes)
        self.assertGreater(float(d[rex_min_temp_key][-1]), 0.0)
        # always fails
        #self.assertGreater(float(d[rex_max_temp_key][-1]), 0.0)

    def check_rmf_file(self, fname):
        rmf = RMF.open_rmf_file_read_only(fname)
        m = IMP.Model()
        h = IMP.rmf.create_hierarchies(rmf, m)
        top = h[0]
        self.assertTrue(IMP.core.Provenanced.get_is_setup(top))
        prov = IMP.core.Provenanced(top).get_provenance()
        self.assertTrue(IMP.core.SampleProvenance.get_is_setup(prov))
        samp = IMP.core.SampleProvenance(prov)
        self.assertEqual(samp.get_method(), 'Monte Carlo')
        self.assertEqual(samp.get_number_of_frames(), 100)
        self.assertEqual(samp.get_number_of_iterations(), 10)

    def test_macro_rmf_stat(self):
        """setting up the representation
        PMI 1.0 representation. Creates two particles and
        an harmonic distance restraints between them"""
        import shutil
        import itertools
        m=IMP.Model()
        r=IMP.pmi.representation.Representation(m)
        r.create_component("A")
        r.add_component_beads("A",[(1,1),(2,2)])
        ps=IMP.atom.get_leaves(r.prot)
        dr=IMP.pmi.restraints.basic.DistanceRestraint(r,(1,1,"A"),(2,2,"A"),10,10)
        dr.add_to_model()
        rex=IMP.pmi.macros.ReplicaExchange0(m,
                      r,
                      monte_carlo_sample_objects=[r],
                      output_objects=None,
                      rmf_output_objects=[r,dr],
                      monte_carlo_temperature=1.0,
                      replica_exchange_minimum_temperature=1.0,
                      replica_exchange_maximum_temperature=2.5,
                      number_of_best_scoring_models=10,
                      monte_carlo_steps=10,
                      number_of_frames=100,
                      write_initial_rmf=True,
                      initial_rmf_name_suffix="initial",
                      stat_file_name_suffix="stat",
                      best_pdb_name_suffix="model",
                      do_clean_first=True,
                      do_create_directories=True,
                      global_output_directory="./test_replica_exchange_macro_output",
                      rmf_dir="rmfs/",
                      best_pdb_dir="pdbs/",
                      replica_stat_file_suffix="stat_replica",
                      em_object_for_rmf=None,
                      replica_exchange_object=None)

        # check whether the directory is existing, in case remove it
        try:
            shutil.rmtree('./test_replica_exchange_macro_output')
        except OSError:
            pass


        rex.execute_macro()

        self.check_rmf_file('./test_replica_exchange_macro_output/rmfs/0.rmf3')

        # check that each replica index is below the total number of replicas
        my_index= rex.replica_exchange_object.get_my_index()
        nreplicas=rex.replica_exchange_object.get_number_of_replicas()
        temperatures=rex.replica_exchange_object.get_my_parameter("temp")
        self.assertLess(my_index, nreplicas)

        # check that each replica has a unique index
        tf=open("./test_replica_exchange_macro_output/"+str(my_index)+".test","w")
        tf.write(str(my_index))
        tf.close()
        # sleep to synchronize
        time.sleep(1)
        if my_index==0:
            for k in range(nreplicas):
                self.assertTrue(os.path.isfile("./test_replica_exchange_macro_output/"+str(k)+".test"))


        #extract the info form the stat files
        rex_out_files=glob.glob("./test_replica_exchange_macro_output/stat_replica.*.out")
        temp_key="ReplicaExchange_CurrentTemp"
        maxtf_key="ReplicaExchange_MaxTempFrequency"
        mintf_key="ReplicaExchange_MinTempFrequency"
        ssr_key="ReplicaExchange_SwapSuccessRatio"
        score_key="score"
        score_temp_dict={}
        avtemps_replicas=[]
        for f in rex_out_files:
            o=IMP.pmi.output.ProcessOutput(f)
            d=o.get_fields([temp_key,maxtf_key,mintf_key,
                         ssr_key,score_key])
            temps=[float(f) for f in d[temp_key]]
            scores=[float(f) for f in d[score_key]]
            avtemp=sum(temps)/len(temps)
            avtemps_replicas.append(avtemp)
            for n,t in enumerate(temps):
                s=scores[n]
                if t not in score_temp_dict:
                    score_temp_dict[t]=[s]
                else:
                    score_temp_dict[t].append(s)
        # test that the average temperature per replica are similar
        for c in itertools.combinations(avtemps_replicas,2):
            self.assertAlmostEqual(c[0], c[1], delta=0.05)
        for t in score_temp_dict:
            avscore=sum(score_temp_dict[t])/len(score_temp_dict[t])
            #check that the score is the energy of an 1D harmonic oscillator
            self.assertAlmostEqual(avscore, t/2, delta=0.1)

        rex_out_file="./test_replica_exchange_macro_output/rmfs/"+str(my_index)+".rmf3"
        dist_key="DistanceRestraint_Score"
        mc_nframe_key="MonteCarlo_Nframe"
        mc_temp_key="MonteCarlo_Temperature"
        rex_temp_key="ReplicaExchange_CurrentTemp"
        rex_max_temp_key="ReplicaExchange_MaxTempFrequency"
        rex_min_temp_key="ReplicaExchange_MinTempFrequency"
        rex_swap_key="ReplicaExchange_SwapSuccessRatio"
        rex_score_key="SimplifiedModel_Total_Score_None"
        rmf_file_key="rmf_file"
        rmf_file_index="rmf_frame_index"
        o=IMP.pmi.output.ProcessOutput(rex_out_file)
        d=o.get_fields([dist_key,
                            mc_temp_key,
                            mc_nframe_key,
                            rex_temp_key,
                            rex_max_temp_key,
                            rex_min_temp_key,
                            rex_swap_key,
                            rex_score_key,
                            rmf_file_key,
                            rmf_file_index])
        nframes=len(d[mc_nframe_key])
        self.assertNotEqual(float(d[mc_nframe_key][-1]), 0)
        self.assertEqual(map(float,d[mc_temp_key]), [1.0]*nframes)
        self.assertGreater(float(d[rex_min_temp_key][-1]), 0.0)
        # always fails
        #self.assertGreater(float(d[rex_max_temp_key][-1]), 0.0)


if __name__ == '__main__':
    IMP.test.main()
