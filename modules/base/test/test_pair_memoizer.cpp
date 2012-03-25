/**
 *  \file test_cache.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/base/cache.h>
#include <IMP/base/random.h>
#include <boost/random/uniform_int.hpp>
#include <numeric>
struct SortedPairs {
  struct StarLess {
    bool operator()(int *a, int *b) const {
      return *a < *b;
    }
  };
  typedef std::pair<int*, int*> Entry;
  typedef IMP::base::Vector<Entry> result_type;
  typedef int *KeyPart;
  typedef IMP::base::Vector<KeyPart> first_argument_type;
  typedef first_argument_type second_argument_type;
  template <class Table>
  result_type operator()(first_argument_type t,
                         second_argument_type domain,
                         const Table &) const {
    std::cout << "Generating pairs from ";
    for (unsigned int i=0; i< t.size(); ++i) {
      std::cout << *t[i] << " ";
    }
    std::cout << " over ";
    for (unsigned int i=0; i< domain.size(); ++i) {
      std::cout << *domain[i] << " ";
    }
    std::cout << std::endl;
    result_type ret;
    for (unsigned int i=0; i< t.size(); ++i) {
      for (unsigned int j=0; j< domain.size(); ++j) {
        bool found=false;
        for (unsigned int k=0; k < i; ++k) {
          if (domain[j]== t[k]) found=true;
        }
        if (t[i] != domain[j] && std::abs(*t[i] - *domain[j]) < 2
            && !found) {
          ret.push_back(std::make_pair(t[i], domain[j]));
        }
      }
    }
    std::cout << "Returning ";
    for (unsigned int i=0; i< ret.size(); ++i) {
      std::cout << *ret[i].first << "-" << *ret[i].second << " ";
    }
    std::cout << std::endl;
    return ret;
  }
};

struct SetEquals {
  struct LessPair {
    bool operator()(SortedPairs::Entry a,
                    SortedPairs::Entry b) const {
      if (a.first > a.second) std::swap(a.first, a.second);
      if (b.first > b.second) std::swap(b.first, b.second);
      if (a.first < b.first) return true;
      else if (a.first > b.first) return false;
      else if (a.second < b.second) return true;
      else if (a.second > b.second) return false;
      else return false;
    }
  };
  bool operator()(SortedPairs::result_type t0,
                  SortedPairs::result_type t1) const {
    std::sort(t0.begin(), t0.end(), LessPair());
    std::sort(t1.begin(), t1.end(), LessPair());
    std::cout << "Comparing " << t0 << " and " << t1
              << "= ";
    for (unsigned int i=0; i< t0.size(); ++i) {
      std::cout << *t0[i].first << "-" << *t0[i].second << " ";
    }
    std::cout << " and ";
    for (unsigned int i=0; i< t1.size(); ++i) {
      std::cout << *t1[i].first << "-" << *t1[i].second << " ";
    }
    std::cout << std::endl;
    if (t0.size() != t1.size()) return false;
    SortedPairs::result_type u;
    std::set_union(t0.begin(), t0.end(),
                   t1.begin(), t1.end(),
                   std::back_inserter(u),
                   LessPair());
    return u.size()==t0.size();
  }
};


typedef IMP::base::SparseSymmetricPairMemoizer<SortedPairs, SetEquals> Table;

struct Sum {
  int value;
  Sum(): value(0){}
  void operator()(std::pair<int*,int*> a) {
    value+=*a.first+*a.second;
  }
};


struct Show {
  void operator()(std::pair<int*,int*> a) {
    std::cout << *a.first << "-" << *a.second << ", ";
  }
};

void check(Table &t, IMP::base::Vector<int> values) {
  t.apply(Show());
  std::cout << std::endl;
  std::sort(values.begin(), values.end());
  int sum= t.apply(Sum()).value;
  int rsum=0;
  for (unsigned int i=0; i< values.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      if (std::abs(values[i]-values[j]) < 2) {
        rsum+= values[i]+values[j];
      }
    }
  }
  assert(sum==rsum);
}

int main(int, char *[]) {
  IMP::base::set_log_level(IMP::base::VERBOSE);
  const int n=5;
  IMP::base::Vector<int> values;
  boost::uniform_int<> ui(0,n*2);
  boost::uniform_int<> pi(0,n-1);

  for ( int i=0; i <n; ++i) {
    values.push_back(ui(IMP::base::random_number_generator));
  }

  IMP::base::Vector<int*> pointers(values.size());
  for (unsigned int i=0; i <values.size(); ++i) {
    pointers[i]= &values[i];
  }
  Table table(pointers);
  check(table, values);
  for (unsigned int i=0; i< 1000; ++i) {
    int c= pi(IMP::base::random_number_generator);
    int nv=  ui(IMP::base::random_number_generator);
    std::cout << "Update item " << c << " from " << values[c]
              << " to " << nv << std::endl;
    SortedPairs::result_type ret
      = table.get(pointers[c]);
    for (unsigned int i=0; i< ret.size(); ++i) {
      table.remove(ret[i]);
    }
    values[c]=nv;
    check(table, values);
  }
  return 0;
}
