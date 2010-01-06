import IMP.core
import IMP.em
import IMP.saxs
import IMP.helper

class _RestraintSets(object):
    def __init__(self):
        self.restraint_sets = dict()
        self.set_by_restraint = dict()
    def add(self, restraint_type, restraint, model = None):
        if restraint_type is None:
            return
        try:
            current_set = self.restraint_sets[restraint_type]
        except KeyError:
            current_set = self.restraint_sets[restraint_type] = IMP.core.RestraintSet()
            if model:
                model.add_restraint(current_set)
        current_set.add_restraint(restraint)
        self.set_by_restraint[restraint] = current_set
    def set_model(self, model):
        for weight, rest_set in self.restraint_sets.iteritems():
            model.add_restraint(rest_set)
            rest_set.set_weight(float(weight))
    def remove_restraint(self, restraint):
        try:
            current_set = self.set_by_restraint[restraint]
        except KeyError:
            return
        current_set.remove_restraint(restraint)
    def reset_weight(self, new_weight):
        try:
            current_set = self.restraint_sets[str(new_weight)]
        except KeyError:
            return
        current_set.set_weight(float(new_weight))

class _Restraint(object):
    def __init__(self):
        self.children = list()
        self.restraint_sets = _RestraintSets()

    def add_to_representation(self, repr):
        if repr.model is None:
            repr.to_model()
        for child in self.children:
            child.create_restraint(repr, self.restraint_sets)
        self.restraint_sets.set_model(repr.model)
        self.set_root()
        self.model = repr.model


    def get_all_restraints_by_name(self, name): # assuming there are many obj with the same id

        def find_rec(node):
            if node.name == name:
                found.append(node)
            for child in node.children:
                find_rec(child)

        found = list()
        for child in self.children:
            find_rec(child)
        return found

    def get_restraint_by_name(self, name): # assuming there is just one obj with the same id

        def find_rec(node):
            if node.name == name:
                return node
            for child in node.children:
                r = find_rec(child)
                if r:
                    return r
            return None

        for child in self.children:
            r = find_rec(child)
            if r:
                return r
        return None

    def set_root(self):

        def set_root_rec(node):
            node.root = self
            for child in node.children:
                set_root_rec(child)

        for child in self.children:
            set_root_rec(child)


class _RestraintNode(object):
    counter = 0

    def __init__(self, attributes, restraint_type=None):
        name = attributes.get('name')
        self.weight = float(attributes.get('weight', 1))
        if name:
            self.name = name
        else:
            self.name = 'object_%d' % _RestraintNode.counter
            _RestraintNode.counter += 1
        self.children = list()
        self.child_restraints = list()
        self.restraint_type = restraint_type

    def get_particle(self):
        particle_list = list()
        for child in self.children:
            if isinstance(child, _RestraintParticle):
                particle_list.append(child)
        return particle_list

    def get_source(self):
        source_list = list()
        for child in self.children:
            counter = 0
            if isinstance(child, _RestraintSource):
                source_list.append(child)
                source_list[counter].author_list = child.get_author()
                source_list[counter].journal = child.get_journal()
                source_list[counter].title   = child.get_title()
                source_list[counter].year    = child.get_year()
            counter += 1
        return source_list

    def create_restraint(self, repr, restraint_sets):
        for child in self.children:
            restraint = child.create_restraint(repr, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)
                if self.restraint_type:
                    restraint_sets.add(str(self.weight), restraint)

class _RigidBodyRestraintNode(_RestraintNode):
    def __init__(self, attributes):
        _RestraintNode.__init__(self, attributes, 'RigidBody')

    def create_restraint(self, repr, restraint_sets):
        for child in self.children:
            restraint = child.create_rigid_body_restraint(repr, restraint_sets)

class _ExcludedVolumeRestraintNode(_RestraintNode):
    def __init__(self, attributes, restraint_type):
        _RestraintNode.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr, restraint_sets):
        for child in self.children:
            restraint = child.create_excluded_volume_restraint(repr, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)
                if self.restraint_type:
                    restraint_sets.add(str(self.weight), restraint)

