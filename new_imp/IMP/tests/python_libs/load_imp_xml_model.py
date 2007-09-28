import xml.dom.minidom

# set the appropriate search path
import sys

sys.path.append("/Users/bret/Dev/new_imp/IMP")
sys.path.append("/Users/bret/Dev/new_imp/IMP/tests")
sys.path.append("../")
sys.path.append("../../")
import imp2
import IMP_Modeller_Intf

def load_imp_model(model, xml_file_path, base_particle = 0):
    """ Load imp model from given xml file """
    doc = xml.dom.minidom.parse(xml_file_path)

    # there should only be a single imp node, but just in case ...
    for head in doc.childNodes:
        if (head.nodeName == 'imp'):
            for n in head.childNodes:
                # process particle set nodes
                if n.nodeName == 'particles':
                    load_particles(model, n, base_particle)

                # process restraint set nodes
                elif n.nodeName == 'restraint_set':
                    load_restraint_set(model, n, base_particle)

    return doc


def load_particles(model, particles_node, base_particle):
    """ Load particles from the DOM """
    # save particle pointers to prevent destructors from running
    try:
        model.particles
    except AttributeError:
        print "particle list built"
        model.particles = []
        model.num_particles = 0
    else:
        print "particle list exists"

    # process individual particle nodes
    for part in particles_node.childNodes:
        if (part.nodeName == 'particle'):
            load_particle(model, part, base_particle)


def load_particle(model, particle_node, base_particle):
    """ Load particle from the DOM """
    id = particle_node.getAttribute('id')
    print "Load particle", id
    model.particles.append(IMP_Modeller_Intf.Particle(model, 0.0, 0.0, 0.0))
    p_idx = len(model.particles)-1

    # process particle attributes
    for attr in particle_node.childNodes:
        if attr.nodeName == 'float':
            load_float(model, p_idx, attr)

        elif attr.nodeName == 'int':
            load_int(model, p_idx, attr)

        elif attr.nodeName == 'string':
            load_string(model, p_idx, attr)


def load_float(model, p_idx, attr_node):
    """ Load float attribute from the DOM and add it to the particle """
    name = attr_node.getAttribute('name')
    print "Load float attribute", name
    p = model.particles[p_idx].imp()

    # defaults
    optimize = True
    value = 0.0
    for n in attr_node.childNodes:
        if n.nodeName == 'optimize':
            optimize = (n.childNodes[0].nodeValue == 'T')

        if n.nodeName == 'value':
            value = n.childNodes[0].nodeValue

    print name, value, optimize
    p.add_float(str(name), float(value), bool(optimize))


def load_int(model, p_idx, attr_node):
    """ Load int attribute from the DOM and add it to the particle """
    name = attr_node.getAttribute('name')
    print "Load int attribute", name
    p = model.particles[p_idx].imp()

    # defaults
    value = 0
    for n in attr_node.childNodes:
        if n.nodeName == 'value':
            value = n.childNodes[0].nodeValue

    print name, value
    p.add_int(str(name), int(value))


def load_string(model, p_idx, attr_node):
    """ Load string attribute from the DOM and add it to the particle """
    name = attr_node.getAttribute('name')
    print "Load string attribute", name
    p = model.particles[p_idx].imp()

    # defaults
    value = ""
    for n in attr_node.childNodes:
        if n.nodeName == 'value':
            value = n.childNodes[0].nodeValue

    p.add_string(str(name), str(value))


def load_restraint_set(model, restraint_set_node, base_particle):
    """ Load restraint set from the DOM """

    # save pointers to prevent destructors from running
    try:
        model.restraint_sets
    except AttributeError:
        print "restraint set list built"
        model.restraint_sets = []
    else:
        print "restraint set list exists"

    try:
        model.restraints
    except AttributeError:
        print "restraint list built"
        model.restraints = []
    else:
        print "restraint list exists"

    try:
        model.score_funcs
    except AttributeError:
        print "score functions list built"
        model.score_funcs = {}
    else:
        print "restraint list exists"

    # get restraint set parameters
    name = str(restraint_set_node.getAttribute('name'))
    model.restraint_sets.append(imp2.Restraint_Set(name))
    rs_idx = len(model.restraint_sets) - 1
    model.add_restraint_set(model.restraint_sets[rs_idx])
    print "Load restraint set", name

    # process restraints in subnodes
    for r in restraint_set_node.childNodes:
        if (r.nodeName == 'distance_restraint'):
            load_rsr_distance(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'torus_restraint'):
            load_rsr_torus(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'coordinate_restraint'):
            load_rsr_coordinate(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'connectivity_restraint'):
            load_rsr_connectivity(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'pair_connectivity_restraint'):
            load_rsr_pair_connectivity(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'proximity_restraint'):
            load_rsr_proximity(model, rs_idx, r, base_particle)


