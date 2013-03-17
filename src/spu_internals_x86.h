/**

Quadwords on little endian is a full 16-byte reverse copy of the big endian
memory. Thus on LE, the preferred slot is the 4th word in the qword.

Operations that work on sub-elements only, like words, should work out
of the box.
Those that cross boundaries -like 128bit shift, qword rotations- must
take extra care that if the result were to byte-reversed it would give the
correct big endian value.

*/

#pragma once
#ifndef __SPU_INTERNALS_X86_H__
#define __SPU_INTERNALS_X86_H__

#include <cstdint>
#include "sse_extensions.h"

typedef __m128 GPR_t;

static const __m128i __fsmb_mask_8 = _mm_setr_epi8( 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80,
													0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 );
static const __m128i __fsmb_mask_16 = _mm_setr_epi16( 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 );
static const __m128i __fsmb_mask_32 = _mm_setr_epi32( 0x1, 0x2, 0x4, 0x8 );


// Helpers
//////////////////////////////////////////////////////////////////////////

inline uint64_t si_to_ullong_aux(__m128 ra)
{ 
const uint64_t r0 = (uint16_t)_mm_extract_epi16(_mm_castps_si128(ra), 4);
const uint64_t r1 = (uint16_t)_mm_extract_epi16(_mm_castps_si128(ra), 5);
const uint64_t r2 = (uint16_t)_mm_extract_epi16(_mm_castps_si128(ra), 6);
const uint64_t r3 = (uint16_t)_mm_extract_epi16(_mm_castps_si128(ra), 7);
return (r3 << 48) | (r2 << 32) | (r1 << 16) | r0;
}

inline float si_to_float_aux(__m128 ra)
{
const uint32_t u = (uint32_t)si_to_ullong_aux(ra);
return (float&)u;
}

inline double si_to_double_aux(__m128 ra)
{
const uint64_t u = si_to_ullong_aux(ra);
return (double&)u;
}

/*
 * Convert to scalar
 */
#if 0
#define si_to_char(ra)		(int8_t)(uint16_t)_mm_extract_epi16(_mm_castps_si128((ra)), 6)
#define si_to_uchar(ra)     (uint8_t)(uint16_t)_mm_extract_epi16(_mm_castps_si128((ra)), 6)
#define si_to_short(ra)     (int16_t)_mm_extract_epi16(_mm_castps_si128((ra)), 6)
#define si_to_ushort(ra)    (uint16_t)_mm_extract_epi16(_mm_castps_si128((ra)), 6)
#define si_to_int(ra)       (int32_t)(((uint32_t)_mm_extract_epi16(_mm_castps_si128((ra)), 7) << 16) | (uint32_t)_mm_extract_epi16(_mm_castps_si128((ra)), 6))
#define si_to_uint(ra)      (uint32_t)(((uint32_t)_mm_extract_epi16(_mm_castps_si128((ra)), 7) << 16) | (uint32_t)_mm_extract_epi16(_mm_castps_si128((ra)), 6))
#define si_to_llong(ra)     (int64_t)si_to_ullong_aux((ra))
#define si_to_ullong(ra)    (uint64_t)si_to_ullong_aux((ra))
#define si_to_float(ra)     si_to_float_aux((ra))
#define si_to_double(ra)    si_to_double_aux((ra))
//#define si_to_ptr(ra)       (void*)si_to_ullong_aux((ra))
#else
#define si_to_char(ra)		(int8_t)_mm_extract_epi8(_mm_castps_si128((ra)), 12)
#define si_to_uchar(ra)     (uint8_t)_mm_extract_epi8(_mm_castps_si128((ra)), 12)
#define si_to_short(ra)     (int16_t)_mm_extract_epi16(_mm_castps_si128((ra)), 6)
#define si_to_ushort(ra)    (uint16_t)_mm_extract_epi16(_mm_castps_si128((ra)), 6)
#define si_to_int(ra)       (int32_t)_mm_extract_epi32(_mm_castps_si128((ra)), 3)
#define si_to_uint(ra)      (uint32_t)_mm_extract_epi32(_mm_castps_si128((ra)), 3)
#define si_to_llong(ra)     (int64_t)_mm_extract_epi64(_mm_castps_si128((ra)), 1)
#define si_to_ullong(ra)    (uint64_t)_mm_extract_epi64(_mm_castps_si128((ra)), 1)
#define si_to_float(ra)     (float)_mm_extract_ps(_mm_castps_si128((ra)), 3)
#define si_to_double(ra)    (double)_mm_extract_epi64(_mm_castps_si128((ra)), 1)
//#define si_to_ptr(ra)       (void*)si_to_ullong_aux((ra))\

#define si_from_char(s)		_mm_insert_epi8(_mm_setzero_si128(), (int)s, 12)
#define si_from_uchar(s)    _mm_insert_epi8(_mm_setzero_si128(), (int)s, 12)
#define si_from_short(s)    _mm_insert_epi16(_mm_setzero_si128(), (int)s, 6)
#define si_from_ushort(s)   _mm_insert_epi16(_mm_setzero_si128(), (int)s, 6)
#define si_from_int(s)      _mm_insert_epi32(_mm_setzero_si128(), (int)s, 3)
#define si_from_uint(s)     _mm_insert_epi32(_mm_setzero_si128(), (int)s, 3)
#define si_from_llong(s)    _mm_insert_epi64(_mm_setzero_si128(), (int64_t)s, 1)
#define si_from_ullong(s)   _mm_insert_epi64(_mm_setzero_si128(), (int64_t)s, 1)
#define si_from_float(s)    _mm_insert_epi32(_mm_setzero_si128(), (int)s, 3)
#define si_from_double(s)   _mm_insert_epi64(_mm_setzero_si128(), (int64_t)s, 1)
#endif

/*
 * Constant-Formation Instructions 
 */

typedef const int8_t  I8_t;
typedef const int16_t I16_t;
typedef const int32_t I32_t;
typedef const int64_t I64_t;


inline GPR_t si_ilh	( const int16_t I16 )			{ return _mm_castsi128_ps( _mm_set1_epi16( I16 ) ); }
inline GPR_t si_ilhu( const int16_t I16 )			{ return _mm_castsi128_ps( _mm_set1_epi32( (int32_t)I16 << 16 ) ); }
inline GPR_t si_il	( const int16_t I16 )			{ return _mm_castsi128_ps( _mm_set1_epi32( (int32_t)I16 ) ); }
inline GPR_t si_ila	( const int32_t I18 )			{ return _mm_castsi128_ps( _mm_set1_epi32( 0x3FFFF & I18 ) ); }
inline GPR_t si_iohl( GPR_t RA, const int16_t I16 )	{ return _mm_castsi128_ps( _mm_or_si128( _mm_castps_si128( RA ), _mm_set1_epi32( I16 ) ) ); }

inline GPR_t si_fsmbi( const int16_t I16 ){
	const __m128i mask_all_set = _mm_setr_epi8( 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80i8,
		0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80i8 );

	const __m128i r1 = _mm_set1_epi8( 0xff & I16 );
	const __m128i r2 = _mm_set1_epi8( 0xff & (I16 >> 8) );
	const __m128i r3 = _mm_srli_si128( r1, 8 );
	const __m128i r4 = _mm_slli_si128( r2, 8 );
	const __m128i r5 = _mm_and_si128( r3, mask_all_set );
	const __m128i r6 = _mm_and_si128( r4, mask_all_set );
	const __m128i r7 = _mm_or_si128( r5, r6 );
	const __m128i r8 = _mm_cmpeq_epi8( mask_all_set, r7 );
	return _mm_castsi128_ps( r8 );
}

