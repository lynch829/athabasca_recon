#include "filtering/TaperedCosineWindowFilter.hpp"
#include <gtest/gtest.h>
#include <boost/math/constants/constants.hpp>
#include <cmath>

#ifdef USE_VDSP
#include "filtering/ProjectionFilterer_vDSP.hpp"
#define ENABLE_TEST_TRANSFORM_PROPERTIES
#elif defined USE_FFTW
#include "filtering/ProjectionFilterer_fftw.hpp"
#define ENABLE_TEST_TRANSFORM_PROPERTIES
#endif

namespace constants = boost::math::constants;
using namespace athabasca_recon;

typedef float TValue;
typedef int TIndex;
typedef float TSpace;
typedef bonelab::Array<1,TValue,TIndex> TArray;
typedef TaperedCosineWindowFilter<TArray> TFilter;
typedef bonelab::Image<2,TValue,TIndex,TSpace> TProjection;
#ifdef USE_VDSP
typedef athabasca_recon::ProjectionFilterer_vDSP<TProjection> TProjectionFilterer;
#elif defined USE_FFTW
typedef athabasca_recon::ProjectionFilterer_fftw<TProjection> TProjectionFilterer;
#endif

template <typename T> inline T sqr(T x) { return x*x; }

TValue evalFilter(TValue f, TValue f0, TValue f1)
  {
  if (f <= f0)
    { return 1; }
  else if (f < f1)
    {
    return 0.5 + 0.5*cos(constants::pi<TValue>()*(f-f0)/(f1-f0));
    }
  else
    { return 0; }
  }

// Create a test fixture class.
class TaperedCosineWindowFilterTests : public ::testing::Test
  {
  };

// --------------------------------------------------------------------
// test implementations

TEST_F (TaperedCosineWindowFilterTests, FilterConstruction)
  {
  TFilter filter;
  TValue f0 = 1.0/3.0;  // As a fraction of the Nyquist frequency
  TValue f1 = 2.0/3.0;  // As a fraction of the Nyquist frequency
  filter.SetLength(16);
  filter.SetF1(f0);
  filter.SetF2(f1);
  TArray A;
  filter.ConstructTransferFunction(A);
  // Check the DC component
  ASSERT_FLOAT_EQ(A(0), 1);
  // Compare with formula
  for (int i=1; i<=8; ++i)
    {
    ASSERT_FLOAT_EQ(A(i), evalFilter(TValue(i)/8, f0, f1));
    }
  // Check that we have a symmetric filter
  for (int i=1; i<8; ++i)
    {
    ASSERT_FLOAT_EQ(A(16-i), A(i));
    }
  // Check the Nyquist component (at L/2)
  ASSERT_FLOAT_EQ(A(8), evalFilter(1.0, f0, f1));
  }

#ifdef ENABLE_TEST_TRANSFORM_PROPERTIES
TEST_F (TaperedCosineWindowFilterTests, TransformProperties)
  {
  // Create and compare two filterers: one without smoothing, and one with a
  // tapered cosine smoothing filter.

  bonelab::Tuple<2,TIndex> dims(2,256);
  bonelab::Tuple<2,TSpace> spacing(1,1);
  bonelab::Tuple<2,TSpace> origin(0,0);  // not used.
  TProjectionFilterer filterer_no_smoothing;
  filterer_no_smoothing.SetDimensions(dims);
  filterer_no_smoothing.SetWeight(1);
  filterer_no_smoothing.SetPixelSpacing(spacing[1]);
  filterer_no_smoothing.Initialize();
  TProjection projIn(dims, spacing, origin);
  TProjection projOut_no_smoothing(dims, spacing, origin);
  projIn.zero();
  int locationOfDelta = dims[1]/2;
  projIn(0,locationOfDelta) = 1;
  filterer_no_smoothing.FilterProjection(projIn, projOut_no_smoothing);
  TValue mean_no_smoothing = 0;
  for (int i=0; i<dims[1]; ++i)
    { mean_no_smoothing += projOut_no_smoothing(0,i); }

  typedef ProjectionFilterer<TProjection>::TFunction1D TFunction1D;
  typedef TaperedCosineWindowFilter<TFunction1D> TFilter;
  TFilter smoothingFilter;
  smoothingFilter.SetLength(dims[1]);
  smoothingFilter.SetF1(0.3333);
  smoothingFilter.SetF2(0.6666);
  TProjectionFilterer filterer_with_smoothing;
  filterer_with_smoothing.SetDimensions(dims);
  filterer_with_smoothing.SetWeight(1);
  filterer_with_smoothing.SetPixelSpacing(spacing[1]);
  filterer_with_smoothing.SetSmoothingFilter(&smoothingFilter);
  filterer_with_smoothing.Initialize();
  TProjection projOut_with_smoothing(dims, spacing, origin);
  projIn.zero();
  projIn(0,locationOfDelta) = 1;
  filterer_with_smoothing.FilterProjection(projIn, projOut_with_smoothing);
  TValue mean_with_smoothing = 0;
  for (int i=0; i<dims[1]; ++i)
    { mean_with_smoothing += projOut_with_smoothing(0,i); }

  // At the location of the delta function, smoothing reduces.
  ASSERT_LE (projOut_with_smoothing(0,locationOfDelta),
             projOut_no_smoothing(0,locationOfDelta));

  // Filters need to preserve the DC components.  Note that some weight
  // is smoothed beyond the limits (and there is no wrap-around).  This
  // is why this is tested with a relatively long length (256).
  ASSERT_NEAR (mean_no_smoothing, mean_with_smoothing, 1E-6);
  }
#endif