class _ConnectivityRestraintNode(_RestraintNode):
    def __init__(self, attributes, restraint_type):
        _RestraintNode.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr, restraint_sets):
        for child in self.children:
            restraint = child.create_connectivity_restraint(repr, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)
                if self.restraint_type:
                    restraint_sets.add(str(self.weight), restraint)

class _CrosslinkRestraintNode(_RestraintNode):
    def __init__(self, attributes, restraint_type):
        _RestraintNode.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr, restraint_sets):
        for child in self.children:
            restraint = child.create_crosslink_restraint(repr, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)
                if self.restraint_type:
                    restraint_sets.add(str(self.weight), restraint)

class _DistanceRestraintNode(_RestraintNode):
    def __init__(self, attributes, restraint_type):
        _RestraintNode.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr, restraint_sets):
        for child in self.children:
            restraint = child.create_distance_restraint(repr, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)
                if self.restraint_type:
                    restraint_sets.add(str(self.weight), restraint)

class _DiameterRestraintNode(_RestraintNode):
    def __init__(self, attributes, restraint_type):
        _RestraintNode.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr, restraint_sets):
        for child in self.children:
            restraint = child.create_diameter_restraint(repr, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)
                if self.restraint_type:
                    restraint_sets.add(str(self.weight), restraint)

class _SAXSRestraintNode(_RestraintNode):
    def __init__(self, attributes, restraint_type):
        _RestraintNode.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr, restraint_sets):
        for child in self.children:
            restraint = child.create_saxs_restraint(repr, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)
                if self.restraint_type:
                    restraint_sets.add(str(self.weight), restraint)


class _SANSRestraintNode(_RestraintNode):
    def __init__(self, attributes, restraint_type):
        _RestraintNode.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr, restraint_sets):
        pass

class _EMRestraintNode(_RestraintNode):
    def __init__(self, attributes, restraint_type):
        _RestraintNode.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr, restraint_sets):
        for child in self.children:
            restraint = child.create_em_restraint(repr, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)
                if self.restraint_type:
                    restraint_sets.add(str(self.weight), restraint)

class _RestraintY2H(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "Y2H")

class _RestraintExcludedVolume(_ExcludedVolumeRestraintNode):
    def __init__(self, attributes):
        _ExcludedVolumeRestraintNode.__init__(self, attributes, "ExcludedVolume")

class _RestraintPulldown(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "Pulldown")

class _RestraintXrayStruc(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "XrayStruc")

class _RestraintMSMS(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "MSMS")

class _RestraintArray(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "Array")

class _RestraintCopurification(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "Copurification")

class _RestraintCrossLink(_CrosslinkRestraintNode):
    def __init__(self, attributes):
        _CrosslinkRestraintNode.__init__(self, attributes, "CrossLink")

class _RestraintDistance(_DistanceRestraintNode):
    def __init__(self, attributes):
        _DistanceRestraintNode.__init__(self, attributes, "Distance")

class _RestraintDiameter(_DiameterRestraintNode):
    def __init__(self, attributes):
        _DiameterRestraintNode.__init__(self, attributes, "Diameter")

class _RestraintSAXS(_SAXSRestraintNode):
    def __init__(self, attributes):
        _SAXSRestraintNode.__init__(self, attributes, "SAXS")

class _RestraintSANS(_SANSRestraintNode):
    def __init__(self, attributes):
        _SANSRestraintNode.__init__(self, attributes, "SANS")

class _RestraintEM(_EMRestraintNode):
    def __init__(self, attributes):
        _EMRestraintNode.__init__(self, attributes, "EM")

