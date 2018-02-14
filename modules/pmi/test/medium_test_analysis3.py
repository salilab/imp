import IMP
import IMP.pmi
import IMP.pmi.macros
import IMP.test
import glob

class Tests(IMP.test.TestCase):

    def test_analysis_replica_exchange(self):
        try:
            import matplotlib
        except ImportError:
            self.skipTest("no matplotlib package")
        model=IMP.Model()
        sts=sorted(glob.glob(self.get_input_file_name("output_test/stat.0.out").replace(".0.",".*.")))
        are=IMP.pmi.macros.AnalysisReplicaExchange(model,sts,10)
        are.set_alignment_selection(molecule="Rpb4")
        are.save_data()
        are.cluster(20)
        self.assertEqual(len(are),4)
        print(are)
        are.refine(40)
        print(are)
        self.assertEqual(len(are),2)

        dcr={"Rpb4":["Rpb4"],"Rpb7":["Rpb7"],"All":["Rpb4","Rpb7"]}



        for cluster in are:
            are.save_coordinates(cluster)
            are.save_densities(cluster,dcr,prefix="densities_out/")
            are.compute_cluster_center(cluster)
            are.precision(cluster)

            for mol in ["Rpb4","Rpb7"]:
                rmsf=are.rmsf(cluster,mol)
                rs=[]
                rmsfs=[]
                for r in rmsf:
                    rs.append(r)
                    rmsfs.append(rmsf[r])
                IMP.pmi.output.plot_xy_data(rs,rmsfs,out_fn=mol+"."+str(cluster.cluster_id)+".rmsf.pdf")

            print(cluster)
            #for member in cluster:
            #    print(member)
            are.contact_map(cluster)

        for c1 in are:
            for c2 in are:
                print(c1.cluster_id,c2.cluster_id,are.bipartite_precision(c1,c2))

        are.apply_molecular_assignments(1)
        are.save_clusters()


if __name__ == '__main__':
    IMP.test.main()
