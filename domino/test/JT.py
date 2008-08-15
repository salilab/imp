# Prevent matplotlib (used by networkx) from trying to open the display:
import matplotlib
matplotlib.use('Agg')

import networkx as NX
import sys
import IMP

class JT(NX.XGraph):
    #TODO: in the future we will create the topology file on the fly
    def init_graph(self,topology_filename_, mdl,ps):
        self.nodes_data = []
        self.mdl = mdl
        self.load_topology(topology_filename_,ps)

    def load_topology(self,topology_filename_,ps):
        x_counter = -1
        status = -1
        for line in open(topology_filename_,"r"):
            s_line = line.split()
            if (s_line[0]=='nodes:') or (s_line[0]=='edges:'):
                status +=1
                continue
            if status == 0:
                x_counter = x_counter + 1
                #keep the protein ids as appear in mdl
                self.nodes_data.append(IMP.Ints())
                for n in s_line:
                    n_found=False
                    for p in ps:
                        if p.get_value(IMP.StringKey("name")) == n:
                            self.nodes_data[-1].push_back(
                                                    p.get_index().get_index())
                            n_found = True
                    if not n_found:
                        print "JT::load_topology could not find: " + n
                self.add_node(x_counter)
            if status == 1:
                (n1,n2) = (int(s_line[0]),int(s_line[1]))
                self.add_edge(n1,n2)




    def __str__(self):
        output = 'nodes: \n'
        for n in self.nodes_iter():
            output += "node: " + str(n) + ' : '
            particles = self.mdl.get_particles()
            namekey = IMP.StringKey("name")
            for d in self.nodes_data[n]:
                output += str(particles[d].get_value(namekey)) + " :: "
            output += "\n"
        output += 'edges: \n'
        for e in self.edges_iter():
            output += str(e[0]) + '  ' + str(e[1]) + '  ' + str(e[2])  + '\n'
        return output
