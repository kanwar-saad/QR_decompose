/* -*-mode:c++-*- *********************************************************
 *                                                                        *
 *  Algorithmic C (tm) Datatypes                                          *
 *                                                                        *
 *  Software Version: 1.0                                                 *
 *                                                                        *
 *  Release Date    : Thu Apr 20 18:26:55 PDT 2006                        *
 *  Release Type    : Production                                          *
 *  Release Build   : 1.0.4                                               *
 *                                                                        *
 *  Copyright 1996-2006, Mentor Graphics Corporation,                     *
 *                                                                        *
 *  All Rights Reserved.                                                  *
 *                                                                        *
 **************************************************************************
 *                                                                        *
 *  The most recent version of this package can be downloaded from:       *
 *     http://www.mentor.com/products/c-based_design/ac_datatypes         *
 *                                                                        *
 **************************************************************************
 *                                                                        *
 *  IMPORTANT - THIS SOFTWARE IS COPYRIGHTED AND SUBJECT TO LICENSE       *
 *  RESTRICTIONS                                                          *
 *                                                                        *
 *  THE LICENSE THAT CONTROLS YOUR USE OF THE SOFTWARE IS:                *
 *  END-USER LICENSE AGREEMENT FOR ALGORITHMIC C DATATYPES SOURCE CODE    *
 *                                                                        *
 *  This license authorizes use and distribution of unmodified versions   *
 *  of the software. The complete text of the license may be found at:    *
 *  www.mentor.com/products/c-based_design/download/ac_datatypes_eula.cfm *
 *                                                                        *
 *  CAREFULLY READ THE LICENSE AGREEMENT BEFORE USING THE SOFTWARE.       *
 *  MODIFICATION OF THE SOFTWARE IS NOT AUTHORIZED                        *
 *                                                                        *
 *                                                                        *
 *  THESE COMMENTS ARE NOT THE LICENSE.  PLEASE CONSULT THE FULL LICENSE  *
 *  FOR THE ACTUAL TERMS AND CONDITIONS                                   *
 *                                                                        *
 *                                                                        *
 **************************************************************************
 *                                                                        *
 *  YOUR USE OF THE SOFTWARE INDICATES YOUR COMPLETE AND UNCONDITIONAL    *
 *  ACCEPTANCE OF THE TERMS AND CONDITIONS SET FORTH IN THE LICENSE. IF   *
 *  YOU DO NOT  AGREE TO THE LICENSE TERMS AND CONDITIONS, DO NOT USE THE *
 *  SOFTWARE, REMOVE IT FROM YOUR SYSTEM, AND DESTROY ALL COPIES.         *
 *                                                                        *
 **************************************************************************/
 


/*
//  Source:         ac_fixed.h
//  Description:    fixed class for fixed point operation handling in C++
*/

#ifndef __AC_FIXED_H
#define __AC_FIXED_H

#include "ac_int.h"

#if (defined(__GNUC__) && __GNUC__ < 3 && !defined(__EDG__))
#error GCC version 3 or greater is required to include this header file
#endif

#if (defined(_MSC_VER) && _MSC_VER < 1400 && !defined(__EDG__))
#error Microsoft Visual Studio 8 or newer is required to include this header file
#endif

#if (defined(_MSC_VER) && !defined(__EDG__))
#pragma warning( push )
#pragma warning( disable: 4127 )
#endif

//////////////////////////////////////////////////////////////////////////////
//  ac_fixed 
//////////////////////////////////////////////////////////////////////////////

//enum ac_q_mode { AC_TRN, AC_RND, AC_TRN_ZERO, AC_RND_ZERO, AC_RND_INF, AC_RND_MIN_INF, AC_RND_CONV };
//enum ac_o_mode { AC_WRAP, AC_SAT, AC_SAT_ZERO, AC_SAT_SYM };

template<int W, int I, bool S=true, ac_q_mode Q=AC_TRN, ac_o_mode O=AC_WRAP>
class ac_fixed : private ac_private::iv<(W+31+!S)/32> {
#if defined(__SYNTHESIS__) && !defined(AC_IGNORE_BUILTINS)
#pragma builtin
#endif

  enum {N=(W+31+!S)/32};
  typedef ac_private::iv<N> Base;

  inline void bit_adjust() {
    const unsigned rem = (32-W)&31;
    Base::v[N-1] =  S ? ((Base::v[N-1]  << rem) >> rem) : (rem ? 
                  ((unsigned) Base::v[N-1]  << rem) >> rem : 0); 
  }
  inline Base &base() { return *this; }
  inline const Base &base() const { return *this; }

  inline void overflow_adjust(bool underflow, bool overflow) {
    if(O==AC_WRAP) {
      bit_adjust();
      return;
    } 
    else if(O==AC_SAT_ZERO) {
      if((overflow || underflow))
        ac_private::iv_extend<N>(Base::v, 0);
      else
        bit_adjust();
    }
    else if(S) {
      if(overflow) {
        ac_private::iv_extend<N-1>(Base::v, ~0);
        Base::v[N-1] = ~(~0 << ((W-1)&31));
      } else if(underflow) {
        ac_private::iv_extend<N-1>(Base::v, 0);
        Base::v[N-1] = (~0 << ((W-1)&31));
        if(O==AC_SAT_SYM)
          Base::v[0] |= 1;
      } else
        bit_adjust();
    }
    else {
      if(overflow) {
        ac_private::iv_extend<N-1>(Base::v, ~0);
        Base::v[N-1] = ~(~0 << (W&31));
      } else if(underflow)
        ac_private::iv_extend<N>(Base::v, 0);
      else
        bit_adjust();
    }
  }

  inline bool quantization_adjust(bool qb, bool r, bool s) {
    if(Q==AC_TRN)
      return false;
    if(Q==AC_RND_ZERO)
      qb &= s || r; 
    else if(Q==AC_RND_MIN_INF) 
      qb &= r;
    else if(Q==AC_RND_INF) 
      qb &= !s || r;
    else if(Q==AC_RND_CONV) 
      qb &= (Base::v[0] & 1) || r;
    else if(Q==AC_TRN_ZERO) 
      qb = s && ( qb || r );
    return ac_private::iv_uadd_carry<N>(Base::v, qb, Base::v);
  }