/*
 * Add Instructions 
 */



inline GPR_t si_ah( GPR_t RA, GPR_t RB ){
	return _mm_castsi128_ps( _mm_add_epi16( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_ahi( GPR_t RA, int16_t I10 ){
	return _mm_castsi128_ps( _mm_add_epi16( _mm_castps_si128( RA ), _mm_set1_epi16( I10 ) ) );
}

inline GPR_t si_a( GPR_t RA, GPR_t RB ){
	return _mm_castsi128_ps( _mm_add_epi32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_ai( GPR_t RA, int16_t I10 ){
	return _mm_castsi128_ps( _mm_add_epi32( _mm_castps_si128( RA ), _mm_set1_epi32( I10 ) ) );
}

inline GPR_t si_addx( GPR_t RA, GPR_t RB, GPR_t RT ){
	const __m128i sum			= _mm_add_epi32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) );
	const __m128i carry			= _mm_and_si128( _mm_castps_si128( RT ), _mm_set1_epi32( 1 ) );
	return _mm_castsi128_ps( _mm_add_epi32( sum, carry ) );
}

inline GPR_t si_cg( GPR_t RA, GPR_t RB ){
	const __m128i RA_and_RB		= _mm_and_si128( _mm_castps_si128( RA ), _mm_castps_si128( RB ) );
	const __m128i RA_and_RB_LSB = _mm_srli_epi32( _mm_slli_epi32( RA_and_RB, 31 ), 31 );
	const __m128i RA_shift		= _mm_srli_epi32( _mm_castps_si128( RA ), 1 );
	const __m128i RB_shift		= _mm_srli_epi32( _mm_castps_si128( RB ), 1 );
	const __m128i sum			= _mm_add_epi32( RA_shift, RB_shift );
	const __m128i sum_adjusted	= _mm_add_epi32( sum, RA_and_RB_LSB );
	const __m128i carry			= _mm_srli_epi32( sum_adjusted, 31 );
	return _mm_castsi128_ps( carry );
}

inline GPR_t si_cgx( GPR_t RA, GPR_t RB, GPR_t RT ){
	const __m128i RA_and_RB		= _mm_and_si128( _mm_castps_si128( RA ), _mm_castps_si128( RB ) );
	const __m128i RA_and_RB_LSB = _mm_srli_epi32( _mm_slli_epi32( RA_and_RB, 31 ), 31 );
	const __m128i RA_shift		= _mm_srli_epi32( _mm_castps_si128( RA ), 1 );
	const __m128i RB_shift		= _mm_srli_epi32( _mm_castps_si128( RB ), 1 );
	const __m128i sum			= _mm_add_epi32( RA_shift, RB_shift );
	const __m128i sum_adjusted	= _mm_add_epi32( sum, RA_and_RB_LSB );
	const __m128i RT_carry		= _mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128( RT ), 31 ), 31 );
	return _mm_castsi128_ps( _mm_srli_epi32( _mm_add_epi32( RT_carry, sum_adjusted ), 31 ) );
}


/*
 * Sub Instructions 
 */

// "sf a, b" -> "a substituted from b" -> "b - a"

inline GPR_t si_sfh( GPR_t RA, GPR_t RB ){
	return _mm_castsi128_ps( _mm_sub_epi16( _mm_castps_si128( RB ), _mm_castps_si128( RA ) ) );
}

inline GPR_t si_sfhi( GPR_t RA, int16_t IMM ){
	return _mm_castsi128_ps( _mm_sub_epi16( _mm_set1_epi16( IMM ), _mm_castps_si128( RA ) ) );
}

inline GPR_t si_sf( GPR_t RA, GPR_t RB ){
	return _mm_castsi128_ps( _mm_sub_epi32( _mm_castps_si128( RB ), _mm_castps_si128( RA ) ) );
}

inline GPR_t si_sfi( GPR_t RA, int16_t IMM ){
	return _mm_castsi128_ps( _mm_sub_epi16( _mm_set1_epi32( (int32_t)IMM ), _mm_castps_si128( RA ) ) );
}

inline GPR_t si_bg( GPR_t RA, GPR_t RB )
{
	// returns 0 if it borrows ((RB - RA) < 0)
	const __m128i unsigned_less_than = _mm_cmplt_epu32( _mm_castps_si128( RB ), _mm_castps_si128( RA ) );
	return _mm_castsi128_ps( _mm_andnot_si128( unsigned_less_than, _MM_CONST_1(RA) ) );
}

inline GPR_t si_bgx( GPR_t RA, GPR_t RB, GPR_t RT )
{
	// returns 0 if it borrows ((RB - RA - (RT_LSB-1)) < 0)
	const __m128i RT_LSB				= _mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128( RT ), 31 ), 31 );
	const __m128i borrow				= _mm_sub_epi32( RT_LSB, _mm_set1_epi32(1) ); // 1 -> 0, 0 -> -1
	const __m128i RB_adjusted			= _mm_add_epi32( _mm_castps_si128( RB ), borrow );
	const __m128i unsigned_less_than	= _mm_cmplt_epu32( RB_adjusted, _mm_castps_si128( RB ) );
	return _mm_castsi128_ps( _mm_andnot_si128( unsigned_less_than, _MM_CONST_1(RA) ) );
}

inline GPR_t si_sfx( GPR_t RA, GPR_t RB, GPR_t RT )
{
	const __m128i diff = _mm_sub_epi32( _mm_castps_si128( RB ), _mm_castps_si128( RA ) );
	// sub. 1 if RT_LSB == 0
	const __m128i RT_LSB = _mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128( RT ), 31 ), 31 );
	const __m128i borrow = _mm_sub_epi32( RT_LSB, _mm_set1_epi32(1) ); // 1 -> 0, 0 -> -1
	return _mm_castsi128_ps( _mm_add_epi32( diff, borrow ) );
}


/*
 * Multiply Instructions 
 */

inline GPR_t si_mpy( GPR_t RA, GPR_t RB )
{
	const __m128i RT_lo		= _mm_mullo_epi16( _mm_castps_si128(RA), _mm_castps_si128(RB) );
	const __m128i RT_hi		= _mm_mulhi_epi16( _mm_castps_si128(RA), _mm_castps_si128(RB) );

	const __m128i RT_lo_32	= _mm_srli_epi32( _mm_slli_epi32( RT_lo, 16 ), 16 );
	const __m128i RT_hi_32	= _mm_slli_epi32( RT_hi, 16 );

	return _mm_castsi128_ps( _mm_or_si128( RT_hi_32, RT_lo_32 ) ); 
}

inline GPR_t si_mpyu( GPR_t RA, GPR_t RB )
{
	const __m128i RT_lo		= _mm_mullo_epi16( _mm_castps_si128(RA), _mm_castps_si128(RB) );
	const __m128i RT_hi		= _mm_mulhi_epu16( _mm_castps_si128(RA), _mm_castps_si128(RB) );

	const __m128i RT_lo_32	= _mm_srli_epi32( _mm_slli_epi32( RT_lo, 16 ), 16 );
	const __m128i RT_hi_32	= _mm_slli_epi32( RT_hi, 16 );

	return _mm_castsi128_ps( _mm_or_si128( RT_hi_32, RT_lo_32 ) ); 
}

inline GPR_t si_mpyi( GPR_t RA, int64_t IMM )
{
	return si_mpy( RA, _mm_castsi128_ps( _mm_set1_epi32( (uint16_t)IMM ) ) );
}

inline GPR_t si_mpyui( GPR_t RA, int64_t IMM )
{
	return si_mpyu( RA, _mm_castsi128_ps( _mm_set1_epi32( (uint16_t)IMM ) ) );
}

