from __future__ import print_function
import IMP
import RMF
import IMP.atom
import IMP.rmf
import IMP.pmi
import IMP.pmi.analysis
import IMP.pmi.output
from collections import defaultdict


class TopologyPlot(object):
    """A class to read RMF files and make a network contact map"""
    def __init__(self, model, selections, cutoff, frequency_cutoff,
                 colors=None, fixed=None, pos=None, proteomic_edges=None,
                 quantitative_proteomic_data=None):
        """Set up a new graphXL object
        @param model          The IMP model
        @param selection_dict A dictionary containing component names.
                              Keys are labels
                              values are either moleculename or
                              start,stop,moleculename
        @param cutoff        The distance cutoff
        @param frequency_cutoff The frequency cutoff
        @param colors        A dictionary of colors (HEX code,values)
                             for subunits (keywords)
        @param fixed         A list of subunits that are kept fixed
        @param pos           A dictionary with positions (tuple, values)
                             of subunits (keywords)
        @param proteomic_edges A list edges to represent proteomic data
        @param quantitative_proteomic_data A dictionary of edges to represent
               quantitative proteomic data such as PE Scores,
               or genetic interactions
        """
        import itertools

        self.model = model
        self.selections = selections
        self.contact_counts = {}
        self.edges = defaultdict(int)
        for (name1, name2) in itertools.combinations(self.selections.keys(),
                                                     2):
            self.edges[tuple(sorted((name1, name2)))] = 0
        self.cutoff = cutoff
        self.frequency_cutoff = frequency_cutoff
        self.gcpf = IMP.core.GridClosePairsFinder()
        self.gcpf.set_distance(self.cutoff)
        self.names = list(self.selections.keys())
        self.colors = colors
        self.fixed = fixed
        self.pos = pos
        self.proteomic_edges = proteomic_edges
        self.quantitative_proteomic_data = quantitative_proteomic_data
        self.num_rmf = 0

    def add_rmf(self, rmf_fn, nframe):
        """Add selections from an RMF file"""
        print('reading from RMF file', rmf_fn)
        rh = RMF.open_rmf_file_read_only(rmf_fn)
        prots = IMP.rmf.create_hierarchies(rh, self.model)
        hier = prots[0]
        IMP.rmf.load_frame(rh, RMF.FrameID(0))
        ps_per_component = defaultdict(list)
        if self.num_rmf == 0:
            self.size_per_component = defaultdict(int)
        self.model.update()

        # gathers particles for all components
        part_dict = IMP.pmi.analysis.get_particles_at_resolution_one(hier)
        all_particles_by_resolution = []
        for name in part_dict:
            all_particles_by_resolution += part_dict[name]

        for component_name in self.selections:
            for seg in self.selections[component_name]:
                if type(seg) == str:
                    s = IMP.atom.Selection(hier, molecule=seg)
                elif type(seg) == tuple:
                    s = IMP.atom.Selection(
                        hier, molecule=seg[2],
                        residue_indexes=range(seg[0], seg[1] + 1))
                else:
                    raise Exception('could not understand selection tuple '
                                    + str(seg))
                parts = list(set(s.get_selected_particles())
                             & set(all_particles_by_resolution))
                ps_per_component[component_name] += IMP.get_indexes(parts)
                if self.num_rmf == 0:
                    self.size_per_component[component_name] += \
                        sum(len(IMP.pmi.tools.get_residue_indexes(p))
                            for p in parts)

        for n1, name1 in enumerate(self.names):
            for name2 in self.names[n1+1:]:
                ncontacts = len(self.gcpf.get_close_pairs(
                    self.model, ps_per_component[name1],
                    ps_per_component[name2]))
                if ncontacts > 0:
                    self.edges[tuple(sorted((name1, name2)))] += 1.0

        self.num_rmf += 1

    def make_plot(self, groups, out_fn, quantitative_proteomic_data=False):
        '''
        plot the interaction matrix
        @param groups is the list of groups of domains, eg,
                      [["protA_1-10","prot1A_11-100"],["protB"]....]
                      it will plot a space between different groups
        @param out_fn name of the plot file
        @param quantitative_proteomic_data plot the quantitative proteomic data
        '''
        import matplotlib.pyplot as plt

        ax = plt.gca()
        ax.set_aspect('equal', 'box')
        ax.xaxis.set_major_locator(plt.NullLocator())
        ax.yaxis.set_major_locator(plt.NullLocator())

        largespace = 0.6
        smallspace = 0.5
        squaredistance = 1.0
        squaresize = 0.99
        domain_xlocations = {}
        domain_ylocations = {}

        xoffset = squaredistance
        yoffset = squaredistance
        for group in groups:
            xoffset += largespace
            yoffset += largespace
            for subgroup in group:
                xoffset += smallspace
                yoffset += smallspace
                for domain in subgroup:
                    domain_xlocations[domain] = xoffset
                    domain_ylocations[domain] = yoffset
                    xoffset += squaredistance
                    yoffset += squaredistance

        for edge, count in self.edges.items():

            if quantitative_proteomic_data:
                # normalize
                maxqpd = max(self.quantitative_proteomic_data.values())
                minqpd = min(self.quantitative_proteomic_data.values())
                if edge in self.quantitative_proteomic_data:
                    value = self.quantitative_proteomic_data[edge]
                elif (edge[1], edge[0]) in self.quantitative_proteomic_data:
                    value = self.quantitative_proteomic_data[(edge[1],
                                                              edge[0])]
                else:
                    value = 0.0
                print(minqpd, maxqpd)
                density = (1.0-(value-minqpd)/(maxqpd-minqpd))
            else:
                density = (1.0-float(count)/self.num_rmf)
            color = (density, density, 1.0)
            x = domain_xlocations[edge[0]]
            y = domain_ylocations[edge[1]]
            if x > y:
                xtmp = y
                ytmp = x
                x = xtmp
                y = ytmp
            rect = plt.Rectangle([x - squaresize / 2, y - squaresize / 2],
                                 squaresize, squaresize,
                                 facecolor=color, edgecolor='Gray',
                                 linewidth=0.1)
            ax.add_patch(rect)
            rect = plt.Rectangle([y - squaresize / 2, x - squaresize / 2],
                                 squaresize, squaresize,
                                 facecolor=color, edgecolor='Gray',
                                 linewidth=0.1)
            ax.add_patch(rect)

        ax.autoscale_view()
        plt.savefig(out_fn)
        plt.show()
        exit()

    def make_graph(self, out_fn):
        edges = []
        weights = []
        print('num edges', len(self.edges))
        for edge, count in self.edges.items():
            # filter if frequency of contacts is greater than frequency_cutoff
            if float(count)/self.num_rmf > self.frequency_cutoff:
                print(count, edge)
                edges.append(edge)
                weights.append(count)
        for nw, w in enumerate(weights):
            weights[nw] = float(weights[nw])/max(weights)
        IMP.pmi.output.draw_graph(edges,
                                  node_size=dict(self.size_per_component),
                                  node_color=self.colors,
                                  fixed=self.fixed,
                                  pos=self.pos,
                                  edge_thickness=1,  # weights,
                                  edge_alpha=0.3,
                                  edge_color='gray',
                                  out_filename=out_fn,
                                  validation_edges=self.proteomic_edges)


