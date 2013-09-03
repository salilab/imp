"""Interface between XML Representation and IMP Model."""

import IMP
import IMP.atom
import random
import math

class Representation(object):
    """Store Representation."""

    def __init__(self):
        """ """
        self._children = list()
        self._model = None

    def get_imp_hierarchy_by_id(self, id):
        """Return an IMP::atom::Hierarchy by particle id.
           @param id Particle id.
           @return An IMP::atom::Hierarchy hierarchy."""
        return self.find_by_id(id).model_decorator

    def get_root_imp_hierarchy(self):
        """Return the root of the IMP::atom::Hierarchy"""
        return self.model_decorator

    def find_all_by_id(self, id): # assuming there are many obj with the same id
        """Return a list of all nodes that have the id given by the parameter"""

        def _find_rec(node):
            if node.id == id:
                found.append(node)
            for child in node._children:
                _find_rec(child)

        found = list()
        for child in self._children:
            _find_rec(child)
        return found

    def find_by_id(self, id): # assuming there is just one obj with the same id
        """Return a node that have the id given by the parameter"""

        def _find_rec(node):
            if node.id == id:
                return node
            for child in node._children:
                r = _find_rec(child)
                if r:
                    return r
            return None

        for child in self._children:
            r = _find_rec(child)
            if r:
                return r
        return None

    def get_model(self, model=None):
        """Return an IMP::Model that contains the representation"""
        if model is None:
            model = self._model
            if model is None:
                self._model = model = IMP.kernel.Model()
            else:
                return model
        else:
            self._model = model
        repr_particle = IMP.kernel.Particle(model)
        decorator = IMP.atom.Hierarchy.setup_particle(repr_particle)
        self.model_decorator = decorator
        for child in self._children:
            child.set_parent(self)
        for child in self._children:
            child.add_as_child(decorator, model)
        return model

    def __str__(self):
        return '<Representation>\n%s\n</Representation>' %\
            ('\n'.join([child._to_str(1) for child in self._children]))


class _RepresentationNode(object):
    counter = 0
    def __init__(self, attributes):
        id = attributes.get('id')
        if id:
            self.id = id
        else:
            self.id = 'object_%d' % _RepresentationNode.counter
            _RepresentationNode.counter += 1
        self._children = list()
        self.model_decorator = None
        self.parent = None
        self.force_field = 0

    def set_parent(self, parent):
        self.parent = parent
        for child in self._children:
            child.set_parent(self)

    def add_as_child(self, particle, model):
        """Add this node as a child of particle."""
        #print 'add_as_child', self
        if not self.model_decorator:
            decorator = self.to_particle(model)
            self.model_decorator = decorator
        #print 'model_decorator:',[self.model_decorator]
        if decorator:
            particle.add_child(decorator)
        else:
            decorator = particle
        for child in self._children:
            child.add_as_child(decorator, model)

    def to_particle(self, model):
        self.this_particle = IMP.kernel.Particle(model)
        decorator = IMP.atom.Hierarchy.setup_particle(self.this_particle)
        return decorator

    def _attr_to_str(self):
        return ('RepresentationNode', 'id="%s"' % self.id)

    def _to_str(self, level):
        indent = '  '*level
        name, strattr = self._attr_to_str()
        if not self._children:
            return '%s<%s %s/>' % (indent, name, strattr)
        else:
            return '%s<%s %s>\n%s\n%s</%s>' %\
                (indent, name, strattr,
                '\n'.join([child._to_str(level + 1)
                  for child in self._children]), indent, name)

    def __str__(self):
        return self._to_str(0)


class _RepUniverse(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)

    def _attr_to_str(self):
        return ('Universe', 'id="%s"' % self.id)


class _RepCollection(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)

    def _attr_to_str(self):
        return ('Collection', 'id="%s"' % self.id)


class _RepAssembly(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)

    def _attr_to_str(self):
        return ('Assembly', 'id="%s"' % self.id)


class _RepSegment(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)

    def _attr_to_str(self):
        return ('Segment', 'id="%s"' % self.id)


class _RepMolecule(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)

    def _attr_to_str(self):
        return ('Molecule', 'id="%s"' % self.id)


class _RepProtein(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)

    def _attr_to_str(self):
        return ('Protein', 'id="%s"' % self.id)


class _RepNucleicAcid(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)

    def _attr_to_str(self):
        return ('NucleicAcid', 'id="%s"' % self.id)


