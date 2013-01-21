#include <string>
#include <iosfwd>
#include <cstdint>
#include <emmintrin.h>

void SPUTest_ReportErrors();

void SPUTest_Expect( __m128 Result, __m128 Expected, const char* ErrorMsg );

class xmm_proxy
{
public:
	__m128 data;

public:
	xmm_proxy()
		: data(_mm_setzero_ps())
	{
	}

	inline xmm_proxy( __m128 x ) : data(x) {}
	inline xmm_proxy( __m128i x ) : data(_mm_castsi128_ps(x)) {}
	inline xmm_proxy( __m128d x ) : data(_mm_castpd_ps(x)) {}

	inline operator __m128() const { return data; }
	inline operator __m128i() const { _mm_castps_si128(data); }
	inline operator __m128d() const { _mm_castps_pd(data); }
};


#define __SPU_TEST_RRR( f, a, b, c, e ) SPUTest_Expect( f(xmm_proxy(a),xmm_proxy(b),xmm_proxy(c)), xmm_proxy(e), \
						#f "(" #a ", " #b ", " #c " ) ... " );

#define __SPU_TEST_RR( f, a, b, e )		SPUTest_Expect( f(xmm_proxy(a),xmm_proxy(b)), xmm_proxy(e), \
						#f "(" #a ", " #b " ) ... " );

#define __SPU_TEST_RI( f, a, i, e )		SPUTest_Expect( f(xmm_proxy(a),(i)), xmm_proxy(e), \
						#f "(" #a ", " #i " ) ... " );

#define __SPU_TEST_R( f, a, e )			SPUTest_Expect( f(xmm_proxy(a)), xmm_proxy(e), \
						#f "(" #a " ) ... " );