inline GPR_t si_mpya( GPR_t RA, GPR_t RB, GPR_t RC )
{
	const __m128i product = _mm_castps_si128( si_mpy( RA, RB ) );
	return _mm_castsi128_ps( _mm_add_epi32( product, _mm_castps_si128( RC ) ) );
}

inline GPR_t si_mpyh( GPR_t RA, GPR_t RB )
{
	const __m128i RA_hi		= _mm_srli_epi32( _mm_castps_si128(RA), 16 );
	const __m128i RB_lo		= _mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128(RB), 16 ), 16 );

	const __m128i res_lo	= _mm_mullo_epi16( RA_hi,  RB_lo );

	return _mm_castsi128_ps( res_lo );
}

inline GPR_t si_mpys( GPR_t RA, GPR_t RB )
{
	const __m128i product = _mm_castps_si128( si_mpy( RA, RB ) );
	return _mm_castsi128_ps( _mm_srai_epi32( _mm_slli_epi32( product, 16 ), 16 ) );
}

inline GPR_t si_mpyhh( GPR_t RA, GPR_t RB )
{
	const __m128i RA_hi		= _mm_srli_epi32( _mm_castps_si128(RA), 16 );
	const __m128i RB_hi		= _mm_srli_epi32( _mm_castps_si128(RB), 16 );

	const __m128i res_lo	= _mm_mullo_epi16( RA_hi,  RB_hi );
	const __m128i ires_hi	= _mm_mulhi_epi16( RA_hi,  RB_hi );

	return _mm_castsi128_ps( _mm_or_si128( res_lo, _mm_slli_epi32(ires_hi, 16) ) );
}

inline GPR_t si_mpyhha( GPR_t RA, GPR_t RB, GPR_t RC )
{
	const __m128i product = _mm_castps_si128( si_mpyhh( RA, RB ) );
	return _mm_castsi128_ps( _mm_add_epi32( product, _mm_castps_si128( RC ) ) );
}

inline GPR_t si_mpyhhu( GPR_t RA, GPR_t RB )
{
	const __m128i RA_hi		= _mm_srli_epi32( _mm_castps_si128(RA), 16 );
	const __m128i RB_hi		= _mm_srli_epi32( _mm_castps_si128(RB), 16 );

	const __m128i res_lo	= _mm_mullo_epi16( RA_hi,  RB_hi );
	const __m128i ures_hi	= _mm_mulhi_epu16( RA_hi,  RB_hi );

	return _mm_castsi128_ps( _mm_or_si128( res_lo, _mm_slli_epi32(ures_hi, 16) ) );
}

inline GPR_t si_mpyhhau( GPR_t RA, GPR_t RB, GPR_t RC )
{
	const __m128i product = _mm_castps_si128( si_mpyhhu( RA, RB ) );
	return _mm_castsi128_ps( _mm_add_epi32( product, _mm_castps_si128( RC ) ) );
}


/*
 * Integer Misc. Instructions 
 */



inline GPR_t si_clz( GPR_t RA )
{
	const __m128i t0 = _mm_or_si128( _mm_castps_si128( RA ), _mm_srli_epi32( _mm_castps_si128( RA ), 1 ) );
	const __m128i t1 = _mm_or_si128( t0, _mm_srli_epi32( t0, 2 ) );
	const __m128i t2 = _mm_or_si128( t1, _mm_srli_epi32( t1, 4 ) );
	const __m128i t3 = _mm_or_si128( t2, _mm_srli_epi32( t2, 8 ) );
	const __m128i t4 = _mm_or_si128( t3, _mm_srli_epi32( t3, 16 ) );
	const __m128i ones = _mm_cntb_epi32( t4 );
	return _mm_castsi128_ps( _mm_sub_epi32( _mm_set1_epi32(32), ones ) );
}

inline GPR_t si_cntb( GPR_t RA )
{
	return _mm_castsi128_ps( _mm_cntb_epui8( _mm_castps_si128( RA ) ) );
}

inline GPR_t si_fsmb( GPR_t RA )
{
	const uint16_t mask = si_to_ushort(RA);

	__m128i mask_lo_byte_sat = _mm_srli_si128( _mm_set1_epi8( mask & 0xff ), 8 );
	__m128i mask_hi_byte_sat = _mm_slli_si128( _mm_set1_epi8( (mask >> 8) & 0xff ), 8 );
	mask_lo_byte_sat = _mm_and_si128( mask_lo_byte_sat, __fsmb_mask_8 );
	mask_hi_byte_sat = _mm_and_si128( mask_hi_byte_sat, __fsmb_mask_8 );
	__m128i mask5 = _mm_or_si128( mask_lo_byte_sat, mask_hi_byte_sat );
	return _mm_castsi128_ps( _mm_cmpeq_epi8( __fsmb_mask_8, mask5 ) );
}

inline GPR_t si_fsmh( GPR_t RA )
{
	const uint8_t mask = si_to_uchar(RA);

	return _mm_castsi128_ps( 
		_mm_cmpeq_epi16( 
			__fsmb_mask_16, 
			_mm_and_si128( 
				_mm_set1_epi16( mask ), 
				__fsmb_mask_16 ) ) );
}

inline GPR_t si_fsm( GPR_t RA )
{
	const uint8_t mask = si_to_uchar(RA);

	return _mm_castsi128_ps( 
		_mm_cmpeq_epi32( 
			__fsmb_mask_32, 
			_mm_and_si128( 
				_mm_set1_epi32( 0xf & mask ), 
				__fsmb_mask_32 ) ) );
}

inline GPR_t si_gbb( GPR_t RA )
{
	const __m128i LSB_to_MSB = _mm_slli_epi8( _mm_castps_si128( RA ), 7 );
	const int mask = (uint16_t)~_mm_movemask_epi8(LSB_to_MSB);
	return _mm_castsi128_ps(_mm_set_epi32(mask,0,0,0));

	/*uint32_t mask = (RA.m128_u8[0] & 0x1);

	for ( int i = 1; i < 16; ++i )
	{
		mask <<= 1;
		mask |= (RA.m128_u8[i] & 0x1); 		
	}

	return _mm_castsi128_ps(_mm_set_epi32(mask,0,0,0));*/
}

inline GPR_t si_gbh( GPR_t RA )
{
	/*const __m128i LSB_to_MSB = _mm_slli_epi16( _mm_castps_si128( RA ), 15 );
	int mask = (uint16_t)_mm_movemask_epi8(LSB_to_MSB);
	int mask2 = 0;
	mask2 |= (mask>>8)&0x80;
	mask2 |= (mask>>7)&0x40;
	mask2 |= (mask>>6)&0x20;
	mask2 |= (mask>>5)&0x10;
	mask2 |= (mask>>4)&0x08;
	mask2 |= (mask>>3)&0x04;
	mask2 |= (mask>>2)&0x02;
	mask2 |= (mask>>1)&0x01;
	return _mm_castsi128_ps(_mm_set_epi32((uint8_t)~mask2,0,0,0));*/

	uint32_t mask = (RA.m128_u16[0] & 0x1);

	for ( int i = 1; i < 8; ++i )
	{
		mask <<= 1;
		mask |= (RA.m128_u16[i] & 0x1);
	}

	return _mm_castsi128_ps(_mm_set_epi32(mask,0,0,0));
}

inline GPR_t si_gb( GPR_t RA )
{
	uint32_t mask = (RA.m128_u32[0] & 0x1);

	for ( int i = 1; i < 4; ++i )
	{
		mask <<= 1;
		mask |= (RA.m128_u32[i] & 0x1);
	}

	return _mm_castsi128_ps(_mm_set_epi32(mask,0,0,0));
}

