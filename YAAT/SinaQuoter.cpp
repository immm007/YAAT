#include "pch.h"
#include "SinaQuoter.h"
#include <boost\algorithm\string.hpp>
#include <iostream>


using namespace std;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

SinaQuoter::SinaQuoter() : _mem{"sina_quotation",4096}
{
	_results = _resolver.resolve(_query);
	_request.method(http::verb::get);
	_request.set(http::field::host, "hq.sinajs.cn");
	_request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
}

void SinaQuoter::subscribe(const string&  symbol)
{
	_symbols.push_back(addPrefix(symbol));
}

void SinaQuoter::buildTarget()
{
	_target = "/list=" + boost::join(_symbols, ",");
	_request.target(_target);
}

void SinaQuoter::writeQuotation()
{
	static auto iter = _mem.begin();
	boost::asio::connect(_socket,_results);
	http::write(_socket, _request);
	http::response<http::string_body> res;
	http::read(_socket, _buffer, res);

	SinaResponse sr{ res.body() };
	while (!sr.eof())
	{
		SinaRecord record = sr.next();
		//std::cout << record.look();
		record.parseAndWrite(&iter);
		++iter;
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

inline void SinaRecord::parseAndWrite(Quotation* q)
{
	const char* m = find(_cbegin, _cend,'=');
	std::memcpy(q, m - 6, 6);
}
