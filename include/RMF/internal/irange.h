// Boost.Range library
//
//  Copyright Neil Groves 2010. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//
// For more information, see http://www.boost.org/libs/range/
//
#ifndef RMF_BOOST_RANGE_IRANGE_HPP_INCLUDED
#define RMF_BOOST_RANGE_IRANGE_HPP_INCLUDED

#include <boost/version.hpp>
#if BOOST_VERSION > 104400
#include <boost/range/irange.hpp>  // IWYU pragma: export
#else
#include <boost/assert.hpp>
#include <boost/iterator/iterator_facade.hpp>  // IWYU pragma: export
#include <boost/range/iterator_range.hpp>      // IWYU pragma: export
#endif

namespace RMF {

namespace internal {

#if BOOST_VERSION > 104400
using boost::irange;
using boost::range_detail::integer_iterator;
#else
// integer_iterator is an iterator over an integer sequence that
// is bounded only by the limits of the underlying integer
// representation.
//
// This is useful for implementing the irange(first, last)
// function.
//
// Note:
// This use of this iterator and irange is appreciably less
// performant than the corresponding hand-written integer
// loop on many compilers.
template <typename Integer>
class integer_iterator
    : public boost::iterator_facade<integer_iterator<Integer>, Integer,
                                    boost::random_access_traversal_tag, Integer,
                                    std::ptrdiff_t> {
  typedef boost::iterator_facade<integer_iterator<Integer>, Integer,
                                 boost::random_access_traversal_tag, Integer,
                                 std::ptrdiff_t> base_t;

 public:
  typedef typename base_t::value_type value_type;
  typedef typename base_t::difference_type difference_type;
  typedef typename base_t::reference reference;

  integer_iterator() : m_value() {}
  explicit integer_iterator(value_type x) : m_value(x) {}

 private:
  void increment() { ++m_value; }

  void decrement() { --m_value; }

  void advance(difference_type offset) { m_value += offset; }

  difference_type distance_to(const integer_iterator& other) const {
    return other.m_value - m_value;
  }

  bool equal(const integer_iterator& other) const {
    return m_value == other.m_value;
  }

  reference dereference() const { return m_value; }

  friend class ::boost::iterator_core_access;
  value_type m_value;
};

// integer_iterator_with_step is similar in nature to the
// integer_iterator but provides the ability to 'move' in
// a number of steps specified at construction time.
//
// The three variable implementation provides the best guarantees
// of loop termination upon various combinations of input.
//
// While this design is less performant than some less
// safe alternatives, the use of ranges and iterators to
// perform counting will never be optimal anyhow, hence
// if optimal performance is desired a hand-coded loop
// is the solution.
template <typename Integer>
class integer_iterator_with_step
    : public boost::iterator_facade<integer_iterator_with_step<Integer>,
                                    Integer, boost::random_access_traversal_tag,
                                    Integer, std::ptrdiff_t> {
  typedef boost::iterator_facade<integer_iterator_with_step<Integer>, Integer,
                                 boost::random_access_traversal_tag, Integer,
                                 std::ptrdiff_t> base_t;

 public:
  typedef typename base_t::value_type value_type;
  typedef typename base_t::difference_type difference_type;
  typedef typename base_t::reference reference;

  integer_iterator_with_step(value_type first, difference_type step,
                             value_type step_size)
      : m_first(first), m_step(step), m_step_size(step_size) {}

 private:
  void increment() { ++m_step; }

  void decrement() { --m_step; }

  void advance(difference_type offset) { m_step += offset; }

  difference_type distance_to(const integer_iterator_with_step& other) const {
    return other.m_step - m_step;
  }

  bool equal(const integer_iterator_with_step& other) const {
    return m_step == other.m_step;
  }

  reference dereference() const { return m_first + (m_step * m_step_size); }

  friend class ::boost::iterator_core_access;
  value_type m_first;
  value_type m_step;
  difference_type m_step_size;
};

template <typename Integer>
class integer_range : public boost::iterator_range<integer_iterator<Integer> > {
  typedef integer_iterator<Integer> iterator_t;
  typedef boost::iterator_range<iterator_t> base_t;

 public:
  integer_range(Integer first, Integer last)
      : base_t(iterator_t(first), iterator_t(last)) {}
};

template <typename Integer>
integer_range<Integer> irange(Integer first, Integer last) {
  BOOST_ASSERT(first <= last);
  return integer_range<Integer>(first, last);
}

#endif
}
}

#endif  // include guard
