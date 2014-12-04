Stage 4 - Analysis Part 2 {#rnapolii_4_2}
=========================

## Post-clustering analysis
In this stage we perform post-clustering analysis, determining the within-group precision and between-group similarity. These metrics can provide evidence of the accuracy of the clustering - meaning, to what degree the clusters truly represent the most likely set of configurations given the limited information used in the scoring functions.

### Determining cluster precision (precision_rmsf.py)

The `precision_rmsf.py` script can be used to determine the within- and between-cluster RMSD (i.e., precision). To run, use
\code{.sh}
python precision_rmsf.py
\endcode
It will generate `precision.*.*.out` files in the `kmeans*` subdirectory containing precision information in text format, while in each cluster directory it generates `.pdf` files showing the within-cluster residue mean square fluctuation. In a similar way to earlier scripts, subsets of the structure can be selected for the calculation - in this case, we select the Rpb4 and Rpb7 subunits.

### Accuracy evaluation (accuracy.py)
We have provided a script to evaluate the accuracy of a model against a native configuration. When run, it will enumerate the structures in the first cluster and print the average and minimum distance between those structures and a given reference model. This is useful for benchmarking (but obviously is of no use when we don't know the 'real' structure).

To run, use
\code{.sh}
python accuracy.py
\endcode

### Determining sampling exhaustiveness
We can also determine sampling exhaustiveness by dividing the models into multiple sets, performing clustering on each set separately, and comparing the clusters. This step is left as an exercise to the reader. To aide with splitting the data, we have added the optional keyword `first_and_last_frames` in the `clustering()` function of the 
[AnalysisReplicaExchange0](https://integrativemodeling.org/2.3.1/doc/html/classIMP_1_1pmi_1_1macros_1_1AnalysisReplicaExchange0.html) macro. If you set this keyword to a tuple (values are percentages, e.g. [0,0.5]), it will only analyze that fraction of the data. Some things you can try:
* cluster two subsets of the data
* qualitative analysis: look at the localization densities - they should be similar for the two subsets
* quantitative analysis: combine the cluster results into one folder (rename as needed) and call `precision_rmsf.py`, which will automatically compute cross-precision for the clusters. 

If the sampling is exhaustive, then similar clusters should be obtained from each independent set, and the inter-cluster precision between two equivalent clusters should be very low (that is, there should be a 1:1 correspondence between the two sets of clusters, though the ordering may be different).
