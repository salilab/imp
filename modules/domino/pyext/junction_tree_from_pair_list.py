############################################################
##                junction_tree.py
##
## description: The script gets as input a pairs list
## and calculates a valid junction tree

############################################################

import sys,os
import networkx  as NX  # import networkx after pylab
import interaction_graph
import copy
import operator
def Usage():
    if len(sys.argv) != 3:
        print sys.argv[0] + ' <pair_list.input>  <junction_tree.txt> '
        sys.exit(-1)


def load_dependencies(G,nodes,assembly_topology_filename):
    ig = interaction_graph.InteractionGraph()
    ig.init_graph(assembly_topology_filename)
    ig.load_dependencies()
    G = ig

    for i in xrange(G.number_of_nodes()):
        nodes.append(i)


def select_node(G):
    selected_node_index = 0

    for i in range(G.number_of_nodes()):
        if G.degree(G.nodes()[i]) < G.degree(G.nodes()[selected_node_index]):
            selected_node_index = i

    return (selected_node_index,G.nodes()[selected_node_index])

#the algorithm works as follows:
#at each stage select the node with the max num of neighbors.
#connect each pair of its nodes with  an edge and remove the node.
def triangulate(G,nodes):
    g1 = copy.deepcopy(G)
    while g1.number_of_nodes() != 0:
        (node2remove_index,node2remove) = select_node(g1)
       # print "junction_tree log : selected node  " + str(node2remove)
        #update the edges
        for n1 in g1.neighbors(node2remove):
            for n2 in g1.neighbors(node2remove):
#                print 'neighbors : ' + str(n1) + '   '+ str(n2)
                if n1 == n2:
                    continue
                if g1.has_edge(n1,n2):
                    continue
                g1.add_edge(n1,n2)
                G.add_edge(n1,n2)#nodes[n1],nodes[n2])
                #print "junction_tree log : add edge between " + str(n1) + " and " + str(n2)
        g1.remove_node(node2remove)



def max_spanning_tree(G):
    t = NX.Graph()
    for node in G.nodes():
        t.add_node(node)
    for e in reversed(sorted(G.edges(),key=operator.itemgetter(2))):
        try:
            l = shortest_path_length(t,e[0],e[1])
        except networkx.NetworkXError:
            if e[2] > 0:
                t.add_edge(e[0],e[1],e[2])
    return t


def calc_jt(G,nodes,result_filename):
#    print "junction_tree log start : nodes : " + str(G.nodes()) + " edges : " + str(G.edges())
    triangulate(G,nodes)
#    print "junction_tree log after triangulation nodes : " + str(G.nodes()) + " edges : " + str(G.edges())
    clique_list =  list(NX.find_cliques(G))
    #print "junction_tree log start : cliques " + str(clique_list)
    gt = NX.Graph()
    gt_nodes = []
    for i in range(len(clique_list)):
        gt.add_node(i)

    for i in range(gt.number_of_nodes()):
        j = i+1
        while j < gt.number_of_nodes():

            counter = 0
            for e_i in clique_list[i]:
                for e_j in clique_list[j]:
                    if e_i == e_j:
                        #                    print str(e_i) + '   ' + str(e_j)
                        counter +=1
                        continue
            #print "junction adding edges between : " + str(gt.nodes()[i]) + " and " + str(gt.nodes()[j]) + " with weight of : " + str(counter)
            gt.add_edge(gt.nodes()[i],gt.nodes()[j],weight=-counter)
            j += 1
    mst=NX.kruskal_mst(gt) # a generator of MST edges
    t=sorted(list(mst))
    #t = max_spanning_tree(gt)
    result_file = open(result_filename,'w')
    result_file.write('nodes: ' + str(len(clique_list))+'\n')
    display_graph = NX.Graph()
    display_labels = {}
    node_ind = -1
    for cl in clique_list:
        node_ind += 1
        node_label=""#"["
        for e in cl:
            node_label += str(e)+' '
        node_label+=""#']'
        result_file.write(node_label+'\n')
        display_labels[node_ind] = node_label
    result_file.write('edges: ' + str(len(t)) +'\n')
    for e in t:
        result_file.write(str(e[0]) + ' '+ str(e[1])+ '\n')#  ' + str(e) + '\n')
        display_graph.add_edge(e[0],e[1])
    result_file.close()
    #print "display_graph: " + str(display_graph.nodes()) + " edgds: " + str(display_graph.edges())
    return (display_graph,display_labels)

def main():
    Usage()
    pair_list_filename = sys.argv[1]
    result_filename = sys.argv[2]
    G = NX.Graph()
    nodes = []

    #open pairlist file
    f=open(pair_list_filename)
    pair_list = f.readlines()
    pairs=[]
    for p in pair_list:
        s=p.split()
        pairs.append([int(s[0]),int(s[1])])
    ig = interaction_graph.InteractionGraph()
    ig.init_graph(pairs)
    G = ig

    for i in xrange(G.number_of_nodes()):
        nodes.append(i)
    calc_jt(G,nodes,result_filename)

if __name__ == "__main__":
    main()
