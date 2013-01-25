## \example domino/marina_party.py
##
## This is a not very serious example that shows how to use domino from scratch to
## solve a problem. It is illustrative of creating your own type of particles,
## a pair score, particle states for DOMINO, a decorator, and using subset filter
## tables with DOMINO.
##
## Solution of the Marina Party Problem with DOMINO:
##
## - Girls attend to a party
## - Each girl has dresses of different price
## - There can not be repeated dresses in the party within a group of connected
##   people
## - Girls that are friends can call each other to agree
## - The objective is to maximize the total price of all dresses in the party
##
##


import IMP
import IMP.domino as domino
import IMP.core as core
import random


class SumPricePairScore(IMP.PairScore):

    def evaluate(self, pair, accum):
        price0 = Price(pair[0]).get_price()
        price1 = Price(pair[1]).get_price()
        return price0+price1

    def _do_get_inputs(self, m, pis):
        return [m.get_particle(i) for i in pis]

class PriceStates(domino.ParticleStates):

    def __init__(self, prices):
        self.prices = prices
        domino.ParticleStates.__init__(self)

    def load_particle_state(self, i, particle):
        pr = Price(particle)
        pr.set_price(self.prices[i])

    def get_number_of_particle_states(self):
        return len(self.prices)


class Price(IMP.Decorator):
    price_key = IMP.IntKey("price")

    def __init__(self, p):
        if not self.particle_is_instance(p):
            self.setup_particle(p)
        self.particle = p

    def setup_particle(self, p):
        p.add_attribute(self.price_key, 0)
        self.particle = p

    def get_price(self):
        return self.particle.get_value(self.price_key)

    def set_price(self, x):
        self.particle.set_value(self.price_key, int(x))

    def particle_is_instance(self, p):
        return p.has_attribute(self.price_key)



def get_total_price(states_table, subset, assignment):
    total_price = 0
    for i, p in enumerate(subset):
        price_states = states_table.get_particle_states(p)
        price_states.load_particle_state(assignment[i], p)
        price = Price(p).get_price()
        total_price += price
    return total_price


def print_assignment(states_table, subset, assignment):
    total_price = 0
    print "########## solution assignment", assignment
    for i, p in enumerate(subset):
        price_states = states_table.get_particle_states(p)
        price_states.load_particle_state(assignment[i], p)
        price = Price(p).get_price()
        print p.get_name(), "price", price


n_girls = 10
n_edges = 20
girls = []
prices = [100, 200, 400, 600, 800 ]


model = IMP.Model()

# prepare filter tables for DOMINO
states_table = domino.ParticleStatesTable()
sampler = domino.DominoSampler(model,states_table)
all_possible_states = PriceStates(prices)


# set_states
for i in range(n_girls):
    p = IMP.Particle(model, "girl-%d" % i)
    pr = Price(p)
    girls.append(p)
    # add possible dresses
    states_table.set_particle_states(p, all_possible_states)

    # dresses
    if len(prices) == 1:
        n_dresses = 1
    else:
        n_dresses = random.randrange(1,len(prices)+1)

    # Each girl has a selection of dresses
    selection = random.sample(prices, n_dresses)
    allowed_states_indices = [prices.index(price) for price in selection]
    print p.get_name(), "prices selected",selection, "indices", allowed_states_indices
    list_states_table = domino.ListSubsetFilterTable(states_table)
    list_states_table.set_allowed_states(p, allowed_states_indices)
    sampler.add_subset_filter_table(list_states_table)

# create restraints
for z in xrange(n_edges):
    # pair of friends
    i = random.randrange(0,n_girls)
    j = random.randrange(0,n_girls)
    friends = IMP.ParticlePair(girls[i], girls[j])
    # restraint
    score =  SumPricePairScore()
    r = core.PairRestraint(score, friends)
    model.add_restraint(r)
    # Exclusion states. Two girls can't have same dress
    ft = domino.ExclusionSubsetFilterTable()
    ft.add_pair(friends)
    sampler.add_subset_filter_table(ft)

subset = states_table.get_subset()
solutions = sampler.get_sample_assignments(subset)

if len(solutions) == 0:
    print "There are no solutions to the problem"
else:
    most_expensive = 0
    best_solution  = solutions[0]
    for assignment in solutions:
        total_price = get_total_price(states_table, subset, assignment)
        if(total_price > most_expensive):
            most_expensive = total_price
            best_solution = assignment
    print " There are", len(solutions), "possible solutions"
    print "=================> BEST SOLUTION <=============="
    print_assignment(states_table, subset, best_solution)
