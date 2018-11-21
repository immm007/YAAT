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

	const Quotation* pq;
	ReadSharedMemory<Header, Quotation> m{ "sina_quotation" };
	auto iter = m.begin();
	while (true)
	{
		pq = *iter;
		if (pq)
		{
			cout << pq->symbol;
			++iter;
		}
	}
	cin.get();
	return 0;
}