class _RestraintRestraint(_RestraintNode):
    def __init__(self, attributes):
        _RestraintNode.__init__(self, attributes)
        self.imp_restraint = None
        self.std_dev = float(attributes.get('std_dev', 1))
        self.distance = float(attributes.get('distance', -1))
        self.max_diameter = float(attributes.get('max_diameter', -1))
        self.profile_filename = attributes.get('profile_filename', '')
        self.density_filename = attributes.get('density_filename', '')
        self.resolution = float(attributes.get('resolution', -1))
        self.spacing = float(attributes.get('spacing', -1))
        self.xorigin = float(attributes.get('xorigin', 0.0))
        self.yorigin = float(attributes.get('yorigin', 0.0))
        self.zorigin = float(attributes.get('zorigin', 0.0))
        self.linker_length = float(attributes.get('linker_length', -1))

    def set_weight(self, new_weight):
        if self.imp_restraint:
            self.root.restraint_sets.remove_restraint(self.imp_restraint)
            self.root.restraint_sets.add(str(new_weight), self.imp_restraint, self.root.model)
            self.weight = new_weight
            self.root.restraint_sets.reset_weight(new_weight)

    def get_weight(self):
        return self.weight

    def create_rigid_body_restraint(self, repr, restraint_sets):
        _RestraintNode.create_restraint(self, repr, restraint_sets)
        self.mhs = IMP.atom.Hierarchies()
        for child in self.child_restraints:
            self.mhs.append(child)
        self.rigid_bodies = IMP.helper.set_rigid_bodies(self.mhs)
        return None


    def create_connectivity_restraint(self, repr, restraint_sets):
        _RestraintNode.create_restraint(self, repr, restraint_sets)
        if self.std_dev > 0:
            k = 1.0/(2.0*self.std_dev*self.std_dev)
        else:
            k = 1.0
        mhs = IMP.atom.Hierarchies()
        if not self.child_restraints:
            raise Exception, "Restraint %s is empty" % self.id
        for child in self.child_restraints:
            IMP.atom.show_molecular_hierarchy(child)
            mhs.append(child)
        first_particle = self.child_restraints[0].get_particle()
        if IMP.core.RigidBody.particle_is_instance(first_particle):
            rbs_tmp = IMP.Particles()
            for mh in mhs:
                rbs_tmp.append(mh.get_particle())
            rbs = IMP.core.RigidBodies(rbs_tmp)
            self.sc = IMP.helper.create_simple_connectivity_on_rigid_bodies(rbs)
        else:
            self.sc = IMP.helper.create_simple_connectivity_on_molecules(mhs)
        self.sc.set_k(k)
        connectivity_restraint = self.sc.get_restraint()
        self.imp_restraint = connectivity_restraint
        return connectivity_restraint

    def create_excluded_volume_restraint(self, repr, restraint_sets):
        _RestraintNode.create_restraint(self, repr, restraint_sets)
        mhs = IMP.atom.Hierarchies()
        if not self.child_restraints:
            raise Exception, "Restraint %s is empty" % self.id
        for child in self.child_restraints:
            mhs.append(child)
        first_particle = self.child_restraints[0].get_particle()
        if IMP.core.RigidBody.particle_is_instance(first_particle):
            rbs_tmp = IMP.Particles()
            for mh in mhs:
                rbs_tmp.append(mh.get_particle())
            rbs = IMP.core.RigidBodies(rbs_tmp)
            self.sev = IMP.helper.create_simple_excluded_volume_on_rigid_bodies(rbs)
        else:
            self.sev = IMP.helper.create_simple_excluded_volume_on_molecules(mhs)
        ev_restraint = self.sev.get_restraint()
        self.imp_restraint = ev_restraint
        return ev_restraint

    def create_saxs_restraint(self, repr, restraint_sets):
        _RestraintNode.create_restraint(self, repr, restraint_sets)
        if self.profile_filename:
            self.exp_profile = IMP.saxs.Profile(self.profile_filename)
            particles = IMP.atom.Hierarchies()
            for child in self.child_restraints:
                for atoms in IMP.atom.get_by_type(child, IMP.atom.ATOM_TYPE):
                    particles.append(atoms)
            if particles:
                saxs_restraint = IMP.saxs.Restraint(particles, self.exp_profile)
                self.imp_restraint = self.saxs_restraint
                return saxs_restraint

    def create_em_restraint(self, repr, restraint_sets):
        _RestraintNode.create_restraint(self, repr, restraint_sets)
        if self.density_filename:
            mhs = IMP.atom.Hierarchies()
            self.dmap = IMP.helper.load_em_density_map(self.density_filename,
                           self.spacing, self.resolution)
            self.dmap_header = self.dmap.get_header_writable()

            self.dmap.set_origin (self.xorigin, self.yorigin, self.zorigin)
            self.dmap_header.set_spacing (self.spacing)
            self.dmap_header.set_resolution (self.resolution)

            for child in self.child_restraints:
                mhs.append(child)
            if mhs:
                self.sef = IMP.helper.create_simple_em_fit(mhs, self.dmap)
                em_restraint = self.sef.get_restraint()
                self.imp_restraint = em_restraint
                return em_restraint

    def create_crosslink_restraint(self, repr, restraint_sets):
        pass

    def create_distance_restraint(self, repr, restraint_sets):
        _RestraintNode.create_restraint(self, repr, restraint_sets)
        ps = IMP.Particles()
        for child in self.child_restraints:
            ps.append(child.get_particle())
        self.distance_restraint = IMP.helper.create_simple_distance(ps)
        self.imp_restraint = self.distance_restraint.get_restraint()
        return self.imp_restraint

    def create_diameter_restraint(self, repr, restraint_sets):
        _RestraintNode.create_restraint(self, repr, restraint_sets)
        ps = IMP.Particles()
        for child in self.child_restraints:
            ps.append(child.get_particle())
        self.diameter_restraint = IMP.helper.create_simple_diameter(
                                 ps, self.max_diameter)
        self.imp_restraint = self.diameter_restraint.get_restraint()
        return self.imp_restraint