  inline bool is_neg() const { return S && Base::v[N-1] < 0; }

public:
  template<int W2, int I2, bool S2>
  struct rt {
    enum {
      F=W-I, 
      F2=W2-I2,
      mult_w = W+W2,
      mult_i = I+I2,
      mult_s = S||S2,
      plus_w = MAX(I+(S2&&!S),I2+(S&&!S2))+1+MAX(F,F2),
      plus_i = MAX(I+(S2&&!S),I2+(S&&!S2))+1,
      plus_s = S||S2,
      minus_w = MAX(I+(S2&&!S),I2+(S&&!S2))+1+MAX(F,F2),
      minus_i = MAX(I+(S2&&!S),I2+(S&&!S2))+1,
      minus_s = true,
      div_w = W+MAX(W2-I2,0)+S2,
      div_i = I+(W2-I2)+S2,
      div_s = S||S2,
      logic_w = MAX(I+(S2&&!S),I2+(S&&!S2))+MAX(F,F2),
      logic_i = MAX(I+(S2&&!S),I2+(S&&!S2)),
      logic_s = S||S2
    };
    typedef ac_fixed<mult_w, mult_i, mult_s> mult;
    typedef ac_fixed<plus_w, plus_i, plus_s> plus;
    typedef ac_fixed<minus_w, minus_i, minus_s> minus;
    typedef ac_fixed<logic_w, logic_i, logic_s> logic;
    typedef ac_fixed<div_w, div_i, div_s> div;
    typedef ac_fixed<W, I, S> arg1;
  };

  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2> friend class ac_fixed;
  ac_fixed() {}
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  inline ac_fixed (const ac_fixed<W2,I2,S2,Q2,O2> &op) {
    enum {N2=(W2+31+!S2)/32, F=W-I, F2=W2-I2, QUAN_INC = F2>F && !(Q==AC_TRN || Q==AC_TRN_ZERO && !S2) };
    bool carry = false;
    // handle quantization
    if(F2 == F)
      Base::operator =(op);
    else if(F2 > F) {
      op.template const_shift_r<N,F2-F>(*this);
//      ac_private::iv_const_shift_r<N2,N,F2-F>(op.v, Base::v);
      if(Q!=AC_TRN && !(Q==AC_TRN_ZERO && !S2)) {
        bool qb = (F2-F > W2) ? (op.v[N2-1] < 0) : (bool) op[F2-F-1];
        bool r = (F2 > F+1) ? !ac_private::iv_equal_zeros_to<F2-F-1,N2>(op.v) : false;
        carry = quantization_adjust(qb, r, S2 && op.v[N2-1] < 0);
      }
    }
    else  // no quantization
      op.template const_shift_l<N,F-F2>(*this); 
//      ac_private::iv_const_shift_l<N2,N,F-F2>(op.v, Base::v);
    // handle overflow/underflow
    if(O!=AC_WRAP && (!S && S2 || I-S < I2-S2+(QUAN_INC || S2 && O==AC_SAT_SYM)) ) { // saturation 
      bool deleted_bits_zero = !(W&31)&S || !(Base::v[N-1] >> (W&31));
      bool deleted_bits_one = !(W&31)&S || !~(Base::v[N-1] >> (W&31));
      bool neg_src;
      if(F2-F+32*N < W2) {
        bool all_ones = ac_private::iv_equal_ones_from<F2-F+32*N,N2>(op.v);
        deleted_bits_zero = deleted_bits_zero && (carry ? all_ones : ac_private::iv_equal_zeros_from<F2-F+32*N,N2>(op.v)); 
        deleted_bits_one = deleted_bits_one && (carry ? ac_private::iv_equal_ones_from<1+F2-F+32*N,N2>(op.v) && !op[F2-F+32*N] : all_ones); 
        neg_src = S2 && op.v[N2-1] < 0 && !(carry & all_ones); 
      }
      else
        neg_src = S2 && Base::v[N-1] < 0;
      bool neg_trg = S && (bool) this->operator[](W-1); 
      bool overflow = !neg_src && (neg_trg || !deleted_bits_zero); 
      bool underflow = neg_src && (!neg_trg || !deleted_bits_one); 
      if(O==AC_SAT_SYM && S && S2)
        underflow |= neg_src && (W > 1 ? ac_private::iv_equal_zeros_to<W-1,N>(Base::v) : true);
      overflow_adjust(underflow, overflow);
    }
    else 
      bit_adjust();
  }

  template<int W2, bool S2>
  inline ac_fixed (const ac_int<W2,S2> &op) {
    ac_fixed<W2,W2,S2> f_op;
    f_op.base().operator =(op);
    *this = f_op;
  }

  template<int W2>
  ac_fixed<MAX(W+W2,1),I,S> shiftl() const {
    enum {NR=MAX((W+W2+31+!S)/32,1)};
    ac_fixed<MAX(W+W2,1),I,S> r;
//    ac_private::iv_const_shift_l<N,NR,W2>(Base::v, r.v);
    Base::template const_shift_l<NR,W2>(r);
    return r;
  }

  inline ac_fixed( bool b ) { *this = (ac_int<1,false>) b; }
  inline ac_fixed( char b ) { *this = (ac_int<8,true>) b; }
  inline ac_fixed( signed char b ) { *this = (ac_int<8,true>) b; }
  inline ac_fixed( unsigned char b ) { *this = (ac_int<8,false>) b; }
  inline ac_fixed( signed short b ) { *this = (ac_int<16,true>) b; }
  inline ac_fixed( unsigned short b ) { *this = (ac_int<16,false>) b; }
  inline ac_fixed( signed int b ) { *this = (ac_int<32,true>) b; }
  inline ac_fixed( unsigned int b ) { *this = (ac_int<32,false>) b; }
  inline ac_fixed( signed long b ) { *this = (ac_int<32,true>) b; }
  inline ac_fixed( unsigned long b ) { *this = (ac_int<32,false>) b; }
  inline ac_fixed( Slong b ) { *this = (ac_int<64,true>) b; }
  inline ac_fixed( Ulong b ) { *this = (ac_int<64,false>) b; }

