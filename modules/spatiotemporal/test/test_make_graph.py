import IMP
import IMP.test
import IMP.spatiotemporal as spatiotemporal

def setup_system():
    # Input variables.
    dict = {'0min': 2, '5min': 3, '10min': 2}
    subcomplexes = ['A1', 'A2', 'B1', 'B2']
    # exp_comp_map is a dictionary that describes protein stoicheometery. The key describes the protein, which should correspond to names within the expected_subcomplexes. For each of these proteins, a csv file should be provided with protein copy number data
    exp_comp = {'A': 'exp_comp_A.csv', 'B': 'exp_comp_B.csv'}
    return dict, subcomplexes, exp_comp

class Tests(IMP.test.TestCase):

    def test_simple(self):
        dict, subcomplexes, exp_comp = setup_system()
        graph, graph_scores = spatiotemporal.create_DAG(dict,out_cdf=False,out_labeled_pdf=False, input_dir=input, output_dir=output,draw_dag=False)