class _RestraintSource(_RestraintNode):
    def __init__(self, attributes):
        _RestraintNode.__init__(self, attributes)
        self.pubmed_id = int(attributes.get('pubmed_id', -1))

        self.__author_list = list()
        self.__journal = None
        self.__title = None
        self.__year = None

    def get_author(self):
        if not self.__author_list:
            for child in self.children:
                if isinstance(child, _RestraintAuthor):
                    self.__author_list.append(child)
                    break
        return self.__author_list

    def get_journal(self):
        if self.__journal is None:
            for child in self.children:
                if isinstance(child, _RestraintJournal):
                    self.__journal = child.text
                    break
        return self.__journal

    def get_title(self):
        if self.__title is None:
            for child in self.children:
                if isinstance(child, _RestraintTitle):
                    self.__title = child.text
                    break
        return self.__title

    def get_year(self):
        if self.__year is None:
            for child in self.children:
                if isinstance(child, _RestraintYear):
                    self.__year = child.text
                    break
        return self.__year

class _RestraintAuthor(_RestraintNode):
    def __init__(self, attributes):
        _RestraintNode.__init__(self, attributes)
        self.first_name = attributes.get('first_name', '')
        self.last_name = attributes.get('last_name', '')

class _RestraintJournal(_RestraintNode):
    def __init__(self, attributes, text):
        _RestraintNode.__init__(self, attributes)
        self.text = text

class _RestraintTitle(_RestraintNode):
    def __init__(self, attributes, text):
        _RestraintNode.__init__(self, attributes)
        self.text = text

class _RestraintYear(_RestraintNode):
    def __init__(self, attributes, text):
        _RestraintNode.__init__(self, attributes)
        self.text = text

class _RestraintParticle(_RestraintNode):
    def __init__(self, attributes):
        _RestraintNode.__init__(self, attributes)
        self.id         = attributes.get('id', '')

    def create_restraint(self, repr, restraint_sets):

        repr_particle = self.find_representation(repr)
        if not repr_particle:
            raise Exception, "Particle with id=%s not found" % (self.id)
        return repr_particle.model_decorator

    def find_representation(self, repr):
        return repr.find_by_id(self.id)
