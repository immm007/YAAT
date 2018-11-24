#include "pch.h"
#include <iostream>
#include "sina.h"


using namespace std;

int main()
{
	SinaQuoter q;
	q.subscribe("600571");
	q.subscribe("000996");
	q.subscribe("601965");
	q.buildTarget();
	q.writeQuotation();
	return 0;
}