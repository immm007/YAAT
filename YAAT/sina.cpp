#include "pch.h"
#include "sina.h"
#include <boost\algorithm\string.hpp>

using namespace sina;

unordered_map<string, string> SinaQuoter::_symbols;

SinaQuoter::SinaQuoter()
{
	_results = _resolver.resolve(_query);
	_request.method(verb::get);
	_request.set(field::host, "hq.sinajs.cn");
	_request.set(field::user_agent, BOOST_BEAST_VERSION_STRING);
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

void SinaQuoter::get(response<string_body>& res)
{
	connect(_socket, _results);
	write(_socket, _request);
	read(_socket, _buffer, res);
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

bool SinaRecord::parseAndWrite(Quotation* q)
{
	const char* e = find(_cbegin, _cend, '=');
	string symbol{ e - 6,e };
	string dt{ _cend-24,_cend - 5 };
	if (dt> SinaQuoter::_symbols[symbol])
	{
		SinaQuoter::_symbols[symbol] = dt;
		memcpy(q->symbol, symbol.c_str(), 6);
		const char* m = find(_cbegin, _cend, ',');
		sscanf(m + 1, "%f,%f,%f,%f,%f,%f,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%d,%f,%19s",
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
