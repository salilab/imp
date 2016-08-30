"""@namespace IMP.pmi.io.xltable
   Tools to plot a contact map overlaid with cross-links.
"""

from __future__ import print_function
from math import sqrt
from scipy.spatial.distance import cdist
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.cm as cm
import matplotlib.pyplot as plt

from collections import defaultdict
import pickle
import IMP
import IMP.atom
import IMP.rmf
import RMF
import IMP.pmi
import IMP.pmi.io
import IMP.pmi.io.crosslink
import IMP.pmi.io.utilities
import IMP.pmi.topology
import IMP.pmi.analysis
class XLTable():
    """ class to read, analyze, and plot xlink data on contact maps
    Canonical way to read the data:
    1) load sequences and name them
    2) load coordinates for those sequences from PDB file
    3) add crosslinks
    4) create contact map
    5) plot
    """

    def __init__(self,contact_threshold):
        self.sequence_dict={}
        self.cross_link_db = None
        self.residue_pair_list = []          # list of special residue pairs to display
        self.distance_maps = []              # distance map for each copy of the complex
        self.contact_freqs = None
        self.num_pdbs = 0
        self.num_rmfs = 0
        self.index_dict = defaultdict(list)  # location in the dmap of each residue
        self.contact_threshold = contact_threshold
        # internal things
        self._first = True
        self.index_dict={}
        self.stored_dists={}
        self.mdl = IMP.Model()

    def _colormap_distance(self, dist, threshold=35, tolerance=0):
        if dist < threshold - tolerance:
            return "Green"
        elif dist >= threshold + tolerance:
            return "Orange"
        else:
            return "Red"

    def _colormap_satisfaction(self, sat, threshold=0.5, tolerance=0.1):
        if sat >=  threshold + tolerance:
            print(sat, "green")
            return "Green"
        elif sat < threshold + tolerance and sat >= threshold - tolerance :
            print(sat, "orange")
            return "Orange"
        else:
            print(sat, "orange")
            return "Orange"

    def _get_percentage_satisfaction(self,r1,c1,r2,c2,threshold=35):
        try:
            idx1=self.index_dict[c1][r1]
        except:
            return None
        try:
            idx2=self.index_dict[c2][r2]
        except:
            return None
        nsatisfied=0
        for dists in self.dist_maps:
            dist=dists[idx1,idx2]
            if dist<threshold: nsatisfied+=1
        return float(nsatisfied)/len(self.dist_maps)

    def _get_distance(self,r1,c1,r2,c2):
        if self.index_dict is not None:
            try:
                idx1=self.index_dict[c1][r1]
            except:
                return None
            try:
                idx2=self.index_dict[c2][r2]
            except:
                return None
            return self.av_dist_map[idx1,idx2]
        else:
            if (r1,c1,r2,c2) not in self.stored_dists.keys():
                sel=IMP.atom.Selection(self.prots,molecule=c1,residue_index=r1)
                selpart=sel.get_selected_particles()
                selpart_res_one=list(set(self.particles_resolution_one) & set(selpart))
                if len(selpart_res_one)>1: return None
                if len(selpart_res_one)==0: return None
                selpart_res_one_1=selpart_res_one[0]
                sel=IMP.atom.Selection(self.prots,molecule=c2,residue_index=r2)
                selpart=sel.get_selected_particles()
                selpart_res_one=list(set(self.particles_resolution_one) & set(selpart))
                if len(selpart_res_one)>1: return None
                if len(selpart_res_one)==0: return None
                selpart_res_one_2=selpart_res_one[0]
                d1=IMP.core.XYZ(selpart_res_one_1)
                d2=IMP.core.XYZ(selpart_res_one_2)
                dist=IMP.core.get_distance(d1,d2)
                self.stored_dists[(r1,c1,r2,c2)]=dist
            else:
                dist=self.stored_dists[(r1,c1,r2,c2)]
            return dist


    def _get_distance_and_particle_pair(self,r1,c1,r2,c2):
        '''more robust and slower version of above'''
        sel=IMP.atom.Selection(self.prots,molecule=c1,residue_index=r1)
        selpart=sel.get_selected_particles()
        selpart_res_one=list(set(self.particles_resolution_one) & set(selpart))
        if len(selpart_res_one)>1: return None
        if len(selpart_res_one)==0: return None
        selpart_res_one_1=selpart_res_one[0]
        sel=IMP.atom.Selection(self.prots,molecule=c2,residue_index=r2)
        selpart=sel.get_selected_particles()
        selpart_res_one=list(set(self.particles_resolution_one) & set(selpart))
        if len(selpart_res_one)>1: return None
        if len(selpart_res_one)==0: return None
        selpart_res_one_2=selpart_res_one[0]
        d1=IMP.core.XYZ(selpart_res_one_1)
        d2=IMP.core.XYZ(selpart_res_one_2)
        dist=IMP.core.get_distance(d1,d2)
        return (dist,selpart_res_one_1,selpart_res_one_2)

    def _internal_load_maps(self,maps_fn):
        npzfile = np.load(maps_fn)
        cname_array=npzfile['cname_array']
        idx_array=npzfile['idx_array']
        index_dict={}
        for cname,idxs in zip(cname_array,idx_array):
            tmp=list(idxs)
            if -1 in tmp:
                index_dict[cname]=tmp[0:tmp.index(-1)]
            else:
                index_dict[cname]=tmp
        av_dist_map = npzfile['av_dist_map']
        contact_map = npzfile['contact_map']
        return index_dict,av_dist_map,contact_map

    def load_sequence_from_fasta_file(self,fasta_file,id_in_fasta_file,protein_name):
        """ read sequence. structures are displayed in the same order as sequences are read.
        fasta_file:        file to read
        id_in_fasta_file:  id of desired sequence
        protein_name:      identifier for this sequence (use same name when handling coordinates)
        can provide the fasta name (for retrieval) and the protein name (for storage) """

        record_dict = IMP.pmi.topology.Sequences(fasta_file)
        if id_in_fasta_file is None:
            id_in_fasta_file = name
        if id_in_fasta_file not in record_dict:
            raise KeyError("id %s not found in fasta file" % id_in_fasta_file)
        length = len(record_dict[id_in_fasta_file])

        self.sequence_dict[protein_name] = str(record_dict[id_in_fasta_file])

    def load_pdb_coordinates(self,pdbfile,chain_to_name_map):
        """ read coordinates from a pdb file. also appends to distance maps
        @param pdbfile             file for reading coords
        @param chain_to_name_map   correspond chain ID with protein name (will ONLY read these chains)
                                   Key: PDB chain ID. Value: Protein name (set in sequence reading)
        \note This function returns an error if the sequence for each chain has NOT been read
        """
        mh = IMP.atom.read_pdb(pdbfile,self.mdl,IMP.atom.CAlphaPDBSelector())
        total_len = sum(len(self.sequence_dict[s]) for s in self.sequence_dict)
        coords = np.ones((total_len,3)) * 1e5 #default to coords "very far away"
        prev_stop = 0
        for cid in chain_to_name_map:
            cname = chain_to_name_map[cid]
            if cname not in self.sequence_dict:
                print("ERROR: chain",cname,'has not been read or has a naming mismatch')
                return
            if self._first:
                self.index_dict[cname]=range(prev_stop,prev_stop+len(self.sequence_dict[cname]))
            sel = IMP.atom.Selection(mh,chain_id=cid)
            for p in sel.get_selected_particles():
                rnum = IMP.atom.get_residue(IMP.atom.Atom(p)).get_index()
                coords[rnum+prev_stop-1,:] = list(IMP.core.XYZ(p).get_coordinates())
            prev_stop+=len(self.sequence_dict[cname])
        dists = cdist(coords, coords)
        binary_dists = np.where((dists <= self.contact_threshold) & (dists >= 1.0), 1.0, 0.0)
        if self._first:
            self.dist_maps= [dists]
            self.av_dist_map = dists
            self.contact_freqs = binary_dists
            self._first=False
        else:
            self.dist_maps.append(dists)
            self.av_dist_map += dists
            self.contact_freqs += binary_dists
        IMP.atom.destroy(mh)
        del mh
        self.num_pdbs+=1

    def load_rmf_coordinates(self,rmf_name,rmf_frame_index, chain_names, nomap=False):
        """ read coordinates from a rmf file. It needs IMP to run.
        rmf has been created using IMP.pmi conventions. It gets the
        highest resolution automatically. Also appends to distance maps
        @param rmf_name             file for reading coords
        @param rmf_frame_index      frame index from the rmf
        @param nomap                Default False, if True it will not calculate the contact map
        """
        (particles_resolution_one, prots)=self._get_rmf_structure(rmf_name,rmf_frame_index)

        total_len = sum(len(self.sequence_dict[s]) for s in self.sequence_dict)
        print(self.sequence_dict,total_len)

        coords = np.ones((total_len,3)) * 1e6 #default to coords "very far away"
        prev_stop=0
        sorted_particles=IMP.pmi.tools.sort_by_residues(particles_resolution_one)

        self.prots=prots
        self.particles_resolution_one=particles_resolution_one

        if nomap:
            return

        for cname in chain_names:
            print(cname)
            if self._first:
                self.index_dict[cname]=range(prev_stop,prev_stop+len(self.sequence_dict[cname]))
            rindexes=range(1,len(self.sequence_dict[cname])+1)
            for rnum in rindexes:
                sel=IMP.atom.Selection(prots,molecule=cname,residue_index=rnum)
                selpart=sel.get_selected_particles()
                selpart_res_one=list(set(particles_resolution_one) & set(selpart))
                if len(selpart_res_one)>1: continue
                if len(selpart_res_one)==0: continue
                selpart_res_one=selpart_res_one[0]
                try:
                    coords[rnum+prev_stop-1,:]=IMP.core.XYZ(selpart_res_one).get_coordinates()
                except IndexError:
                    print("Error: exceed max size",prev_stop,total_len,cname,rnum)
                    exit()
            prev_stop+=len(self.sequence_dict[cname])
        dists = cdist(coords, coords)
        binary_dists = np.where((dists <= self.contact_threshold) & (dists >= 1.0), 1.0, 0.0)
        if self._first:
            self.dist_maps= [dists]
            self.av_dist_map = dists
            self.contact_freqs = binary_dists
            self._first=False
        else:
            self.dist_maps.append(dists)
            self.av_dist_map += dists
            self.contact_freqs += binary_dists
        self.num_rmfs+=1


    def _get_rmf_structure(self,rmf_name,rmf_frame_index):
        rh= RMF.open_rmf_file_read_only(rmf_name)
        prots=IMP.rmf.create_hierarchies(rh, self.mdl)
        IMP.rmf.load_frame(rh, rmf_frame_index)
        print("getting coordinates for frame %i rmf file %s" % (rmf_frame_index, rmf_name))
        del rh

        particle_dict=IMP.pmi.analysis.get_particles_at_resolution_one(prots[0])

        protein_names=particle_dict.keys()
        particles_resolution_one=[]
        for k in particle_dict:
            particles_resolution_one+=(particle_dict[k])

        return particles_resolution_one, prots


    def save_maps(self,maps_fn):
        maxlen=max(len(self.index_dict[key]) for key in self.index_dict)
        cnames=[]
        idxs=[]
        for cname,idx in self.index_dict.iteritems():
            cnames.append(cname)
            idxs.append(idx+[-1]*(maxlen-len(idx)))
        idx_array=np.array(idxs)
        cname_array=np.array(cnames)
        np.savez(maps_fn,
                 cname_array=cname_array,
                 idx_array=idx_array,
                 av_dist_map=self.av_dist_map,
                 contact_map=self.contact_freqs)

    def load_maps(self,maps_fn):
        self.index_dict,self.av_dist_map,self.contact_freqs=self._internal_load_maps(maps_fn)

    def load_crosslinks(self,CrossLinkDataBase,distance_field=None):
        """ read crosslinks from a CSV file.
        provide a CrossLinkDataBaseKeywordsConverter to explain the columns
        @distance_field is the optional keyword for the distance to be read form the file.
        This can skip the rmf reading to calculate the distance of cross-links if
        already provided in the csv file."""
        if type(CrossLinkDataBase) is not IMP.pmi.io.crosslink.CrossLinkDataBase:
            raise TypeError("Crosslink database must be a IMP.pmi.io.CrossLinkDataBase type")
        self.cross_link_db=CrossLinkDataBase
        if distance_field is not None:
            total_len = sum(len(self.sequence_dict[s]) for s in self.sequence_dict)
            zeros = np.zeros((total_len,3))
            self.av_dist_map = cdist(zeros,zeros)
            for xl in self.cross_link_db:
                c1=xl[self.cross_link_db.protein1_key]
                c2=xl[self.cross_link_db.protein2_key]
                r1=xl[self.cross_link_db.residue1_key]
                r2=xl[self.cross_link_db.residue2_key]
                try:
                    self.stored_dists[(r1,c1,r2,c2)]=float(xl[distance_field])
                    self.stored_dists[(r2,c2,r1,c1)]=float(xl[distance_field])
                except ValueError:
                    self.stored_dists[(r1,c1,r2,c2)]=10e6
                    self.stored_dists[(r2,c2,r1,c1)]=10e6
        else:
            for xl in self.cross_link_db:
                c1=xl[self.cross_link_db.protein1_key]
                c2=xl[self.cross_link_db.protein2_key]
                r1=xl[self.cross_link_db.residue1_key]
                r2=xl[self.cross_link_db.residue2_key]
                self.stored_dists[(r1,c1,r2,c2)]=self._get_distance(r1,c1,r2,c2)
                self.stored_dists[(r2,c2,r1,c1)]=self._get_distance(r2,c2,r1,c1)

    def set_residue_pairs_to_display(self,residue_type_pair):
        """ select the atom names of residue pairs to plot on the contact map
        list of residues types must be single letter code
        e.g. residue_type_pair=("K","K")
        """
        rtp=sorted(residue_type_pair)
        for prot1 in self.sequence_dict:
            seq1=self.sequence_dict[prot1]
            for nres1,res1 in enumerate(seq1):
                for prot2 in self.sequence_dict:
                    seq2=self.sequence_dict[prot2]
                    for nres2,res2 in enumerate(seq2):
                        if sorted((res1,res2))==rtp:
                            self.residue_pair_list.append((nres1+1,prot1,nres2+1,prot2))

    def setup_contact_map(self):
        """ loop through each distance map and get frequency of contacts
        """
        if self.num_pdbs!=0 and self.num_rmfs==0:
            self.av_dist_map = 1.0/self.num_pdbs * self.av_dist_map
            self.contact_freqs = 1.0/self.num_pdbs * self.contact_freqs
        if self.num_pdbs==0 and self.num_rmfs!=0:
            self.av_dist_map = 1.0/self.num_rmfs * self.av_dist_map
            self.contact_freqs = 1.0/self.num_rmfs * self.contact_freqs

    def setup_difference_map(self,maps_fn1,maps_fn2,thresh):
        idx1,av1,contact1=self._internal_load_maps(maps_fn1)
        idx2,av2,contact2=self._internal_load_maps(maps_fn2)
        if idx1!=idx2:
            print("UH OH: index dictionaries do not match!")
            exit()
        self.index_dict=idx1
        self.av_dist_map=av1 # should we store both somehow? only needed for XL

        def logic(c1,c2):
            if c1==0 and c2==0:             # white
                return 0
            elif c1>thresh and c2<thresh:   # red
                return 1
            elif c1<thresh and c2>thresh:   # blue
                return 2
            else:                           # green
                return 3
        f = np.vectorize(logic,otypes=[np.int])
        print('computing contact map')
        self.contact_freqs = f(contact1,contact2)
        print('done')



    def spring_layout(self,ax,plt,data, annotations, iterations = 100, k=1):

        import networkx as nx
        import numpy
        import scipy.spatial


        #import matplotlib.pyplot as plt

        """
        Author: G. Bouvier, Pasteur Institute, Paris
        Website: http://bloggb.fr/2015/10/19/spring_layout_for_annotating_plot_in_matplotlib.html
        - data: coordinates of your points [(x1,y1), (x2,y2), ..., (xn,yn)]
        - annotations: text for your annotation ['str_1', 'str_2', ..., 'str_n']
        - iterations: number of iterations for spring layout
        - k: optimal distance between nodes
        """
        G = nx.Graph()
        init_pos = {} # initial position
        x, y = [e[0] for e in data], [e[1] for e in data]
        xmin, xmax = min(x), max(x)
        ymin, ymax = min(y), max(y)

        m=IMP.Model()
        particles={}
        particles_fixed={}
        mvs = []

        rs = IMP.RestraintSet(m, 'distance')

        for i,xy in enumerate(data):
            p=IMP.Particle(m)
            G.add_node(i)
            init_pos[i] = xy
            IMP.core.XYZR.setup_particle(p)
            IMP.core.XYZ(p).set_coordinates((xy[0],xy[1],0))
            IMP.core.XYZR(p).set_radius(100)
            IMP.core.XYZ(p).set_coordinates_are_optimized(True)
            particles[i]=p
            Xfloatkey = IMP.core.XYZ.get_xyz_keys()[0]
            Yfloatkey = IMP.core.XYZ.get_xyz_keys()[1]
            mvs.append(IMP.core.NormalMover([p], [Xfloatkey], 5))
            mvs.append(IMP.core.NormalMover([p], [Yfloatkey], 5))
            p=IMP.Particle(m)
            IMP.core.XYZR.setup_particle(p)
            IMP.core.XYZ(p).set_coordinates((xy[0],xy[1],0))
            IMP.core.XYZR(p).set_radius(100)
            particles_fixed[i]=p
            ts1 = IMP.core.HarmonicUpperBound(200, 0.001)
            rs.add_restraint(
            IMP.core.DistanceRestraint(m, ts1,
                                       particles[i],
                                       particles_fixed[i]))

        ssps = IMP.core.SoftSpherePairScore(1.0)
        lsa = IMP.container.ListSingletonContainer(m)

        ps = particles.values()+particles_fixed.values()

        lsa.add(IMP.get_indexes(ps))

        rbcpf = IMP.core.RigidClosePairsFinder()
        cpc = IMP.container.ClosePairContainer(lsa, 0.0, rbcpf, 10.0)
        evr = IMP.container.PairsRestraint(ssps, cpc)
        rs.add_restraint(evr)

        '''
            #G.add_node(text)
            #G.add_edge(xy, text)
            #init_pos[text] = xy

        delTri = scipy.spatial.Delaunay(data)
        #plt.scatter([e[0] for e in data], [e[1] for e in data])
        # create a set for edges that are indexes of the points
        edges = set()
        # for each Delaunay triangle
        for n in xrange(delTri.nsimplex):
            # for each edge of the triangle
            # sort the vertices
            # (sorting avoids duplicated edges being added to the set)
            # and add to the edges set
            edge = sorted([delTri.vertices[n,0], delTri.vertices[n,1]])
            edges.add((edge[0], edge[1]))
            edge = sorted([delTri.vertices[n,0], delTri.vertices[n,2]])
            edges.add((edge[0], edge[1]))
            edge = sorted([delTri.vertices[n,1], delTri.vertices[n,2]])
            edges.add((edge[0], edge[1]))


        for e in edges:
            G.add_edge(e[0],e[1])

            d1=IMP.core.XYZ(particles[e[0]])
            d2=IMP.core.XYZ(particles[e[1]])
            dist=IMP.core.get_distance(d1,d2)
            ts1 = IMP.core.Harmonic(dist+150, 0.001)
            rs.add_restraint(
            IMP.core.DistanceRestraint(m, ts1,
                                       particles[e[0]],
                                       particles[e[1]]))
        '''

        smv = IMP.core.SerialMover(mvs)

        mc = IMP.core.MonteCarlo(m)
        mc.set_scoring_function(rs)
        mc.set_return_best(False)
        mc.set_kt(1.0)
        mc.add_mover(smv)


        '''
        pos = nx.spring_layout(G ,pos=init_pos)
        x_list=[]
        y_list=[]
        for i,xy in enumerate(data):
            xynew =  pos[i] * [xmax-xmin, ymax-ymin] + [xmin, ymin]
            plt.plot((xy[0],xynew[0]), (xy[1],xynew[1]), 'k-')
            x_list.append(xynew[0])
            y_list.append(xynew[1])
            #plt.annotate(name, xy, size=5,  xycoords='data', xytext=xytext, textcoords='data', bbox=dict(boxstyle='round,pad=0.2', fc='yellow', alpha=0.3),\
            #            arrowprops=dict(arrowstyle="-", connectionstyle="arc3", color='gray'))

        points=ax.scatter(x_list,y_list,alpha=1)
        #pos*=[numpy.ptp([e[0] for e in data]), numpy.ptp([e[1] for e in data])]
        #plt.show()
        return points
        '''

        for i in range(10):
            mc.optimize(1000*len(particles.keys())*2)
            print(rs.evaluate(False))
        x_list=[]
        y_list=[]
        for i,xy in enumerate(data):
            p=particles[i]
            coord=IMP.core.XYZ(p).get_coordinates()
            xynew =  (coord[0],coord[1])
            plt.plot((xy[0],xynew[0]), (xy[1],xynew[1]), 'k-')
            x_list.append(xynew[0])
            y_list.append(xynew[1])
        points=ax.scatter(x_list,y_list,alpha=1,facecolors='none', edgecolors='k')
        return points



    def show_mpld3(self,fig,ax,points,xl_list,xl_labels):
        import mpld3
        from mpld3 import plugins
        import pandas as pd

        # Define some CSS to control our custom labels
        css = """
        table
        {
          border-collapse: collapse;
        }
        th
        {
          color: #000000;
          background-color: #ffffff;
        }
        td
        {
          background-color: #cccccc;
        }
        table, th, td
        {
          font-family:Arial, Helvetica, sans-serif;
          border: 1px solid black;
          text-align: right;
          font-size: 10px;
        }
        """
        df = pd.DataFrame(index=xl_labels)

        sorted_keys=sorted(xl_list[0].keys())

        for k in sorted_keys:
            df[k] = np.array([xl[k] for xl in xl_list])

        labels = []
        for i in range(len(xl_labels)):
            label = df.ix[[i], :].T
            # .to_html() is unicode; so make leading 'u' go away with str()
            labels.append(str(label.to_html()))

        tooltip = plugins.PointHTMLTooltip(points, labels,
                                   voffset=10, hoffset=10, css=css)
        plugins.connect(fig, tooltip)
        mpld3.save_html(fig,"output.html")

    def save_xl_distances(self,filename):

        import csv

        data=[]
        sorted_ids=None
        sorted_group_ids=sorted(self.cross_link_db.data_base.keys())
        for group in sorted_group_ids:
            group_block=[]
            group_dists=[]
            for xl in self.cross_link_db.data_base[group]:
                if not sorted_ids:
                    sorted_ids=sorted(xl.keys())
                    data.append(sorted_ids+["UniqueID","Distance","MinAmbiguousDistance"])
                (c1,c2,r1,r2)=IMP.pmi.io.crosslink._ProteinsResiduesArray(xl)
                try:
                   (mdist,p1,p2)=self._get_distance_and_particle_pair(r1,c1,r2,c2)
                except:
                   mdist=0.0
                values=[xl[k] for k in sorted_ids]
                values+=[group,mdist]
                group_dists.append(mdist)
                group_block.append(values)
            for l in group_block:
                l.append(min(group_dists))
            data+=group_block

        with open(filename, 'w') as fp:
            a = csv.writer(fp, delimiter=',')
            a.writerows(data)

    def save_rmf_snapshot(self,filename,color_id=None):
        if color_id is None:
           color_id=self.cross_link_db.id_score_key
        sorted_ids=None
        sorted_group_ids=sorted(self.cross_link_db.data_base.keys())
        list_of_pairs=[]
        color_scores=[]
        for group in sorted_group_ids:
            group_xls=[]
            group_dists_particles=[]
            for xl in self.cross_link_db.data_base[group]:
                xllabel=self.cross_link_db.get_short_cross_link_string(xl)
                (c1,c2,r1,r2)=IMP.pmi.io.crosslink._ProteinsResiduesArray(xl)
                print (c1,c2,r1,r2)
                try:
                    (mdist,p1,p2)=self._get_distance_and_particle_pair(r1,c1,r2,c2)
                except TypeError:
                    print("TypeError or missing chain/residue ",r1,c1,r2,c2)
                    continue
                group_dists_particles.append((mdist,p1,p2,xllabel,float(xl[color_id])))
            if group_dists_particles:
                (minmdist,minp1,minp2,minxllabel,mincolor_score)=min(group_dists_particles, key = lambda t: t[0])
                color_scores.append(mincolor_score)
                list_of_pairs.append((minp1,minp2,minxllabel,mincolor_score))
            else:
                continue

         
        m=self.prots[0].get_model()
        linear = IMP.core.Linear(0, 0.0)
        linear.set_slope(1.0)
        dps2 = IMP.core.DistancePairScore(linear)
        rslin = IMP.RestraintSet(m, 'linear_dummy_restraints')
        sgs=[]
        offset=min(color_scores)
        maxvalue=max(color_scores)
        for pair in list_of_pairs:
            pr = IMP.core.PairRestraint(m, dps2, (pair[0], pair[1]))
            pr.set_name(pair[2])
            factor=(pair[3]-offset)/(maxvalue-offset)
            print(factor)
            c=IMP.display.get_rgb_color(factor)
            seg=IMP.algebra.Segment3D(IMP.core.XYZ(pair[0]).get_coordinates(),IMP.core.XYZ(pair[1]).get_coordinates())
            rslin.add_restraint(pr)
            sgs.append(IMP.display.SegmentGeometry(seg,c,pair[2]))

        rh = RMF.create_rmf_file(filename)
        IMP.rmf.add_hierarchies(rh, self.prots)
        IMP.rmf.add_restraints(rh,[rslin])
        IMP.rmf.add_geometries(rh, sgs)
        IMP.rmf.save_frame(rh)
        del rh


    def plot_table(self, prot_listx=None,
                   prot_listy=None,
                   no_dist_info=False,
                   confidence_info=False,
                   filter=None,
                   display_residue_pairs=False,
                   contactmap=False,
                   filename=None,
                   confidence_classes=None,
                   alphablend=0.1,
                   scale_symbol_size=1.0,
                   gap_between_components=0,
                   dictionary_of_gaps={},
                   colormap=cm.binary,
                   crosslink_threshold=None,
                   colornorm=None,
                   cbar_labels=None,
                   labels=False,
                   show_mpld3=False,
                   color_crosslinks_by_distance=True):
        """ plot the xlink table with optional contact map.
        prot_listx:             list of protein names on the x-axis
        prot_listy:             list of protein names on the y-axis
        no_dist_info:           plot only the cross-links as grey spots
        confidence_info:
        filter:                 list of tuples to filter on. each one contains:
                                    keyword in the database to be filtered on
                                    relationship ">","==","<"
                                    a value
                                example ("ID_Score",">",40)
        display_residue_pairs:  display all pairs defined in self.residue_pair_list
        contactmap:             display the contact map
        filename:               save to file (adds .pdf extension)
        confidence_classes:
        alphablend:
        scale_symbol_size:      rescale the symbol for the crosslink
        gap_between_components: the numbeber of residues to leave blannk between each component
        dictionary_of_gaps:     add extra space after the given protein. dictionary_of_gaps={prot_name:extra_gap}
        """
        # prepare figure


        fig = plt.figure(figsize=(100, 100))
        ax = fig.add_subplot(111)
        ax.set_xticks([])
        ax.set_yticks([])

        if cbar_labels is not None:
            if len(cbar_labels)!=4:
                print("to provide cbar labels, give 3 fields (first=first input file, last=last input) in oppose order of input contact maps")
                exit()
        # set the list of proteins on the x axis
        if prot_listx is None:
            prot_listx = self.sequence_dict.keys()
            prot_listx.sort()
        nresx = gap_between_components + \
            sum([len(self.sequence_dict[name])
                + gap_between_components for name in prot_listx]) + \
            sum([dictionary_of_gaps[prot] for prot in dictionary_of_gaps.keys()])

        # set the list of proteins on the y axis
        if prot_listy is None:
            prot_listy = self.sequence_dict.keys()
            prot_listy.sort()
        nresy = gap_between_components + \
            sum([len(self.sequence_dict[name])
                + gap_between_components for name in prot_listy]) + \
            sum([dictionary_of_gaps[prot] for prot in dictionary_of_gaps.keys()])

        # this is the residue offset for each protein
        resoffsetx = {}
        resendx = {}
        res = gap_between_components
        for prot in prot_listx:
            resoffsetx[prot] = res
            res += len(self.sequence_dict[prot])
            resendx[prot] = res
            res += gap_between_components
            if prot in dictionary_of_gaps.keys(): res+= dictionary_of_gaps[prot]

        resoffsety = {}
        resendy = {}
        res = gap_between_components
        for prot in prot_listy:
            resoffsety[prot] = res
            res += len(self.sequence_dict[prot])
            resendy[prot] = res
            res += gap_between_components
            if prot in dictionary_of_gaps.keys(): res+= dictionary_of_gaps[prot]

        resoffsetdiagonal = {}
        res = gap_between_components
        for prot in IMP.pmi.io.utilities.OrderedSet(prot_listx + prot_listy):
            resoffsetdiagonal[prot] = res
            res += len(self.sequence_dict[prot])
            res += gap_between_components
            if prot in dictionary_of_gaps.keys(): res+= dictionary_of_gaps[prot]

        # plot protein boundaries
        xticks = []
        xlabels = []
        for n, prot in enumerate(prot_listx):
            res = resoffsetx[prot]
            end = resendx[prot]
            for proty in prot_listy:
                resy = resoffsety[proty]
                endy = resendy[proty]
                ax.plot([res, res], [resy, endy], linestyle='-',color='gray', lw=0.4)
                ax.plot([end, end], [resy, endy], linestyle='-',color='gray', lw=0.4)
            xticks.append((float(res) + float(end)) / 2)
            xlabels.append(prot)

        yticks = []
        ylabels = []
        for n, prot in enumerate(prot_listy):
            res = resoffsety[prot]
            end = resendy[prot]
            for protx in prot_listx:
                resx = resoffsetx[protx]
                endx = resendx[protx]
                ax.plot([resx, endx], [res, res], linestyle='-',color='gray', lw=0.4)
                ax.plot([resx, endx], [end, end], linestyle='-',color='gray', lw=0.4)
            yticks.append((float(res) + float(end)) / 2)
            ylabels.append(prot)

        # plot the contact map
        if contactmap:
            tmp_array = np.zeros((nresx, nresy))
            for px in prot_listx:
                for py in prot_listy:
                    resx = resoffsetx[px]
                    lengx = resendx[px] - 1
                    resy = resoffsety[py]
                    lengy = resendy[py] - 1
                    indexes_x = self.index_dict[px]
                    minx = min(indexes_x)
                    maxx = max(indexes_x)
                    indexes_y = self.index_dict[py]
                    miny = min(indexes_y)
                    maxy = max(indexes_y)
                    tmp_array[resx:lengx,resy:lengy] = self.contact_freqs[minx:maxx,miny:maxy]

            cax = ax.imshow(tmp_array,
                      cmap=colormap,
                      norm=colornorm,
                      origin='lower',
                      alpha=0.6,
                      interpolation='nearest')

        ax.set_xticks(xticks)
        ax.set_xticklabels(xlabels, rotation=90)
        ax.set_yticks(yticks)
        ax.set_yticklabels(ylabels)
        plt.setp(ax.get_xticklabels(), fontsize=30)
        plt.setp(ax.get_yticklabels(), fontsize=30)

        # set the crosslinks
        already_added_xls = []
        xl_coordinates_tuple_list = []
        xl_labels = []
        x_list=[]
        y_list=[]
        color_list=[]
        xl_list=[]

        markersize = 5 * scale_symbol_size
        for xl in self.cross_link_db:

            (c1,c2,r1,r2)=IMP.pmi.io.crosslink._ProteinsResiduesArray(xl)
            label=xl[self.cross_link_db.unique_sub_id_key]
            if color_crosslinks_by_distance:

                try:
                    mdist=self._get_distance(r1,c1,r2,c2)
                    if mdist is None: continue
                    color = self._colormap_distance(mdist,threshold=crosslink_threshold)
                except KeyError:
                    color="gray"

            else:

                try:
                    ps=self._get_percentage_satisfaction(r1,c1,r2,c2)
                    if ps is None: continue
                    color = self._colormap_satisfaction(ps,threshold=0.2,tolerance=0.1)
                except KeyError:
                    color="gray"

            try:
                pos1 = r1 + resoffsetx[c1]
            except:
                continue
            try:
                pos2 = r2 + resoffsety[c2]
            except:
                continue


            # everything below is used for plotting the diagonal
            # when you have a rectangolar plots
            pos_for_diagonal1 = r1 + resoffsetdiagonal[c1]
            pos_for_diagonal2 = r2 + resoffsetdiagonal[c2]
            if confidence_info:
                if confidence == '0.01':
                    markersize = 14 * scale_symbol_size
                elif confidence == '0.05':
                    markersize = 9 * scale_symbol_size
                elif confidence == '0.1':
                    markersize = 6 * scale_symbol_size
                else:
                    markersize = 15 * scale_symbol_size
            else:
                markersize = 5 * scale_symbol_size
            '''
            ax.plot([pos1],
                    [pos2],
                    'o',
                    c=color,
                    alpha=alphablend,
                    markersize=markersize)

            ax.plot([pos2],
                    [pos1],
                    'o',
                    c=color,
                    alpha=alphablend,
                    markersize=markersize)
            '''
            x_list.append(pos1)
            x_list.append(pos2)
            y_list.append(pos2)
            y_list.append(pos1)
            color_list.append(color)
            color_list.append(color)
            xl["Distance"]=mdist
            xl_list.append(xl)
            xl_list.append(xl)

            xl_labels.append(label)
            xl_coordinates_tuple_list.append((float(pos1),float(pos2)))
            xl_labels.append(label+"*")
            xl_coordinates_tuple_list.append((float(pos2),float(pos1)))

        points=ax.scatter(x_list,y_list,s=markersize,c=color_list,alpha=alphablend)

        # plot requested residue pairs
        if display_residue_pairs:
            for rp in self.residue_pair_list:
                r1=rp[0]
                c1=rp[1]
                r2=rp[2]
                c2=rp[3]

                try:
                    dist=self._get_distance(r1,c1,r2,c2)
                except:
                    continue

                if dist<=40.0:
                    print(rp)
                    try:
                        pos1 = r1 + resoffsetx[c1]
                    except:
                        continue
                    try:
                        pos2 = r2 + resoffsety[c2]
                    except:
                        continue

                    ax.plot([pos1],
                            [pos2],
                            '+',
                            c="blue",
                            alpha=0.1,
                            markersize=markersize)

        # display and write to file
        fig.set_size_inches(0.002 * nresx, 0.002 * nresy)
        [i.set_linewidth(2.0) for i in ax.spines.itervalues()]
        if cbar_labels is not None:
            cbar = fig.colorbar(cax, ticks=[0.5,1.5,2.5,3.5])
            cbar.ax.set_yticklabels(cbar_labels)# vertically oriented colorbar

        # set the labels using spring layout
        if labels:
            points=self.spring_layout(ax,plt,xl_coordinates_tuple_list, xl_labels)

        if filename:
            #plt.show()
            plt.savefig(filename, dpi=300,transparent="False")


        if show_mpld3:
            self.show_mpld3(fig,ax,points,xl_list,xl_labels)