class _RepChain(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)
        self.filename = attributes.get('filename', '')
        self.chain_label = attributes.get('chain_label', '')
        self.selector =  attributes.get('selector', '')
        if self.filename:
            self.force_field = int(attributes.get('force_field', '1'))
        else:
            self.force_field = 0
        self.topology_file = attributes.get('topology_filename', IMP.atom.get_data_path('top_heav.lib'))
        self.param_file = attributes.get('param_filename', IMP.atom.get_data_path('par.lib'))

    def _attr_to_str(self):
        return ('Chain',
            'id="%s" filename="%s" chain_label="%s" selector="%s"' %
            (self.id, self.filename, self.chain_label, self.selector))

    def to_particle(self, model):
        if self.filename:
            if self.selector == 'CAlpha':
                selector = IMP.atom.CAlphaPDBSelector()
            elif self.selector == 'CBeta':
                selector = IMP.atom.CBetaPDBSelector()
            elif self.selector == 'C':
                selector = IMP.atom.CPDBSelector()
            elif self.selector == 'N':
                selector = IMP.atom.NPDBSelector()
            elif self.selector == 'All':
                selector = IMP.atom.NonAlternativePDBSelector()
            elif self.selector == 'Chain':
                selector = IMP.atom.ChainPDBSelector(self.chain_label)
            elif self.selector == 'Water':
                selector = IMP.atom.WaterPDBSelector()
            elif self.selector == 'Hydrogen':
                selector = IMP.atom.HydrogenPDBSelector()
            elif self.selector == 'NonWater':
                selector = IMP.atom.NonWaterPDBSelector()
            elif self.selector == 'P':
                selector = IMP.atom.PPDBSelector()
            elif self.selector == 'NonAlternatives':
                selector = IMP.atom.NonAlternativesPDBSelector()
            elif self.selector == 'NonWaterNonHydrogen':
                selector = IMP.atom.NonWaterPDBSelector()
            else:
                selector = IMP.atom.NonWaterNonHydrogenPDBSelector()
            decorator = IMP.atom.read_pdb(self.filename, model, selector)
            IMP.atom.add_radii(decorator)
            chains = IMP.atom.get_by_type(decorator, IMP.atom.CHAIN_TYPE)
            self.fragment_decorator = chains[0]
            parent = self.fragment_decorator.get_parent()
            parent.remove_child(self.fragment_decorator)
        if self._children and not self.filename:
            particle = IMP.kernel.Particle(model)
            decorator = IMP.atom.Chain.setup_particle(particle,
                self.chain_label)
        else:
            if not self.filename and not self._children:
                raise Exception, "Filename must be present for childless Chain %s" % self.id
            decorator = self.fragment_decorator
        return decorator

class _RepFragment(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)

    def _attr_to_str(self):
        return ('Fragment', 'id="%s"' % self.id)

    def to_particle(self, model):
        if len(self._children) != 1:
            raise Exception, "Fragment %s must have exactly one child" % self.id
        particle = IMP.kernel.Particle(model)
        child = self._children[0]
        child.add_attributes(particle)
        decorator = IMP.atom.Fragment.setup_particle(particle)
        if isinstance(child, _RepAtomicRep):
            if not isinstance(self.parent, _RepChain):
                raise Exception, "Parent of Fragment %s must be a chain" % self.id
            if particle.has_attribute(IMP.IntKey('start_residue')):
                start_residue = particle.get_value(IMP.IntKey('start_residue'))
            else:
                raise Exception, "Start residue is required for atomic rep of Fragment %s" % self.id
            if particle.has_attribute(IMP.IntKey('end_residue')):
                end_residue = particle.get_value(IMP.IntKey('end_residue'))
            else:
                raise Exception, "End residue is required for atomic rep of Fragment %s" % self.id
            for x in xrange(int(start_residue), int(end_residue) + 1):
                res_part = IMP.atom.get_residue(self.parent.fragment_decorator, x)
                if res_part != IMP.atom.Hierarchy():
                    res_parent = res_part.get_parent()
                    res_parent.remove_child(res_part)
                    decorator.add_child(res_part)
        else:
            if not particle.has_attribute(IMP.FloatKey('x')):
                particle.add_attribute(IMP.FloatKey('x'), random.uniform(-300, 300))
                particle.set_is_optimized(IMP.FloatKey('x'), True)
            if not particle.has_attribute(IMP.FloatKey('y')):
                particle.add_attribute(IMP.FloatKey('y'), random.uniform(-300, 300))
                particle.set_is_optimized(IMP.FloatKey('y'), True)
            if not particle.has_attribute(IMP.FloatKey('z')):
                particle.add_attribute(IMP.FloatKey('z'), random.uniform(-300, 300))
                particle.set_is_optimized(IMP.FloatKey('z'), True)

            if particle.get_value(IMP.FloatKey('radius')) < 0:
                particle.set_value(IMP.FloatKey('radius'),
                particle.get_value(IMP.FloatKey('calc_radius')))
        return decorator


