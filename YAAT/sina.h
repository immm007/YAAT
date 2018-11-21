#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <boost\asio.hpp>
#include <boost\beast.hpp>
#include "memory.hpp"

struct Header
{
	enum MemoryStatus
	{
		Raw,Written
	};
	volatile MemoryStatus status;
};

struct Quotation
{
	char symbol[6];
	long dt;
	float open;
	float close;
	float high;
	float low;
	int volume;
	float money;
	int bvol1;
	float bid1;
	int bvol2;
	float bid2;
	int bvol3;
	float bid3;
	int bvol4;
	float bid4;
	int bvol5;
	float bid5;
	int avol1;
	float ask1;
	int avol2;
	float ask2;
	int avol3;
	float ask3;
	int avol4;
	float ask4;
	int avol5;
	float ask5;
};

//一对指针的包装，避免Quotation解析后返回产生一次复制
class SinaRecord
{
public:
	SinaRecord(const char* cbegin, const char* cend) :
		_cbegin{ cbegin },
		_cend{ cend }
	{

	}
	//测试用
	std::string look()
	{
		char tmp[255]{ 0 };
		std::memcpy(tmp, _cbegin, _cend - _cbegin);
		return tmp;
	}
	inline void parseAndWrite(void* q);
private:
	const char* _cbegin;
	const char* _cend;
};

//http response指针的封装，避免字符串额外复制
class SinaResponse
{
public:
	SinaResponse(const std::string& response) :
		_cbegin{ response.c_str() },
		_cend{ _cbegin + response.length() }
	{

	}
	inline bool eof() const
	{
		//依赖于新浪行情回复倒数第二个字符一定是'\n'，省去每次next比较
		return _cbegin == _cend-1;
	}
	//开销很小，两个指针
	inline SinaRecord next()
	{
		const char* obegin = _cbegin;
		_cbegin = std::find(_cbegin + 1, _cend, '\n');
		return SinaRecord(obegin, _cbegin);
	}
private:
	const char* _cbegin;
	const char* _cend;
};

class SinaQuoter
{
public:
	SinaQuoter();
	void subscribe(const std::string& symbol);
	void buildTarget();
	void writeQuotation();
private:
	WriteSharedMemory<Header, Quotation> _wmem;
	ReadSharedMemory<Header, Quotation> _rmen;
	std::unordered_map<std::string, long> _symbols;
	std::vector<std::string> _prefixed_symbols;
	std::string _target;

	boost::asio::io_context _context;
	boost::asio::ip::tcp::socket _socket{_context};
	boost::asio::ip::tcp::resolver _resolver{_context};
	boost::asio::ip::tcp::resolver::query _query{ "hq.sinajs.cn","http" };
	boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> _results;
	boost::beast::http::request<boost::beast::http::string_body> _request;
	boost::beast::flat_buffer _buffer{ 4096 };

	std::string addPrefix(const std::string& symbol);
};