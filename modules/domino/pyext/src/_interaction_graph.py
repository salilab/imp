import networkx as NX
(NO_EDGE, EDGE_TYPE_SOLID) = range(2)


class InteractionGraph(NX.Graph):

    def init_graph(self, pairs):
        self.load_dependencies(pairs)

    def load_dependencies(self, pairs):
        for p in pairs:
            compA = p[0]
            compB = p[1]
            self.add_edge(compA, compB)
