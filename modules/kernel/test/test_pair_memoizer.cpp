/**
 *  \file test_cache.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/cache.h>
#include <IMP/showable_macros.h>
#include <IMP/comparison_macros.h>
#include <IMP/hash_macros.h>
#include <IMP/nullptr_macros.h>
#include <IMP/tuple_macros.h>
#include <IMP/flags.h>
#include <IMP/test/test_macros.h>
#include <IMP/random.h>
#include <boost/random/uniform_int.hpp>
#include <numeric>

// Skip test on g++ 4.2, since it fails to compile due to a g++ bug
#if __GNUC__ == 4 && __GNUC_MINOR__ == 2
int main(int, char * []) {
  std::cout << "Skipped due to g++ 4.2 bug" << std::endl;
  return 0;
}

#else

const int threshold = 2;
namespace IMP {
struct IP {
  int *p_;
  IP() : p_(IMP_NULLPTR) {}
  IP(int *p) : p_(p) {}
  operator int *() const { return p_; }
  // operator int*(){return p_;}
  int operator*() const { return *p_; }
  IMP_SHOWABLE_INLINE(IP, out << p_ << "(" << *p_ << ")");
  IMP_HASHABLE_INLINE(IP, return boost::hash_value(p_););
  IMP_COMPARISONS_1(IP, p_);
};

inline std::ostream &operator<<(std::ostream &out, const IP &ip) {
  out << static_cast<const int *>(ip) << "(" << *ip << ")";
  return out;
}
}

IMP::Vector<int> values;
IMP::Vector<IMP::IP> pointers;

// fix koenig lookup of hash function
namespace IMP {
// Can't use std::pair since VC gets confused between std::pair
// and std::pair_base
typedef IMP::Array<2, IMP::IP> Entry;
}

namespace {
typedef IMP::IP KeyPart;

IMP::Vector<IMP::Entry> get_list(
    IMP::IP pi, IMP::Vector<IMP::IP> excluded) {
  IMP::Vector<IMP::Entry> ret;
  for (unsigned int j = 0; j < pointers.size(); ++j) {
    if (std::abs(*pi - *pointers[j]) < threshold) {
      if (std::find(excluded.begin(), excluded.end(), pointers[j]) ==
          excluded.end()) {
        ret.push_back(IMP::Entry(pi, pointers[j]));
      }
    }
  }
  return ret;
}

IMP::Vector<IMP::Entry> get_list() {
  IMP::Vector<IMP::Entry> ret;
  IMP::Vector<IMP::IP> excluded;
  for (unsigned int i = 0; i < pointers.size(); ++i) {
    excluded.push_back(pointers[i]);
    ret += get_list(pointers[i], excluded);
  }
  return ret;
}

struct SortedPairs {
  struct StarLess {
    bool operator()(IMP::IP a, IMP::IP b) const { return *a < *b; }
  };
  SortedPairs() {}
  typedef IMP::Vector<IMP::Entry> result_type;
  typedef IMP::Vector<KeyPart> argument_type;
  template <class Table>
  result_type operator()(argument_type t, const Table &) const {
    std::cout << "Generating pairs from ";
    for (unsigned int i = 0; i < t.size(); ++i) {
      std::cout << *t[i] << " ";
    }
    std::cout << " over ";
    for (unsigned int i = 0; i < pointers.size(); ++i) {
      std::cout << *pointers[i] << " ";
    }
    std::cout << std::endl;
    result_type ret;
    IMP::Vector<IMP::IP> excluded;
    for (unsigned int i = 0; i < t.size(); ++i) {
      excluded.push_back(t[i]);
      ret += get_list(t[i], excluded);
    }
    std::cout << "Returning ";
    for (unsigned int i = 0; i < ret.size(); ++i) {
      std::cout << *ret[i][0] << "-" << *ret[i][1] << " ";
    }
    std::cout << std::endl;
    return ret;
  }
};

struct SetEquals {
  struct LessPair {
    bool operator()(IMP::Entry a, IMP::Entry b) const {
      if (a[0] > a[1]) std::swap(a[0], a[1]);
      if (b[0] > b[1]) std::swap(b[0], b[1]);
      if (a[0] < b[0])
        return true;
      else if (a[0] > b[0])
        return false;
      else if (a[1] < b[1])
        return true;
      else
        return false;
    }
  };
  bool operator()(SortedPairs::result_type t0) const {
    SortedPairs::result_type t1 = get_list();
    std::sort(t0.begin(), t0.end(), LessPair());
    std::sort(t1.begin(), t1.end(), LessPair());
    std::cout << "Comparing " << t0 << " and " << t1 << "= ";
    for (unsigned int i = 0; i < t0.size(); ++i) {
      std::cout << *t0[i][0] << "-" << *t0[i][1] << " ";
    }
    std::cout << " and ";
    for (unsigned int i = 0; i < t1.size(); ++i) {
      std::cout << *t1[i][0] << "-" << *t1[i][1] << " ";
    }
    std::cout << std::endl;
    if (t0.size() != t1.size()) return false;
    SortedPairs::result_type u;
    std::set_union(t0.begin(), t0.end(), t1.begin(), t1.end(),
                   std::back_inserter(u), LessPair());
    return u.size() == t0.size();
  }
};

typedef IMP::SparseSymmetricPairMemoizer<SortedPairs, SetEquals> Table;

struct Sum {
  int value;
  Sum() : value(0) {}
  void operator()(IMP::Entry a) { value += *a[0] + *a[1]; }
};

struct Show {
  void operator()(IMP::Entry a) {
    std::cout << *a[0] << "-" << *a[1] << ", ";
  }
};

void check(Table &t, IMP::Vector<int> values) {
  t.apply(Show());
  std::cout << std::endl;
  std::sort(values.begin(), values.end());
  int sum = t.apply(Sum()).value;
  int rsum = 0;
  for (unsigned int i = 0; i < values.size(); ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      if (std::abs(values[i] - values[j]) < 2) {
        rsum += values[i] + values[j];
      }
    }
  }
  IMP_TEST_EQUAL(sum, rsum);
}
}

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test memoizer");
  IMP::set_log_level(IMP::VERBOSE);
  const int n = 5;
  boost::uniform_int<> ui(0, n * 2);
  boost::uniform_int<> pi(0, n - 1);

  for (int i = 0; i < n; ++i) {
    values.push_back(ui(IMP::random_number_generator));
  }
  pointers.resize(values.size());
  for (unsigned int i = 0; i < values.size(); ++i) {
    pointers[i] = &values[i];
  }
  Table table(pointers);
  check(table, values);
  for (unsigned int i = 0; i < 1000; ++i) {
    int c = pi(IMP::random_number_generator);
    int nv = ui(IMP::random_number_generator);
    std::cout << "Update item " << c << " from " << values[c] << " to " << nv
              << std::endl;
    table.remove(pointers[c]);
    values[c] = nv;
    check(table, values);
  }
  return 0;
}

#endif  // GNUC 4.2
