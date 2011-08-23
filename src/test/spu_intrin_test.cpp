#include <iostream>
#include <cassert>
#include "..\..\src\spu_internals_x86.h"
#include "spu_unittest.h"

using namespace std;

#define BSWAP128( xmm ) _mm_castsi128_ps( _mm_setr_epi32( \
	_byteswap_ulong((xmm).m128i_u32[3]), \
	_byteswap_ulong((xmm).m128i_u32[2]), \
	_byteswap_ulong((xmm).m128i_u32[1]), \
	_byteswap_ulong((xmm).m128i_u32[0]) ) )

int main( int argc, char** argv )
{
	_CRT_ALIGN(16) uint8_t LE_buf[] = { 0,1,2,3,4,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf };
	__m128 BE = BSWAP128(*(__m128i*)LE_buf);
	__m128 LE = BSWAP128((__m128i&)BE);
	__m128 BE2 = BSWAP128((__m128i&)LE);
	assert( 0 == memcmp(&BE, &BE2, sizeof(__m128)) );

	_CRT_ALIGN(16) uint8_t scalar_buf1[] = { 0,0,0,0x80,0,0,0,0,0,0,0,0,0,0,0,0 };
	_CRT_ALIGN(16) uint8_t scalar_buf2[] = { 0,0,0x80,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	_CRT_ALIGN(16) uint8_t scalar_buf3[] = { 0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	uint8_t s1 = si_to_uchar( BSWAP128(*(__m128i*)scalar_buf1));
	int8_t s2 = si_to_char( BSWAP128(*(__m128i*)scalar_buf1));
	uint16_t s3 = si_to_ushort( BSWAP128(*(__m128i*)scalar_buf2));
	int16_t s4 = si_to_short( BSWAP128(*(__m128i*)scalar_buf2));
	uint32_t s5 = si_to_uint( BSWAP128(*(__m128i*)scalar_buf3));
	int32_t s6 = si_to_int( BSWAP128(*(__m128i*)scalar_buf3));
	uint64_t s7 = si_to_ullong( BSWAP128(*(__m128i*)scalar_buf3));
	int64_t s8 = si_to_llong( BSWAP128(*(__m128i*)scalar_buf3));

	if ( s1 != 0x80 ) cout << "si_to_uchar...FAIL" << endl;
	if ( s2 != (int8_t)0x80 ) cout << "si_to_char...FAIL" << endl;
	if ( s3 != 0x8000 ) cout << "si_to_ushort...FAIL" << endl;
	if ( s4 != (int16_t)0x8000 ) cout << "si_to_short...FAIL" << endl;
	if ( s5 != 0x80000000 ) cout << "si_to_uint...FAIL" << endl;
	if ( s6 != (int32_t)0x80000000 ) cout << "si_to_int...FAIL" << endl;
	if ( s7 != 0x8000000000000000 ) cout << "si_to_ullong...FAIL" << endl;
	if ( s8 != (int64_t)0x8000000000000000 ) cout << "si_to_llong...FAIL" << endl;

	__SPU_TEST_RR( si_ah, _mm_set1_epi16(1), _mm_set1_epi16(1), _mm_set1_epi16(2) )

	__SPU_TEST_RI( si_ahi, _mm_set1_epi16(1), 1, _mm_set1_epi16(2) )

	__SPU_TEST_RR( si_a, _mm_set1_epi32(1), _mm_set1_epi32(1), _mm_set1_epi32(2) )

	__SPU_TEST_RI( si_ai, _mm_set1_epi32(1), 1ui32, _mm_set1_epi32(2) )

	__SPU_TEST_RRR( si_addx, _mm_set1_epi32(1), _mm_set1_epi32(1), _mm_set1_epi32(1), _mm_set1_epi32(3) )

	__SPU_TEST_RR( si_cg, _mm_set1_epi32(0xFFFFFFFF), _mm_set1_epi32(1), _mm_set1_epi32(1) )

	__SPU_TEST_RRR( si_cgx, _mm_set1_epi32(0xFFFFFFFE), _mm_set1_epi32(1), _mm_set1_epi32(1), _mm_set1_epi32(1) )


	__SPU_TEST_RR( si_sfh, _mm_set1_epi16(2), _mm_set1_epi16(2), _mm_set1_epi16(0) )

	__SPU_TEST_RI( si_sfhi, _mm_set1_epi16(2), 2, _mm_set1_epi16(0) )

	__SPU_TEST_RR( si_sf, _mm_set1_epi32(2), _mm_set1_epi32(2), _mm_set1_epi32(0) )

	__SPU_TEST_RI( si_sfi, _mm_set1_epi32(2), 2, _mm_set1_epi32(0) )

	__SPU_TEST_RR( si_bg, _mm_set1_epi32(0xFFFFFFFE), _mm_set1_epi32(0xFFFFFFFF), _mm_set1_epi32(1) )

	__SPU_TEST_RR( si_bg, _mm_set1_epi32(0xFFFFFFFF), _mm_set1_epi32(0xFFFFFFFE), _mm_set1_epi32(0) )

	__SPU_TEST_RRR( si_sfx, _mm_set1_epi32(1), _mm_set1_epi32(5), _mm_set1_epi32(1), _mm_set1_epi32(4) )

	__SPU_TEST_RRR( si_sfx, _mm_set1_epi32(1), _mm_set1_epi32(5), _mm_set1_epi32(0), _mm_set1_epi32(3) )


		__SPU_TEST_RR(
		si_mpy,
		_mm_set1_epi32(0xFFFFi16),
		_mm_set1_epi32(0xFi16),
		_mm_set1_epi32(0xFFFFi16 * 0xFi16))

		__SPU_TEST_RR(
		si_mpyu,
		_mm_set1_epi32(0xFFFFi16),
		_mm_set1_epi32(0xFi16),
		_mm_set1_epi32(0xFFFFui16 * 0xFui16))

		__SPU_TEST_RI(
		si_mpyi,
		_mm_set1_epi32(0xFFFFi16),
		0xFi16,
		_mm_set1_epi32(0xFFFFi16 * 0xFi16))

		__SPU_TEST_RI(
		si_mpyui,
		_mm_set1_epi32(0xFFFFi16),
		0xFi16,
		_mm_set1_epi32(0xFFFFui16 * 0xFui16))

		__SPU_TEST_RRR(
		si_mpya,
		_mm_set1_epi32(0xFFFFi16),
		_mm_set1_epi32(0xFi16),
		_mm_set1_epi32(0xFi16),
		_mm_set1_epi32(0xFFFFi16 * 0xFi16 + 0xFi16))


		__SPU_TEST_RRR(
		si_selb,
		_mm_set1_epi32(0xFF00aaaa),
		_mm_set1_epi32(0x00FF5555),
		_mm_set1_epi32(0x00FF5555),
		_mm_set1_epi32(0xFFFFFFFF))

		__SPU_TEST_R(
		si_cntb,
		_mm_set1_epi8(1),
		_mm_set1_epi8(1))

		__SPU_TEST_R(
		si_cntb,
		_mm_set1_epi8(0),
		_mm_set1_epi8(0))

		__SPU_TEST_R(
		si_cntb,
		_mm_set1_epi8(-1),
		_mm_set1_epi8(8))

		__SPU_TEST_R(
		si_clz,
		_mm_set1_epi32(1),
		_mm_set1_epi32(31))

		__SPU_TEST_R(
		si_clz,
		_mm_set1_epi32(0),
		_mm_set1_epi32(32))

		__SPU_TEST_R(
		si_clz,
		_mm_set1_epi32(0xFFFFFFFF),
		_mm_set1_epi32(0))
		
		__SPU_TEST_R(si_fsmb, _mm_set_epi32(0x000F,0,0,0), _mm_set_epi32(0,0,0,0xFFFFFFFF))
		__SPU_TEST_R(si_fsmb, _mm_set_epi32(0xF0F0,0,0,0), _mm_set_epi32(0xFFFFFFFF,0,0xFFFFFFFF,0))

		__SPU_TEST_R(si_fsmh, _mm_set_epi32(0x03,0,0,0), _mm_set_epi32(0,0,0,0xFFFFFFFF))
		__SPU_TEST_R(si_fsmh, _mm_set_epi32(0xCC,0,0,0), _mm_set_epi32(0xFFFFFFFF,0,0xFFFFFFFF,0))

		__SPU_TEST_R(si_fsm, _mm_set_epi32(0x1,0,0,0), _mm_set_epi32(0,0,0,0xFFFFFFFF))
		__SPU_TEST_R(si_fsm, _mm_set_epi32(0xA,0,0,0), _mm_set_epi32(0xFFFFFFFF,0,0xFFFFFFFF,0))

		__SPU_TEST_R(si_gbb, _mm_set_epi32(0xFFFFFFFF,0,0xFFFFFFFF,0), _mm_set_epi32(0x0F0F,0,0,0))

		__SPU_TEST_R(si_gbh, _mm_set_epi32(0xFFFFFFFF,0,0xFFFFFFFF,0), _mm_set_epi32(0x33,0,0,0))

		__SPU_TEST_RR( si_avgb, _mm_set_epi8(100,2,4,0,0,0,0,0,0,0,0,0,0,0,0,0), 
								_mm_set_epi8(200,4,3,0,0,0,0,0,0,0,0,0,0,0,0,0), 
								_mm_set_epi8(150,3,4,0,0,0,0,0,0,0,0,0,0,0,0,0))

		__SPU_TEST_RRR( si_shufb, _mm_setr_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15), 
						_mm_setr_epi8(16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31), 
						_mm_set_epi8(0x80,0xC0,0xE0,0,0x10,2,0x12,4,0x14,6,0x16,8,0,0,0,0),
						_mm_set_epi8(0,	  0xFF,0x80,0,16,  2,18,  4,20,  6,22,  8,0,0,0,0))

		// Shift left

		__SPU_TEST_RR( si_shlh, _mm_set1_epi16(1), _mm_set1_epi16(4), _mm_set1_epi16(16) )

		__SPU_TEST_RI( si_shlhi, _mm_set1_epi16(1), 4, _mm_set1_epi16(16) )

		__SPU_TEST_RR( si_shl, _mm_set1_epi32(1), _mm_set1_epi32(4), _mm_set1_epi32(16) )

		__SPU_TEST_RI( si_shli, _mm_set1_epi32(1), 4, _mm_set1_epi32(16) )

		_CRT_ALIGN(16) uint8_t buf1[] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
		_CRT_ALIGN(16) uint8_t buf2[] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0 };
		_CRT_ALIGN(16) uint8_t buf3[] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00 };

		__SPU_TEST_RR( si_shlqbi, _mm_set1_epi32(0xFFFFFFFF), _mm_set1_epi32(4), BSWAP128(*(__m128i*)buf2) )

		__SPU_TEST_RI( si_shlqbii, _mm_set1_epi32(0xFFFFFFFF), 4, BSWAP128(*(__m128i*)buf2) )

		__SPU_TEST_RI( si_shlqbyi, _mm_set1_epi32(0xFFFFFFFF), 3, BSWAP128(*(__m128i*)buf3) )

		__SPU_TEST_RR( si_shlqby, _mm_set1_epi32(0xFFFFFFFF), _mm_set1_epi32(3), BSWAP128(*(__m128i*)buf3) )

		__SPU_TEST_RR( si_shlqbybi, _mm_set1_epi32(0xFFFFFFFF), _mm_set1_epi32(3<<3), BSWAP128(*(__m128i*)buf3) )

		// Rotate left

		_CRT_ALIGN(16) uint8_t buf4[] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0xFF,0xFF,0xFF };

		__SPU_TEST_RR( si_rotqby, BSWAP128(*(__m128i*)buf3), _mm_set1_epi32(3), BSWAP128(*(__m128i*)buf4) )


		SPUTest_ReportErrors();

	return 0;
}