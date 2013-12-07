from xml.dom import minidom
import sys
import IMP.algebra
import IMP.multifit


class Marker:

    def __init__(self, x, y, z, r, i):
        self.x = x
        self.y = y
        self.z = z
        self.radius = r
        self.ind = i
        self.r = 0.
        self.g = 0.
        self.b = 0.

    def __str__(self):
        return "(" + str(self.x) + "," + str(self.y) + "," + str(self.z) + ")"


def load_anchor_graph(cmm_filename):
    try:
        cmm_doc = minidom.parse(cmm_filename)
    except IOError as xxx_todo_changeme:
        (e) = xxx_todo_changeme
        print e
        sys.exit(-1)
    positions = IMP.algebra.Vector3Ds()
    links = []
    for node in cmm_doc.getElementsByTagName('marker'):
        positions.append(IMP.algebra.Vector3D(
            float(node.attributes["x"].value),
            float(node.attributes["y"].value),
            float(node.attributes["z"].value)))
    g = IMP.multifit.ProbabilisticAnchorGraph(positions)
    for link in cmm_doc.getElementsByTagName('link'):
        g.add_edge(int(link.attributes["id1"].value),
                   int(link.attributes["id2"].value))
    return g
