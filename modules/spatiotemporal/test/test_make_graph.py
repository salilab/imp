import IMP
import IMP.test
import IMP.spatiotemporal as spatiotemporal
import IMP.spatiotemporal.graphNode as graphNode
import shutil
import os
import sys
import itertools
import numpy as np

def setup_system():
    """
    Function to set up initial variables
    """
    # Input variables.
    dict = {'0min': 2, '5min': 3, '10min': 2}
    subcomplexes = ['A1', 'A2', 'B1', 'B2']
    # exp_comp_map is a dictionary that describes protein stoicheometery. The key describes the protein, which should correspond to names within the expected_subcomplexes. For each of these proteins, a csv file should be provided with protein copy number data
    exp_comp = {'A': 'exp_comp_A.csv', 'B': 'exp_comp_B.csv'}
    return dict, subcomplexes, exp_comp

class Tests(IMP.test.TestCase):

    def test_graph_setup(self):
        """
        Test setting up a graph. Tests functionality of graphNode.py
        """
        state_dict, expected_subcomplexes, exp_comp_map = setup_system()
                # set input dir
        with IMP.test.temporary_directory() as tmpdir:
            input_dir = os.path.join(tmpdir, 'data')
            shutil.copytree(self.get_input_file_name('data'), input_dir)
            temp_key = list(exp_comp_map.keys())
            for key in temp_key:
                found = 0
                for subcomplex in expected_subcomplexes:
                    if key in subcomplex:
                        found = found + 1
                if found == 0:
                    raise Exception('WARNING!!! Check exp_comp_map and expected_subcomplexes. protein ' + key + ' is not found in expected_subcomplexes. This could cause illogical results.')

            # Step 1: Initialize graph with static scores ----------------------------------------------------------------------
            # list of all nodes
            nodes = []
            # keys correspond to all timepoints
            keys = list(state_dict.keys())
            if len(input_dir) > 0:
                if os.path.exists(input_dir):
                    os.chdir(input_dir)
                else:
                    raise Exception("Error!!! Does not exist: " + input_dir + '\nClosing...')

            # Loop over all keys and all states
            for key in keys:
                for i in range(state_dict[key]):
                    index = i + 1
                    node = graphNode.graphNode()
                    node.init_graphNode(key, str(index), '_scores.log', '.config', [])
                    nodes.append(node)

            # build up candidate edges in graph
            tpairs = [(keys[i], keys[i + 1]) for i in range(0, len(keys) - 1)]
            for a, b in tpairs:
                # get time marginalized nodes
                anode = [n for n in nodes if n.get_time() == a]
                bnode = [n for n in nodes if n.get_time() == b]
                # draw edges between pairs. Include whether or not to include spatio_temporal_rule
                for na, nb in itertools.product(anode, bnode):
                    graphNode.draw_edge(na, nb, False)
            # set indeces for all nodes. These are unique for each node, unlike labels, which can overlap
            for ni, node in enumerate(nodes):
                node.set_index(ni)
            # check that all nodes are graphNode objects
            for node in nodes:
                self.assertIsInstance(node,graphNode.graphNode)

    def test_graph_scoring(self):
        """
        Tests scoring the same graph built above. Tests score_graph function (score_graph.py)
        """
        state_dict, expected_subcomplexes, exp_comp_map = setup_system()
        # set input dir
        with IMP.test.temporary_directory() as tmpdir:
            input = os.path.join(tmpdir, 'data')
            shutil.copytree(self.get_input_file_name('data'), input)
            nodes, graph, graph_prob, graph_scores = spatiotemporal.create_DAG(state_dict, input_dir=input, out_cdf=False,out_labeled_pdf=False,draw_dag=False)
            # Check the overall score for the first trajectory
            for trajectory in graph_scores:
                if trajectory[0][0].get_label() == '1' and trajectory[0][1].get_label() == '1' and trajectory[0][2].get_label() == '1':
                    self.assertAlmostEqual(trajectory[1], 2.0, delta=1e-4)

    @unittest.skipIf(sys.version_info[0] < 3,
                     "Does not work with ancient numpy in Python 2")
    def test_temporal_scoring(self):
        """
        Tests spatiotemporal rule functionality. Found in draw_edge function of graphNode
        """
        state_dict, expected_subcomplexes, exp_comp_map = setup_system()
        # set input dir
        with IMP.test.temporary_directory() as tmpdir:
            input = os.path.join(tmpdir, 'data')
            shutil.copytree(self.get_input_file_name('data'), input)
            nodes, graph, graph_prob, graph_scores= spatiotemporal.create_DAG(state_dict, input_dir=input, out_cdf=False,out_labeled_pdf=False,spatio_temporal_rule=True,expected_subcomplexes=expected_subcomplexes,draw_dag=False)
            # Check the overall score for the first trajectory
            for trajectory in graph_scores:
                if trajectory[0][0].get_label() == '1' and trajectory[0][1].get_label() == '1' and trajectory[0][2].get_label() == '1':
                    self.assertAlmostEqual(trajectory[1], 2.0, delta=1e-4)

    def test_composition_scoring(self):
        """
        Tests composition scoring functionality. Found in composition_scoring.py, calc_likelihood
        """
        try:
            import pandas
        except ImportError:
            self.skipTest(
                'pandas not available, will not test composition scoring')
        state_dict, expected_subcomplexes, exp_comp_map = setup_system()
        # set input dir
        with IMP.test.temporary_directory() as tmpdir:
            input = os.path.join(tmpdir, 'data')
            shutil.copytree(self.get_input_file_name('data'), input)
            nodes, graph, graph_prob, graph_scores = spatiotemporal.create_DAG(state_dict, input_dir=input, out_cdf=False,out_labeled_pdf=False, score_comp=True,exp_comp_map=exp_comp_map,draw_dag=False,spatio_temporal_rule=False,expected_subcomplexes=expected_subcomplexes)
            # Check the overall score for the lowest energy trajectory
            for trajectory in graph_scores:
                if trajectory[0][0].get_label() == '1' and trajectory[0][1].get_label() == '2' and trajectory[0][2].get_label() == '1':
                    self.assertAlmostEqual(trajectory[1], 6.701131199228036, delta=1e-4)

    @unittest.skipIf(sys.version_info[0] < 3,
                     "Does not work with ancient numpy in Python 2")
    def test_writing_output(self):
        """
        Tests writing text output. From write_output.py
        """
        state_dict, expected_subcomplexes, exp_comp_map = setup_system()
        # set input dir
        with IMP.test.temporary_directory() as tmpdir:
            input = os.path.join(tmpdir, 'data')
            shutil.copytree(self.get_input_file_name('data'), input)
            # set output dir
            output = os.path.join(tmpdir, 'output')
            spatiotemporal.create_DAG(state_dict, input_dir=input,output_dir=output,out_labeled_pdf=True,out_cdf=True,out_pdf=True,draw_dag=False)
            # Read in input files
            cdf=np.loadtxt(output+'/cdf.txt')
            # the 2nd most likely path in the cdf
            self.assertAlmostEqual(cdf[1], 0.42117519, delta=1e-4)
            # the 2nd most likely path in the pdf
            pdf = np.loadtxt(output + '/pdf.txt')
            self.assertAlmostEqual(pdf[1], 0.2105876, delta=1e-4)
            check_label_pdf=open(output+'/labeled_pdf.txt','r')
            # read in the 2nd line,
            line=check_label_pdf.readline()
            line=check_label_pdf.readline()
            line_split=line.split()
            check_label_pdf.close()
            # 2 possibilities are acceptable: '1_0min|2_5min|2_10min|','1_0min|2_5min|1_10min|'. Check this output in 3 parts:
            self.assertEqual(line_split[0][0:14], '1_0min|2_5min|')
            self.assertAlmostEqual(int(line_split[0][14]), 1.5,delta=0.5000001)
            self.assertEqual(line_split[0][15:], '_10min|')

    def test_writing_dag(self):
        """
        Tests writing DAG output. From write_output.py
        """
        state_dict, expected_subcomplexes, exp_comp_map = setup_system()
        # set input dir
        with IMP.test.temporary_directory() as tmpdir:
            input = os.path.join(tmpdir, 'data')
            shutil.copytree(self.get_input_file_name('data'), input)
            # set output dir
            output=self.get_tmp_file_name('output')
            skip=0
            try:
                from graphviz import Digraph
            except ImportError:
                self.skipTest('graphviz not available, will not test drawing graph')
            try:
                from matplotlib import cm
                from matplotlib import colors as clr
            except ImportError:
                self.skipTest('matplotlib not available, will not test drawing graph')
            nodes, graph, graph_prob, graph_scores = spatiotemporal.create_DAG(state_dict, input_dir=input,output_dir=output, draw_dag=True,out_labeled_pdf=False,out_cdf=False,out_pdf=False)
            # scores for the first trajectory match
            for trajectory in graph_scores:
                if trajectory[0][0].get_label() == '1' and trajectory[0][1].get_label() == '1' and trajectory[0][2].get_label() == '1':
                    self.assertAlmostEqual(trajectory[1], 2.0, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
