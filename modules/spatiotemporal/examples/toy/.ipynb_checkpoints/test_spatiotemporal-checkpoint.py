import sys
sys.path.insert(0, "..")
import spatiotemporal

dict={'0min':2,'5min':3,'10min':2}
exp_comp={'A':'exp_comp_A.csv','B':'exp_comp_B.csv'}
subcomplexes=['A1','A2','B1','B2']
input='data'
output='../output'

spatiotemporal.create_DAG(dict,input_dir=input,output_dir=output,scorestr='_scoresA.log',spatio_temporal_rule=True,expected_subcomplexes=subcomplexes,score_comp=True,exp_comp_map=exp_comp,dag_heatmap=True)
