Add support for rejecting BD moves that result in collisions



Add a list of predicates that must be satisfied to accept a move. We can then implement a SingletonPredicate that applies a PairPredicate to close pairs. Need to implement

 - changes in BD
 -  expose a ScoreState that allows fetching of near neighbors of particles
 -  a few PairPredicates to check for collisions (RB based and single particle)
 -  a SingletonPredicate that implements the above.

relates #443