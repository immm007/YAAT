#include "pch.h"
#include "sina.h"
#include <iostream>
#include <boost\algorithm\string.hpp>


using namespace std;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

unordered_map<string, string> SinaQuoter::_symbols;

SinaQuoter::SinaQuoter() :
	_wmem{ "sina_quotation",4096 }
{
	_results = _resolver.resolve(_query);
	_request.method(http::verb::get);
	_request.set(http::field::host, "hq.sinajs.cn");
	_request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
}

void SinaQuoter::subscribe(const string&  symbol)
{
	_symbols[symbol] = "1970-01-01,00:00:00";
	_prefixed_symbols.push_back(addPrefix(symbol));
}

void SinaQuoter::buildTarget()
{
	_target = "/list=" + boost::join(_prefixed_symbols, ",");
	_request.target(_target);
}

void SinaQuoter::writeQuotation()
{
	static auto iter = _wmem.begin();
	static auto end = _wmem.end();
	static int i = 0;
	while (true)
	{
		boost::asio::connect(_socket, _results);
		http::write(_socket, _request);
		http::response<http::string_body> res;
		http::read(_socket, _buffer, res);

		SinaResponse sr{ res.body() };
		while (!sr.eof())
		{
			SinaRecord record = sr.next();
			//std::cout << sizeof(record);
			//std::cout << record.look()
			bool r = record.parseAndWrite(&iter);
			if (r)
			{
				++iter;
				std::cout << i++ << endl;
			}
			if (iter == end)
				return;
		}
	}
}

std::string SinaQuoter::addPrefix(const std::string& symbol)
{
	switch (symbol[0])
	{
	case '6':
		return "sh" + symbol;
	case '0':
	case '3':
		return "sz" + symbol;
	default:
		assert(false);
	}
}

inline bool SinaRecord::parseAndWrite(Quotation* q)
{
	const char* e = find(_cbegin, _cend, '=');
	string symbol{ e - 6,e };
	string dt{ _cend-24,_cend - 5 };
	if (dt> SinaQuoter::_symbols[symbol])
	{
		SinaQuoter::_symbols[symbol] = dt;
		const char* m = find(_cbegin, _cend, ',');
		sscanf(m + 1, "%f,%f,%f,%f,%f,%f,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%s",
			&q->open, &q->close, &q->price, &q->high, &q->low, &q->bid, &q->ask, &q->volume, &q->money,
			&q->bvol1, &q->bid1, &q->bvol2, &q->bid2, &q->bvol3, &q->bid3, &q->bvol4, &q->bid4, &q->bvol5, &q->bid5,
			&q->avol1, &q->ask1, &q->avol2, &q->ask2, &q->avol3, &q->ask3, &q->avol4, &q->ask4, &q->avol5, &q->ask5,
			&q->dt);
		return true;
	}
	else
	{
		return false;
	}

}
