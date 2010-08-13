/**
 *  \file GridClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/GridClosePairsFinder.h"
#include "IMP/core/QuadraticClosePairsFinder.h"
#include "IMP/core/XYZR.h"

#include "IMP/core/internal/ParticleGrid.h"
#include <IMP/internal/Vector.h>
#include <IMP/core/utility.h>

IMPCORE_BEGIN_NAMESPACE

namespace {
  struct ParticleID {
    typedef Particle* result_type;
    Particle* operator()(Particle *p) const {return p;}
  };
  struct ParticleCenter {
    algebra::Vector3D operator()(Particle *p) const {
      return XYZ(p).get_coordinates();
    }
  };
  struct ParticleRadius {
    double operator()(Particle *p) const {
      return XYZR(p).get_radius();
    }
  };
  struct ParticleClose {
    double operator()(Particle *a, Particle *b, double d) const {
      return get_distance(XYZR(a), XYZR(b)) < d;
    }
  };
  struct BBID {
    algebra::BoundingBox3Ds::const_iterator it_;
    BBID(algebra::BoundingBox3Ds::const_iterator it): it_(it){}
    typedef unsigned int result_type;
    unsigned int operator()(const algebra::BoundingBox3D &bb) const {
      return &bb-&*it_;
    }
  };
  struct BBRadius {
    algebra::BoundingBox3Ds::const_iterator it_;
    BBRadius(algebra::BoundingBox3Ds::const_iterator it): it_(it){}
    double operator()(unsigned int i) const {
      return algebra::get_maximum_length(*(it_+i));
    }
  };
  struct BBCenter {
    algebra::BoundingBox3Ds::const_iterator it_;
    BBCenter(algebra::BoundingBox3Ds::const_iterator it): it_(it){}
    algebra::Vector3D operator()(unsigned int i) const {
      return algebra::Vector3D(.5*((it_+i)->get_corner(0)
                                   +(it_+i)->get_corner(1)));
    }
  };
  struct BBClose {
    algebra::BoundingBox3Ds::const_iterator it0_, it1_;
    BBClose(algebra::BoundingBox3Ds::const_iterator it0,
            algebra::BoundingBox3Ds::const_iterator it1): it0_(it0), it1_(it1){}
    double operator()(unsigned int a, unsigned int b, double dist) const {
      algebra::BoundingBox3D ag= *(it0_+a)+dist;
      return algebra::get_interiors_intersect(ag, *(it1_+b));
    }
  };

  double get_side(const algebra::BoundingBox3D &bb, unsigned int nump,
                  double r, double d) {
    return 1.1*(2*r+d);
  }

  std::string do_show(Particle*p) {
    return p->get_name();
  }
  algebra::BoundingBox3D do_show(algebra::BoundingBox3D bb) {
    return bb;
  }
  unsigned int do_show(unsigned int i) {
    return i;
  }

  template <class IDF, class CenterF, class RadiusF, class CloseF, class Out>
  struct Helper {
    typedef typename IDF::result_type ID;
    typedef std::vector<ID> IDs;
    typedef algebra::Grid3D<IDs> Grid;

    template <class It>
    struct ParticleSet {
      It b_, e_;
      IDF id_;
      CenterF c_;
      RadiusF r_;
      ParticleSet(It b, It e, IDF id, CenterF c, RadiusF r): b_(b), e_(e),
      id_(id), c_(c), r_(r){}
    };
    template <class It>
    static ParticleSet<It> get_particle_set(It b, It e,
                                            IDF id, CenterF c, RadiusF r) {
      return ParticleSet<It>(b,e,id, c,r);
    }
    template <class It>
    struct IDSet {
      It b_, e_;
      CenterF c_;
      RadiusF r_;
      IDSet(It b, It e, CenterF c, RadiusF r): b_(b), e_(e),
      c_(c), r_(r){}
    };
    template <class It>
    static IDSet<It> get_id_set(It b, It e,
                                CenterF c, RadiusF r) {
      return IDSet<It>(b,e,c,r);
    }


    static bool get_is_wrapped(unsigned int merged, unsigned int k) {
      switch (k) {
        case 0:
          return merged & GridClosePairsFinder::X;
        case 1:
          return merged & GridClosePairsFinder::Y;
        default:
          return merged & GridClosePairsFinder::Z;
      }
    }




    static bool wrap(const Grid &g,
                     typename Grid::ExtendedIndex bblb,
                     typename Grid::ExtendedIndex bbub,
                     typename Grid::ExtendedIndex center,
                     typename Grid::ExtendedIndex ei,
                     unsigned int merged,
                     bool half,
                     typename Grid::Index &out) {
      int is[3];
      if (merged) {
        IMP_LOG(VERBOSE, "    Wrapping " << ei << " in grid of size "
                << g.get_number_of_voxels(0) << ", "
                << g.get_number_of_voxels(1)
                << ", " << g.get_number_of_voxels(2));
        IMP_LOG(VERBOSE, " in space " << bblb << " to " << bbub << std::endl);
      } else {
        //IMP_LOG(VERBOSE,  std::endl);
      }
      for (unsigned int k=0; k< 3; ++k) {
        is[k]= ei[k];
        if (get_is_wrapped(merged, k)) {
          while (is[k] < bblb[k]) is[k]+=bbub[k]-bblb[k];
          while (is[k] > bbub[k]) is[k]-= bbub[k]-bblb[k];
        }
        if (is[k] <0 || is[k] >= static_cast<int>(g.get_number_of_voxels(k))) {
           if (merged) IMP_LOG(VERBOSE,
                               "    rejected due to outside of grid"
                               << std::endl);
           return false;
        }
      }
      out=typename Grid::Index(is[0], is[1], is[2]);
      if (half && !(out < center)) {
        IMP_LOG(VERBOSE, "    rejected half with center "
                << center << std::endl);
        return false;
      }
      if (out != ei) IMP_LOG(VERBOSE, "    returning " << out << std::endl);
      return true;
    }





    template <class It>
    static void create_grids(const ParticleSet<It> &ps,
                             double distance,
                             typename std::vector<Grid> &bins,
                             typename std::vector<IDs > &bin_contents,
                             typename std::vector<double> &bin_ubs) {
      double maxr=0;

      for (It c= ps.b_; c != ps.e_; ++c) {
        maxr=std::max(ps.r_(ps.id_(*c))+0, maxr);
      }
      bin_ubs.push_back(maxr);
      bin_contents.push_back(IDs());
      for (It c= ps.b_; c != ps.e_; ++c) {
        double cr= ps.r_(ps.id_(*c))+0;
        while (cr < std::max(.5*bin_ubs.back()-.5*distance, 0.0)) {
          // v+distance=.5*(vo+distance)->v= .5*vo-.5*distance
          double v=std::max(.5*bin_ubs.back()-.5*distance, 0.0);
          bin_ubs.push_back(v);
          bin_contents.push_back(IDs());
        }
        for ( int i=bin_ubs.size()-1; i >=0; --i) {
          if (cr <= bin_ubs[i]) {
            bin_contents[i].push_back(ps.id_(*c));
            break;
          }
        }
      }
      for (unsigned int i=0; i< bin_contents.size(); ++i) {
        if (bin_contents[i].empty()) {
          bin_contents.erase(bin_contents.begin()+i);
          bin_ubs.erase(bin_ubs.begin()+i);
        }
      }
      IMP_IF_LOG(VERBOSE) {
        for (unsigned int i=0; i< bin_contents.size(); ++i) {
          IMP_LOG(VERBOSE, "  Grid " << i << " has ");
          for (unsigned int j=0; j< bin_contents[i].size(); ++j) {
            IMP_LOG(VERBOSE, do_show(bin_contents[i][j]) << " ");
          }
          IMP_LOG(VERBOSE, std::endl);
        }
      }
      // compute split points int ubs
      // split based on the points
      // create grids using bounding box/number of points
      for (unsigned int i=0; i< bin_contents.size(); ++i) {
        algebra::BoundingBox3D cbb;
        for (unsigned int j=0; j< bin_contents[i].size(); ++j) {
          cbb+= ps.c_(bin_contents[i][j]);
        }
        double side= get_side(cbb, bin_contents[i].size(),
                              distance, bin_ubs[i]);
        bins.push_back(Grid(side,cbb));
        IMP_LOG(TERSE, "  Grid " << i << " had box " << cbb
                << " and side " << side << std::endl);
        for (unsigned int j=0; j< bin_contents[i].size(); ++j) {
          algebra::Vector3D v= ps.c_(bin_contents[i][j]);
          bins.back()[bins.back().get_index(v)]
               .push_back(bin_contents[i][j]);
        }
      }
    }






    template <class It>
    static void do_fill_close_pairs_from_list(It b, It e,
                                              CloseF close, double distance,
                                              Out &out) {
      for (It c= b; c != e; ++c) {
        for (It cp= b; cp != c; ++cp) {
          if (close(*c, *cp, distance)) {
            IMP_LOG(VERBOSE, "Found pair " << do_show(*c) << " "
                    << do_show(*cp) << std::endl);
            out.push_back( typename Out::value_type(*c, *cp));
          }
        }
      }
    }






    template <class ItA, class ItB>
    static void do_fill_close_pairs_from_lists(ItA ab, ItA ae,
                                               ItB bb, ItB be,
                                               CloseF close,
                                               double distance, Out &out) {
      for (ItA c= ab; c != ae; ++c) {
        for (ItB cp= bb; cp != be; ++cp) {
          if (close(*c, *cp, distance)) {
            IMP_LOG(VERBOSE, "Found pair " << do_show(*c) << " "
                    << do_show(*cp) << std::endl);
            out.push_back( typename Out::value_type(*c, *cp));
          }
        }
      }
    }







    static void
    do_fill_close_pairs_around_cell(const Grid &g,
                                    typename Grid::ExtendedIndex bblb,
                                    typename Grid::ExtendedIndex bbub,
                                    typename Grid::ExtendedIndex eindex,
                                    const IDs &in,
                                    CloseF close,
                                    double distance,
                                    double radius_ubg,
                                    double radius_uqq,
                                    bool half,
                                    unsigned int merged,
                                    Out &out) {
      // C++ is dumb about unary minus on unsigned ints
      // (or rather takes a bit manipulating
      // view rather than an arithmetic one.
      const double doffset=(distance+ radius_ubg+radius_uqq)
                            /g.get_unit_cell()[0];
      const int offset= std::ceil(doffset);
      //std::cout << doffset << " " << offset << std::endl;
      int idx[3];
      bool good=true;
      for (unsigned int i=0; i< 3; ++i) {
        if (eindex[i] < 0) {
          if (eindex[i] < -offset-1) {
            // IMP_LOG(VERBOSE, "  Cell is too far from grid "
            // << eindex << std::endl);
            return;
          } else {
            good=false;
          }
        } else if (eindex[i] >= static_cast<int>(g.get_number_of_voxels(i))) {
          if (eindex[i] > static_cast<int>(g.get_number_of_voxels(i))+offset) {
            //IMP_LOG(VERBOSE,
            // "  Cell is too far from grid " << eindex << std::endl);
            return;
          } else {
            good=false;
          }
        } else {
          idx[i]= eindex[i];
        }
      }
      IMP_IF_LOG(VERBOSE) {
        IMP_LOG(VERBOSE, "Input vertices are ");
        for (unsigned int i=0; i< in.size(); ++i) {
          IMP_LOG(VERBOSE, do_show(in[i]) << ' ');
        }
        IMP_LOG(VERBOSE, std::endl);
      }
      if (good) {
        typename Grid::Index index(idx[0], idx[1], idx[2]);
        IMP_LOG(VERBOSE, "  Searching around "
                << index << " from " << eindex << std::endl);
        if (half) {
          do_fill_close_pairs_from_list(g[index].begin(), g[index].end(),
                                        close, distance, out);
        } else {
          do_fill_close_pairs_from_lists(g[index].begin(), g[index].end(),
                                        in.begin(), in.end(),
                                        close, distance, out);
        }
      } else {
        IMP_LOG(VERBOSE, "  Searching around " << eindex << std::endl);
      }
      IMP_IF_LOG(VERBOSE) {
        IMP_LOG(VERBOSE, "  ");
        for (unsigned int i=0; i< in.size(); ++i) {
          IMP_LOG(VERBOSE, do_show(in[i]) << " ");
        }
        IMP_LOG(VERBOSE, std::endl);
      }
      // compute number of cells to look at:
      // assume cubic cells
      typename Grid::ExtendedIndex lb
        = g.get_offset(eindex, -offset, -offset, -offset);
      typename Grid::ExtendedIndex ub
        = g.get_offset(eindex, offset, offset, offset);
      for (typename Grid::ExtendedIndexIterator eit
           = g.extended_indexes_begin(lb, ub);
           eit != g.extended_indexes_end(lb, ub); ++eit) {
        //IMP_LOG(VERBOSE, "  Visiting cell " << *eit << "..." << std::endl);
        typename Grid::Index ind;
        if (!wrap(g, bblb, bbub, eindex, *eit, merged, half, ind)) {
          continue;
        } else {
          IMP_LOG(VERBOSE, "    Inspecting cell " << *eit << ind << std::endl);
        }
        do_fill_close_pairs_from_lists(g[ind].begin(), g[ind].end(),
                                       in.begin(), in.end(),
                                       close, distance, out);
      }
    }




    template <class ItQ>
    static void do_fill_close_pairs(const Grid &g,
                                    CloseF close,
                                    double distance, double radius_ub,
                                    const IDSet<ItQ> &ids,
                                    const algebra::BoundingBox3D &bb,
                                    unsigned int merged, Out &out) {
      typename Grid::ExtendedIndex bblb, bbub;
      if (merged) {
        bblb= g.get_extended_index(bb.get_corner(0));
        bbub= g.get_extended_index(bb.get_corner(1));
      }
      for (ItQ qc= ids.b_; qc != ids.e_; ++qc) {
        do_fill_close_pairs_around_cell(g, bblb, bbub,
                                        g.get_extended_index(ids.c_(*qc)),
                                        IDs(1, *qc),
                                        close,
                                        distance, radius_ub, ids.r_(*qc),
                                        false, merged, out);
      }
    }

    template <class It>
    static void fill_close_pairs(const ParticleSet<It> &ps,
                                 CloseF close,
                                 double distance,
                                 const algebra::BoundingBox3D& bb,
                                 unsigned int merged,
                                 Out &out) {
      std::vector<Grid > bins;
      std::vector<IDs > bin_contents;
      std::vector<double> bin_ubs;
      create_grids(ps, distance,
                   bins, bin_contents, bin_ubs);
      for (unsigned int i=0; i< bins.size(); ++i) {
        IMP_LOG(VERBOSE, "Handling bin " << i << std::endl);
        typename Grid::ExtendedIndex bblb, bbub;
        if (merged) {
          bblb= bins[i].get_extended_index(bb.get_corner(0));
          bbub= bins[i].get_extended_index(bb.get_corner(1));
        }
        for (unsigned int j=0; j < i; ++j) {
          IMP_LOG(VERBOSE, "Against bin " << j << std::endl);
          do_fill_close_pairs(bins[i], close, distance, bin_ubs[i],
                              get_id_set(bin_contents[j].begin(),
                                         bin_contents[j].end(),
                                         ps.c_, ps.r_), bb, merged, out);
        }
        for (typename Grid::AllIndexIterator it = bins[i].all_indexes_begin();
             it != bins[i].all_indexes_end(); ++it) {
          do_fill_close_pairs_around_cell(bins[i], bblb, bbub, *it,
                                          bins[i][*it], close, distance,
                                          bin_ubs[i], bin_ubs[i],
                                          true, merged,out);
        }
      }
    }






    template <class ItG, class ItQ>
    static void fill_close_pairs(const ParticleSet<ItG> &psg,
                                 const ParticleSet<ItQ> &psq,
                                 CloseF close,
                                 double distance,
                                 const algebra::BoundingBox3D& bb,
                                 unsigned int merged,
                                 Out &out) {
      std::vector<Grid > bins;
      std::vector<IDs > bin_contents;
      std::vector<double> bin_ubs;
      create_grids(psg, distance,
                   bins, bin_contents, bin_ubs);
      for (unsigned int i=0; i< bins.size(); ++i) {
        do_fill_close_pairs(bins[i], close, distance, bin_ubs[i],
                            get_id_set(
                                       boost::make_transform_iterator(psq.b_,
                                                                      psq.id_),
                                       boost::make_transform_iterator(psq.e_,
                                                                      psq.id_),
                                       psq.c_, psq.r_), bb, merged, out);
      }
    }
  };

  typedef Helper<ParticleID, ParticleCenter, ParticleRadius, ParticleClose,
  ParticlePairsTemp> ParticleHelper;
  typedef Helper<BBID, BBCenter, BBRadius, BBClose, IntPairs> BBHelper;
}

GridClosePairsFinder::GridClosePairsFinder(): ClosePairsFinder("GridCPF"),
qp_(new QuadraticClosePairsFinder()),
merged_(0)
{}

GridClosePairsFinder::GridClosePairsFinder(const algebra::BoundingBox3D &bb,
                                           unsigned int merged_boundaries):
ClosePairsFinder("PeriodicGridCPF"), qp_(new QuadraticClosePairsFinder()),
merged_(merged_boundaries){}


ParticlePairsTemp GridClosePairsFinder
::get_close_pairs(SingletonContainer *ca,
                  SingletonContainer *cb) const {
  IMP_OBJECT_LOG;
  ca->set_was_used(true);
  cb->set_was_used(true);
  set_was_used(true);
  ParticlePairsTemp out;
  ParticleHelper
  ::fill_close_pairs(ParticleHelper::get_particle_set(ca->particles_begin(),
                                                      ca->particles_end(),
                                                      ParticleID(),
                                                      ParticleCenter(),
                                                      ParticleRadius()),
                     ParticleHelper::get_particle_set(cb->particles_begin(),
                                                      cb->particles_end(),
                                                      ParticleID(),
                                                      ParticleCenter(),
                                                      ParticleRadius()),
                     ParticleClose(),
                     get_distance(), bb_, merged_,
                     out);
  return out;
}

ParticlePairsTemp GridClosePairsFinder
::get_close_pairs(SingletonContainer *c) const {
  IMP_OBJECT_LOG;
  c->set_was_used(true);
  set_was_used(true);
  IMP_LOG(TERSE, "Rebuilding NBL with Grid and cutoff "
          << get_distance() << std::endl );
  ParticlePairsTemp out;
  ParticleHelper
  ::fill_close_pairs(ParticleHelper::get_particle_set(c->particles_begin(),
                                                      c->particles_end(),
                                                      ParticleID(),
                                                      ParticleCenter(),
                                                      ParticleRadius()),
                     ParticleClose(),
                     get_distance(), bb_, merged_,
                     out);
  return out;
}

IntPairs GridClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas,
                  const algebra::BoundingBox3Ds &bbs) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IntPairs out;
  BBHelper::
  fill_close_pairs(BBHelper::get_particle_set(bas.begin(),
                                              bas.end(),
                                              BBID(bas.begin()),
                                              BBCenter(bas.begin()),
                                              BBRadius(bas.begin())),
                   BBHelper::get_particle_set(bbs.begin(),
                                              bbs.end(),
                                              BBID(bbs.begin()),
                                              BBCenter(bbs.begin()),
                                              BBRadius(bbs.begin())),
                   BBClose(bas.begin(), bbs.begin()),
                   get_distance(), bb_, merged_, out);
  return out;
}

IntPairs GridClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IntPairs out;
  BBHelper
  ::fill_close_pairs(BBHelper::get_particle_set(bas.begin(),
                                                bas.end(),
                                                BBID(bas.begin()),
                                                BBCenter(bas.begin()),
                                                BBRadius(bas.begin())),
                     BBClose(bas.begin(), bas.begin()),
                     get_distance(), bb_, merged_, out);
  return out;
}


void GridClosePairsFinder::do_show(std::ostream &out) const {
  out << "distance " << get_distance() << std::endl;
}


ParticlesTemp
GridClosePairsFinder::get_input_particles(SingletonContainer *sc) const {
  ParticlesTemp ret= sc->get_particles();
  return ret;
}

ParticlesTemp
GridClosePairsFinder::get_input_particles(SingletonContainer *a,
                                          SingletonContainer *b) const {
  ParticlesTemp ret0= a->get_particles();
  ParticlesTemp ret1= b->get_particles();
  ret0.insert(ret0.end(), ret1.begin(), ret1.end());
  return ret0;
}

ContainersTemp
GridClosePairsFinder::get_input_containers(SingletonContainer *sc) const {
  ContainersTemp ret(1,sc);
  return ret;
}

ContainersTemp
GridClosePairsFinder::get_input_containers(SingletonContainer *a,
                                           SingletonContainer *b) const {
  ContainersTemp ret(2);
  ret[0]= a;
  ret[1]= b;
  return ret;
}

IMPCORE_END_NAMESPACE