class _RepAtomicRep(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)
        self.start_residue = int(attributes.get('start_residue', -1))
        if not self.start_residue:
            self.start_residue = int(attributes.get('start_nucleotide', -1))
        self.end_residue = int(attributes.get('end_residue', -1))
        if not self.end_residue:
            self.end_residue = int(attributes.get('end_nucleotide', -1))
        if self.start_residue < 0 or self.end_residue < 0:
            raise Exception, "AtomicRep %s must have both start_(residue|nucleotide) and end_(residue|nucleotide)" % self.id

    def _attr_to_str(self):
        return ('AtomicRep',
          'id="%s" start_residue="%s" end_residue="%s"' %
          (self.id, self.start_residue, self.end_residue))

    def to_particle(self, model):
        return None

    def add_attributes(self, parent):
        parent.add_attribute(IMP.IntKey("start_residue"), self.start_residue)
        parent.add_attribute(IMP.IntKey("end_residue"), self.end_residue)
        for child in self._children:
            child.add_attributes(parent)

class _RepGeometricShapeRep(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)
        self.start_residue = int(attributes.get('start_residue', -1))
        if self.start_residue < 0:
            self.start_residue = int(attributes.get('start_nucleotide', -1))
        self.end_residue = int(attributes.get('end_residue', -1))
        if self.end_residue < 0:
            self.end_residue = int(attributes.get('end_nucleotide', -1))
        self.total_residue = int(attributes.get('total_residue', -1))
        if self.total_residue >= 0 and self.end_residue >= 0 and \
            self.start_residue >= 0:
            if self.total_residue != self.end_residue + 1 - self.start_residue:
                raise Exception, "Total residues dubious consistency in Geometric Shape _Rep %s" % self.id

    def _attr_to_str(self):
        return ('GeometricShapeRep',
          'id="%s" start_residue="%s" end_residue="%s" total_residue="%s"' %
          (self.id, self.start_residue, self.end_residue, self.total_residue))

    def to_particle(self, model):
        return None

    def add_attributes(self, parent):
        parent.add_attribute(IMP.IntKey('start_residue'), self.start_residue)
        parent.add_attribute(IMP.IntKey('end_residue'), self.end_residue)
        r = -1
        if self.start_residue < 0 and self.end_residue < 0:
            total_residue = self.total_residue
        else:
            total_residue = self.end_residue-self.start_residue+1
        if total_residue >= 0:
            m = IMP.atom.get_mass_from_number_of_residues(total_residue)
            v = IMP.atom.get_volume_from_mass(m)
            r = (v/(4.0*math.pi)*3.0)**(1.0/3)
            parent.add_attribute(IMP.FloatKey('calc_radius'), r)
        for child in self._children:
            child.add_attributes(parent)


class _RepSphere(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)
        self.radius = float(attributes.get('radius', -1))
        self.weight = float(attributes.get('weight', 0))
        self.__initial_position = None

    def _attr_to_str(self):
        return ('Sphere',
          'id="%s" radius="%s" weight="%s"' %
          (self.id, self.radius, self.weight))

    def initial_position(self):
        if self.__initial_position is None:
            for child in self._children:
                if isinstance(child, _RepInitialPosition):
                    self.__initial_position = child
                    break
        return self.__initial_position

    def to_particle(self, model):
        return None

    def add_attributes(self, parent):
        parent.add_attribute(IMP.FloatKey("radius"), self.radius)
        parent.add_attribute(IMP.FloatKey("weight"), self.weight)
        parent.add_attribute(IMP.FloatKey("mass"), self.weight)
        for child in self._children:
            child.add_attributes(parent)

class _RepInitialPosition(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)
        self.optimize = int(attributes.get('optimize', -1))
        self.x = float(attributes['x'])
        self.y = float(attributes['y'])
        self.z = float(attributes['z'])

    def _attr_to_str(self):
        return ('InitialPosition',
          'id="%s" x="%s" y="%s" z="%s" optimize="%s"' %
          (self.id, self.x, self.y, self.z, self.optimize))

    def to_particle(self, model):
        return None

    def add_attributes(self, parent):
        fl_x = IMP.FloatKey("x")
        fl_y = IMP.FloatKey("y")
        fl_z = IMP.FloatKey("z")
        parent.add_attribute(fl_x, self.x)
        parent.add_attribute(fl_y, self.y)
        parent.add_attribute(fl_z, self.z)
        parent.add_attribute(IMP.IntKey("optimize"), self.optimize)
        if self.optimize == 1:
            parent.set_is_optimized(fl_x, True)
            parent.set_is_optimized(fl_y, True)
            parent.set_is_optimized(fl_z, True)
