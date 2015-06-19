'''
This module works jointly with IMP.pmi.io.crosslink
and it is used to plot a contact map overlaid with the cross-links
'''

from math import sqrt
from Bio import SeqIO
from Bio.PDB.PDBParser import PDBParser
import numpy as np
from scipy.spatial.distance import cdist
import matplotlib.pyplot as plt
import matplotlib.cm as cm

from collections import defaultdict
import pickle
import IMP.pmi
import IMP.pmi.io
import IMP.pmi.io.crosslink
import IMP.pmi.io.utilities

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

    def _colormap_distance(self, dist, threshold=35, tolerance=0):
        if dist < threshold - tolerance:
            return "Green"
        elif dist >= threshold + tolerance:
            return "Orange"
        else:
            return "Red"

    def _colormap_satisfaction(self, sat, threshold=0.5, tolerance=0.1):
        if sat >=  threshold + tolerance:
            print sat, "green"
            return "Green"
        elif sat < threshold + tolerance and sat >= threshold - tolerance :
            print sat, "orange"
            return "Orange"
        else:
            print sat, "orange"
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
        try:
            idx1=self.index_dict[c1][r1]
        except:
            return None
        try:
            idx2=self.index_dict[c2][r2]
        except:
            return None
        return self.av_dist_map[idx1,idx2]

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
        handle = open(fasta_file, "rU")
        record_dict = SeqIO.to_dict(SeqIO.parse(handle, "fasta"))
        handle.close()
        if id_in_fasta_file is None:
            id_in_fasta_file = name
        try:
            length = len(record_dict[id_in_fasta_file].seq)
        except KeyError:
            print "add_component_sequence: id %s not found in fasta file" % id_in_fasta_file
            exit()
        self.sequence_dict[protein_name] = str(record_dict[id_in_fasta_file].seq).replace("*", "")

    def load_pdb_coordinates(self,pdbfile,chain_to_name_map):
        """ read coordinates from a pdb file. also appends to distance maps
        @param pdbfile             file for reading coords
        @param chain_to_name_map   correspond chain ID with protein name (will ONLY read these chains)
        \note This function returns an error if the sequence for each chain has NOT been read
        """
        pdbparser = PDBParser()
        structure = pdbparser.get_structure(pdbfile,pdbfile)
        total_len = sum(len(self.sequence_dict[s]) for s in self.sequence_dict)
        coords = np.ones((total_len,3)) * 1e5 #default to coords "very far away"
        prev_stop=0
        for n,model in enumerate(structure):
            for cid in chain_to_name_map:
                cname=chain_to_name_map[cid]
                if cname not in self.sequence_dict:
                    print "ERROR: chain",cname,'has not been read or has a naming mismatch'
                    return
                if self._first:
                    self.index_dict[cname]=range(prev_stop,prev_stop+len(self.sequence_dict[cname]))
                for residue in model[cid]:
                    if "CA" in residue:
                        ca=residue["CA"]
                        rnum=residue.id[1]
                        coords[rnum+prev_stop-1,:]=ca.get_coord()
                    #else:
                    #    print residue
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
        self.num_pdbs+=1

    def load_rmf_coordinates(self,rmf_name,rmf_frame_index, chain_names):
        """ read coordinates from a rmf file. It needs IMP to run.
        rmf has been created using IMP.pmi conventions. It gets the
        highest resolution atomatically. Also appends to distance maps
        @param rmf_name             file for reading coords
        @param rmf_frame_index      frame index from the rmf
        """
        import IMP
        import IMP.atom
        import IMP.pmi
        import IMP.pmi.tools

        self.imp_model=IMP.Model()
        (particles_resolution_one, prots)=self._get_rmf_structure(rmf_name,rmf_frame_index)

        pdbparser = PDBParser()
        total_len = sum(len(self.sequence_dict[s]) for s in self.sequence_dict)


        coords = np.ones((total_len,3)) * 1e5 #default to coords "very far away"
        prev_stop=0
        sorted_particles=IMP.pmi.tools.sort_by_residues(particles_resolution_one)
        print chain_names
        for cname in chain_names:
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
                coords[rnum+prev_stop-1,:]=IMP.core.XYZ(selpart_res_one).get_coordinates()
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
        import IMP.pmi
        import IMP.pmi.analysis
        import IMP.rmf
        import RMF

        rh= RMF.open_rmf_file_read_only(rmf_name)
        prots=IMP.rmf.create_hierarchies(rh, self.imp_model)
        IMP.rmf.load_frame(rh, rmf_frame_index)
        print "getting coordinates for frame %i rmf file %s" % (rmf_frame_index, rmf_name)
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

    def load_crosslinks(self,CrossLinkDataBase):
        """ read crosslinks from a CSV file.
        provide a CrossLinkDataBaseKeywordsConverter to explain the columns"""
        if type(CrossLinkDataBase) is not IMP.pmi.io.crosslink.CrossLinkDataBase:
            raise TypeError("Crosslink database must be a IMP.pmi.io.CrossLinkDataBase type")
        self.cross_link_db=CrossLinkDataBase

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
        upperbound:   maximum distance to be marked
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
            print "UH OH: index dictionaries do not match!"
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
        print 'computing contact map'
        self.contact_freqs = f(contact1,contact2)
        print 'done'


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
                   colormap=cm.binary,
                   crosslink_threshold=None,
                   colornorm=None,
                   cbar_labels=None,
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
        gap_between_components:
        """
        # prepare figure
        fig = plt.figure(figsize=(10, 10))
        ax = fig.add_subplot(111)
        ax.set_xticks([])
        ax.set_yticks([])

        if cbar_labels is not None:
            if len(cbar_labels)!=4:
                print "to provide cbar labels, give 3 fields (first=first input file, last=last input) in oppose order of input contact maps"
                exit()
        # set the list of proteins on the x axis
        if prot_listx is None:
            prot_listx = self.sequence_dict.keys()
            prot_listx.sort()
        nresx = gap_between_components + \
            sum([len(self.sequence_dict[name])
                + gap_between_components for name in prot_listx])

        # set the list of proteins on the y axis
        if prot_listy is None:
            prot_listy = self.sequence_dict.keys()
            prot_listy.sort()
        nresy = gap_between_components + \
            sum([len(self.sequence_dict[name])
                + gap_between_components for name in prot_listy])

        # this is the residue offset for each protein
        resoffsetx = {}
        resendx = {}
        res = gap_between_components
        for prot in prot_listx:
            resoffsetx[prot] = res
            res += len(self.sequence_dict[prot])
            resendx[prot] = res
            res += gap_between_components

        resoffsety = {}
        resendy = {}
        res = gap_between_components
        for prot in prot_listy:
            resoffsety[prot] = res
            res += len(self.sequence_dict[prot])
            resendy[prot] = res
            res += gap_between_components

        resoffsetdiagonal = {}
        res = gap_between_components
        for prot in IMP.pmi.io.utilities.OrderedSet(prot_listx + prot_listy):
            resoffsetdiagonal[prot] = res
            res += len(self.sequence_dict[prot])
            res += gap_between_components

        # plot protein boundaries
        xticks = []
        xlabels = []
        for n, prot in enumerate(prot_listx):
            res = resoffsetx[prot]
            end = resendx[prot]
            for proty in prot_listy:
                resy = resoffsety[proty]
                endy = resendy[proty]
                ax.plot([res, res], [resy, endy], 'k-', lw=0.4)
                ax.plot([end, end], [resy, endy], 'k-', lw=0.4)
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
                ax.plot([resx, endx], [res, res], 'k-', lw=0.4)
                ax.plot([resx, endx], [end, end], 'k-', lw=0.4)
            yticks.append((float(res) + float(end)) / 2)
            ylabels.append(prot)

        # plot the contact map
        if contactmap:
            tmp_array = np.zeros((nresx, nresy))
            for px in prot_listx:
                for py in prot_listy:
                    resx = resoffsety[px]
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

        # set the crosslinks
        already_added_xls = []
        for xl in self.cross_link_db:

            (c1,c2,r1,r2)=IMP.pmi.io.crosslink._ProteinsResiduesArray(xl)

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
                    print rp
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

        if filename:
            plt.savefig(filename, dpi=300,transparent="False")
        #plt.show()
