#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  uint32_t m = ( zero_point.raw_value_ + n ) % ( uint64_t { 1 } << 32 );
  return Wrap32 { m };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // uint64_t raw_value_1 =
  uint64_t max_32 = ( uint64_t { 1 } << 32 );
  uint64_t zp = uint64_t( zero_point.raw_value_ );
  uint64_t rw = uint64_t( raw_value_ );
  uint64_t baseline = ( zp + checkpoint );

  uint64_t abs_below = 0;
  uint64_t abs_equal = 0;
  uint64_t abs_up = 0;
  if ( baseline / max_32 == 0 || ( ( baseline / max_32 - 1 ) * max_32 + rw ) < zp ) {
    abs_below = -1;
  } else {
    abs_below = baseline - ( ( baseline / max_32 - 1 ) * max_32 + rw );
  }

  if ( ( ( baseline / max_32 ) * max_32 + rw ) < zp ) {
    abs_equal = -1;
  } else {
    uint64_t equal = ( ( baseline / max_32 ) * max_32 + rw );
    if ( equal > baseline ) {
      abs_equal = equal - baseline;
    } else {
      abs_equal = baseline - equal;
    }
  }

  abs_up = ( ( baseline / max_32 + 1 ) * max_32 + rw ) - baseline;

  if ( abs_below <= abs_equal && abs_below <= abs_up ) {
    return ( ( baseline / max_32 - 1 ) * max_32 + rw ) - zp;
  }
  if ( abs_equal <= abs_below && abs_equal <= abs_up ) {
    return ( ( baseline / max_32 ) * max_32 + rw ) - zp;
  }
  if ( abs_up <= abs_equal && abs_up <= abs_below ) {
    return ( ( baseline / max_32 + 1 ) * max_32 + rw ) - zp;
  }
  return 0;
}
