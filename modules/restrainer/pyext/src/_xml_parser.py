import os
import xml.dom.minidom
from _representation import Representation
from _representation import _RepUniverse
from _representation import _RepCollection
from _representation import _RepAssembly
from _representation import _RepMolecule
from _representation import _RepProtein
from _representation import _RepNucleicAcid
from _representation import _RepChain
from _representation import _RepFragment
from _representation import _RepAtomicRep
from _representation import _RepGeometricShapeRep
from _representation import _RepSphere
from _representation import _RepInitialPosition
from _restraint import Restraint
from _restraint import _RestraintY2H
from _restraint import _RestraintPulldown
from _restraint import _RestraintArray
from _restraint import _RestraintMSMS
from _restraint import _RestraintXrayStruc
from _restraint import _RestraintCopurification
from _restraint import _RestraintCrossLink
from _restraint import _RestraintDistance
from _restraint import _RestraintDiameter
from _restraint import _RestraintSAXS
from _restraint import _RestraintSANS
from _restraint import _RestraintEM
from _restraint import _RestraintExcludedVolume
from _restraint import _RestraintRestraint
from _restraint import _RestraintParticle
from _restraint import _RestraintSource
from _restraint import _RestraintAuthor
from _restraint import _RestraintJournal
from _restraint import _RestraintTitle
from _restraint import _RestraintYear
from _restraint import _RigidBodyRestraintNode
from _display import Display
from _display import _DisplayNode
from _display import _DisplayColor
from _display import _DisplayResidue
from _display import _DisplayUniverse
from _display import _DisplayCollection
from _display import _DisplayAssembly
from _display import _DisplaySegment
from _display import _DisplayMolecule
from _display import _DisplayProtein
from _display import _DisplayNucleicAcid
from _display import _DisplayChain
from _display import _DisplayFragment
from _optimization import Optimization
from _optimization import _OptimizationConjugateGradients
from _optimization import _OptimizationRestraint


