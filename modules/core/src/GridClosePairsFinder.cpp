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
  static bool get_is_wrapped(unsigned int merged, unsigned int k) {
      switch (k) {
      case 0:
        return merged & GridClosePairsFinder::X;
      case 1:
        return merged & GridClosePairsFinder::Y;
      default:
        IMP_USAGE_CHECK(k==2, "bad k");
        return merged & GridClosePairsFinder::Z;
      }
    }
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
  struct PeriodicParticleClose {
    algebra::Vector3D uc_;
    unsigned int merged_;
    PeriodicParticleClose(const algebra::BoundingBox3D &bb,
                          unsigned int merged):
      uc_(bb.get_corner(1)- bb.get_corner(0)),
      merged_(merged){}
    double operator()(Particle *a, Particle *b, double distance) const {
      algebra::Vector3D diff
        = XYZR(a).get_coordinates()- XYZR(b).get_coordinates();
      for (unsigned int i=0; i< 3; ++i) {
        if (get_is_wrapped(merged_, i)) {
          if (diff[i] > .5*uc_[i]) diff[i]-=uc_[i];
          else if (diff[i] < -.5*uc_[i]) diff[i]+=uc_[i];
        }
      }
      return diff.get_magnitude() - XYZR(a).get_radius() - XYZR(b).get_radius()
        < distance;
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
  struct PeriodicBBClose {
    algebra::Vector3D uc_;
    unsigned int merged_;
    algebra::BoundingBox3Ds::const_iterator it0_, it1_;
    PeriodicBBClose(algebra::BoundingBox3Ds::const_iterator it0,
            algebra::BoundingBox3Ds::const_iterator it1,
            const algebra::BoundingBox3D &bb,
            unsigned int merged): uc_(bb.get_corner(1)- bb.get_corner(0)),
                                  merged_(merged), it0_(it0), it1_(it1){}
    double operator()(unsigned int a, unsigned int b, double distance) const {
      for (unsigned int i=0; i< 3; ++i) {
        if (get_is_wrapped(merged_, i)) {
          bool ok=false;
          for (int o=-1; o < 2; ++o) {
            double ub0= uc_[i]*o + (it1_+b)->get_corner(1)[i]+distance;
            if ((it0_+a)->get_corner(0)[i] > ub0) {
              continue;
            }
            double ub1= uc_[i]*o + (it0_+1)->get_corner(1)[i]+distance;
            if ((it1_+b)->get_corner(0)[i] > ub1) {
              continue;
            }
            ok=true;
            break;
          }
          if (!ok) return false;
        } else {
          if ((it0_+a)->get_corner(0)[i]
              > (it1_+b)->get_corner(1)[i]+distance) return false;
          if ((it0_+a)->get_corner(1)[i]+distance
              < (it1_+b)->get_corner(0)[i]) return false;
        }
      }
      return true;
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

  template <bool PERIODIC, class IDF, class CenterF,
            class RadiusF, class CloseF, class Out>
  struct Helper {
    typedef typename IDF::result_type ID;
    typedef std::vector<ID> IDs;
    typedef algebra::Grid3D<IDs, algebra::SparseGridStorage3D<IDs> > Grid;
    typedef std::vector<Grid> Grids;

    template <class It>
    struct ParticleSet {
      It b_, e_;
      IDF id_;
      CenterF c_;
      RadiusF r_;
      ParticleSet(It b, It e, IDF id,
                  CenterF c, RadiusF r): b_(b), e_(e),
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


    static int get_index(int ii, int which, int lb, int ub) {
      if (which==1) {
        if (ii== lb) ii= ub;
        else if (ii == ub) ii= lb;
      }
      return ii;
    }
    static unsigned int get_bound(bool is_merged, int raw, int lb, int ub) {
      if (!is_merged) return 1;
      if (raw != lb && raw != ub) return 1;
      return 2;
    }
    template <class Index>
    static void fill_copies(const Grid &g,
                            typename Grid::ExtendedIndex center,
                            Index cur,
                            std::vector<typename Grid::Index> &out) {
      typename Grid::ExtendedIndex cei(cur[0], cur[1], cur[2]);
      if (g.get_is_index(cei)&& cei != center
          && !g.get_voxel(g.get_index(cei)).empty()) {
        out.push_back(g.get_index(cei));
      }
    }

    template <class Index>
    static void fill_copies_periodic(const Grid &g,
                                     typename Grid::ExtendedIndex center,
                                     Index cur,
                                     unsigned int merged,
                                     typename Grid::ExtendedIndex bblb,
                                     typename Grid::ExtendedIndex bbub,
                                     std::vector<typename Grid::Index> &out) {
      for (int io=-1; io < 2; ++io) {
        if ((!(merged &GridClosePairsFinder::X)) && io != 0) continue;
        int ii=cur[0]+io*(bbub[0]-bblb[0]);
        if (ii < 0
            || ii >= static_cast<int>(g.get_number_of_voxels(0))) continue;
        for (int jo=-1; jo < 2; ++jo) {
          if ((!(merged &GridClosePairsFinder::Y)) && jo != 0) continue;
          int ij=cur[1]+jo*(bbub[1]-bblb[1]);
          if (ij < 0
              || ij >= static_cast<int>(g.get_number_of_voxels(1))) continue;
          for (int ko=-1; ko < 2; ++ko) {
            if ((!(merged &GridClosePairsFinder::Z)) && ko != 0) continue;
            int ik=cur[2]+ko*(bbub[2]-bblb[2]);
            if (ik < 0
                || ik >= static_cast<int>(g.get_number_of_voxels(2))) continue;
            typename Grid::ExtendedIndex cei(ii, ij, ik);
            IMP_INTERNAL_CHECK(g.get_is_index(cei), "Not a voxel " << cei);
            typename Grid::Index ci= g.get_index(cei);
            // make sure equivalent voxels are only added once
            if (cur[0] != center[0] || cur[1] != center[1]
                || cur[2] != center[2] || ci < center) {
              if (!g.get_voxel(ci).empty()) {
                out.push_back(g.get_index(cei));
              }
            }
          }
        }
      }
    }

    static void fill_nearby(const Grid &g,
                            typename Grid::ExtendedIndex center,
                            int offset,
                            double distance,
                            double half,
                            unsigned int merged,
                            const algebra::BoundingBox3D &bb,
                            typename Grid::ExtendedIndex bblb,
                            typename Grid::ExtendedIndex bbub,
                            std::vector<typename Grid::Index> &out) {
      IMP_INTERNAL_CHECK(distance <= offset * g.get_unit_cell()[0],
                         "Offset is too small " << distance << " > "
                         << offset << " * " << g.get_unit_cell()[0]
                         << " all unit cell is " << g.get_unit_cell());
      if (PERIODIC) {
        IMP_LOG(VERBOSE, "Filling for " << center << " "
                << offset << " " << half << " in " << bblb << " to " << bbub
                << " distance " << distance << " "
                << g.get_unit_cell() << std::endl);
      } else {
        IMP_LOG(VERBOSE, "Filling for " << center << " "
                << offset << " " << half
                << " distance " << distance << " " << g.get_unit_cell()
                << std::endl);
      }
      typename Grid::ExtendedIndex lb(g.get_offset(center,
                                                   -offset, -offset, -offset)),
        ub(g.get_offset(center, offset, offset, offset));
      if (half) {
        IMP_GRID3D_FOREACH_SMALLER_VOXEL_RANGE(g, center, lb, ub,
                                               if (PERIODIC) {
                    fill_copies_periodic(g, center, voxel_index,
                                         merged, bblb, bbub, out);
                  } else {
                    fill_copies(g, center, voxel_index, out);
                    });
      } else {
        for (typename Grid::ExtendedIndexIterator it
               = g.extended_indexes_begin(lb, ub);
             it != g.extended_indexes_end(lb, ub); ++it) {
          if (PERIODIC) {
            fill_copies_periodic(g, center, *it,merged, bblb, bbub, out);
          } else {
            fill_copies(g, center, *it, out);
          }
        }
      }
      if (PERIODIC) {
        fill_copies_periodic(g, center, center,merged, bblb, bbub, out);
      } else {
        fill_copies(g, center, center, out);
      }
      IMP_IF_LOG(VERBOSE) {
        IMP_LOG(VERBOSE, "Unwrapped found ");
        for (unsigned int i=0; i< out.size(); ++i) {
          IMP_LOG(VERBOSE, out[i] << " ");
        }
        IMP_LOG(VERBOSE, std::endl);
      }
      if (merged) {
        // would be nice to not have duplicates
        std::sort(out.begin(), out.end());
        out.erase(std::unique(out.begin(), out.end()), out.end());
      }
      IMP_IF_LOG(VERBOSE) {
        if (merged) {
          IMP_LOG(VERBOSE, "Wrapped found ");
          for (unsigned int i=0; i< out.size(); ++i) {
            IMP_LOG(VERBOSE, out[i] << " ");
          }
          IMP_LOG(VERBOSE, std::endl);
        }
      }
      IMP_IF_CHECK(USAGE_AND_INTERNAL) {
        if (!half) {
          std::vector<typename Grid::Index> check_out;
          std::vector<algebra::BoundingBox3D> cbb;
          if (merged) {
            for (int i=-1; i< 2; ++i) {
              for (int j=-1; j< 2; ++j) {
                for (int k=-1; k< 2; ++k) {
                  algebra::BoundingBox3D cell= g.get_bounding_box(center)
                    +.99*distance;
                  algebra::Vector3D lc(cell.get_corner(0)[0]
                                       +i*(bb.get_corner(1)[0]
                                           - bb.get_corner(0)[0]),
                                       cell.get_corner(0)[1]
                                       +j*(bb.get_corner(1)[1]
                                           - bb.get_corner(0)[1]),
                                       cell.get_corner(0)[2]
                                       +k*(bb.get_corner(1)[2]
                                           - bb.get_corner(0)[2]));
                  algebra::Vector3D uc(cell.get_corner(1)[0]
                                       +i*(bb.get_corner(1)[0]
                                           - bb.get_corner(0)[0]),
                                       cell.get_corner(1)[1]
                                       +j*(bb.get_corner(1)[1]
                                           - bb.get_corner(0)[1]),
                                       cell.get_corner(1)[2]
                                       +k*(bb.get_corner(1)[2]
                                           - bb.get_corner(0)[2]));
                  cbb.push_back(algebra::BoundingBox3D(lc, uc));
                }
              }
            }
          } else {
            cbb.push_back( g.get_bounding_box(center)+distance);
          }
          IMP_INTERNAL_CHECK(!PERIODIC || cbb.size()==27,
                             "Huh, 3*3*3=27, not " << cbb.size());
          for (typename Grid::AllNonEmptyConstIterator it
                 = g.all_non_empty_begin();
               it != g.all_non_empty_end(); ++it) {
            algebra::BoundingBox3D curbb= g.get_bounding_box(it->first);
            for (unsigned int i=0; i< cbb.size(); ++i) {
              if (algebra::get_interiors_intersect(curbb, cbb[i])
                  && it->first != center
                  && (!half || it->first < center)
                  && g.get_is_non_empty(it->first)) {
                check_out.push_back(it->first);
              }
            }
          }
          std::sort(check_out.begin(), check_out.end());
          check_out.erase(std::unique(check_out.begin(), check_out.end()),
                          check_out.end());
          std::sort(out.begin(), out.end());
          IMP_IF_LOG(VERBOSE) {
            IMP_LOG(VERBOSE, "Check found ");
            for (unsigned int i=0; i< check_out.size(); ++i) {
              IMP_LOG(VERBOSE, check_out[i] << " ");
            }
            IMP_LOG(VERBOSE, std::endl);
          }
          IMP_INTERNAL_CHECK(out.size() >= check_out.size(),
                             "Sizes don't match " << out.size() << " vs "
                             << check_out.size());
          for (unsigned int i=0; i< check_out.size(); ++i) {
            IMP_INTERNAL_CHECK(std::binary_search(out.begin(), out.end(),
                                                  check_out[i]),
                               "Entries not found " << i
                               << " " <<  check_out[i]);
          }
        }
      }
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
        //fix_grid(merged, bb, side, cbb);
        bins.push_back(Grid(side,cbb));
        IMP_LOG(TERSE, "  Grid " << i << " had box " << cbb
                << " and side " << side << std::endl);
        for (unsigned int j=0; j< bin_contents[i].size(); ++j) {
          algebra::Vector3D v= ps.c_(bin_contents[i][j]);
          typename Grid::Index ind=bins.back().get_index(v);
          if (bins.back().get_is_non_empty(ind)) {
            bins.back()[ind]
              .push_back(bin_contents[i][j]);
          } else {
            bins.back().set_voxel(ind, IDs(1, bin_contents[i][j]));
          }
        }
      }
      IMP_IF_LOG(VERBOSE) {
        for (unsigned int i=0; i< bins.size(); ++i) {
          IMP_LOG(VERBOSE, "Grid level " << i << " with bounds "
                  << bin_ubs[i] << std::endl);
          for (typename Grid::AllNonEmptyConstIterator it
                 = bins[i].all_non_empty_begin();
               it != bins[i].all_non_empty_end(); ++it) {
            IMP_LOG(VERBOSE, "Bin " << it->first << " contains ");
            for (unsigned int j=0; j< it->second.size(); ++j) {
              IMP_LOG(VERBOSE, do_show(it->second[j])<< " ");
            }
            IMP_LOG(VERBOSE, std::endl);
          }
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
                                    typename Grid::ExtendedIndex center,
                                    const IDs &in,
                                    CloseF close,
                                    int offset,
                                    double distance,
                                    bool half,
                                    unsigned int merged,
                                    const algebra::BoundingBox3D &bb,
                                    typename Grid::ExtendedIndex bblb,
                                    typename Grid::ExtendedIndex bbub,
                                    Out &out) {
      // C++ is dumb about unary minus on unsigned ints
      // (or rather takes a bit manipulating
      // view rather than an arithmetic one.
      //std::cout << doffset << " " << offset << std::endl;
      if (in.empty()) return;
      IMP_IF_LOG(VERBOSE) {
        IMP_LOG(VERBOSE, "Input vertices are ");
        for (unsigned int i=0; i< in.size(); ++i) {
          IMP_LOG(VERBOSE, do_show(in[i]) << ' ');
        }
        IMP_LOG(VERBOSE, std::endl);
      }
      if (g.get_is_index(center)) {
        typename Grid::Index index=g.get_index(center);
        IMP_LOG(VERBOSE, "  Searching "
                << index << std::endl);
        if (half) {
          do_fill_close_pairs_from_list(g.get_voxel(index).begin(),
                                        g.get_voxel(index).end(),
                                        close, distance, out);
        } else {
          do_fill_close_pairs_from_lists(g.get_voxel(index).begin(),
                                         g.get_voxel(index).end(),
                                         in.begin(), in.end(),
                                         close, distance, out);
        }
      } else {
        IMP_LOG(VERBOSE, "  Skipped " << center << std::endl);
      }
      std::vector<typename Grid::Index> nearby;
      if (PERIODIC) {
        fill_nearby(g, center, offset, distance, half,
                    merged, bb, bblb, bbub, nearby);
      } else {
        fill_nearby(g, center, offset, distance, half,
                    0, algebra::BoundingBox3D(),
                    typename Grid::ExtendedIndex(),
                    typename Grid::ExtendedIndex(),
                    nearby);
      }
      for (unsigned int i=0; i< nearby.size(); ++i) {
        //IMP_LOG(VERBOSE, "  Visiting cell " << *eit << "..." << std::endl);
        std::vector<typename Grid::Index> ind;
        IMP_LOG(VERBOSE, "    Inspecting pair " << center << " " << nearby[i]
                << std::endl);
        do_fill_close_pairs_from_lists(g.get_voxel(nearby[i]).begin(),
                                       g.get_voxel(nearby[i]).end(),
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
        const double doffset=(distance+radius_ub+ids.r_(*qc))
          /g.get_unit_cell()[0];
        const int offset= std::ceil(doffset);
        //IMP_LOG(VERBOSE, "Offset is " << offset << std::endl);
        if (PERIODIC) {
          do_fill_close_pairs_around_cell(g,
                                          g.get_extended_index(ids.c_(*qc)),
                                          IDs(1, *qc),
                                          close,
                                          offset,
                                          distance+radius_ub+ids.r_(*qc),
                                          false, merged, bb, bblb, bbub, out);
        } else {
          do_fill_close_pairs_around_cell(g,
                                          g.get_extended_index(ids.c_(*qc)),
                                          IDs(1, *qc),
                                          close,
                                          offset,
                                          distance+radius_ub+ids.r_(*qc),
                                          false,
                                          0, algebra::BoundingBox3D(),
                                          typename Grid::ExtendedIndex(),
                                          typename Grid::ExtendedIndex(),
                                          out);
        }
      }
    }

    template <class It>
    static void fill_close_pairs(const ParticleSet<It> &ps,
                                 CloseF close,
                                 double distance,
                                 const algebra::BoundingBox3D& bb,
                                 unsigned int merged,
                                 Out &out) {
      Grids bins;
      std::vector<IDs > bin_contents;
      std::vector<double> bin_ubs;
      create_grids(ps, distance,
                   bins, bin_contents, bin_ubs);
      for (unsigned int i=0; i< bins.size(); ++i) {
        IMP_LOG(VERBOSE, "Handling bin " << i << std::endl);
        typename Grid::ExtendedIndex bblb, bbub;
        if (PERIODIC) {
          IMP_LOG(VERBOSE, "Locating corners " << bb << std::endl);
          bblb= bins[i].get_extended_index(bb.get_corner(0));
          bbub= bins[i].get_extended_index(bb.get_corner(1));
          IMP_LOG(VERBOSE, "Got " << bblb << " and " << bbub << std::endl);
        }
        for (unsigned int j=0; j < i; ++j) {
          IMP_LOG(VERBOSE, "Against bin " << j << std::endl);
          if (PERIODIC) {
            do_fill_close_pairs(bins[i], close, distance, bin_ubs[i],
                                get_id_set(bin_contents[j].begin(),
                                           bin_contents[j].end(),
                                           ps.c_, ps.r_), bb, merged, out);
          } else {
            do_fill_close_pairs(bins[i], close, distance, bin_ubs[i],
                                get_id_set(bin_contents[j].begin(),
                                           bin_contents[j].end(),
                                           ps.c_, ps.r_),
                                algebra::BoundingBox3D(), 0, out);
          }
        }
        const double doffset=(distance+bin_ubs[i]+bin_ubs[i])
          /bins[i].get_unit_cell()[0];
        const int offset= std::ceil(doffset);
        for (typename Grid::AllNonEmptyConstIterator it
               = bins[i].all_non_empty_begin();
             it != bins[i].all_non_empty_end(); ++it) {
          //IMP_LOG(VERBOSE, "Offset is " << offset << std::endl);
          if (PERIODIC) {
            do_fill_close_pairs_around_cell(bins[i], it->first,
                                            it->second, close, offset,
                                            distance+bin_ubs[i]+bin_ubs[i],
                                            true,
                                            merged, bb, bblb, bbub,
                                            out);
          } else {
            do_fill_close_pairs_around_cell(bins[i],it->first,
                                            it->second, close, offset,
                                            distance+bin_ubs[i]+bin_ubs[i],
                                            true,
                                            0, algebra::BoundingBox3D(),
                                            typename Grid::ExtendedIndex(),
                                            typename Grid::ExtendedIndex(),
                                            out);
          }
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
        if (PERIODIC) {
          do_fill_close_pairs(bins[i], close, distance, bin_ubs[i],
                              get_id_set(
                                         boost::make_transform_iterator(psq.b_,
                                                                      psq.id_),
                                         boost::make_transform_iterator(psq.e_,
                                                                      psq.id_),
                                         psq.c_, psq.r_),
                              bb, merged,
                              out);
        } else {
          do_fill_close_pairs(bins[i], close, distance, bin_ubs[i],
                              get_id_set(
                                         boost::make_transform_iterator(psq.b_,
                                                                      psq.id_),
                                         boost::make_transform_iterator(psq.e_,
                                                                      psq.id_),
                                         psq.c_, psq.r_),
                              algebra::BoundingBox3D(), 0,
                              out);
        }
      }
    }
  };

  typedef Helper<false, ParticleID, ParticleCenter, ParticleRadius,
                 ParticleClose, ParticlePairsTemp> ParticleHelper;
  typedef Helper<false, BBID, BBCenter, BBRadius, BBClose, IntPairs> BBHelper;
  typedef Helper<true, ParticleID, ParticleCenter, ParticleRadius,
                 PeriodicParticleClose, ParticlePairsTemp> PParticleHelper;
  typedef Helper<true, BBID, BBCenter, BBRadius, PeriodicBBClose,
                 IntPairs> PBBHelper;
}

GridClosePairsFinder::GridClosePairsFinder(): ClosePairsFinder("GridCPF"),
                                              merged_(0)
{}

GridClosePairsFinder::GridClosePairsFinder(const algebra::BoundingBox3D &bb,
                                           unsigned int merged_boundaries):
  ClosePairsFinder("PeriodicGridCPF"), bb_(bb), merged_(merged_boundaries) {}


ParticlePairsTemp GridClosePairsFinder
::get_close_pairs(SingletonContainer *ca,
                  SingletonContainer *cb) const {
  IMP_OBJECT_LOG;
  ca->set_was_used(true);
  cb->set_was_used(true);
  set_was_used(true);
  ParticlePairsTemp out;
  if (merged_) {
    PParticleHelper
      ::fill_close_pairs(PParticleHelper
                         ::get_particle_set(ca->particles_begin(),
                                            ca->particles_end(),
                                            ParticleID(),
                                            ParticleCenter(),
                                            ParticleRadius()),
                         PParticleHelper
                         ::get_particle_set(cb->particles_begin(),
                                            cb->particles_end(),
                                            ParticleID(),
                                            ParticleCenter(),
                                            ParticleRadius()),
                         PeriodicParticleClose(bb_, merged_),
                         get_distance(), bb_, merged_,
                         out);
  } else {
    ParticleHelper
      ::fill_close_pairs(ParticleHelper
                         ::get_particle_set(ca->particles_begin(),
                                            ca->particles_end(),
                                            ParticleID(),
                                            ParticleCenter(),
                                            ParticleRadius()),
                         ParticleHelper
                         ::get_particle_set(cb->particles_begin(),
                                            cb->particles_end(),
                                            ParticleID(),
                                            ParticleCenter(),
                                            ParticleRadius()),
                         ParticleClose(),
                         get_distance(), bb_, merged_,
                         out);
  }
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
  if (merged_) {
    PParticleHelper
      ::fill_close_pairs(PParticleHelper::get_particle_set(c->particles_begin(),
                                                          c->particles_end(),
                                                          ParticleID(),
                                                          ParticleCenter(),
                                                          ParticleRadius()),
                         PeriodicParticleClose(bb_, merged_),
                         get_distance(), bb_, merged_,
                         out);
  } else {
    ParticleHelper
      ::fill_close_pairs(ParticleHelper::get_particle_set(c->particles_begin(),
                                                          c->particles_end(),
                                                          ParticleID(),
                                                          ParticleCenter(),
                                                          ParticleRadius()),
                         ParticleClose(),
                         get_distance(), bb_, merged_,
                         out);
  }
  return out;
}

IntPairs GridClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas,
                  const algebra::BoundingBox3Ds &bbs) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IntPairs out;
  if (merged_) {
    PBBHelper::
      fill_close_pairs(PBBHelper::get_particle_set(bas.begin(),
                                                  bas.end(),
                                                  BBID(bas.begin()),
                                                  BBCenter(bas.begin()),
                                                  BBRadius(bas.begin())),
                       PBBHelper::get_particle_set(bbs.begin(),
                                                  bbs.end(),
                                                  BBID(bbs.begin()),
                                                  BBCenter(bbs.begin()),
                                                  BBRadius(bbs.begin())),
                       PeriodicBBClose(bas.begin(), bbs.begin(), bb_, merged_),
                       get_distance(), bb_, merged_, out);
  } else {
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
  }
  return out;
}

IntPairs GridClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IntPairs out;
  if (merged_) {
    PBBHelper
    ::fill_close_pairs(PBBHelper::get_particle_set(bas.begin(),
                                                  bas.end(),
                                                  BBID(bas.begin()),
                                                  BBCenter(bas.begin()),
                                                  BBRadius(bas.begin())),
                       PeriodicBBClose(bas.begin(), bas.begin(),
                                bb_, merged_),
                       get_distance(), bb_, merged_, out);
  } else {
  BBHelper
    ::fill_close_pairs(BBHelper::get_particle_set(bas.begin(),
                                                  bas.end(),
                                                  BBID(bas.begin()),
                                                  BBCenter(bas.begin()),
                                                  BBRadius(bas.begin())),
                       BBClose(bas.begin(), bas.begin()),
                       get_distance(), bb_, merged_, out);
  }
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
