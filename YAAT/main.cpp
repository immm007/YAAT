#include "pch.h"
#include "sina.h"
#include "memory.hpp"
#include <ctime>


using namespace sina;

int main()
{
	SMForWrite<Header, Quotation> mem{ "sina_quotation",4096 * 100 };
	auto begin = mem.begin();
	auto end = mem.end();
	SinaQuoter quoter;
	quoter.subscribe("600571");
	quoter.buildTarget();
	while (begin!=end)
	{
		response<string_body> res;
		quoter.get(res);
		SinaResponse sres{ res.body() };
		while (!sres.eof())
		{
			SinaRecord record =  sres.next();
			if (record.parseAndWrite(&begin))
			{
				begin.markWritten();
				++begin;
			}
		}
	}
	assert(false);
}