  inline ac_fixed( double d ) {
    double di = ac_private::ldexpr<-(I+!S+((32-W-!S)&31))>(d);
    bool o, qb, r;
    bool neg_src = d < 0;
    Base::conv_from_fraction(di, &qb, &r, &o); 
    quantization_adjust(qb, r, neg_src);
    // a neg number may become non neg (0) after quantization
    neg_src &= o || Base::v[N-1] < 0;

    if(O!=AC_WRAP) { // saturation 
      bool overflow, underflow;
      bool neg_trg = S && (bool) this->operator[](W-1); 
      if(o) {
        overflow = !neg_src;
        underflow = neg_src;
      } else {
        bool deleted_bits_zero = !(W&31)&S || !(Base::v[N-1] >> (W&31));
        bool deleted_bits_one = !(W&31)&S || !~(Base::v[N-1] >> (W&31));
        overflow = !neg_src && (neg_trg || !deleted_bits_zero); 
        underflow = neg_src && (!neg_trg || !deleted_bits_one); 
     }
      overflow_adjust(underflow, overflow);
    } else 
      bit_adjust();
  }

  // Explicit conversion functions to ac_int that captures all integer bits (bits are truncated)
  inline ac_int<MAX(I,1),S> to_ac_int() const { return ((ac_fixed<MAX(I,1),MAX(I,1),S>) *this).slc<MAX(I,1)>(0); }

  // Explicit conversion functions to C built-in integral types -------------
  inline int to_int() const { return ((I-W) >= 32) ? 0 : (signed int) to_ac_int(); } 
  inline unsigned to_uint() const { return ((I-W) >= 32) ? 0 : (unsigned int) to_ac_int(); }
  inline long to_long() const { return ((I-W) >= 32) ? 0 : (signed long) to_ac_int(); } 
  inline unsigned long to_ulong() const { return ((I-W) >= 32) ? 0 : (unsigned long) to_ac_int(); } 
  inline Slong to_int64() const { return ((I-W) >= 64) ? 0 : (Slong) to_ac_int(); } 
  inline Ulong to_uint64() const { return ((I-W) >= 64) ? 0 : (Ulong) to_ac_int(); } 
  inline double to_double() const { return ac_private::ldexpr<I-W>(Base::to_double()); } 

  inline int length() const { return W; }

#ifndef __SYNTHESIS__
  inline std::string to_string(ac_base_mode base_rep, bool sign_mag = false) const {
    // base_rep == AC_DEC => sign_mag == don't care (always print decimal in sign magnitude)
    char r[(W-MIN(MIN(W-I,I),0)+31)/32*32+4] = {0};
    int i = 0;
    if(sign_mag)
      r[i++] = is_neg() ? '-' : '+';
    else if (base_rep == AC_DEC && is_neg())
      r[i++] = '-';
    if(base_rep != AC_DEC) {
      r[i++] = '0';
      r[i++] = base_rep == AC_BIN ? 'b' : (base_rep == AC_OCT ? 'o' : 'x');
    }
    ac_fixed<W+!S, I+!S, true> t;
    if( (base_rep == AC_DEC || sign_mag) && is_neg() )
      t = operator -();
    else
      t = *this;
    ac_fixed<MAX(I+!S,1),MAX(I+!S,1),true> i_part = t;
    ac_fixed<MAX(W-I,1),0,true> f_part = t;
    i += ac_private::to_string(i_part.v, MAX(I+!S,1), sign_mag, base_rep, false, r+i);
    if(W-I > 0) {
      r[i++] = '.';
      if(!ac_private::to_string(f_part.v, W-I, false, base_rep, true, r+i))
        r[--i] = 0;
    }
    if(!i) {
      r[0] = '0';
      r[1] = 0;
    }
    return std::string(r);
  }
#endif