class XMLRepresentation(object):

    """Construct Representation from XML file"""

    def __init__(self, filename):
        # Define a dictionary with tags as keys and functions as values
        self.handlers = {
            'Universe': self._handle_universe,
            'Collection': self._handle_collection,
            'Assembly': self._handle_assembly,
            'Segment': self._handle_segment,
            'Molecule': self._handle_molecule,
            'Protein': self._handle_protein,
            'NucleicAcid': self._handle_nucleic_acid,
            'Chain': self._handle_chain,
            'Fragment': self._handle_fragment,
            'AtomicRep': self._handle_atomic_rep,
            'GeometricShapeRep': self._handle_geometric_shape_rep,
            'Sphere': self._handle_sphere,
            'InitialPosition': self._handle_initial_position}
        self.filename = filename
        self.base_dir = os.path.dirname(filename)
        _document = open(filename).read()
        self.dom = xml.dom.minidom.parseString(_document)
        self.depth = 0

    def run(self):
        """Return
           <a href="classIMP_1_1restrainer_1_1representation_1_1Representation.html">
           Representation</a>
           object such that each node in the representation
           corresponds to the node in the XML nodes"""
        try:
            representation_dom = self.dom.getElementsByTagName(
                'Representation')[0]
        except:
            print "\"%s\" does not contain <Representation> tag." % (
                self.filename)
            print "Please check the input file.\nExit..."
            raise
        result = Representation()
        for node in representation_dom.childNodes:
            r = self._handle_node(node)
            if r:
                result._children.append(r)
        return result

    def _handle_node(self, node):
        # Return a proper function for the node
        handler = self.handlers.get(node.nodeName, self._handle_nothing)
        return handler(node)

    def _get_attributes(self, node):
        # To store node attributes in python dictionary
        attr_dict = dict()
        attr_map = node.attributes
        if attr_map:
            for i in xrange(attr_map.length):
                attr = attr_map.item(i)
                attr_name = str(attr.name)
                # make sure filenames are relative to XML directory
                if attr_name.endswith('filename'):
                    attr_value = os.path.abspath(os.path.join(self.base_dir,
                                                              str(attr.value)))
                else:
                    attr_value = str(attr.value)
                attr_dict[attr_name] = attr_value
        return attr_dict

    def _print_node_info(self, node):
        # Printing node info for debugging purposes
        attrs = self._get_attributes(node)
        attr_list = ['%s=%s' %
                     (at_name, at_val) for (at_name, at_val) in attrs.iteritems()]
        print '%s%s' % ('  ' * (self.depth + 1), ','.join(attr_list))

    def _handle_nothing(self, node):
        # To ignore unwanted nodes
        pass

    def _handle_universe(self, node):
        # To create object for universe and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepUniverse(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_collection(self, node):
        # To create object for collection and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepCollection(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_assembly(self, node):
        # To create object for assembly and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepAssembly(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_segment(self, node):
        # To create object for segment and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepSegment(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_molecule(self, node):
        # To create object for molecule and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepMolecule(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_protein(self, node):
        # To create object for protein and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepProtein(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_nucleic_acid(self, node):
        # To create object for nucleic acid and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepNucleicAcid(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_chain(self, node):
        # To create object for chain and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepChain(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_fragment(self, node):
        # To create object for fragment and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepFragment(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_atomic_rep(self, node):
        # To create object for atomic rep and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepAtomicRep(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_geometric_shape_rep(self, node):
        # To create object for geometric shape rep and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepGeometricShapeRep(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_sphere(self, node):
        # To create object for sphere and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepSphere(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_initial_position(self, node):
        # To create object for initial position and all of its descendents
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RepInitialPosition(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result


class XMLDisplay(object):

    """Construct Display from XML file"""

    def __init__(self, filename):
        self.handlers = {
            'Universe': self._handle_universe,
            'Collection': self._handle_collection,
            'Assembly': self._handle_assembly,
            'Segment': self._handle_segment,
            'Molecule': self._handle_molecule,
            'Protein': self._handle_protein,
            'NucleicAcid': self._handle_nucleic_acid,
            'Chain': self._handle_chain,
            'Fragment': self._handle_fragment,
            'Color': self._handle_color,
            'Residue': self._handle_residue}
        _document = open(filename).read()
        self.dom = xml.dom.minidom.parseString(_document)
        self.depth = 0

    def run(self):
        """Return
           <a href="classIMP_1_1restrainer_1_1display_1_1Display.html">
           Display</a> object such that each node in the display
           corresponds to the node in the XML nodes"""
        display_dom = self.dom.getElementsByTagName('Display')[0]
        result = Display()
        for node in display_dom.childNodes:
            r = self._handle_node(node)
            if r:
                result._children.append(r)
        return result

    def _handle_node(self, node):
        handler = self.handlers.get(node.nodeName, self._handle_nothing)
        return handler(node)

    def _get_attributes(self, node):
        attr_dict = dict()
        attr_map = node.attributes
        if attr_map:
            for i in xrange(attr_map.length):
                attr = attr_map.item(i)
                attr_dict[str(attr.name)] = str(attr.value)
        return attr_dict

    def _print_node_info(self, node):
        attrs = self._get_attributes(node)
        attr_list = ['%s=%s' %
                     (at_name, at_val) for (at_name, at_val) in attrs.iteritems()]
        print '%s%s' % ('  ' * (self.depth + 1), ','.join(attr_list))

    def _handle_nothing(self, node):
        pass

    def _handle_universe(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplayUniverse(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_collection(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplayCollection(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_assembly(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplayAssembly(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_segment(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplaySegment(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_molecule(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplayMolecule(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_protein(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplayProtein(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_nucleic_acid(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplayNucleicAcid(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_chain(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplayChain(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_fragment(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplayFragment(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_residue(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplayResidue(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_color(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _DisplayColor(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result


class XMLRestraint(object):

    """Construct Restraint from XML file"""

    def __init__(self, filename):
        self.handlers = {
            'Y2H': self._handle_y2h,
            'Pulldown': self._handle_pulldown,
            'RigidBody': self._handle_rigidbody,
            'XrayStruc': self._handle_xray_struc,
            'MSMS': self._handle_msms,
            'Array': self._handle_array,
            'Copurification': self._handle_copurification,
            'CrossLink': self._handle_crosslink,
            'Distance': self._handle_distance,
            'Diameter': self._handle_diameter,
            'ExcludedVolume': self._handle_excluded_volume,
            'SAS': self._handle_sas,
            'SAXS': self._handle_saxs,
            'SANS': self._handle_sans,
            'EM': self._handle_em,
            'Restraint': self._handle_restraint,
            'Particle': self._handle_particle,
            'Source': self._handle_source,
            'Author': self._handle_author,
            'Journal': self._handle_journal,
            'Title': self._handle_title,
            'Year': self._handle_year}
        self.base_dir = os.path.dirname(filename)
        _document = open(filename).read()
        self.dom = xml.dom.minidom.parseString(_document)
        self.depth = 0

    def run(self):
        """Return
           <a href="classIMP_1_1restrainer_1_1restraint_1_1Restraint.html">
           Restraint</a>
           object such that each node in the restraint
           corresponds to the node in the XML nodes"""
        restraint_dom = self.dom.getElementsByTagName('RestraintSet')[0]
        result = Restraint()
        for node in restraint_dom.childNodes:
            r = self._handle_node(node)
            if r:
                result._children.append(r)
        return result

    def _handle_node(self, node):
        handler = self.handlers.get(node.nodeName, self._handle_nothing)
        return handler(node)

    def _get_text(self, node):
        return node.firstChild.wholeText

    def _get_attributes(self, node):
        attr_dict = dict()
        attr_map = node.attributes
        if attr_map:
            for i in xrange(attr_map.length):
                attr = attr_map.item(i)
                attr_name = str(attr.name)
                # make sure filenames are relative to XML directory
                if attr_name.endswith('filename'):
                    attr_value = os.path.abspath(os.path.join(self.base_dir,
                                                              str(attr.value)))
                else:
                    attr_value = str(attr.value)
                attr_dict[attr_name] = attr_value
        return attr_dict

    def _print_node_info(self, node):
        attrs = self._get_attributes(node)
        attr_list = ['%s=%s' %
                     (at_name, at_val) for (at_name, at_val) in attrs.iteritems()]
        print '%s%s' % ('  ' * (self.depth + 1), ','.join(attr_list))

    def _handle_nothing(self, node):
        pass

    def _handle_y2h(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintY2H(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_rigidbody(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RigidBodyRestraintNode(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_excluded_volume(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintExcludedVolume(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_pulldown(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintPulldown(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_xray_struc(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintXrayStruc(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_msms(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintMSMS(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_array(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintArray(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_copurification(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintCopurification(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_crosslink(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintCrossLink(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_distance(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintDistance(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_diameter(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintDiameter(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_sas(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintSAS(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_saxs(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintSAXS(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_sans(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintSANS(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_em(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintEM(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_restraint(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintRestraint(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_particle(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintParticle(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_source(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintSource(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_author(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        result = _RestraintAuthor(attrs)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_journal(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        text = self._get_text(node)
        result = _RestraintJournal(attrs, text)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_title(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        text = self._get_text(node)
        result = _RestraintTitle(attrs, text)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_year(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        text = self._get_text(node)
        result = _RestraintYear(attrs, text)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result


class XMLOptimization(object):

    """Construct Optimization from XML file"""

    def __init__(self, filename):
        self.handlers = {
            'ConjugateGradients': self._handle_conjugate_gradients,
            'Restraint': self._handle_restraint}
        _document = open(filename).read()
        self.dom = xml.dom.minidom.parseString(_document)
        self.depth = 0

    def run(self):
        opt_dom = self.dom.getElementsByTagName('Optimization')[0]
        result = Optimization()
        for node in opt_dom.childNodes:
            r = self._handle_node(node)
            if r:
                result._children.append(r)
        return result

    def _handle_node(self, node):
        handler = self.handlers.get(node.nodeName, self._handle_nothing)
        return handler(node)

    def _get_attributes(self, node):
        attr_dict = dict()
        attr_map = node.attributes
        if attr_map:
            for i in xrange(attr_map.length):
                attr = attr_map.item(i)
                attr_dict[str(attr.name)] = str(attr.value)
        return attr_dict

    def _get_text(self, node):
        return ''

    def _print_node_info(self, node):
        attrs = self._get_attributes(node)
        attr_list = ['%s=%s' %
                     (at_name, at_val) for (at_name, at_val) in attrs.iteritems()]
        print '%s%s' % ('  ' * (self.depth + 1), ','.join(attr_list))

    def _handle_nothing(self, node):
        pass

    def _handle_conjugate_gradients(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        text = self._get_text(node)
        result = _OptimizationConjugateGradients(attrs, text)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result

    def _handle_restraint(self, node):
        self.depth += 1
        attrs = self._get_attributes(node)
        text = self._get_text(node)
        result = _OptimizationRestraint(attrs, text)
        for child in node.childNodes:
            r = self._handle_node(child)
            if r:
                result._children.append(r)
        self.depth -= 1
        return result
