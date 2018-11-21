#pragma once
#include <string>
#include <vector>
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
	float price;
};

//һ��ָ��İ�װ������Quotation�����󷵻ز���һ�θ���
class SinaRecord
{
public:
	SinaRecord(const char* cbegin, const char* cend) :
		_cbegin{ cbegin },
		_cend{ cend }
	{

	}
	//������
	std::string look()
	{
		char tmp[255]{ 0 };
		std::memcpy(tmp, _cbegin, _cend - _cbegin);
		return tmp;
	}
	inline void parseAndWrite(Quotation* q);
private:
	const char* _cbegin;
	const char* _cend;
};

//http responseָ��ķ�װ�������ַ������⸴��
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
		//��������������ظ������ڶ����ַ�һ����'\n'��ʡȥÿ��next�Ƚ�
		return _cbegin == _cend-1;
	}
	//������С������ָ��
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

	WriteSharedMemory<Header, Quotation> _mem;
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

