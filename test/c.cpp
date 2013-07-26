// Copyright 2013 Kevin Cox

/*******************************************************************************
*                                                                              *
*  This software is provided 'as-is', without any express or implied           *
*  warranty. In no event will the authors be held liable for any damages       *
*  arising from the use of this software.                                      *
*                                                                              *
*  Permission is granted to anyone to use this software for any purpose,       *
*  including commercial applications, and to alter it and redistribute it      *
*  freely, subject to the following restrictions:                              *
*                                                                              *
*  1. The origin of this software must not be misrepresented; you must not     *
*     claim that you wrote the original software. If you use this software in  *
*     a product, an acknowledgment in the product documentation would be       *
*     appreciated but is not required.                                         *
*                                                                              *
*  2. Altered source versions must be plainly marked as such, and must not be  *
*     misrepresented as being the original software.                           *
*                                                                              *
*  3. This notice may not be removed or altered from any source distribution.  *
*                                                                              *
*******************************************************************************/

#include <stdio.h>
#include <fstream>

#include <string.h>

#include <sctp.hpp>

#include <sys/types.h>
#include <netinet/sctp.h>

#include <arpa/inet.h>

using namespace std;

int main ( int argc, char **argv )
{
	ifstream t("test.dat");
	vector<uint8_t> d((std::istreambuf_iterator<char>(t)),
					   std::istreambuf_iterator<char>());
	//cout << str << endl;
	cout << d.size() << endl;
	
	string sdata = "this is some test data!\n";
	vector<uint8_t> data(sdata.begin(), sdata.end());
	
	SCTP s;
	Remote r;
	
	r.setIP("127.0.0.1");
	r.setPort(1234);
	
	s.send(&r, 1, data);
	s.send(&r, 2, d);
}
