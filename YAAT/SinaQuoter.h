#pragma once
#include <string>
#include <vector>
#include <boost\beast.hpp>
#include "SharedMemory.hpp"


struct Header
{
	volatile int status;
};

struct Quotation
{
	char symbol[6];
	float price;
};


class SinaQuoter
{
public:
	SinaQuoter(WriteOnlySharedMemory<Quotation, Header>& mem);

	void subscribe(const std::string& symbol);
	void buildTarget();
	void writeQuotation();
private:
	WriteOnlySharedMemory<Quotation, Header>& _mem;
	std::string _target;
	std::vector<std::string> _symbols;
	boost::asio::io_context _context;
	boost::asio::ip::tcp::socket _socket{_context};
	boost::asio::ip::tcp::resolver _resolver{_context};
	boost::asio::ip::tcp::resolver::query _query{ "hq.sinajs.cn","http" };
	boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> _results;
	std::string addPrefix(const std::string& symbol);
	boost::beast::http::request<boost::beast::http::string_body> _request;
	boost::beast::flat_buffer _buffer{ 4096 };
};


class SinaRecord
{
public:
	SinaRecord(const char* cbegin,const char* cend) :
		_cbegin{cbegin},
		_cend{cend}
	{

	}
	//≤‚ ‘”√
	std::string look()
	{
		char tmp[255]{0};
		std::memcpy(tmp, _cbegin, _cend - _cbegin);
		return tmp;
	}
private:
	const char* _cbegin;
	const char* _cend;
};

class SinaResponse
{
public:
	SinaResponse(const std::string& response):
		_cbegin{response.c_str()},
		_cend{ _cbegin +response.length()}
	{

	}

	inline bool next(SinaRecord& record)
	{
		const char* obegin = _cbegin;
		_cbegin = std::find(_cbegin+1, _cend, '\n');
		if (_cbegin == _cend)
		{
			return false;
		}
		else
		{
			record = SinaRecord(obegin, _cbegin);
		}
	}
private:
	const char* _cbegin;
	const char* _cend;
};
