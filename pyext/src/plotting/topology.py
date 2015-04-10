from __future__ import print_function
import IMP
import RMF
import IMP.atom
import IMP.rmf
import IMP.pmi
import IMP.pmi.analysis
import IMP.pmi.output
from collections import defaultdict
import numpy as np
from scipy.spatial.distance import cdist

class TopologyPlot(object):
    """A class to read RMF files and make a network contact map"""
    def __init__(self,model,selections,cutoff,frequency_cutoff,
                      colors=None,fixed=None,pos=None,proteomic_edges=None,quantitative_proteomic_data=None):
        """Set up a new graphXL object
        @param model          The IMP model
        @param selection_dict A dictionary containing component names.
                              Keys are labels
                              values are either moleculename or start,stop,moleculename
        @param cutoff        The distance cutoff
        @param frequency_cutoff The frequency cutoff
        @param colors        A dictionary of colors (HEX code,values) for subunits (keywords)
        @param fixed         A list of subunits that are kept fixed
        @param pos           A dictionary with positions (tuple, values) of subunits (keywords)
        @param proteomic_edges A list edges to represent proteomic data
        @param quantitative_proteomic_data A dictionary of edges to represent
                             quantitative proteomic data such as PE Scores, or genetic interactions
        """
        import itertools\

        self.mdl = model
        self.selections = selections
        self.contact_counts={}
        self.edges=defaultdict(int)
        for (name1,name2) in itertools.combinations(self.selections.keys(),2):
            self.edges[tuple(sorted((name1,name2)))]=0
        self.cutoff = cutoff
        self.frequency_cutoff=frequency_cutoff
        self.gcpf = IMP.core.GridClosePairsFinder()
        self.gcpf.set_distance(self.cutoff)
        self.names = list(self.selections.keys())
        self.colors=colors
        self.fixed=fixed
        self.pos=pos
        self.proteomic_edges=proteomic_edges
        self.quantitative_proteomic_data=quantitative_proteomic_data
        self.num_rmf=0

    def add_rmf(self,rmf_fn,nframe):
        """Add selections from an RMF file"""
        print('reading from RMF file',rmf_fn)
        rh = RMF.open_rmf_file_read_only(rmf_fn)
        prots = IMP.rmf.create_hierarchies(rh, self.mdl)
        hier = prots[0]
        IMP.rmf.load_frame(rh,0)
        ps_per_component=defaultdict(list)
        if self.num_rmf==0:
            self.size_per_component=defaultdict(int)
        self.mdl.update()

        #gathers particles for all components
        part_dict = IMP.pmi.analysis.get_particles_at_resolution_one(hier)
        all_particles_by_resolution = []
        for name in part_dict:
            all_particles_by_resolution += part_dict[name]

        for component_name in self.selections:
            for seg in self.selections[component_name]:
                if type(seg) == str:
                    s = IMP.atom.Selection(hier,molecule=seg)
                elif type(seg) == tuple:
                    s = IMP.atom.Selection(hier,molecule=seg[2],
                        residue_indexes=range(seg[0], seg[1] + 1))
                else:
                    raise Exception('could not understand selection tuple '+str(seg))
                parts = list(set(s.get_selected_particles()) & set(all_particles_by_resolution))
                ps_per_component[component_name] += IMP.get_indexes(parts)
                if self.num_rmf==0:
                    self.size_per_component[component_name] += sum(len(IMP.pmi.tools.get_residue_indexes(p)) for p in parts)

        for n1,name1 in enumerate(self.names):
            for name2 in self.names[n1+1:]:
                ncontacts = len(self.gcpf.get_close_pairs(self.mdl,
                                                     ps_per_component[name1],
                                                     ps_per_component[name2]))
                if ncontacts>0:
                    self.edges[tuple(sorted((name1,name2)))]+=1.0

        self.num_rmf+=1

    def make_plot(self,groups,out_fn,quantitative_proteomic_data=False):
        '''
        plot the interaction matrix
        @param groups is the list of groups of domains, eg,
                      [["protA_1-10","prot1A_11-100"],["protB"]....]
                      it will plot a space between different groups
        @param quantitative_proteomic_data plot the quantitative proteomic data
        '''
        import numpy as np
        import matplotlib.pyplot as plt
        from matplotlib import cm

        ax=plt.gca()
        ax.set_aspect('equal', 'box')
        ax.xaxis.set_major_locator(plt.NullLocator())
        ax.yaxis.set_major_locator(plt.NullLocator())

        largespace=0.6
        smallspace=0.5
        squaredistance=1.0
        squaresize=0.99
        domain_xlocations={}
        domain_ylocations={}

        xoffset=squaredistance
        yoffset=squaredistance
        xlabels=[]
        ylabels=[]
        for group in groups:
            xoffset+=largespace
            yoffset+=largespace
            for subgroup in group:
                xoffset+=smallspace
                yoffset+=smallspace
                for domain in subgroup:
                    domain_xlocations[domain]=xoffset
                    domain_ylocations[domain]=yoffset
                    #rect = plt.Rectangle([xoffset- squaresize / 2, yoffset - squaresize / 2], squaresize, squaresize,
                    #                     facecolor=(1,1,1), edgecolor=(0.1,0.1,0.1))

                    #ax.add_patch(rect)
                    #ax.text(xoffset , yoffset ,domain,horizontalalignment='left',verticalalignment='center',rotation=-45.0)
                    xoffset+=squaredistance
                    yoffset+=squaredistance

        for edge,count in self.edges.items():

            if quantitative_proteomic_data:
                #normalize
                maxqpd=max(self.quantitative_proteomic_data.values())
                minqpd=min(self.quantitative_proteomic_data.values())
                if edge in self.quantitative_proteomic_data:
                    value=self.quantitative_proteomic_data[edge]
                elif (edge[1],edge[0]) in self.quantitative_proteomic_data:
                    value=self.quantitative_proteomic_data[(edge[1],edge[0])]
                else:
                    value=0.0
                print(minqpd,maxqpd)
                density=(1.0-(value-minqpd)/(maxqpd-minqpd))
            else:
                density=(1.0-float(count)/self.num_rmf)
            color=(density,density,1.0)
            x=domain_xlocations[edge[0]]
            y=domain_ylocations[edge[1]]
            if x>y: xtmp=y; ytmp=x; x=xtmp; y=ytmp
            rect = plt.Rectangle([x - squaresize / 2, y - squaresize / 2], squaresize, squaresize,
                             facecolor=color, edgecolor='Gray', linewidth=0.1)
            ax.add_patch(rect)
            rect = plt.Rectangle([y - squaresize / 2, x - squaresize / 2], squaresize, squaresize,
                             facecolor=color, edgecolor='Gray', linewidth=0.1)
            ax.add_patch(rect)

        ax.autoscale_view()
        plt.savefig(out_fn)
        plt.show()
        exit()

    def make_graph(self,out_fn):
        edges=[]
        weights=[]
        print('num edges',len(self.edges))
        for edge,count in self.edges.items():
            # filter if frequency of contacts is greater than frequency_cutoff
            if float(count)/self.num_rmf>self.frequency_cutoff:
                print(count,edge)
                edges.append(edge)
                weights.append(count)
        for nw,w in enumerate(weights):
            weights[nw]=float(weights[nw])/max(weights)
        IMP.pmi.output.draw_graph(edges,#node_size=1000,
                                  node_size=dict(self.size_per_component),
                                  node_color=self.colors,
                                  fixed=self.fixed,
                                  pos=self.pos,
                                  edge_thickness=1, #weights,
                                  edge_alpha=0.3,
                                  edge_color='gray',
                                  out_filename=out_fn,
                                  validation_edges=self.proteomic_edges)