inline GPR_t si_avgb( GPR_t RA, GPR_t RB )
{
	__m128 RT;

	for ( size_t i = 0; i < 16; ++i )
	{
		RT.m128_u8[i] = ( (uint16_t)RA.m128_u8[i] + (uint16_t)RB.m128_u8[i] + 1 ) / 2;
	}

	return RT;
}

inline GPR_t si_absdb( GPR_t RA, GPR_t RB )
{
	const __m128i RA_sub_RB = _mm_sub_epi8( _mm_castps_si128( RA ), _mm_castps_si128( RB ) );
	const __m128i RB_sub_RA = _mm_sub_epi8( _mm_castps_si128( RB ), _mm_castps_si128( RA ) );
	const __m128i RA_gt_RB_mask = _mm_cmpgt_epi8( _mm_castps_si128( RA ), _mm_castps_si128( RB ) );
	const __m128i RB_gt_RA_mask = _mm_cmpgt_epi8( _mm_castps_si128( RB ), _mm_castps_si128( RA ) );
	return _mm_castsi128_ps(
		_mm_or_si128( _mm_and_si128( RA_gt_RB_mask, RA_sub_RB ), _mm_and_si128( RB_gt_RA_mask, RB_sub_RA ) ) );
}

inline GPR_t si_sumb( GPR_t RA, GPR_t RB )
{
	const uint8_t* a8 = RA.m128_u8;
	const uint8_t* b8 = RB.m128_u8;
	GPR_t RT;
	uint32_t* t32 = RT.m128_u32;

	for ( int i = 0; i < 16; i += 4 )
	{
		uint32_t sum_a = 0;
		sum_a += a8[i+0];
		sum_a += a8[i+1];
		sum_a += a8[i+2];
		sum_a += a8[i+3];
		uint32_t sum_b = 0;
		sum_b += b8[i+0];
		sum_b += b8[i+1];
		sum_b += b8[i+2];
		sum_b += b8[i+3];
		t32[i/4] = (sum_b << 16) | sum_a;
	}

	return RT;
}

inline GPR_t si_selb( GPR_t RA, GPR_t RB, GPR_t RC )
{
	const __m128 bitsRA = _mm_andnot_ps( RC, RA );
	const __m128 bitsRB = _mm_and_ps( RC, RB );
	return _mm_or_ps( bitsRA, bitsRB );
}

/************************************************************************/
/* 
Fully compatible version. Most shufb calls are only used to move words around
or insert so depending on RC - if visible at compile time - shufb can be
substituted with a simpler _mm_shuffle_XX call.
*/
/************************************************************************/
inline GPR_t si_shufb( GPR_t RA, GPR_t RB, GPR_t RC )
{
	_CRT_ALIGN(16) uint8_t RA_RB[32];
	*(uint64_t*)(RA_RB + 0) = RA.m128_i64[0];
	*(uint64_t*)(RA_RB + 8) = RA.m128_i64[1];
	*(uint64_t*)(RA_RB + 16) = RB.m128_i64[0];
	*(uint64_t*)(RA_RB + 24) = RB.m128_i64[1];

	__m128 RT;

	for( int i = 0; i < 16; ++i )
	{
		if ( (0xC0 & RC.m128_u8[i]) == 0x80 )
			RT.m128_u8[i] = 0x00;
		else if ( (0xE0 & RC.m128_u8[i]) == 0xC0 )
			RT.m128_u8[i] = 0xFF;
		else if ( (0xE0 & RC.m128_u8[i]) == 0xE0 )
			RT.m128_u8[i] = 0x80;
		else
			RT.m128_u8[i] = RA_RB[ 0x1F & RC.m128_u8[i] ];
	}

	return RT;
}





inline GPR_t si_xsbh( GPR_t RA )
{
	return _mm_castsi128_ps(_mm_srai_epi16( _mm_slli_epi16( _mm_castps_si128(RA), 8 ), 8 ));
}

inline GPR_t si_xshw( GPR_t RA )
{
	return _mm_castsi128_ps(_mm_srai_epi32( _mm_slli_epi32( _mm_castps_si128(RA), 16 ), 16 ));
}

inline GPR_t si_xswd( GPR_t RA )
{
	__m128i RA_unsigned			= _mm_and_si128( _mm_castps_si128( RA ), _mm_set_epi32( 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 ) );
	__m128i RA_has_sign_bit		= _mm_and_si128( _mm_castps_si128( RA ), _mm_set_epi32( 0x80000000, 0, 0x80000000, 0 ) );
	__m128i RA_sign_extension	= _mm_and_si128( RA_has_sign_bit, _mm_set_epi32( 0, 0xFFFFFFFF, 0, 0xFFFFFFFF ) );
	return _mm_castsi128_ps( _mm_or_si128( RA_unsigned, RA_sign_extension ) );
}

inline GPR_t si_eqv( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_and_si128( 
		_mm_cmpeq_epi32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ), 
		_mm_set1_epi32( 1 ) ) );
}




/************************************************************************/
/* Comparison: EQ, GT, LGT (unsigned)                                   */
/************************************************************************/ 