  // Arithmetic : Binary ----------------------------------------------------
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  typename rt<W2,I2,S2>::mult operator *( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    typename rt<W2,I2,S2>::mult r;
    Base::mult(op2, r);
    return r;
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  typename rt<W2,I2,S2>::plus operator +( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    typename rt<W2,I2,S2>::plus r;
    if(F == F2)
      Base::add(op2, r);
    else if(F > F2)
      Base::add(op2.shiftl<F-F2>(), r);
    else
      shiftl<F2-F>().add(op2, r);
    return r;
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  typename rt<W2,I2,S2>::minus operator -( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    typename rt<W2,I2,S2>::minus r;
    if(F == F2)
      Base::sub(op2, r);
    else if(F > F2)
      Base::sub(op2.shiftl<F-F2>(), r);
    else
      shiftl<F2-F>().sub(op2, r);
    return r;
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  typename rt<W2,I2,S2>::div operator /( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    typename rt<W2,I2,S2>::div r;
    ac_fixed<W+MAX(W2-I2,0), I, S> t = *this;
    t.div(op2, r);
    return r;
  }
  // Arithmetic assign  ------------------------------------------------------
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  ac_fixed &operator *=( const ac_fixed<W2,I2,S2,Q2,O2> &op2) {
    *this = this->operator *(op2);
    return *this;
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  ac_fixed &operator +=( const ac_fixed<W2,I2,S2,Q2,O2> &op2) {
    *this = this->operator +(op2);
    return *this;
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  ac_fixed &operator -=( const ac_fixed<W2,I2,S2,Q2,O2> &op2) {
    *this = this->operator -(op2);
    return *this;
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  ac_fixed &operator /=( const ac_fixed<W2,I2,S2,Q2,O2> &op2) {
    *this = this->operator /(op2);
    return *this;
  }
  // increment/decrement by quantum (smallest difference that can be represented)
  // Arithmetic prefix increment, decrement ---------------------------------
  ac_fixed &operator ++() {
    operator+=((ac_fixed<1,I-W+1,false>) 1);
    return *this;
  }
  ac_fixed &operator --() {
    operator-=((ac_fixed<1,I-W+1,false>) 1);
    return *this;
  }
  // Arithmetic postfix increment, decrement ---------------------------------
  const ac_fixed operator ++(int) {
    ac_fixed t = *this;
    operator+=((ac_fixed<1,I-W+1,false>) 1);
    return t;
  }
  const ac_fixed operator --(int) {
    ac_fixed t = *this;
    operator-=((ac_fixed<1,I-W+1,false>) 1);
    return t;
  }
  // Arithmetic Unary --------------------------------------------------------
  ac_fixed operator +() {
    return *this;
  }
  ac_fixed<W+1,I+1,true> operator -() const {
    return ((ac_fixed<1,1,false>) 0) - *this;
  }
  // ! ------------------------------------------------------------------------
  bool operator ! () const {
    return Base::equal_zero(); 
  }

  // Bitwise (not arithmetic) unary: complement  -----------------------------
  ac_fixed<W+!S, I+!S, true> operator ~() const {
    ac_fixed<W+!S, I+!S, true> r;
    Base::bitwise_complement(r);
    return r;
  }
  // Bitwise (not arithmetic): and, or, xor ----------------------------------
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  typename rt<W2,I2,S2>::logic operator &( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    typename rt<W2,I2,S2>::logic r;
    if(F == F2)
      Base::bitwise_and(op2, r);
    else if(F > F2)
      Base::bitwise_and(op2.shiftl<F-F2>(), r);
    else
      shiftl<F2-F>().bitwise_and(op2, r);
    return r;
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  typename rt<W2,I2,S2>::logic operator |( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    typename rt<W2,I2,S2>::logic r;
    if(F == F2)
      Base::bitwise_or(op2, r);
    else if(F > F2)
      Base::bitwise_or(op2.shiftl<F-F2>(), r);
    else
      shiftl<F2-F>().bitwise_or(op2, r);
    return r;
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  typename rt<W2,I2,S2>::logic operator ^( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    typename rt<W2,I2,S2>::logic r;
    if(F == F2)
      Base::bitwise_xor(op2, r);
    else if(F > F2)
      Base::bitwise_xor(op2.shiftl<F-F2>(), r);
    else
      shiftl<F2-F>().bitwise_xor(op2, r);
    return r;
  }
  // Bitwise assign (not arithmetic): and, or, xor ----------------------------
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  ac_fixed &operator &= ( const ac_fixed<W2,I2,S2,Q2,O2> &op2 ) {
    *this = this->operator &(op2);
    return *this;
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  ac_fixed &operator |= ( const ac_fixed<W2,I2,S2,Q2,O2> &op2 ) {
    *this = this->operator |(op2);
    return *this;
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  ac_fixed &operator ^= ( const ac_fixed<W2,I2,S2,Q2,O2> &op2 ) {
    *this = this->operator ^(op2);
    return *this;
  }
  // Shift (result constrained by left operand) -------------------------------
  template<int W2>
  ac_fixed operator << ( const ac_int<W2,true> &op2 ) const {
    // currently not written to overflow or quantize (neg shift)
    ac_fixed r;
    Base::shift_l2(op2.to_int(), r);
    r.bit_adjust();
    return r;
  }
  template<int W2>
  ac_fixed operator << ( const ac_int<W2,false> &op2 ) const {
    // currently not written to overflow
    ac_fixed r;
    Base::shift_l(op2.to_uint(), r);
    r.bit_adjust();
    return r;
  }
  template<int W2>
  ac_fixed operator >> ( const ac_int<W2,true> &op2 ) const {
    // currently not written to quantize or overflow (neg shift)
    ac_fixed r;
    Base::shift_r2(op2.to_int(), r);
    r.bit_adjust();
    return r;
  }
  template<int W2>
  ac_fixed operator >> ( const ac_int<W2,false> &op2 ) const {
    // currently not written to quantize 
    ac_fixed r;
    Base::shift_r(op2.to_uint(), r);
    r.bit_adjust();
    return r;
  }
  // Shift assign ------------------------------------------------------------
  template<int W2>
  ac_fixed &operator <<= ( const ac_int<W2,true> &op2 ) {
    // currently not written to overflow or quantize (neg shift)
    Base r;
    Base::shift_l2(op2.to_int(), r);
    Base::operator=(r);
    bit_adjust();
    return *this;
  }
  template<int W2>
  ac_fixed &operator <<= ( const ac_int<W2,false> &op2 ) {
    // currently not written to overflow
    Base r;
    Base::shift_l(op2.to_uint(), r);
    Base::operator=(r);
    bit_adjust();
    return *this;
  }
  template<int W2>
  ac_fixed &operator >>= ( const ac_int<W2,true> &op2 ) {
    // currently not written to quantize or overflow (neg shift)
    Base r;
    Base::shift_r2(op2.to_int(), r);
    Base::operator=(r);
    bit_adjust();
    return *this;
  }
  template<int W2>
  ac_fixed &operator >>= ( const ac_int<W2,false> &op2 ) {
    // currently not written to quantize 
    Base r;
    Base::shift_r(op2.to_uint(), r);
    Base::operator=(r);
    bit_adjust();
    return *this;
  }
  // Relational ---------------------------------------------------------------
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  bool operator == ( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    if(F == F2)
      return Base::equal(op2);
    else if(F > F2)
      return Base::equal(op2.shiftl<F-F2>());
    else
      return shiftl<F2-F>().equal(op2);
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  bool operator != ( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    if(F == F2)
      return ! Base::equal(op2);
    else if(F > F2)
      return ! Base::equal(op2.shiftl<F-F2>());
    else
      return ! shiftl<F2-F>().equal(op2);
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  bool operator < ( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    if(F == F2)
      return Base::less_than(op2);
    else if(F > F2)
      return Base::less_than(op2.shiftl<F-F2>());
    else
      return shiftl<F2-F>().less_than(op2);
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  bool operator >= ( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    if(F == F2)
      return ! Base::less_than(op2);
    else if(F > F2)
      return ! Base::less_than(op2.shiftl<F-F2>());
    else
      return ! shiftl<F2-F>().less_than(op2);
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  bool operator > ( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    if(F == F2)
      return Base::greater_than(op2);
    else if(F > F2)
      return Base::greater_than(op2.shiftl<F-F2>());
    else
      return shiftl<F2-F>().greater_than(op2); 
  }
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2>
  bool operator <= ( const ac_fixed<W2,I2,S2,Q2,O2> &op2) const {
    enum { F=W-I, F2=W2-I2 };
    if(F == F2)
      return ! Base::greater_than(op2);
    else if(F > F2)
      return ! Base::greater_than(op2.shiftl<F-F2>());
    else
      return ! shiftl<F2-F>().greater_than(op2);
  }
  bool operator == ( double d) const {
    if(is_neg() != d < 0.0)
      return false;
    double di = ac_private::ldexpr<-(I+(32-(W&31)))>(d);
    bool overflow, qb, r;
    ac_fixed<W,I,S> t;
    t.conv_from_fraction(di, &qb, &r, &overflow);
    if(qb || r || overflow)
      return false;
    return operator == (t);
  }
  bool operator != ( double d) const {
    return !operator == ( d );
  }
  bool operator < ( double d) const {
    if(is_neg() != d < 0.0)
      return is_neg();
    double di = ac_private::ldexpr<-(I+(32-(W&31)))>(d);
    bool overflow, qb, r;
    ac_fixed<W,I,S> t;
    t.conv_from_fraction(di, &qb, &r, &overflow);
    if(is_neg() && overflow)
      return false;
    return !is_neg() && overflow || (qb || r) && operator <= (t) || operator < (t);
  }
  bool operator >= ( double d) const {
    return !operator < ( d );
  }
  bool operator > ( double d) const {
    if(is_neg() != d < 0.0)
      return !is_neg();
    double di = ac_private::ldexpr<-(I+(32-(W&31)))>(d);
    bool overflow, qb, r;
    ac_fixed<W,I,S> t;
    t.conv_from_fraction(di, &qb, &r, &overflow);
    if(!is_neg() && overflow )
      return false;
    return is_neg() && overflow || operator > (t);
  }
  bool operator <= ( double d) const {
    return !operator > ( d );
  }

  // Bit and Slice Select -----------------------------------------------------
  template<int WS, int WX, bool SX>
  inline ac_int<WS,S> slc(const ac_int<WX,SX> &index) const {
    ac_int<WS,S> r;
    AC_ASSERT(index >= 0, "Attempting to read slc with negative indeces");
    ac_int<WX-SX, false> uindex = index;
    Base::shift_r(uindex.to_uint(), r);
    r.bit_adjust();
    return r; 
  }

  template<int WS>
  inline ac_int<WS,S> slc(signed index) const {
    ac_int<WS,S> r;
    AC_ASSERT(index >= 0, "Attempting to read slc with negative indeces");
    unsigned uindex = index & ((unsigned)~0 >> 1);
    Base::shift_r(uindex, r);
    r.bit_adjust();
    return r; 
  }
  template<int WS>
  inline ac_int<WS,S> slc(unsigned uindex) const {
    ac_int<WS,S> r;
    Base::shift_r(uindex, r);
    r.bit_adjust();
    return r; 
  }

  template<int W2, bool S2, int WX, bool SX>
  inline ac_fixed &set_slc(const ac_int<WX,SX> lsb, const ac_int<W2,S2> &slc) {
    AC_ASSERT(lsb.to_int() + W2 <= W && lsb.to_int() >= 0, "Out of bounds set_slc");
    ac_int<WX-SX, false> ulsb = lsb;
    Base::set_slc(ulsb.to_uint(), W2, (ac_int<W2,true>) slc);
    bit_adjust();   // in case sign bit was assigned 
    return *this;
  }
  template<int W2, bool S2>
  inline ac_fixed &set_slc(signed lsb, const ac_int<W2,S2> &slc) {
    AC_ASSERT(lsb + W2 <= W && lsb >= 0, "Out of bounds set_slc");
    unsigned ulsb = lsb & ((unsigned)~0 >> 1);
    Base::set_slc(ulsb, W2, (ac_int<W2,true>) slc);
    bit_adjust();   // in case sign bit was assigned 
    return *this;
  }
  template<int W2, bool S2>
  inline ac_fixed &set_slc(unsigned ulsb, const ac_int<W2,S2> &slc) {
    AC_ASSERT(ulsb + W2 <= W, "Out of bounds set_slc");
    Base::set_slc(ulsb, W2, (ac_int<W2,true>) slc);
    bit_adjust();   // in case sign bit was assigned 
    return *this;
  }

  class ac_bitref {
# if defined(__SYNTHESIS__) && !defined(AC_IGNORE_BUILTINS)
# pragma builtin
# endif
    ac_fixed &d_bv;
    unsigned d_index;
  public:
    ac_bitref( ac_fixed *bv, unsigned index=0 ) : d_bv(*bv), d_index(index) {}
    operator bool () const { return (d_index < W) ? (d_bv.v[d_index>>5]>>(d_index&31) & 1) : 0; }

    inline ac_bitref operator = ( int val ) {
      // lsb of int (val&1) is written to bit
      if(d_index < W) {
        int *pval = &d_bv.v[d_index>>5];
        *pval ^= (*pval ^ (val << (d_index&31) )) & 1 << (d_index&31);
        d_bv.bit_adjust();   // in case sign bit was assigned
      }
      return *this;
    }
    template<int W2, bool S2>
    inline ac_bitref operator = ( const ac_int<W2,S2> &val ) {
      return operator =(val.to_int());
    }
    inline ac_bitref operator = ( const ac_bitref &val ) {
      return operator =((int) (bool) val);
    }
  };
                                                                                                             
  ac_bitref operator [] ( unsigned int uindex) {
    AC_ASSERT(uindex < W, "Attempting to read bit beyond MSB");
    ac_bitref bvh( this, uindex );
    return bvh;
  }
  ac_bitref operator [] ( int index) {
    AC_ASSERT(index >= 0, "Attempting to read bit with negative index");
    AC_ASSERT(index < W, "Attempting to read bit beyond MSB");
    unsigned uindex = index & ((unsigned)~0 >> 1);
    ac_bitref bvh( this, uindex );
    return bvh;
  }
  template<int W2, bool S2>
  ac_bitref operator [] ( const ac_int<W2,S2> &index) {
    AC_ASSERT(index >= 0, "Attempting to read bit with negative index");
    AC_ASSERT(index < W, "Attempting to read bit beyond MSB");
    ac_int<W2-S2,false> uindex = index;
    ac_bitref bvh( this, uindex.to_uint() );
    return bvh;
  }

  bool operator [] ( unsigned int uindex) const {
    AC_ASSERT(uindex < W, "Attempting to read bit beyond MSB");
    return (uindex < W) ? (Base::v[uindex>>5]>>(uindex&31) & 1) : 0;
  }
  bool operator [] ( int index) const {
    AC_ASSERT(index >= 0, "Attempting to read bit with negative index");
    AC_ASSERT(index < W, "Attempting to read bit beyond MSB");
    unsigned uindex = index & ((unsigned)~0 >> 1);
    return (uindex < W) ? (Base::v[uindex>>5]>>(uindex&31) & 1) : 0;
  }
  template<int W2, bool S2>
  bool operator [] ( const ac_int<W2,S2> &index) const {
    AC_ASSERT(index >= 0, "Attempting to read bit with negative index");
    AC_ASSERT(index < W, "Attempting to read bit beyond MSB");
    ac_int<W2-S2,false> uindex = index;
    return (uindex < W) ? (Base::v[uindex>>5]>>(uindex&31) & 1) : 0;
  }
};

// Specializations for constructors on integers that bypass bit adjusting
//  and are therefore more efficient
template<> inline ac_fixed<1,1,true,AC_TRN,AC_WRAP>::ac_fixed( bool b ) { v[0] = b ? -1 : 0; }

template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( bool b ) { v[0] = b; }
template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( signed char b ) { v[0] = b&1; }
template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( unsigned char b ) { v[0] = b&1; }
template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( signed short b ) { v[0] = b&1; }
template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( unsigned short b ) { v[0] = b&1; }
template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( signed int b ) { v[0] = b&1; }
template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( unsigned int b ) { v[0] = b&1; }
template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( signed long b ) { v[0] = b&1; }
template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( unsigned long b ) { v[0] = b&1; }
template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( Ulong b ) { v[0] = (int) b&1; }
template<> inline ac_fixed<1,1,false,AC_TRN,AC_WRAP>::ac_fixed( Slong b ) { v[0] = (int) b&1; }

template<> inline ac_fixed<8,8,true,AC_TRN,AC_WRAP>::ac_fixed( bool b ) { v[0] = b; }
template<> inline ac_fixed<8,8,false,AC_TRN,AC_WRAP>::ac_fixed( bool b ) { v[0] = b; }
template<> inline ac_fixed<8,8,true,AC_TRN,AC_WRAP>::ac_fixed( signed char b ) { v[0] = b; }
template<> inline ac_fixed<8,8,false,AC_TRN,AC_WRAP>::ac_fixed( unsigned char b ) { v[0] = b; }
template<> inline ac_fixed<8,8,true,AC_TRN,AC_WRAP>::ac_fixed( unsigned char b ) { v[0] = (signed char) b; }
template<> inline ac_fixed<8,8,false,AC_TRN,AC_WRAP>::ac_fixed( signed char b ) { v[0] = (unsigned char) b; }

template<> inline ac_fixed<16,16,true,AC_TRN,AC_WRAP>::ac_fixed( bool b ) { v[0] = b; }
template<> inline ac_fixed<16,16,false,AC_TRN,AC_WRAP>::ac_fixed( bool b ) { v[0] = b; }
template<> inline ac_fixed<16,16,true,AC_TRN,AC_WRAP>::ac_fixed( signed char b ) { v[0] = b; }
template<> inline ac_fixed<16,16,false,AC_TRN,AC_WRAP>::ac_fixed( unsigned char b ) { v[0] = b; }
template<> inline ac_fixed<16,16,true,AC_TRN,AC_WRAP>::ac_fixed( unsigned char b ) { v[0] = b; }
template<> inline ac_fixed<16,16,false,AC_TRN,AC_WRAP>::ac_fixed( signed char b ) { v[0] = (unsigned short) b; }
template<> inline ac_fixed<16,16,true,AC_TRN,AC_WRAP>::ac_fixed( signed short b ) { v[0] = b; }
template<> inline ac_fixed<16,16,false,AC_TRN,AC_WRAP>::ac_fixed( unsigned short b ) { v[0] = b; }
template<> inline ac_fixed<16,16,true,AC_TRN,AC_WRAP>::ac_fixed( unsigned short b ) { v[0] = (signed short) b; }
template<> inline ac_fixed<16,16,false,AC_TRN,AC_WRAP>::ac_fixed( signed short b ) { v[0] = (unsigned short) b; }

template<> inline ac_fixed<32,32,true,AC_TRN,AC_WRAP>::ac_fixed( signed int b ) { v[0] = b; }
template<> inline ac_fixed<32,32,true,AC_TRN,AC_WRAP>::ac_fixed( unsigned int b ) { v[0] = b; }
template<> inline ac_fixed<32,32,false,AC_TRN,AC_WRAP>::ac_fixed( signed int b ) { v[0] = b; v[1] = 0;}
template<> inline ac_fixed<32,32,false,AC_TRN,AC_WRAP>::ac_fixed( unsigned int b ) { v[0] = b; v[1] = 0;}

template<> inline ac_fixed<32,32,true,AC_TRN,AC_WRAP>::ac_fixed( Slong b ) { v[0] = (int) b; }
template<> inline ac_fixed<32,32,true,AC_TRN,AC_WRAP>::ac_fixed( Ulong b ) { v[0] = (int) b; }
template<> inline ac_fixed<32,32,false,AC_TRN,AC_WRAP>::ac_fixed( Slong b ) { v[0] = (int) b; v[1] = 0;}
template<> inline ac_fixed<32,32,false,AC_TRN,AC_WRAP>::ac_fixed( Ulong b ) { v[0] = (int) b; v[1] = 0;}

template<> inline ac_fixed<64,64,true,AC_TRN,AC_WRAP>::ac_fixed( Slong b ) { v[0] = (int) b; v[1] = (int) (b >> 32); }
template<> inline ac_fixed<64,64,true,AC_TRN,AC_WRAP>::ac_fixed( Ulong b ) { v[0] = (int) b; v[1] = (int) (b >> 32);}
template<> inline ac_fixed<64,64,false,AC_TRN,AC_WRAP>::ac_fixed( Slong b ) { v[0] = (int) b; v[1] = (int) ((Ulong) b >> 32); v[2] = 0; }
template<> inline ac_fixed<64,64,false,AC_TRN,AC_WRAP>::ac_fixed( Ulong b ) { v[0] = (int) b; v[1] = (int) (b >> 32); v[2] = 0; }


// Stream --------------------------------------------------------------------

#ifndef __SYNTHESIS__
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
inline std::ostream& operator << (std::ostream &os, const ac_fixed<W,I,S,Q,O> &x) {
  // for now dec is supported for precisions representable in 64 bit signed 
  os << x.to_string(AC_DEC);
  return os;
}
#endif
// Binary Operators with Integers --------------------------------------------

#define FX_BIN_OP_WITH_INT_2I(BIN_OP, C_TYPE, WI, SI, RTYPE)  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O> \
  inline typename ac_fixed<W,I,S>::template rt<WI,WI,SI>::RTYPE operator BIN_OP ( const ac_fixed<W,I,S,Q,O> &op, C_TYPE i_op) {  \
    return op.operator BIN_OP (ac_int<WI,SI>(i_op));  \
  }

#define FX_BIN_OP_WITH_INT(BIN_OP, C_TYPE, WI, SI, RTYPE)  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O> \
  inline typename ac_fixed<WI,WI,SI>::template rt<W,I,S>::RTYPE operator BIN_OP ( C_TYPE i_op, const ac_fixed<W,I,S,Q,O> &op) {  \
    return ac_fixed<WI,WI,SI>(i_op).operator BIN_OP (op);  \
  } \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O> \
  inline typename ac_fixed<W,I,S>::template rt<WI,WI,SI>::RTYPE operator BIN_OP ( const ac_fixed<W,I,S,Q,O> &op, C_TYPE i_op) {  \
    return op.operator BIN_OP (ac_fixed<WI,WI,SI>(i_op));  \
  }

#define FX_REL_OP_WITH_INT(REL_OP, C_TYPE, W2, S2)  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O> \
  inline bool operator REL_OP ( const ac_fixed<W,I,S,Q,O> &op, C_TYPE op2) {  \
    return op.operator REL_OP (ac_fixed<W2,W2,S2>(op2));  \
  }  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O> \
  inline bool operator REL_OP ( C_TYPE op2, const ac_fixed<W,I,S,Q,O> &op) {  \
    return ac_fixed<W2,W2,S2>(op2).operator REL_OP (op);  \
  }

#define FX_ASSIGN_OP_WITH_INT_2(ASSIGN_OP, C_TYPE, W2, S2)  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O> \
  inline ac_fixed<W,I,S,Q,O> &operator ASSIGN_OP ( ac_fixed<W,I,S,Q,O> &op, C_TYPE op2) {  \
    return op.operator ASSIGN_OP (ac_fixed<W2,W2,S2>(op2));  \
  }

#define FX_ASSIGN_OP_WITH_INT_2I(ASSIGN_OP, C_TYPE, W2, S2)  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O> \
  inline ac_fixed<W,I,S,Q,O> &operator ASSIGN_OP ( ac_fixed<W,I,S,Q,O> &op, C_TYPE op2) {  \
    return op.operator ASSIGN_OP (ac_int<W2,S2>(op2));  \
  }

#define FX_OPS_WITH_INT(C_TYPE, WI, SI) \
  FX_BIN_OP_WITH_INT(*, C_TYPE, WI, SI, mult) \
  FX_BIN_OP_WITH_INT(+, C_TYPE, WI, SI, plus) \
  FX_BIN_OP_WITH_INT(-, C_TYPE, WI, SI, minus) \
  FX_BIN_OP_WITH_INT(/, C_TYPE, WI, SI, div) \
  FX_BIN_OP_WITH_INT_2I(>>, C_TYPE, WI, SI, arg1) \
  FX_BIN_OP_WITH_INT_2I(<<, C_TYPE, WI, SI, arg1) \
  FX_BIN_OP_WITH_INT(&, C_TYPE, WI, SI, logic) \
  FX_BIN_OP_WITH_INT(|, C_TYPE, WI, SI, logic) \
  FX_BIN_OP_WITH_INT(^, C_TYPE, WI, SI, logic) \
  \
  FX_REL_OP_WITH_INT(==, C_TYPE, WI, SI) \
  FX_REL_OP_WITH_INT(!=, C_TYPE, WI, SI) \
  FX_REL_OP_WITH_INT(>, C_TYPE, WI, SI) \
  FX_REL_OP_WITH_INT(>=, C_TYPE, WI, SI) \
  FX_REL_OP_WITH_INT(<, C_TYPE, WI, SI) \
  FX_REL_OP_WITH_INT(<=, C_TYPE, WI, SI) \
  \
  FX_ASSIGN_OP_WITH_INT_2(+=, C_TYPE, WI, SI) \
  FX_ASSIGN_OP_WITH_INT_2(-=, C_TYPE, WI, SI) \
  FX_ASSIGN_OP_WITH_INT_2(*=, C_TYPE, WI, SI) \
  FX_ASSIGN_OP_WITH_INT_2(/=, C_TYPE, WI, SI) \
  FX_ASSIGN_OP_WITH_INT_2(%=, C_TYPE, WI, SI) \
  FX_ASSIGN_OP_WITH_INT_2I(>>=, C_TYPE, WI, SI) \
  FX_ASSIGN_OP_WITH_INT_2I(<<=, C_TYPE, WI, SI) \
  FX_ASSIGN_OP_WITH_INT_2(&=, C_TYPE, WI, SI) \
  FX_ASSIGN_OP_WITH_INT_2(|=, C_TYPE, WI, SI) \
  FX_ASSIGN_OP_WITH_INT_2(^=, C_TYPE, WI, SI)

FX_OPS_WITH_INT(bool, 1, false)
FX_OPS_WITH_INT(char, 8, true)
FX_OPS_WITH_INT(signed char, 8, true)
FX_OPS_WITH_INT(unsigned char, 8, false)
FX_OPS_WITH_INT(short, 16, true)
FX_OPS_WITH_INT(unsigned short, 16, false)
FX_OPS_WITH_INT(int, 32, true)
FX_OPS_WITH_INT(unsigned int, 32, false)
FX_OPS_WITH_INT(long, 32, true)
FX_OPS_WITH_INT(unsigned long, 32, false)
FX_OPS_WITH_INT(Slong, 64, true)
FX_OPS_WITH_INT(Ulong, 64, false)


#define FX_BIN_OP_WITH_AC_INT_1(BIN_OP, RTYPE)  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O, int WI, bool SI> \
  inline typename ac_fixed<WI,WI,SI>::template rt<W,I,S>::RTYPE operator BIN_OP ( const ac_int<WI,SI> &i_op, const ac_fixed<W,I,S,Q,O> &op) {  \
    return ac_fixed<WI,WI,SI>(i_op).operator BIN_OP (op);  \
  }

#define FX_BIN_OP_WITH_AC_INT_2(BIN_OP, RTYPE)  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O, int WI, bool SI> \
  inline typename ac_fixed<W,I,S>::template rt<WI,WI,SI>::RTYPE operator BIN_OP ( const ac_fixed<W,I,S,Q,O> &op, const ac_int<WI,SI> &i_op) {  \
    return op.operator BIN_OP (ac_fixed<WI,WI,SI>(i_op));  \
  }

#define FX_BIN_OP_WITH_AC_INT(BIN_OP, RTYPE)  \
  FX_BIN_OP_WITH_AC_INT_1(BIN_OP, RTYPE) \
  FX_BIN_OP_WITH_AC_INT_2(BIN_OP, RTYPE)

#define FX_REL_OP_WITH_AC_INT(REL_OP)  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O, int WI, bool SI> \
  inline bool operator REL_OP ( const ac_fixed<W,I,S,Q,O> &op, const ac_int<WI,SI> &op2) {  \
    return op.operator REL_OP (ac_fixed<WI,WI,SI>(op2));  \
  }  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O, int WI, bool SI> \
  inline bool operator REL_OP ( ac_int<WI,SI> &op2, const ac_fixed<W,I,S,Q,O> &op) {  \
    return ac_fixed<WI,WI,SI>(op2).operator REL_OP (op);  \
  }

#define FX_ASSIGN_OP_WITH_AC_INT(ASSIGN_OP)  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O, int WI, bool SI> \
  inline ac_fixed<W,I,S,Q,O> &operator ASSIGN_OP ( ac_fixed<W,I,S,Q,O> &op, const ac_int<WI,SI> &op2) {  \
    return op.operator ASSIGN_OP (ac_fixed<WI,WI,SI>(op2));  \
  }  \
  template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O, int WI, bool SI> \
  inline ac_int<WI,SI> &operator ASSIGN_OP ( ac_int<WI,SI> &op, const ac_fixed<W,I,S,Q,O> &op2) {  \
    return op.operator ASSIGN_OP (op2.to_ac_int());  \
  }  

FX_BIN_OP_WITH_AC_INT(*, mult)
FX_BIN_OP_WITH_AC_INT(+, plus)
FX_BIN_OP_WITH_AC_INT(-, minus)
FX_BIN_OP_WITH_AC_INT(/, div)
FX_BIN_OP_WITH_AC_INT(&, logic)
FX_BIN_OP_WITH_AC_INT(|, logic)
FX_BIN_OP_WITH_AC_INT(^, logic)

FX_REL_OP_WITH_AC_INT(==)
FX_REL_OP_WITH_AC_INT(!=)
FX_REL_OP_WITH_AC_INT(>)
FX_REL_OP_WITH_AC_INT(>=)
FX_REL_OP_WITH_AC_INT(<)
FX_REL_OP_WITH_AC_INT(<=)

FX_ASSIGN_OP_WITH_AC_INT(+=)
FX_ASSIGN_OP_WITH_AC_INT(-=)
FX_ASSIGN_OP_WITH_AC_INT(*=)
FX_ASSIGN_OP_WITH_AC_INT(/=)
FX_ASSIGN_OP_WITH_AC_INT(%=)
FX_ASSIGN_OP_WITH_AC_INT(&=)
FX_ASSIGN_OP_WITH_AC_INT(|=)
FX_ASSIGN_OP_WITH_AC_INT(^=)

// Relational Operators with double 
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
inline bool operator == ( double op, const ac_fixed<W,I,S,Q,O> &op2) {
  return op2.operator == (op); 
}
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
inline bool operator != ( double op, const ac_fixed<W,I,S,Q,O> &op2) {
  return op2.operator != (op); 
}
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
inline bool operator > ( double op, const ac_fixed<W,I,S,Q,O> &op2) {
  return op2.operator < (op); 
}
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
inline bool operator < ( double op, const ac_fixed<W,I,S,Q,O> &op2) {
  return op2.operator > (op); 
}
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
inline bool operator <= ( double op, const ac_fixed<W,I,S,Q,O> &op2) {
  return op2.operator >= (op); 
}
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
inline bool operator >= ( double op, const ac_fixed<W,I,S,Q,O> &op2) {
  return op2.operator <= (op); 
}

// Global templatized functions for easy initialization to special values
template<ac_special_val V, int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
inline ac_fixed<W,I,S,Q,O> value(ac_fixed<W,I,S,Q,O>) {
  ac_fixed<W,I,S> r;
  if(V == AC_VAL_0)
    r = 0;
  return r;
}

namespace ac {
// PUBLIC FUNCTIONS
// function to initialize (or uninitialize) arrays
  template<ac_special_val V, int W, int I, bool S, ac_q_mode Q, ac_o_mode O> 
  inline bool init_array(ac_fixed<W,I,S,Q,O> *a, int n) {
    ac_fixed<W,I,S> t = value<V>(*a);
    for(int i=0; i < n; i++)
      a[i] = t;
    return true;
  }
}


///////////////////////////////////////////////////////////////////////////////

#if (defined(_MSC_VER) && !defined(__EDG__))
#pragma warning( pop )
#endif

#endif // __AC_FIXED_H
