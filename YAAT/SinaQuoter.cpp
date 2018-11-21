#include "pch.h"
#include "SinaQuoter.h"
#include <boost\algorithm\string.hpp>
#include <boost\asio.hpp>
#include <iostream>

using namespace std;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

SinaQuoter::SinaQuoter(WriteOnlySharedMemory<Quotation, Header>& mem) :
	_mem{ mem }
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
	boost::asio::connect(_socket,_results);
	http::write(_socket, _request);
	http::response<http::string_body> res;
	http::read(_socket, _buffer, res);

	SinaResponse r{ res.body() };
	SinaRecord& record = SinaRecord(0, 0);
	while (r.next(record))
	{
		cout << record.look();
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
