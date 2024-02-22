# import relevant modules
import sys
import IMP.spatiotemporal as spatiotemporal
import os

# Example of model without temporal scoring -----------------------------------------------------------------

# Input variables. 
# dict is a the state dictionary, which returns the number of states at each time point, for each time point.
# Input is the directory where the data is stored. 
# Output is the directory to which the output will be written
dict={'0min':2,'5min':3,'10min':2}
input=spatiotemporal.get_example_path('toy/data')
output=spatiotemporal.get_example_path('toy/output_notemp')

# create DAG
nodes,graph,graph_prob,graph_scores=spatiotemporal.create_DAG(dict,input_dir=input,output_dir=output)

# Example of model with temporal scoring -----------------------------------------------------------------

# Input variables. 
dict={'0min':2,'5min':3,'10min':2}
input=spatiotemporal.get_example_path('toy/data')
output=spatiotemporal.get_example_path('toy/output_temp')
# expected_subcomplexes is a list of all possible subcomplex strings in the model. Should match the configuration files
subcomplexes=['A1','A2','B1','B2']

nodes,graph,graph_prob,graph_scores=spatiotemporal.create_DAG(dict,input_dir=input,output_dir=output,spatio_temporal_rule=True,expected_subcomplexes=subcomplexes)

# Example with stoichiometeric data -------------------------------------------------------------------------

# Input variables. 
dict={'0min':2,'5min':3,'10min':2}
input=spatiotemporal.get_example_path('toy/data')
output=spatiotemporal.get_example_path('toy/output_stoich')
subcomplexes=['A1','A2','B1','B2']
# exp_comp_map is a dictionary that describes protein stoicheometery. The key describes the protein, which should correspond to names within the expected_subcomplexes. For each of these proteins, a csv file should be provided with protein copy number data
exp_comp={'A':'exp_comp_A.csv','B':'exp_comp_B.csv'}


nodes,graph,graph_prob,graph_scores=spatiotemporal.create_DAG(dict,input_dir=input,output_dir=output,spatio_temporal_rule=True,expected_subcomplexes=subcomplexes,score_comp=True,exp_comp_map=exp_comp)

# Example of how model precision dpends on the input data ---------------------------------------------------

# Input variables. 
dict={'0min':2,'5min':3,'10min':2}
input=spatiotemporal.get_example_path('toy/data_precise')
output=spatiotemporal.get_example_path('toy/output_stoich_precise')
subcomplexes=['A1','A2','B1','B2']
exp_comp={'A':'exp_comp_A_precise.csv','B':'exp_comp_B_precise.csv'}


nodes,graph,graph_prob,graph_scores=spatiotemporal.create_DAG(dict,out_pdf=True,npaths=2,input_dir=input,output_dir=output,spatio_temporal_rule=True,expected_subcomplexes=subcomplexes,score_comp=True,exp_comp_map=exp_comp)
