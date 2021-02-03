/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/utility.h>
#include <IMP/log.h>
#include <IMP/Model.h>
#include <IMP/io.h>

#include <limits>
#include <cmath>

#define IMP_CHECK_VALUE(n)                           \
  IMP_IF_CHECK(IMP::USAGE) {                   \
    if (!is_good_value(n)) {                         \
      IMP_LOG_TERSE(#n << " is " << n << std::endl); \
      failure();                                     \
    }                                                \
  }

#define IMP_CG_SCALE

IMPCORE_BEGIN_NAMESPACE

namespace {
//! Estimate of limit of machine precision
const double cg_eps = 1.2e-7;

template <class NT>
bool is_good_value(const NT &f) {
  if (IMP::isnan(f) ||
      std::abs(f) > std::numeric_limits<NT>::max() / 1024.0f) {
    IMP_LOG_VERBOSE("Bad value found in CG: " << f << std::endl);
    return false;
  } else
    return true;
}
}

void ConjugateGradients::failure() {
  IMP_THROW("Failure in ConjugateGradients", ModelException);
}

//! Get the score for a given model state.
/** \param[in] model The model to score.
    \param[in] model_data The corresponding ModelData.
    \param[in] float_indices Indices of optimizable variables.
    \param[in] x Current value of optimizable variables.
    \param[out] dscore First derivatives for current state.
    \return The model score.
 */
ConjugateGradients::NT ConjugateGradients::get_score(
    Vector<FloatIndex> float_indices, Vector<NT> &x,
    Vector<NT> &dscore) {
  int i, opt_var_cnt = float_indices.size();
  /* set model state */
  for (i = 0; i < opt_var_cnt; i++) {
    IMP_CHECK_VALUE(x[i]);
#ifdef IMP_CG_SCALE
    double v = get_scaled_value(float_indices[i]);  // scaled
#else
    double v = get_value(float_indices[i]);  // scaled
#endif
    if (std::abs(x[i] - v) > max_change_) {
      if (x[i] < v) {
        x[i] = v - max_change_;
      } else {
        x[i] = v + max_change_;
      }
    }
#ifdef IMP_CG_SCALE
    set_scaled_value(float_indices[i], x[i]);
#else
    set_value(float_indices[i], x[i]);
#endif
  }

  NT score;
  /* get score */
  try {
    score = get_scoring_function()->evaluate(true);
  }
  catch (const ModelException &) {
    // if we took a bad step, just return a bad score
    return std::numeric_limits<NT>::infinity();
  }
  /* get derivatives */
  for (i = 0; i < opt_var_cnt; i++) {
#ifdef IMP_CG_SCALE
    dscore[i] = get_scaled_derivative(float_indices[i]);  // scaled
#else
    dscore[i] = get_derivative(float_indices[i]);  // scaled
#endif
    IMP_USAGE_CHECK(is_good_value(dscore[i]), "Bad input to CG");
  }
  return score;
}

//! Try to find the minimum of the function in the given direction.
/** \param[out]   x         Current state (updated on output)
    \param[out]   dx        Gradient at current state (updated on output)
    \param[inout] alpha     Current step length (updated on output)
    \param[inout] ifun      Current number of function evaluations (updated
                            on output)
    \param[inout] f         Current function value (updated on output)
    \param[in]    max_steps Maximum number of function evaluations
    \param[in]    search    Direction in which to search
    \param[in]    estimate  Initial state

    \return true if the line search succeeded, false if max_steps was exceeded
            or a minimum could not be found.
 */
bool ConjugateGradients::line_search(
    Vector<NT> &x, Vector<NT> &dx, NT &alpha,
    const Vector<FloatIndex> &float_indices, int &ifun, NT &f, NT &dg,
    NT &dg1, int max_steps, const Vector<NT> &search,
    const Vector<NT> &estimate) {
  NT ap, fp, dp, step, minf, u1, u2;
  int i, n, ncalls = ifun;

  n = float_indices.size();
  /* THE LINEAR SEARCH FITS A CUBIC TO F AND DAL, THE FUNCTION AND ITS
     DERIVATIVE AT ALPHA, AND TO FP AND DP,THE FUNCTION
     AND DERIVATIVE AT THE PREVIOUS TRIAL POINT AP.
     INITIALIZE AP ,FP,AND DP. */
  ap = 0.;
  fp = minf = f;
  dp = dg1;

  /* SAVE THE CURRENT DERIVATIVE TO SCALE THE NEXT SEARCH VECTOR. */
  dg = dg1;

  /* Calculate the current steplength */
  step = 0.;
  for (i = 0; i < n; i++) {
    step += search[i] * search[i];
  }
  step = sqrt(step);

  /* BEGIN THE LINEAR SEARCH ITERATION. */
  while (true) {
    NT dal, at;

    /* TEST FOR FAILURE OF THE LINEAR SEARCH. */
    if (alpha * step <= cg_eps) {
      return false;
    }

    /* CALCULATE THE TRIAL POINT. */
    for (i = 0; i < n; i++) {
      x[i] = estimate[i] + alpha * search[i];
    }

    /* EVALUATE THE FUNCTION AT THE TRIAL POINT. */
    f = get_score(float_indices, x, dx);

    /* TEST IF THE MAXIMUM NUMBER OF FUNCTION CALLS HAVE BEEN USED. */
    if (++ifun > max_steps) {
      return false;
    }

    /* COMPUTE THE DERIVATIVE OF F AT ALPHA. */
    dal = 0.0;
    for (i = 0; i < n; i++) {
      IMP_CHECK_VALUE(dx[i]);
      dal += dx[i] * search[i];
    }

    /* TEST WHETHER THE NEW POINT HAS A NEGATIVE SLOPE BUT A HIGHER
       FUNCTION VALUE THAN ALPHA=0. IF THIS IS THE CASE,THE SEARCH
       HAS PASSED THROUGH A LOCAL MAX AND IS HEADING FOR A DISTANT LOCAL
       MINIMUM. */
    if (f > minf && dal < 0.) {
      /* REDUCE ALPHA AND RESTART THE SEARCH. */
      alpha /= 3.0;
      ap = 0.;
      fp = minf;
      dp = dg;
      continue;
    }

    /* IF NOT, TEST WHETHER THE STEPLENGTH CRITERIA HAVE BEEN MET. */
    if (f <= (minf + 0.0001 * alpha * dg) && fabs(dal / dg) <= 0.9) {

      /* IF THEY HAVE BEEN MET, TEST IF TWO POINTS HAVE BEEN TRIED
         AND IF THE TRUE LINE MINIMUM HAS NOT BEEN FOUND. */
      if (ifun - ncalls > 1 || fabs(dal / dg) <= cg_eps) {
        break;
      }
    }

    /* A NEW POINT MUST BE TRIED. USE CUBIC INTERPOLATION TO FIND
       THE TRIAL POINT AT. */

    u1 = dp + dal - 3.0 * (fp - f) / (ap - alpha);
    u2 = u1 * u1 - dp * dal;
    if (u2 < 0.) {
      u2 = 0.;
    }
    u2 = sqrt(u2);
    at = alpha - (alpha - ap) * (dal + u2 - u1) / (dal - dp + 2. * u2);
    /* TEST WHETHER THE LINE MINIMUM HAS BEEN BRACKETED. */
    if (dal / dp <= 0.) {

      /* THE MINIMUM HAS BEEN BRACKETED. TEST WHETHER THE TRIAL POINT LIES
         SUFFICIENTLY WITHIN THE BRACKETED INTERVAL.
         IF IT DOES NOT, CHOOSE AT AS THE MIDPOINT OF THE INTERVAL. */
      if (at < (1.01 * std::min(alpha, ap)) ||
          at > (0.99 * std::max(alpha, ap))) {
        at = (alpha + ap) / 2.0;
      }

      /* THE MINIMUM HAS NOT BEEN BRACKETED. TEST IF BOTH POINTS ARE
         GREATER THAN THE MINIMUM AND THE TRIAL POINT IS SUFFICIENTLY
         SMALLER THAN EITHER. */
    } else if (dal <= 0.0 || 0.0 >= at || at >= (0.99 * std::min(ap, alpha))) {

      /* TEST IF BOTH POINTS ARE LESS THAN THE MINIMUM AND THE TRIAL POINT
         IS SUFFICIENTLY LARGE. */
      if (dal > 0.0 || at <= (1.01 * std::max(ap, alpha))) {

        /* IF THE TRIAL POINT IS TOO SMALL,DOUBLE THE LARGEST PRIOR POINT. */
        if (dal <= 0.0) {
          at = 2.0 * std::max(ap, alpha);
        }

        /* IF THE TRIAL POINT IS TOO LARGE, HALVE THE SMALLEST PRIOR POINT. */
        if (dal > 0.) {
          at = std::min(ap, alpha) / 2.0;
        }
      }
    }

    /* SET AP=ALPHA, ALPHA=AT,AND CONTINUE SEARCH. */
    ap = alpha;
    fp = f;
    dp = dal;
    alpha = at;
  }
  return true;
}

//! Constructor
ConjugateGradients::ConjugateGradients(Model *m, std::string name)
    : AttributeOptimizer(m, name) {
  threshold_ = std::numeric_limits<Float>::epsilon();
  max_change_ = std::numeric_limits<Float>::max() / 100.0;
}

Float ConjugateGradients::do_optimize(unsigned int max_steps) {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(get_model(),
                  "Must set the model on the optimizer before optimizing");
  clear_range_cache();
  Vector<NT> x, dx;
  int i;
  // ModelData* model_data = get_model()->get_model_data();

  FloatIndexes float_indices = get_optimized_attributes();

  int n = float_indices.size();
  if (n == 0) {
    IMP_THROW("There are no optimizable degrees of freedom.", ModelException);
  }

  x.resize(n);
  dx.resize(n);
  // get initial state in x(n):
  for (i = 0; i < n; i++) {
#ifdef IMP_CG_SCALE
    x[i] = get_scaled_value(float_indices[i]);  // scaled
#else
    x[i] = get_value(float_indices[i]);            // scaled
#endif
    IMP_USAGE_CHECK(
        !IMP::isnan(x[i]) && std::abs(x[i]) < std::numeric_limits<NT>::max(),
        "Bad input to CG");
  }

  // Initialize optimization variables
  int ifun = 0;
  int nrst;
  NT dg1, xsq, dxsq, alpha, step, u1, u2, u3, u4;
  NT f = 0., dg = 1., w1 = 0., w2 = 0., rtst, bestf;
  bool gradient_direction;

  // dx holds the gradient at x
  // search holds the search vector
  // estimate holds the best current estimate to the minimizer
  // destimate holds the gradient at the best current estimate
  // resy holds the restart Y vector
  // ressearch holds the restart search vector
  Vector<NT> search, estimate, destimate, resy, ressearch;
  search.resize(n);
  estimate.resize(n);
  destimate.resize(n);
  resy.resize(n);
  ressearch.resize(n);

/* Calculate the function and gradient at the initial
   point and initialize nrst,which is used to determine
   whether a Beale restart is being done. nrst=n means that this
   iteration is a restart iteration. */
g20:
  f = get_score(float_indices, x, dx);
  if (get_stop_on_good_score() &&
      get_scoring_function()->get_had_good_score()) {
    estimate = x;
    goto end;
  }
  ifun++;
  nrst = n;
  // this is a gradient, not restart, direction:
  gradient_direction = true;

  /* Calculate the initial search direction, the norm of x squared,
     and the norm of dx squared. dg1 is the current directional
     derivative, while xsq and dxsq are the squared norms. */
  dg1 = xsq = 0.;

  for (i = 0; i < n; i++) {
    search[i] = -dx[i];
    xsq += x[i] * x[i];
    dg1 -= dx[i] * dx[i];
  }
  dxsq = -dg1;

  /* Test if the initial point is the minimizer. */
  if (dxsq <= cg_eps * cg_eps * std::max(NT(1.0), xsq)) {
    goto end;
  }

/* Begin the major iteration loop. */
g40:
  update_states();
  /* Begin linear search. alpha is the steplength. */

  if (gradient_direction) {
    /* This results in scaling the initial search vector to unity. */
    alpha = 1.0 / sqrt(dxsq);
  } else if (nrst == 1) {
    /* Set alpha to 1.0 after a restart. */
    alpha = 1.0;
  } else {
    /* Set alpha to the nonrestart conjugate gradient alpha. */
    alpha = alpha * dg / dg1;
  }

  /* Store current best estimate for the score */
  estimate = x;
  destimate = dx;

  /* Try to find a better score by linear search */
  if (!line_search(x, dx, alpha, float_indices, ifun, f, dg, dg1, max_steps,
                   search, estimate)) {
    /* If the line search failed, it was either because the maximum number
       of iterations was exceeded, or the minimum could not be found */
    if (static_cast<unsigned int>(ifun) > max_steps) {
      goto end;
    } else if (gradient_direction) {
      goto end;
    } else {
      goto g20;
    }
  }

  /* THE LINE SEARCH HAS CONVERGED. TEST FOR CONVERGENCE OF THE ALGORITHM. */
  dxsq = xsq = 0.0;
  for (i = 0; i < n; i++) {
    dxsq += dx[i] * dx[i];
    xsq += x[i] * x[i];
  }
  if (dxsq < threshold_) {
    goto end;
  }

  /* Search continues. Set search(i)=alpha*search(i),the full step vector. */
  for (i = 0; i < n; i++) {
    search[i] *= alpha;
  }

  /* COMPUTE THE NEW SEARCH VECTOR;
     TEST IF A POWELL RESTART IS INDICATED. */
  rtst = 0.;
  for (i = 0; i < n; ++i) {
    rtst += dx[i] * destimate[i];
  }

  if (std::abs(rtst / dxsq) > .2) {
    nrst = n;
  }

  /* If a restart is indicated, save the current d and y
     as the Beale restart vectors and save d'y and y'y
     in w1 and w2. */
  if (nrst == n) {
    ressearch = search;
    w1 = w2 = 0.;
    for (i = 0; i < n; i++) {
      resy[i] = dx[i] - destimate[i];
      w1 += resy[i] * resy[i];
      w2 += search[i] * resy[i];
    }
  }

  /* CALCULATE THE RESTART HESSIAN TIMES THE CURRENT GRADIENT. */
  u1 = u2 = 0.0;
  for (i = 0; i < n; i++) {
    u1 -= ressearch[i] * dx[i] / w1;
    u2 += ressearch[i] * dx[i] * 2.0 / w2 - resy[i] * dx[i] / w1;
  }
  u3 = w2 / w1;
  for (i = 0; i < n; i++) {
    estimate[i] = -u3 * dx[i] - u1 * resy[i] - u2 * ressearch[i];
  }

  /* If this is a restart iteration, estimate contains the new search vector. */
  if (nrst != n) {

    /* NOT A RESTART ITERATION. CALCULATE THE RESTART HESSIAN
       TIMES THE CURRENT Y. */
    u1 = u2 = u3 = 0.0;
    for (i = 0; i < n; i++) {
      u1 -= (dx[i] - destimate[i]) * ressearch[i] / w1;
      u2 = u2 - (dx[i] - destimate[i]) * resy[i] / w1 +
           2.0 * ressearch[i] * (dx[i] - destimate[i]) / w2;
      u3 += search[i] * (dx[i] - destimate[i]);
    }
    step = u4 = 0.;
    for (i = 0; i < n; i++) {
      step =
          (w2 / w1) * (dx[i] - destimate[i]) + u1 * resy[i] + u2 * ressearch[i];
      u4 += step * (dx[i] - destimate[i]);
      destimate[i] = step;
    }

    /* CALCULATE THE DOUBLY UPDATED HESSIAN TIMES THE CURRENT
       GRADIENT TO OBTAIN THE SEARCH VECTOR. */
    u1 = u2 = 0.0;
    for (i = 0; i < n; i++) {
      u1 -= search[i] * dx[i] / u3;
      u2 +=
          (1.0 + u4 / u3) * search[i] * dx[i] / u3 - destimate[i] * dx[i] / u3;
    }
    for (i = 0; i < n; i++) {
      estimate[i] = estimate[i] - u1 * destimate[i] - u2 * search[i];
    }
  }

  /* CALCULATE THE DERIVATIVE ALONG THE NEW SEARCH VECTOR. */
  search = estimate;
  dg1 = 0.0;
  for (i = 0; i < n; i++) {
    dg1 += search[i] * dx[i];
  }

  /* IF THE NEW DIRECTION IS NOT A DESCENT DIRECTION,STOP. */
  if (dg1 <= 0.0) {

    /* UPDATE NRST TO ASSURE AT LEAST ONE RESTART EVERY N ITERATIONS. */
    if (nrst == n) {
      nrst = 0;
    }
    nrst++;
    gradient_direction = false;
    goto g40;
  }

/* ROUNDOFF HAS PRODUCED A BAD DIRECTION. */

end:
  // If the 'best current estimate' is better than the current state, return
  // that:
  bestf = get_score(float_indices, estimate, destimate);
  if (bestf < f) {
    f = bestf;
  } else {
    // Otherwise, restore the current state x (note that we already have the
    // state x and its derivatives dx, so it's rather inefficient to
    // recalculate the score here, but it's cleaner)
    f = get_score(float_indices, x, dx);
  }
  update_states();
  return f;
}

IMPCORE_END_NAMESPACE
