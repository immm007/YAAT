#include "pch.h"
#include "sina.h"
#include <iostream>
#include <boost\algorithm\string.hpp>


using namespace std;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

SinaQuoter::SinaQuoter() :
	_wmem{ "sina_quotation",4096 },
	_rmen {"sina_quotation"}
{
	_results = _resolver.resolve(_query);
	_request.method(http::verb::get);
	_request.set(http::field::host, "hq.sinajs.cn");
	_request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
}

void SinaQuoter::subscribe(const string&  symbol)
{
	_symbols[symbol] = 0;
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
	boost::asio::connect(_socket,_results);
	http::write(_socket, _request);
	http::response<http::string_body> res;
	http::read(_socket, _buffer, res);

	SinaResponse sr{ res.body() };
	while (!sr.eof())
	{
		SinaRecord record = sr.next();
		//std::cout << sizeof(record);
		//std::cout << record.look()
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

inline void SinaRecord::parseAndWrite(void* q)
{
	const char* m = find(_cbegin, _cend,'=');
	char tmp[7];
	memcpy(tmp, m - 6, 6);
	tmp[6] = '\0';
	string tmps{ tmp };

	char dt[20]{ 0 };
	memcpy(dt, _cend - 24,19);
}