inline GPR_t si_ceqb( GPR_t RA, GPR_t RB ) {
	return _mm_castsi128_ps( _mm_cmpeq_epi8( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_ceqh( GPR_t RA, GPR_t RB ) {
	return _mm_castsi128_ps( _mm_cmpeq_epi16( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_ceq( GPR_t RA, GPR_t RB ) {
	return _mm_castsi128_ps( _mm_cmpeq_epi32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_ceqbi( GPR_t RA, int16_t IMM ) {
	return _mm_castsi128_ps( _mm_cmpeq_epi8( _mm_castps_si128( RA ), _mm_set1_epi8( (int8_t)IMM ) ) );
}

inline GPR_t si_ceqhi( GPR_t RA, int16_t IMM ) {
	return _mm_castsi128_ps( _mm_cmpeq_epi16( _mm_castps_si128( RA ), _mm_set1_epi16( (int16_t)IMM ) ) );
}

inline GPR_t si_ceqi( GPR_t RA, int16_t IMM ) {
	return _mm_castsi128_ps( _mm_cmpeq_epi32( _mm_castps_si128( RA ), _mm_set1_epi32( (int32_t)IMM ) ) );
}



inline GPR_t si_cgtb( GPR_t RA, GPR_t RB ) {
	return _mm_castsi128_ps( _mm_cmpgt_epi8( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_cgth( GPR_t RA, GPR_t RB ) {
	return _mm_castsi128_ps( _mm_cmpgt_epi16( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_cgt( GPR_t RA, GPR_t RB ) {
	return _mm_castsi128_ps( _mm_cmpgt_epi32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_cgtbi( GPR_t RA, int16_t IMM ) {
	return _mm_castsi128_ps( _mm_cmpgt_epi8( _mm_castps_si128( RA ), _mm_set1_epi8( (int8_t)IMM ) ) );
}

inline GPR_t si_cgthi( GPR_t RA, int16_t IMM ) {
	return _mm_castsi128_ps( _mm_cmpgt_epi16( _mm_castps_si128( RA ), _mm_set1_epi16( (int16_t)IMM ) ) );
}

inline GPR_t si_cgti( GPR_t RA, int16_t IMM ) {
	return _mm_castsi128_ps( _mm_cmpgt_epi32( _mm_castps_si128( RA ), _mm_set1_epi32( (int32_t)IMM ) ) );
}



inline GPR_t si_clgtb( GPR_t RA, GPR_t RB ) {
	return _mm_castsi128_ps( _mm_cmpgt_epu8( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_clgth( GPR_t RA, GPR_t RB ) {
	return _mm_castsi128_ps( _mm_cmpgt_epu16( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_clgt( GPR_t RA, GPR_t RB ) {
	return _mm_castsi128_ps( _mm_cmpgt_epu32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_clgtbi( GPR_t RA, int16_t IMM ) {
	return _mm_castsi128_ps( _mm_cmpgt_epu8( _mm_castps_si128( RA ), _mm_set1_epi8( (int8_t)IMM ) ) );
}

inline GPR_t si_clgthi( GPR_t RA, int16_t IMM ) {
	return _mm_castsi128_ps( _mm_cmpgt_epu16( _mm_castps_si128( RA ), _mm_set1_epi16( (int16_t)IMM ) ) );
}

inline GPR_t si_clgti( GPR_t RA, int16_t IMM ) {
	return _mm_castsi128_ps( _mm_cmpgt_epu32( _mm_castps_si128( RA ), _mm_set1_epi32( (int32_t)IMM ) ) );
}



/************************************************************************/
/* Logical: AND, OR, XOR												*/
/************************************************************************/ 



inline GPR_t si_and( GPR_t RA, GPR_t RB ) {
	return _mm_and_ps( RA, RB );
}

inline GPR_t si_andc( GPR_t RA, GPR_t RB ) {
	return _mm_andnot_ps( RB, RA );
}

inline GPR_t si_andbi( GPR_t RA, uint8_t I10 ) {
	return _mm_castsi128_ps( _mm_and_si128( _mm_castps_si128( RA ), _mm_set1_epi8( I10 ) ) );
}

inline GPR_t si_andhi( GPR_t RA, int16_t I10 ) {
	return _mm_castsi128_ps( _mm_and_si128( _mm_castps_si128( RA ), _mm_set1_epi16( int16_t( I10 ) ) ) );
}

inline GPR_t si_andi( GPR_t RA, int16_t I10 ) {
	return _mm_castsi128_ps( _mm_and_si128( _mm_castps_si128( RA ), _mm_set1_epi32( int32_t( I10 ) ) ) );
}

inline GPR_t si_nand( GPR_t RA, GPR_t RB ) {
	return _mm_andnot_ps( _mm_and_ps( RA, RB ), _mm_castsi128_ps( _MM_CONST_ALL( RA ) ) );
}



inline GPR_t si_or( GPR_t RA, GPR_t RB ) {
	return _mm_or_ps( RA, RB );
}

inline GPR_t si_orc( GPR_t RA, GPR_t RB ) {
	const __m128 not_RB = _mm_andnot_ps( RB, _mm_castsi128_ps( _mm_set1_epi32(0xffffffff) ) );
	return _mm_or_ps( RA, not_RB );
}

inline GPR_t si_orbi( GPR_t RA, int64_t IMM ) {
	return _mm_or_ps( RA, _mm_castsi128_ps( _mm_set1_epi8( (int8_t)IMM ) ) );
}

inline GPR_t si_orhi( GPR_t RA, int64_t IMM ) {
	return _mm_or_ps( RA, _mm_castsi128_ps( _mm_set1_epi16( (int16_t)IMM ) ) );
}

inline GPR_t si_ori( GPR_t RA, int64_t IMM ) {
	return _mm_or_ps( RA, _mm_castsi128_ps( _mm_set1_epi32( (int32_t)IMM ) ) );
}

inline GPR_t si_orx( GPR_t RA ) {
	const uint32_t shuf_reverse = _MM_SHUFFLE(0,3,2,1);
	const __m128 temp1 = _mm_or_ps( RA, _mm_shuffle_ps( RA, RA, shuf_reverse ) );
	const __m128 temp2 = _mm_or_ps( temp1, _mm_shuffle_ps( temp1, temp1, shuf_reverse ) );
	const __m128 temp3 = _mm_or_ps( temp2, _mm_shuffle_ps( temp2, temp2, shuf_reverse ) );
	return _mm_move_ss( _mm_setzero_ps(), temp3 );
}

inline GPR_t si_nor( GPR_t RA, GPR_t RB ) {
	const __m128 RA_or_RB = _mm_or_ps( RA, RB );
	return _mm_andnot_ps( RA_or_RB, _mm_castsi128_ps( _mm_set1_epi32(0xffffffff) ) );
}



inline GPR_t si_xor( GPR_t RA, GPR_t RB ) {
	return _mm_xor_ps( RA, RB );
}

inline GPR_t si_xorbi( GPR_t RA, int64_t IMM ) {
	return _mm_xor_ps( RA, _mm_castsi128_ps( _mm_set1_epi8((uint8_t)IMM) ) );
}

inline GPR_t si_xorhi( GPR_t RA, int64_t IMM ) {
	return _mm_xor_ps( RA, _mm_castsi128_ps( _mm_set1_epi16((int16_t)IMM) ) );
}

inline GPR_t si_xori( GPR_t RA, int64_t IMM ) {
	return _mm_xor_ps( RA, _mm_castsi128_ps( _mm_set1_epi32((int32_t)IMM) ) );
}

// Insertion control

static const __m128i __cb_ltbl[16] =
{
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x03 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x03, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x03, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x03, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x03, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x03, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x03, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x03, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x03, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x03, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x03, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x03, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x03, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x03, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x03, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x03, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
};

static const __m128i __ch_ltbl[8] =
{
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x03, 0x02 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x03, 0x02, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x03, 0x02, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x03, 0x02, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x03, 0x02, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x03, 0x02, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x03, 0x02, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x03, 0x02, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
};

static const __m128i __cw_ltbl[4] =
{
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x03, 0x02, 0x01, 0x00 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x03, 0x02, 0x01, 0x00, 0x13, 0x12, 0x11, 0x10 },
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x03, 0x02, 0x01, 0x00, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
	{ 0x03, 0x02, 0x01, 0x00, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
};

static const __m128i __cd_ltbl[2] =
{
	{ 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 },
	{ 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10 },
};


inline GPR_t si_cbd( GPR_t RA, int64_t IMM )
{
	return _mm_castsi128_ps(__cb_ltbl[(si_to_uint(RA) + IMM)&0xF]);
}

inline GPR_t si_cbx( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps(__cb_ltbl[(si_to_uint(RA) + si_to_uint(RB))&0xF]);
}

inline GPR_t si_chd( GPR_t RA, int64_t IMM )
{
	return _mm_castsi128_ps(__ch_ltbl[((si_to_uint(RA) + IMM)&0xE)>>1]);
}

inline GPR_t si_chx( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps(__ch_ltbl[((si_to_uint(RA) + si_to_uint(RB))&0xE)>>1]);
}

inline GPR_t si_cwd( GPR_t RA, int64_t IMM )
{
	return _mm_castsi128_ps(__cw_ltbl[((si_to_uint(RA) + IMM)&0xC)>>2]);
}

inline GPR_t si_cwx( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps(__cw_ltbl[((si_to_uint(RA) + si_to_uint(RB))&0xC)>>2]);
}

inline GPR_t si_cdd( GPR_t RA, int64_t IMM )
{
	return _mm_castsi128_ps(__cd_ltbl[((si_to_uint(RA) + IMM)&0x8)>>4]);
}

inline GPR_t si_cdx( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps(__cd_ltbl[((si_to_uint(RA) + si_to_uint(RB))&0x8)>>4]);
}


/************************************************************************/
/* Rotate left                                                          */
/************************************************************************/


inline GPR_t si_shlh( GPR_t RA, GPR_t RB )
{
	__m128 RT;

	for( size_t i = 0; i != 8; ++i ) 
	{ 
		RT.m128_u16[i] = RA.m128_u16[i] << RB.m128_u16[i];
	}

	return RT;
}

inline GPR_t si_shlhi( GPR_t RA, int64_t IMM )
{
	return _mm_castsi128_ps( _mm_slli_epi16( _mm_castps_si128( RA ), IMM & 0x1F ) );
}

inline GPR_t si_shl( GPR_t RA, GPR_t RB )
{
	__m128 RT;

	for( size_t i = 0; i != 4; ++i ) 
	{ 
		RT.m128_u32[i] = RA.m128_u32[i] << RB.m128_u32[i];
	}

	return RT;
}

inline GPR_t si_shli( GPR_t RA, int64_t IMM )
{
	return _mm_castsi128_ps( _mm_slli_epi32( _mm_castps_si128( RA ), IMM & 0x3F ) );
}

inline GPR_t si_shlqbii( GPR_t RA, int64_t IMM )
{
	const size_t s = 0x7 & IMM;

	const uint64_t _0 = RA.m128_u64[0];
	const uint64_t _1 = RA.m128_u64[1];
	const uint64_t overflow0 = _0 >> (64 - s);

	__m128 RT;

	RT.m128_u64[0] = (_0 << s);
	RT.m128_u64[1] = (_1 << s) | overflow0;

	return RT;
}

inline GPR_t si_shlqbi( GPR_t RA, GPR_t RB )
{
	return si_shlqbii(RA, si_to_uint(RB));
}

inline GPR_t si_shlqby( GPR_t RA, GPR_t RB )
{
	const size_t bytes = 0x1F & si_to_uchar(RB);

	if ( bytes < 16 )
	{
		const uint64_t _0 = RA.m128_u64[0];
		const uint64_t _1 = RA.m128_u64[1];
		const uint64_t overflow0 = _0 >> (64 - bytes*8);
		//const uint64_t overflow1 = _1 >> (64 - bytes*8);

		__m128 RT;

		RT.m128_u64[0] = (_0 << bytes*8);// | overflow1;
		RT.m128_u64[1] = (_1 << bytes*8) | overflow0;

		return RT;
	}
	else
	{
		return _mm_setzero_ps();
	}	
}

#define si_shlqbyi( RA, IMM )\
	_mm_castsi128_ps( _mm_slli_si128( _mm_castps_si128(RA), (IMM) ) )

inline GPR_t si_shlqbybi( GPR_t RA, GPR_t RB )
{
	const size_t bytes = 0x1F & (si_to_uchar(RB) >> 3);

	if ( bytes < 16 )
	{
		const uint64_t _0 = RA.m128_u64[0];
		const uint64_t _1 = RA.m128_u64[1];
		const uint64_t overflow0 = _0 >> (64 - bytes*8);
		//const uint64_t overflow1 = _1 >> (64 - bytes*8);

		__m128 RT;

		RT.m128_u64[0] = (_0 << bytes*8);// | overflow1;
		RT.m128_u64[1] = (_1 << bytes*8) | overflow0;

		return RT;
	}
	else
	{
		return _mm_setzero_ps();
	}	
}


/************************************************************************/
/* Rotate left                                                          */
/************************************************************************/


inline GPR_t si_roth( GPR_t RA, GPR_t RB )
{
	__m128 RT;

	for ( int i = 0; i < 8; ++i )
	{
		RT.m128_u16[i] = _rotl16( RA.m128_u16[i], 0xF & RB.m128_u16[i] );
	}

	return RT;
}

inline GPR_t si_rothi( GPR_t RA, int64_t IMM )
{
	const uint8_t s = IMM & 0xF;

	const __m128i left	= _mm_slli_epi16( _mm_castps_si128( RA ), s );
	const __m128i right = _mm_srli_epi16( _mm_castps_si128( RA ), 16 - s );

	return _mm_castsi128_ps( _mm_or_si128( left, right ) );
}

inline GPR_t si_rot( GPR_t RA, GPR_t RB )
{
	__m128 RT;

	for ( int i = 0; i < 4; ++i )
	{
		RT.m128_u32[i] = _rotl( RA.m128_u32[i], 0x1F & RB.m128_u32[i] );
	}

	return RT;
}

inline GPR_t si_roti( GPR_t RA, int64_t IMM )
{
	const uint8_t s = IMM & 0x1F;

	const __m128i left	= _mm_slli_epi32( _mm_castps_si128( RA ), s );
	const __m128i right = _mm_srli_epi32( _mm_castps_si128( RA ), 32 - s );

	return _mm_castsi128_ps( _mm_or_si128( left, right ) );
}

inline GPR_t si_rotqbii( GPR_t RA, int64_t IMM )
{
	const size_t s = 0x7 & IMM;

	const uint64_t _0 = RA.m128_u64[0];
	const uint64_t _1 = RA.m128_u64[1];
	const uint64_t overflow0 = _0 >> (64 - s);
	const uint64_t overflow1 = _1 >> (64 - s);

	__m128 RT;

	RT.m128_u64[0] = (_0 << s) | overflow1;
	RT.m128_u64[1] = (_1 << s) | overflow0;

	return RT;
}

inline GPR_t si_rotqbi( GPR_t RA, GPR_t RB )
{
	return si_rotqbii(RA, si_to_int(RB));
}

inline GPR_t si_rotqbyi( GPR_t RA, int64_t IMM )
{
	__m128i RT = _mm_castps_si128(RA);

	const uint8_t RotW = (0x0C & IMM) >> 2;
	const uint8_t RotB = 0x03 & IMM;

	switch (RotB)
	{
	case 1: RT = _mm_or_si128(_mm_slli_si128(_mm_castps_si128(RA), 1), _mm_srli_si128(_mm_castps_si128(RA), 15)); break;
	case 2: RT = _mm_or_si128(_mm_slli_si128(_mm_castps_si128(RA), 2), _mm_srli_si128(_mm_castps_si128(RA), 14)); break;
	case 3: RT = _mm_or_si128(_mm_slli_si128(_mm_castps_si128(RA), 3), _mm_srli_si128(_mm_castps_si128(RA), 13)); break;
	}

	switch (RotW)
	{
	case 1: RT = _mm_shuffle_epi32(RT, _MM_SHUFFLE(2,1,0,3)); break;
	case 2: RT = _mm_shuffle_epi32(RT, _MM_SHUFFLE(1,0,3,2)); break;
	case 3: RT = _mm_shuffle_epi32(RT, _MM_SHUFFLE(0,3,2,1)); break;
	}

	return _mm_castsi128_ps(RT);
}

inline GPR_t si_rotqby( GPR_t RA, GPR_t RB )
{
	return si_rotqbyi(RA, si_to_uchar(RB));
}

inline GPR_t si_rotqbybi( GPR_t RA, GPR_t RB )
{
	return si_rotqbyi(RA, si_to_uchar(RB) >> 3);
}

//#define si_rotqbyi( RA, IMM )\
//	_mm_castsi128_ps( _mm_and_si128( \
//	_mm_slli_si128( _mm_castps_si128(RA), (IMM&0xF) ), \
//	_mm_srli_si128( _mm_castps_si128(RA), (16 - (IMM&0xF)) ) ) )

/************************************************************************/
/* Rotate right and mask                                                */
/************************************************************************/


inline GPR_t si_rothm( GPR_t RA, GPR_t RB )
{
	__m128 RT;

	for( size_t i = 0; i != 8; ++i ) 
	{ 
		RT.m128_u16[i] = RA.m128_u16[i] >> (0x1F & (0 - RB.m128_u16[i]));
	}

	return RT;
}

inline GPR_t si_rothmi( GPR_t RA, int64_t IMM )
{
	return _mm_castsi128_ps( _mm_srli_epi16( _mm_castps_si128( RA ), 0x1F & (0 - IMM) ) );
}

inline GPR_t si_rotm( GPR_t RA, GPR_t RB )
{
	__m128 RT;

	for( size_t i = 0; i != 4; ++i ) 
	{ 
		RT.m128_u32[i] = RA.m128_u32[i] >> (0x3F & (0 - RB.m128_u32[i]));
	}

	return RT;
}

inline GPR_t si_rotmi( GPR_t RA, int64_t IMM )
{
	return _mm_castsi128_ps( _mm_srli_epi32( _mm_castps_si128( RA ), 0x3F & (0 - IMM) ) );
}

inline GPR_t si_rotqmbi( GPR_t RA, GPR_t RB )
{
	const size_t s = 0x07 & (0 - si_to_uchar(RB));

	const uint64_t _0 = RA.m128_u64[0];
	const uint64_t _1 = RA.m128_u64[1];
	//const uint64_t overflow0 = _0 << (64 - s);
	const uint64_t overflow1 = _1 << (64 - s);

	__m128 RT;

	RT.m128_u64[0] = (_0 >> s) | overflow1;
	RT.m128_u64[1] = (_1 >> s);// | overflow0;

	return RT;
}

inline GPR_t si_rotqmbii( GPR_t RA, int64_t IMM )
{
	const size_t s = 0x7 & (0 - IMM);

	const uint64_t _0 = RA.m128_u64[0];
	const uint64_t _1 = RA.m128_u64[1];
	//const uint64_t overflow0 = _0 << (64 - s);
	const uint64_t overflow1 = _1 << (64 - s);

	__m128 RT;

	RT.m128_u64[0] = (_0 >> s) | overflow1;
	RT.m128_u64[1] = (_1 >> s);// | overflow0;

	return RT;
}

inline GPR_t si_rotqmby( GPR_t RA, GPR_t RB )
{
	const size_t s = 0x1F & (0 - si_to_uchar(RB));

	const uint64_t _0 = RA.m128_u64[0];
	const uint64_t _1 = RA.m128_u64[1];
	//const uint64_t overflow0 = _0 << (64 - s);
	const uint64_t overflow1 = _1 << (64 - 8*s);

	__m128 RT;

	RT.m128_u64[0] = (_0 >> 8*s) | overflow1;
	RT.m128_u64[1] = (_1 >> 8*s);// | overflow0;

	return RT;
}

#define si_rotqmbyi( RA, IMM ) \
	_mm_castsi128_ps( _mm_srli_si128( _mm_castps_si128(RA), 0x1F & (0 - (IMM)) ) )

inline GPR_t si_rotqmbybi( GPR_t RA, GPR_t RB )
{
	const size_t s = 0x1F & (0 - (si_to_uchar(RB) >> 3));

	const uint64_t _0 = RA.m128_u64[0];
	const uint64_t _1 = RA.m128_u64[1];
	//const uint64_t overflow0 = _0 << (64 - s);
	const uint64_t overflow1 = _1 << (64 - 8*s);

	__m128 RT;

	RT.m128_u64[0] = (_0 >> 8*s) | overflow1;
	RT.m128_u64[1] = (_1 >> 8*s);// | overflow0;

	return RT;
}

inline GPR_t si_rotmah( GPR_t RA, GPR_t RB )
{
	__m128 RT;

	for( size_t i = 0; i != 8; ++i ) 
	{ 
		RT.m128_u16[i] = RA.m128_i16[i] >> (0x1F & (0 - RB.m128_i16[i]));
	}

	return RT;
}

inline GPR_t si_rotmahi( GPR_t RA, int64_t IMM )
{
	return _mm_castsi128_ps( _mm_srai_epi16( _mm_castps_si128( RA ), 0x1F & (0 - IMM) ) );
}

inline GPR_t si_rotma( GPR_t RA, GPR_t RB )
{
	__m128 RT;

	for( size_t i = 0; i != 4; ++i ) 
	{ 
		RT.m128_i32[i] = RA.m128_i32[i] >> (0x3F & (0 - RB.m128_i32[i]));
	}

	return RT;
}

inline GPR_t si_rotmai( GPR_t RA, int64_t IMM )
{
	return _mm_castsi128_ps( _mm_srai_epi32( _mm_castps_si128( RA ), 0x3F & (0 - IMM) ) );
}

// Channel

enum SPEChannels
{
	$SPU_RdEventStat = 0,
	$SPU_WrEventMask,
	$SPU_WrEventAck,
	$SPU_RdSigNotify1,
	$SPU_RdSigNotify2,
	$5,
	$6,
	$SPU_WrDec,
	$SPU_RdDec,
	$MFC_WrMSSyncReq,
	$11,
	$SPU_RdEventMask,
	$MFC_RdTagMask,
	$SPU_RdMachStat,
	$SPU_WrSRR0,
	$SPU_RdSRR0,
	$MFC_LSA,
	$MFC_EAH,
	$MFC_EAL,
	$MFC_Size,
	$MFC_TagID,
	$MFC_Cmd,
	$MFC_WrTagMask,
	$MFC_WrTagUpdate,
	$MFC_RdTagStat,
	$MFC_RdListStallStat,
	$MFC_WrListStallAck,
	$MFC_RdAtomicStat, 
	$SPU_WrOutMbox, 
	$SPU_RdInMbox, 
	$SPU_WrOutIntrMbox
};

uint32_t Channels[128];
uint8_t ChannelCount[128] =
{
	0,1,1,0,0,0,0,0,1,1,
	0,0,1,1,1,1,1,1,1,1,
	1,1,0,0,0,1,0,0,0,0,
	0
};

GPR_t	si_rdch( int64_t IMM )			{ 
	return _mm_setzero_ps(); 
}
GPR_t	si_rchcnt( int64_t IMM )		{ 
	return _mm_setzero_ps(); 
}
void	si_wrch( int64_t IMM, GPR_t RA ){
	const uint8_t ChannelIndex = IMM & 0x7f;

	switch ( ChannelIndex )
	{
	case $SPU_WrEventMask:
	case $SPU_WrEventAck:
	case $SPU_WrDec:
		Channels[ChannelIndex] = si_to_uint(RA); break;
	case $MFC_WrMSSyncReq:
		break;
	case $SPU_WrSRR0:
	case $MFC_LSA:
	case $MFC_EAH:
	case $MFC_EAL:
		Channels[ChannelIndex] = si_to_uint(RA); break;
	case $MFC_Size:
	case $MFC_TagID:
		Channels[ChannelIndex] = si_to_ushort(RA); break;
	case $MFC_Cmd:
	case $MFC_WrTagMask:
		Channels[ChannelIndex] = si_to_uint(RA); break;
	case $MFC_WrTagUpdate:
		break;
	case $MFC_WrListStallAck:
		Channels[ChannelIndex] = si_to_uint(RA); break;
	case $SPU_WrOutMbox:
		break;
	case $SPU_WrOutIntrMbox:
		break;
	default:
		break;
	}
} 

// Misc

#define si_sync  _WriteBarrier          
#define si_syncc _WriteBarrier
#define si_dsync _ReadWriteBarrier

#endif


/* the orignals SPU intrinsics */

/*si_lqd(ra,imm)       
	si_lqx(ra,rb)        
	si_lqa(imm)          
	si_lqr(imm)          
	si_stqd(rt,ra,imm)   
	si_stqx(rt,ra,rb)    
	si_stqa(rt,imm)      
	si_stqr(rt,imm)      
	si_cbd(ra,imm)       
	si_cbx(ra,rb)        
	si_chd(ra,imm)       
	si_chx(ra,rb)        
	si_cwd(ra,imm)       
	si_cwx(ra,rb)        
	si_cdd(ra,imm)       
	si_cdx(ra,rb)        
	si_ilh(imm)          
	si_ilhu(imm)         
	si_il(imm)           
	si_ila(imm)          
	si_iohl(ra,imm)      
	si_fsmbi(imm)        
	si_ah(ra,rb)         
	si_ahi(ra,imm)       
	si_a(ra,rb)          
	si_ai(ra,imm)        
	si_addx(ra,rb,rt)    
	si_cg(ra,rb)         
	si_cgx(ra,rb,rt)     
	si_sfh(ra,rb)        
	si_sfhi(imm,ra)      
	si_sf(ra,rb)         
	si_sfi(ra,imm)       
	si_sfx(ra,rb,rt)     
	si_bg(ra,rb)         
	si_bgx(ra,rb,rt)     
	si_mpy(ra,rb)        
	si_mpyu(ra,rb)       
	si_mpyi(ra,imm)      
	si_mpyui(ra,imm)     
	si_mpya(ra,rb,rc)    
	si_mpyh(ra,rb)       
	si_mpys(ra,rb)       
	si_mpyhh(ra,rb)      
	si_mpyhhu(ra,rb)     
	si_mpyhha(ra,rb,rc)  
	si_mpyhhau(ra,rb,rc) 
	si_clz(ra)           
	si_cntb(ra)          
	si_fsmb(ra)          
	si_fsmh(ra)          
	si_fsm(ra)           
	si_gbb(ra)           
	si_gbh(ra)           
	si_gb(ra)*/            
	//si_avgb(ra,rb)       
	//si_absdb(ra,rb)      
	//si_sumb(ra,rb)       
	/*si_xsbh(ra)          
	si_xshw(ra)          
	si_xswd(ra)          
	si_and(ra,rb)        
	si_andc(ra,rb)       
	si_andbi(ra,imm)     
	si_andhi(ra,imm)     
	si_andi(ra,imm)      
	si_or(ra,rb)         
	si_orc(ra,rb)        
	si_orbi(ra,imm)      
	si_orhi(ra,imm)      
	si_ori(ra,imm)       
	si_orx(ra)           
	si_xor(ra,rb)        
	si_xorbi(ra,imm)     
	si_xorhi(ra,imm)     
	si_xori(ra,imm)      
	si_nand(ra,rb)       
	si_nor(ra,rb)        
	si_eqv(ra,rb)        
	si_selb(ra,rb,rc)    
	si_shufb(ra,rb,rc)   
	si_shlh(ra,rb)       
	si_shlhi(ra,imm)     
	si_shl(ra,rb)        
	si_shli(ra,imm)      
	si_shlqbi(ra,rb)     
	si_shlqbii(ra,imm)   
	si_shlqby(ra,rb)     
	si_shlqbyi(ra,imm)   
	si_shlqbybi(ra,rb)   
	si_roth(ra,rb)       
	si_rothi(ra,imm)     
	si_rot(ra,rb)        
	si_roti(ra,imm)      
	si_rotqby(ra,rb)     
	si_rotqbyi(ra,imm)   
	si_rotqbybi(ra,rb)   
	si_rotqbi(ra,rb)     
	si_rotqbii(ra,imm)   */
//#define si_rothm(ra,rb)      _mm_setzero_ps()
//#define si_rothmi(ra,imm)    _mm_setzero_ps()
//#define si_rotm(ra,rb)       _mm_setzero_ps()
//#define si_rotmi(ra,imm)     _mm_setzero_ps()
//#define si_rotqmby(ra,rb)    _mm_setzero_ps()
//#define si_rotqmbyi(ra,imm)  _mm_setzero_ps()
//#define si_rotqmbi(ra,rb)    _mm_setzero_ps()
//#define si_rotqmbii(ra,imm)  _mm_setzero_ps()
//#define si_rotqmbybi(ra,rb)  _mm_setzero_ps()
//#define si_rotmah(ra,rb)     _mm_setzero_ps()
//#define si_rotmahi(ra,imm)   _mm_setzero_ps()
//#define si_rotma(ra,rb)      _mm_setzero_ps()
//#define si_rotmai(ra,imm)    _mm_setzero_ps()
#define si_heq(ra,rb)        
#define si_heqi(ra,imm)      
#define si_hgt(ra,rb)        
#define si_hgti(ra,imm)      
#define si_hlgt(ra,rb)       
#define si_hlgti(ra,imm)     
// si_ceqb(ra,rb)       
// si_ceqbi(ra,imm)     
// si_ceqh(ra,rb)       
// si_ceqhi(ra,imm)     
// si_ceq(ra,rb)        
// si_ceqi(ra,imm)      
// si_cgtb(ra,rb)       
// si_cgtbi(ra,imm)     
// si_cgth(ra,rb)       
// si_cgthi(ra,imm)     
// si_cgt(ra,rb)        
// si_cgti(ra,imm)      
// si_clgtb(ra,rb)      
// si_clgtbi(ra,imm)    
// si_clgth(ra,rb)      
// si_clgthi(ra,imm)    
// si_clgt(ra,rb)       
// si_clgti(ra,imm)     
// si_bisled(ra)        
// si_bisledd(ra)       
// si_bislede(ra)       
// si_fa(ra,rb)         
// si_dfa(ra,rb)        
// si_fs(ra,rb)         
// si_dfs(ra,rb)        
// si_fm(ra,rb)         
// si_dfm(ra,rb)        
// si_fma(ra,rb,rc)     
// si_dfma(ra,rb,rc)    
// si_dfnma(ra,rb,rc)   
// si_fnms(ra,rb,rc)    
// si_dfnms(ra,rb,rc)   
// si_fms(ra,rb,rc)     
// si_dfms(ra,rb,rc)    
// si_frest(ra)         
// si_frsqest(ra)       
// si_fi(ra,rb)         
// si_csflt(ra,imm)     
// si_cflts(ra,imm)     
// si_cuflt(ra,imm)     
// si_cfltu(ra,imm)     
// si_frds(ra)          
// si_fesd(ra)          
// si_fceq(ra,rb)       
// si_fcmeq(ra,rb)      
// si_fcgt(ra,rb)       
// si_fcmgt(ra,rb)      
#define si_stop(imm)         
#define si_stopd(ra,rb,rc)   
// si_lnop()            
// si_nop()             
// si_sync()            
// si_syncc()           
// si_dsync()           
// si_mfspr(imm)        
// si_mtspr(imm,ra)     
// si_fscrrd()          
// si_fscrwr(ra)        
// si_rdch(imm)         
// si_rchcnt(imm)       
// si_wrch(imm,ra)      

// si_from_char(scalar)    
// si_from_uchar(scalar)   
// si_from_short(scalar)   
// si_from_ushort(scalar)  
// si_from_int(scalar)     
// si_from_uint(scalar)    
// si_from_llong(scalar)   
// si_from_ullong(scalar)  
// si_from_float(scalar)   
// si_from_double(scalar)  
// si_from_ptr(scalar)     

// si_to_char(ra)      
// si_to_uchar(ra)     
// si_to_short(ra)     
// si_to_ushort(ra)    
// si_to_int(ra)       
// si_to_uint(ra)      
// si_to_llong(ra)     
// si_to_ullong(ra)    
// si_to_float(ra)     
// si_to_double(ra)    
// si_to_ptr(ra)       
