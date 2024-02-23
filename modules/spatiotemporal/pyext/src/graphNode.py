import numpy as np
import os

class graphNode:
    """A class to represent a node in a spatiotemporal process.

    Each graphNode contains a list of it's component subcoplexes,
    an array of scores, a time index and a list of pointers to nodes
    to which edges coming from this node point.
    """

    def __init__(self):
        """Initialize a node.
        """

        self._edges = set()
        self._scores = []
        self._time = None
        self._index = None
        self._label = None
        self._components = []
        self._expected_subcomplexes = []

    def __repr__(self):
        """String representation of a graphNode
        """
        return "graphNode(" + ",".join([str(self._time), str(self._label)]) + ")"

    def init_graphNode(self, time, label, scorestr, subcomplexstr, expected_subcomplexes):
        """Function that initiates a graph node with specific time, label, and expected_subcomplexes. Scores and components are extracted from files named scorestr and subcomplexstr respectively. Returns a single graphNode object.
        @param time: string, time point in the stepwise temporal process
        @param label: string, a number label for the graphNode
        @param scorestr: string, trailing characters at the end of the file with scores for each stage of the spatiotemporal model
        @param subcomplexstr: string, trailing characters after the subcomplex file, which is a list of subcomplexes included in the given label/time
        @param expected_subcomplexes: list of all possible subcomplex strings in the model
        """

        # Set values input from the function call
        self.set_time(time)
        self.set_label(label)
        self.set_expected_subcomplexes(expected_subcomplexes)


        # scores is a list of energies, where each entry corresponds to the score at a given timepoint. It is averaged over all simulations
        scores_fn=label + '_' + time + scorestr
        if os.path.exists(scores_fn):
            scores=np.loadtxt(scores_fn)
        else:
            raise Exception("Error!!! Unable to find scores file: " + scores_fn + '\nClosing...')
        self.add_score(scores.mean())

        if len(expected_subcomplexes)>0:
            with open(label + '_' + time + subcomplexstr, "r") as fh:
                included_prots = fh.read().splitlines()

            self.set_subcomplex_components(included_prots,label + '_' + time + subcomplexstr)

    # Index indexes over all nodes
    def set_index(self, index):
        """Set an index to label node's identity.
        """
        self._index = index

    def get_index(self):
        """Return the node's index.
        """
        return self._index

    # Labels are different states at the same time point
    def set_label(self, label):
        """Set an index to label node's identity.
        """
        self._label = label

    def get_label(self):
        """Return the node's index.
        """
        return self._label

    def get_weight(self):
        """Return the weight of the node. A weight refers in this case to the
        sum score of the scores list.
        """
        return sum(self._scores)

    def get_subcomplex_components(self):
        """Return a list of subcomplexes in this node's representation.
        """
        return self._components

    def set_subcomplex_components(self, scs,fn):
        """Set the list of subcomplex components.

        Should be one of the standard used components.
        """
        for sc in scs:
            if sc not in self._expected_subcomplexes:
                raise Exception("Error!!! Did not recognize the subcomplex name " + sc + " from config file: "+fn)

        self._components = scs

    def set_expected_subcomplexes(self, expected_subcomplexes):
        """Set the list of possible subcomplex components.
        Should include all possible components across the entire spatiotemporal model
        """
        self._expected_subcomplexes = expected_subcomplexes

    def add_edge(self, edge):
        """Add a directed edge to the node.

        Expects a graphNode object.
        """

        assert isinstance(edge, graphNode), "Object " + str(edge) + " is not a graphNode"

        # add if not already present
        self._edges.add(edge)

    def get_edges(self):
        """Return the list of edges for this node.
        """
        return self._edges

    def set_time(self, time):
        """Set the time.
        """
        self._time = time

    def get_time(self):
        """Return the time associated with this node.
        """
        return self._time

    def set_scores(self, scores):
        """Set the score data for this node.

        Expects an list of floats which represent the total score array.
        """
        self._scores = scores

    def add_score(self, score):
        """Update the score list by appending score.
        """
        # assert we're getting a float
        assert isinstance(score, float), "add_score expects a float but got a " + str(type(score))

        self._scores.append(score)

    def get_scores(self):
        """Return the scores array.
        """
        return self._scores

def draw_edge(nodeA, nodeB, spatio_temporal_rule):
    """
    Draws an edge between graphNode objects nodeA and nodeB. If spatio_temporal_rule, node will only be drawn if the components of nodeA are a subset of the components of nodeB.
    If spatio_temporal_rule: determines if an edge should be drawn if Node A comes immediately before
    node B in time and if node B contains node A's components as a subset.
    Else: draws an edge between nodeA and nodeB regardless.
    @param nodeA: first graphNode object
    @param nodeB: second graphNode object
    @param spatio_temporal_rule: Boolean, whether to consider composition when drawing edges
    """

    # assert both are nodes
    assert isinstance(nodeA, graphNode), str(nodeA) + " is not a graphNode."
    assert isinstance(nodeB, graphNode), str(nodeB) + " is not a graphNode."

    # check subcomponents are subsets. All nodes in timestep t are also in t+1
    if spatio_temporal_rule:
        if frozenset(nodeA.get_subcomplex_components()).issubset(set(nodeB.get_subcomplex_components())):
            nodeA.add_edge(nodeB)
    else:
        nodeA.add_edge(nodeB)