def load_rsr_distance(model, rs_idx, rsr, base_particle):
    """ Load distance restraint set from the DOM """
    print "Load distance restraint"

    # process restraint parameters

    # defaults
    distance_attribute = ''
    distance = None
    sd = 0.1
    score_func = None

    # get the parameters
    for param in rsr.childNodes:
        if param.nodeName == 'particle_pairs':
            particle_pairs = get_particle_pairs(param)

        elif param.nodeName == 'distance':
            distance = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'distance_attribute':
            distance_attribute = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func = get_score_func(model, str(param.childNodes[0].nodeValue))

    # make sure we have the needed parameters
    if score_func == None:
        print "No score function specified."
        return

    if distance == None and distance_attribute == '':
        print "No distance specified."
        return

    # add restraint for each particle pair in the list
    for pair in particle_pairs:
        p1 = model.particles[int(pair[0])]
        p2 = model.particles[int(pair[1])]
        if distance_attribute != '':
            distance = p1.get_float(distance_attribute) + p2.get_float(distance_attribute)

        print "adding distance rsr: ", pair[0], pair[1], distance, sd, score_func
        model.restraints.append(imp2.RSR_Distance(model, p1.imp(), p2.imp(), distance, sd, score_func))
        model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])


def load_rsr_torus(model, rs_idx, rsr, base_particle):
    """ Load torus restraint set from the DOM """
    print "Load torus restraint"

    # process restraint parameters

    # defaults
    distance_attribute = ''
    sd = 0.1
    tube_radius = 5.0
    main_radius = 20.0
    score_func = None

    # get the parameters
    for param in rsr.childNodes:
        if param.nodeName == 'particle_list':
            particle_list = get_particle_list(param)

        elif param.nodeName == 'main_radius':
            main_radius = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'tube_radius':
            tube_radius = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func = get_score_func(model, str(param.childNodes[0].nodeValue))

    # make sure we have the needed parameters
    if score_func == None:
        print "No score function specified."
        return

    # add restraint for each particle in the list
    for p_idx in particle_list:
        p = model.particles[int(p_idx)].imp()
        print "adding torus rsr: ", p_idx, main_radius, tube_radius, sd, score_func
        model.restraints.append(imp2.RSR_Torus(model, p, main_radius, tube_radius, sd, score_func))
        model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])


def load_rsr_coordinate(model, rs_idx, rsr, base_particle):
    """ Load coordinate restraint set from the DOM """
    print "Load coordinate restraint"

    # process restraint parameters

    # defaults
    axis = 'X_AXIS'
    sd = 0.1
    distance = 10.0
    score_func = None

    # get the parameters
    for param in rsr.childNodes:
        if param.nodeName == 'particle_list':
            particle_list = get_particle_list(param)

        elif param.nodeName == 'axis_type':
            axis = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'distance':
            distance = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func = get_score_func(model, str(param.childNodes[0].nodeValue))

    # make sure we have the needed parameters
    if score_func == None:
        print "No score function specified."
        return

    # add restraint for each particle in the list
    for p_idx in particle_list:
        p = model.particles[int(p_idx)].imp()
        print "adding coordinate rsr: ", p_idx, distance, sd, score_func
        model.restraints.append(imp2.RSR_Coordinate(model, p, axis, distance, sd, score_func))
        model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])


def load_rsr_connectivity(model, rs_idx, rsr, base_particle):
    """ Load connectivity restraint set from the DOM """
    print "Load connectivity restraint"

    # process restraint parameters

    # defaults
    distance_attribute = ''
    type_attribute = 'protein'
    distance = None
    sd = 0.1
    score_func = None

    # get the parameters
    for param in rsr.childNodes:
        if param.nodeName == 'particle_list':
            particle_list = get_particle_list(param)

        elif param.nodeName == 'distance':
            distance = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'distance_attribute':
            distance_attribute = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'type_attribute':
            type_attribute = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func = get_score_func(model, str(param.childNodes[0].nodeValue))

    # make sure we have the needed parameters
    if distance == None and distance_attribute == '':
        print "No distance specified."
        return

    if score_func == None:
        print "No score function specified."
        return

    # create a vector of particle indexes
    particle_indexes = imp2.vectori()
    for p_idx in particle_list:
        particle_indexes.push_back(int(p_idx))

    # add restraint for for vector of particles
    if distance_attribute:
        print "adding connectivity rsr: ", p_idx, distance_attribute, sd, score_func
        model.restraints.append(imp2.RSR_Connectivity(model, particle_indexes, type_attribute, distance_attribute, sd, score_func))
        model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])
    else:
        print "adding connectivity rsr: ", p_idx, distance_attribute, sd, score_func
        model.restraints.append(imp2.RSR_Connectivity(model, particle_indexes, type_attribute, distance, sd, score_func))
        model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])


