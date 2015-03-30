Stage 4 - Analysis Part 2 {#rnapolii_4_2}
=========================

## Post-clustering analysis
In this stage we perform post-clustering analysis.  Here, we will perform calculations for:

* **Cluster Precision**: Determining the within-group precision and between-group similarity via RMSD
* **Cluster Accuracy**: Fit of the calculated clusters to the true (known) solution
* **Sampling Exhaustiveness**: Qualitative and quantitative measurement of sampling completeness

### Cluster Precision (precision_rmsf.py)

The `precision_rmsf.py` script can be used to determine the within- and between-cluster RMSD (i.e., precision). To run, use:

\code{.sh}
python precision_rmsf.py
\endcode
It will generate `precision.*.*.out` files in the `kmeans*` subdirectory containing precision information in text format, while in each cluster directory it generates `.pdf` files showing the within-cluster residue mean square fluctuation. In a similar way to earlier scripts, subsets of the structure can be selected for the calculation - in this case, we select the Rpb4 and Rpb7 subunits.

\code{.py}
# choose components for the precision calculation
# key is the named precision item
# value is a list of selection tuples [either "domain_name" or (start,stop,"domain_name") ]
selections={"Rpb4":["Rpb4"],
            "Rpb7":["Rpb7"],
            "Rpb4_Rpb7":["Rpb4","Rpb7"]}
\endcode

The script then sets up a model and [Precision](@ref IMP::pmi::analysis::Precision) object for the given `selections` at the desired resolution for computation of the precision (`resolution=1` specifies at the residue level).

\code{.py}
# setup Precision calculator
model = IMP.Model()
pr = IMP.pmi.analysis.Precision(model,resolution=1,selection_dictionary=selections)
pr.set_precision_style('pairwise_rmsd')
\endcode

Next, lists of structures are created that will be passed to the `Precision` object. `rmf_list` references the specific `.rmf` file, which with those `frame_list` is used to reference a particular frame in that `.rmf` to use (in this case, the only frame in the rmf, 0).

\code{.py}
# gather the RMF filenames for each cluster
rmf_list=[]
frame_list=[]
cluster_dirs=glob.glob(root_cluster_directory+'/cluster.*/')
if test_mode:
  # runs on the first 10 structures to test if it runs smoothly
  for d in cluster_dirs:
      rmf_list.append(glob.glob(d+'/*.rmf3')[0::10])
      frame_list.append([0]*len(rmf_list[-1]))
else:
  for d in cluster_dirs:
      rmf_list.append(glob.glob(d+'/*.rmf3'))
      frame_list.append([0]*len(rmf_list[-1]))
\endcode

The list of frames and rmfs are added to the precision object

\code{.py}
# add them to the Precision object
for rmfs,frames,cdir in zip(rmf_list,frame_list,cluster_dirs):
    pr.add_structures(zip(rmfs,frames),cdir)
\endcode

Self-precision and inter-cluster precision is then calculated, using the `rmf_list` and the output is placed in `root_cluster_directory`.

\code{.py}
# calculate intra-cluster and inter-cluster precision
print("calculating precision")
for clus1,clus2 in combinations_with_replacement(range(len(rmf_list)),2):
    pr.get_precision(cluster_dirs[clus1],
                     cluster_dirs[clus2],
                     root_cluster_directory+"/precision."+str(clus1)+"."+str(clus2)+".out")
\endcode

Finally, the RMSFs for each residue in the analyzed components are calculated and stored in `root_cluster_directory/rmsf.COMPONENT_NAME.dat`

\code{.py}
# compute residue mean-square fluctuation (RMSF)
print("calculating RMSF")
for d in cluster_dirs:
    pr.get_rmsf(structure_set_name=d,outdir=d)
\endcode

<img src="rnapolii_rmsf.Rpb7.png" width="500px" alt="residue root mean square fluctuations calculated on a cluster of structures" />

### Accuracy evaluation (accuracy.py)
We have provided a script to evaluate the accuracy of a model against a native configuration. When run, it will enumerate the structures in the first cluster and print the average and minimum distance between those structures and a given reference model. This is useful for benchmarking (but obviously is of no use when we don't know the 'real' structure).

To run, use

\code{.sh}
python accuracy.py
\endcode

First, identify the reference structure and list of `.rmf` structures to use in calculation

\code{.py}
# common settings
reference_rmf = "../data/native.rmf3"
test_mode = False                             # run on every 10 rmf files
rmfs = glob.glob('kmeans_*_1/cluster.0/*.rmf3') # list of the RMFS to calculate on
\endcode

The components that will be compared to reference must be explicitly enumerated in `selections`

\code{.py}
selections = {"Rpb4":["Rpb4"],
              "Rpb7":["Rpb7"],
              "Rpb4_Rpb7":["Rpb4","Rpb7"]}
\endcode

Initialize an %IMP `Model` and `Precision` class object and add the `selections`.  Then add the list of `.rmf` structures 

\code{.py}
# setup Precision calculator
model=IMP.Model()
frames=[0]*len(rmfs)
pr=IMP.pmi.analysis.Precision(model,selection_dictionary=selections)
pr.set_precision_style('pairwise_rmsd')
pr.add_structures(zip(rmfs,frames),"ALL")
\endcode

Average to the reference structure in angstroms for each component in `selections` is then calculated and outputted to the screen.

\code{.py}
# calculate average distance to the reference file
pr.set_reference_structure(reference_rmf,0)
print(pr.get_average_distance_wrt_reference_structure("ALL"))
\endcode

The output of this analysis will be printed in the terminal. For instance,
you will get something like:

    Rpb4 average distance 20.7402052384 minimum distance 11.9324734377
    All average distance 5.05387877292 minimum distance 3.4664144466
    Rpb7 average distance 10.5032807663 minimum distance 5.06599370365
    Rpb4_Rpb7 average distance 16.0757238511 minimum distance 9.63785403195

The average distance is the average RMSD of each model in the cluster with respect to the reference structure. The program prints values for all selections (`Rpb4`, `Rpb7` and `Rpb4_Rpb7`) and automatically for all the complex (`All`)

### Sampling Exhaustiveness
We can also determine sampling exhaustiveness by dividing the models into multiple sets, performing clustering on each set separately, and comparing the clusters. This step is left as an exercise to the reader. To aid with splitting the data, we have added the optional keyword `first_and_last_frames` to the IMP::pmi::macros::AnalysisReplicaExchange0::clustering() method.
If you set this keyword to a tuple (values are percentages, e.g. [0,0.5]), it will only analyze that fraction of the data. Some things you can try:
* cluster two subsets of the data
* qualitative analysis: look at the localization densities - they should be similar for the two subsets
* quantitative analysis: combine the cluster results into one folder (rename as needed) and call `precision_rmsf.py`, which will automatically compute cross-precision for the clusters. 

If the sampling is exhaustive, then similar clusters should be obtained from each independent set, and the inter-cluster precision between two equivalent clusters should be very low (that is, there should be a 1:1 correspondence between the two sets of clusters, though the ordering may be different).
