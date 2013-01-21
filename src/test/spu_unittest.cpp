#include "spu_unittest.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>

using namespace std;


static vector<string> ErrorMessages;

bool operator==( __m128 lhs, __m128 rhs )
{
	return  0 == memcmp( &lhs, &rhs, sizeof(__m128) );
}

void SPUTest_Expect( __m128 Result, __m128 Expected, const char* ErrorMsg )
{
	if ( 0 != memcmp( &Expected, &Result, sizeof(__m128) ) )
	{
		ErrorMessages.push_back(string(ErrorMsg) + "FAIL");
	}
	else
	{
		//ErrorMessages.push_back(string(ErrorMsg) + "PASS");
	}
}

void SPUTest_ReportErrors()
{
	copy( ErrorMessages.cbegin(), ErrorMessages.cend(), ostream_iterator<string>(cout, "\n"));
}