def load_rsr_pair_connectivity(model, rs_idx, rsr, base_particle):
    """ Load pair connectivity restraint set from the DOM """
    print "Load  pair connectivity restraint"

    # process restraint parameters

    # defaults
    distance_attribute = ''
    distance = None
    sd = 0.1
    score_func = None
    num_lists = 0
    number_of_connects = 1

    # get the parameters
    for param in rsr.childNodes:
        if num_lists == 0 and param.nodeName == 'particle_list':
            particle_list1 = get_particle_list(param)
            num_lists = 1

        elif param.nodeName == 'particle_list':
            particle_list2 = get_particle_list(param)

        elif param.nodeName == 'distance':
            distance = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'distance_attribute':
            distance_attribute = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func = get_score_func(model, str(param.childNodes[0].nodeValue))

        elif param.nodeName == 'number_of_connects':
            number_of_connects = int(param.childNodes[0].nodeValue)

    # make sure we have the needed parameters
    if distance == None and distance_attribute == '':
        print "No distance specified."
        return

    if score_func == None:
        print "No score function specified."
        return

    # create a vector of particle indexes
    particle_indexes1 = imp2.vectori()
    particle_indexes1.clear()
    for p_idx in particle_list1:
        particle_indexes1.push_back(int(p_idx))

    particle_indexes2 = imp2.vectori()
    particle_indexes2.clear()
    for p_idx in particle_list2:
        particle_indexes2.push_back(int(p_idx))

    # add restraint for for vector of particles
    if distance_attribute:
        print "adding pair connectivity rsr: ", model, particle_indexes1, particle_indexes2, distance_attribute, sd, score_func, number_of_connects
        r = imp2.RSR_Pair_Connectivity(model, particle_indexes1, particle_indexes2, distance_attribute, sd, score_func, number_of_connects)
        model.restraints.append(r)
        model.restraint_sets[rs_idx].add_restraint(r)
    else:
        print "adding pair connectivity rsr: ", distance_attribute, sd, score_func
        r = imp2.RSR_Pair_Connectivity(model, particle_indexes1, particle_indexes2, distance, sd, score_func, number_of_connects)
        model.restraints.append(r)
        model.restraint_sets[rs_idx].add_restraint(r)


def load_rsr_proximity(model, rs_idx, rsr, base_particle):
    """ Load proximity restraint set from the DOM """
    print "Load proximity restraint"

    # process restraint parameters

    # defaults
    distance = 5.0
    sd = 0.1
    score_func = 'harmonic_upper_bound'

    # get the parameters
    for param in rsr.childNodes:
        if param.nodeName == 'particle_list':
            particle_list = get_particle_list(param)

        elif param.nodeName == 'distance':
            distance = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func = get_score_func(model, str(param.childNodes[0].nodeValue))

    # create a vector of particle indexes
    particle_indexes = imp2.vectori()
    for p_idx in particle_list:
        particle_indexes.push_back(int(p_idx))

    # add restraint for for vector of particles
    print "adding proximity rsr: ", p_idx, distance, sd, score_func
    model.restraints.append(imp2.RSR_Proximity(model, particle_indexes, distance, sd, score_func))
    model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])



def get_score_func(model, score_func_str):
    """ Get score function corresponding to the given name """
    try:
        score_func = model.score_funcs[score_func_str]
    except KeyError:
        if score_func_str == 'harmonic':
            model.score_funcs[score_func_str] = imp2.Harmonic()

        elif score_func_str == 'harmonic_lower_bound':
            model.score_funcs[score_func_str] = imp2.Harmonic_Lower_Bound()

        elif score_func_str == 'harmonic_upper_bound':
            model.score_funcs[score_func_str] = imp2.Harmonic_Upper_Bound()

        score_func = model.score_funcs[score_func_str]

    return score_func


# support different ways of specifying particle pairs in the XML.
# <pair>1 3</pair> direct entering of particle pair indexes:
#   (1, 3)
# <all_pairs>3 4 5 6</all_pairs> all possible pairs from a list
#   (3, 4) (3, 5) (3, 6) (4, 5) (4, 6) (5, 6)
def get_particle_pairs(pairs_node):
    """ Get a list of all particle pairs specified by the given node """
    pairs = []
    for pair_node in pairs_node.childNodes:
        if pair_node.nodeName == 'pair':
            pairs.append(str(pair_node.childNodes[0].nodeValue).split())

        if pair_node.nodeName == 'all_pairs':
            list = get_particle_list(pair_node)
            for i in range(0, len(list)-1):
                for j in range(i+1, len(list)):
                    pairs.append((i, j))

    return pairs


# <particle_list>3 4 5 6</particle_list> list (3, 4, 5, 6)
def get_particle_list(particle_list_node):
    """ Get a list of all particles specified by the given node """
    return str(particle_list_node.childNodes[0].nodeValue).split()