def draw_component_composition(DegreesOfFreedom, max=1000,
                               draw_pdb_names=False, show=True):
    """A function to plot the representation on the sequence
    @param DegreesOfFreedom input a IMP.pmi.dof.DegreesOfFreedom instance"""
    import matplotlib as mpl
    mpl.use('Agg')
    from matplotlib import pyplot
    from operator import itemgetter
    import IMP.pmi.tools

    # first build the movers dictionary
    movers_mols_res = IMP.pmi.tools.OrderedDict()
    for mv in DegreesOfFreedom.movers_particles_map:
        hs = DegreesOfFreedom.movers_particles_map[mv]
        res = []

        for h in hs:
            if IMP.atom.Residue.get_is_setup(h):
                res = [IMP.atom.Residue(h).get_index()]
            elif IMP.atom.Fragment.get_is_setup(h):
                res = IMP.atom.Fragment(h).get_residue_indexes()
            if res:
                is_molecule = False
                hp = h
                while not is_molecule:
                    hp = hp.get_parent()
                    is_molecule = IMP.atom.Molecule.get_is_setup(hp)
                name = (IMP.atom.Molecule(hp).get_name() + "."
                        + str(IMP.atom.Copy(hp).get_copy_index()))
                if mv not in movers_mols_res:
                    movers_mols_res[mv] = IMP.pmi.tools.OrderedDict()
                    movers_mols_res[mv][name] = IMP.pmi.tools.Segments(res)
                else:
                    if name not in movers_mols_res[mv]:
                        movers_mols_res[mv][name] = IMP.pmi.tools.Segments(res)
                    else:
                        movers_mols_res[mv][name].add(res)
    # then get all movers by type
    fb_movers = []
    rb_movers = []
    srb_movers = []
    for mv in movers_mols_res:
        if type(mv) is IMP.core.RigidBodyMover:
            rb_movers.append(mv)
        if type(mv) is IMP.core.BallMover:
            fb_movers.append(mv)
        if type(mv) is IMP.pmi.TransformMover:
            srb_movers.append(mv)

    # now remove residue assigned to BallMovers from RigidBodies
    for mv in fb_movers:
        for mol in movers_mols_res[mv]:
            for i in movers_mols_res[mv][mol].get_flatten():
                for mv_rb in rb_movers:
                    try:
                        movers_mols_res[mv_rb][mol].remove(i)
                    except (ValueError, KeyError):
                        continue

    elements = {}
    rgb_colors = [(240, 163, 255), (0, 117, 220),
                  (153, 63, 0), (76, 0, 92), (25, 25, 25),
                  (0, 92, 49), (43, 206, 72), (255, 204, 153),
                  (128, 128, 128), (148, 255, 181), (143, 124, 0),
                  (157, 204, 0), (194, 0, 136), (0, 51, 128), (255, 164, 5),
                  (255, 168, 187), (66, 102, 0), (255, 0, 16), (94, 241, 242),
                  (0, 153, 143), (224, 255, 102), (116, 10, 255), (153, 0, 0),
                  (255, 255, 128), (255, 255, 0), (255, 80, 5)]
    colors = [(float(c0)/255, float(c1)/255, float(c2)/255)
              for (c0, c1, c2) in rgb_colors]
    mvrb_color = {}
    for mv in movers_mols_res:
        if type(mv) is IMP.core.RigidBodyMover:
            if mv not in mvrb_color:
                color = colors[len(mvrb_color.keys())]
                mvrb_color[mv] = color
            for mol in movers_mols_res[mv]:
                if mol not in elements:
                    elements[mol] = []
                for seg in movers_mols_res[mv][mol].segs:
                    try:
                        elements[mol].append((seg[0], seg[-1], " ", "pdb",
                                              mvrb_color[mv]))
                    except (KeyError, IndexError):
                        continue
        if type(mv) is IMP.core.BallMover:
            for mol in movers_mols_res[mv]:
                if mol not in elements:
                    elements[mol] = []
                for seg in movers_mols_res[mv][mol].segs:
                    elements[mol].append((seg[0], seg[-1], " ", "bead"))

    # sort everything
    for mol in elements:
        elements[mol].sort(key=lambda tup: tup[0])
        elements[mol].append((elements[mol][-1][1],
                              elements[mol][-1][1], " ", "end"))

    for name in elements:
        k = name
        tmplist = sorted(elements[name], key=itemgetter(0))
        try:
            endres = tmplist[-1][1]
        except IndexError:
            print("draw_component_composition: missing information for "
                  "component %s" % name)
            return
        fig = pyplot.figure(figsize=(26.0 * float(endres) / max + 2, 2))
        ax = fig.add_axes([0.05, 0.475, 0.9, 0.15])

        # Set the colormap and norm to correspond to the data for which
        # the colorbar will be used.
        cmap = mpl.cm.cool
        norm = mpl.colors.Normalize(vmin=5, vmax=10)
        bounds = [1]
        colors = []

        for n, l in enumerate(tmplist):
            if l[3] != "end":
                if bounds[-1] != l[0]:
                    colors.append("white")
                    bounds.append(l[0])
                    if l[3] == "pdb":
                        colors.append("#99CCFF")
                    if l[3] == "bead":
                        colors.append("white")
                    if l[3] == "helix":
                        colors.append("#33CCCC")
                    if l[3] != "end":
                        bounds.append(l[1] + 1)
                else:
                    if l[3] == "pdb":
                        colors.append(l[4])
                    if l[3] == "bead":
                        colors.append("white")
                    if l[3] == "helix":
                        colors.append("#33CCCC")
                    if l[3] != "end":
                        bounds.append(l[1] + 1)
            else:
                if bounds[-1] - 1 == l[0]:
                    bounds.pop()
                    bounds.append(l[0])
                else:
                    colors.append("white")
                    bounds.append(l[0])

        bounds.append(bounds[-1])
        colors.append("white")
        cmap = mpl.colors.ListedColormap(colors)
        cmap.set_over('0.25')
        cmap.set_under('0.75')

        norm = mpl.colors.BoundaryNorm(bounds, cmap.N)
        cb2 = mpl.colorbar.ColorbarBase(ax, cmap=cmap,
                                        norm=norm,
                                        # to use 'extend', you must
                                        # specify two extra boundaries:
                                        boundaries=bounds,
                                        ticks=bounds,  # optional
                                        spacing='proportional',
                                        orientation='horizontal')

        extra_artists = []
        npdb = 0

        if draw_pdb_names:
            for tl in tmplist:
                if tl[3] == "pdb":
                    npdb += 1
                    mid = 1.0 / endres * float(tl[0])
                    t = ax.annotate(
                        tl[2], xy=(mid, 1),  xycoords='axes fraction',
                        xytext=(mid + 0.025, float(npdb - 1) / 2.0 + 1.5),
                        textcoords='axes fraction',
                        arrowprops=dict(
                            arrowstyle="->",
                            connectionstyle="angle,angleA=0,angleB=90,rad=10"))
                    extra_artists.append(t)

        # set the title of the bar
        title = ax.text(-0.005, 0.5, k, ha="right", va="center", rotation=90,
                        size=20)

        extra_artists.append(title)
        # changing the xticks labels
        labels = len(bounds) * [" "]
        ax.set_xticklabels(labels)
        mid = 1.0 / endres * float(bounds[0])
        t = ax.annotate(bounds[0], xy=(mid, 0),  xycoords='axes fraction',
                        xytext=(mid - 0.01, -0.5), textcoords='axes fraction',)
        extra_artists.append(t)
        offsets = [0, -0.5, -1.0]
        nclashes = 0
        for n in range(1, len(bounds)):
            if bounds[n] == bounds[n - 1]:
                continue
            mid = 1.0 / endres * float(bounds[n])
            if (float(bounds[n]) - float(bounds[n - 1])) / max <= 0.01:
                nclashes += 1
                offset = offsets[nclashes % 3]
            else:
                nclashes = 0
                offset = offsets[0]
            if offset > -0.75:
                t = ax.annotate(bounds[n], xy=(mid, 0),
                                xycoords='axes fraction',
                                xytext=(mid, -0.5 + offset),
                                textcoords='axes fraction')
            else:
                t = ax.annotate(bounds[n], xy=(mid, 0),
                                xycoords='axes fraction',
                                xytext=(mid, -0.5 + offset),
                                textcoords='axes fraction',
                                arrowprops=dict(arrowstyle="-"))
            extra_artists.append(t)

        cb2.add_lines(bounds, ["black"] * len(bounds), [1] * len(bounds))
        # cb2.set_label(k)

        pyplot.savefig(k + "structure.pdf")
        if show:
            pyplot.show()
