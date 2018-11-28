#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <boost\asio.hpp>
#include <boost\beast.hpp>

namespace sina 
{
	using namespace std;
	using namespace boost::asio;
	using namespace boost::asio::ip;
	using namespace boost::beast;
	using namespace boost::beast::http;

	struct Header
	{
		enum Status
		{
			Raw, Written
		};
		volatile Status status;
	};

	struct Quotation
	{
		char dt[20];
		char symbol[6];
		float open;
		float close;
		float price;
		float high;
		float low;
		float bid;
		float ask;
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

	//һ��ָ��İ�װ������Quotation�����󷵻ز���һ�θ���
	class SinaRecord
	{
	public:
		SinaRecord(const char* cbegin, const char* cend) :
			_cbegin{ cbegin },
			_cend{ cend }
		{

		}
		bool parseAndWrite(Quotation* q);
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
			return _cbegin == _cend - 1;
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
		static std::unordered_map<std::string, std::string> _symbols;

		SinaQuoter();
		void subscribe(const std::string& symbol);
		void buildTarget();
		void get(response<string_body>& res);
		bool isTradeTime() const;
	private:
		std::vector<std::string> _prefixed_symbols;
		std::string _target;

		boost::asio::io_context _context;
		tcp::socket _socket{ _context };
		tcp::resolver _resolver{ _context };
		tcp::resolver::query _query{ "hq.sinajs.cn","http" };
		basic_resolver_results<tcp> _results;
		request<string_body> _request;
		flat_buffer _buffer{ 4096 };

		std::string addPrefix(const std::string& symbol);
	};
}