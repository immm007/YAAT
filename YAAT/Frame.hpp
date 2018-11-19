#pragma once

template<typename T>
class Frame
{
public:
	enum Status
	{
		Raw, Written
	};
	struct Header
	{
		volatile Status status;
	};
	static constexpr int headerLength = sizeof(Header);
	static constexpr int contentLength = sizeof(T);
	static constexpr int length = headerLength + contentLength;

	Frame(char* p) :
		_header{ reinterpret_cast<Header*>(p) }
	{
		static_assert(std::is_pod<T>::value, "only POD");
	}

	inline Status status() const
	{
		return _header->status;
	}

	inline void operator=(const T& data)
	{
		*reinterpret_cast<T*>(dataEntry()) = data;
		_header->status = Status::Written;
	}
	inline const T* data() const
	{
		return reinterpret_cast<const T*>(dataEntry());
	}
private:
	Header* _header;

	inline void* dataEntry() const
	{
		return  reinterpret_cast<void*>(reinterpret_cast<char*>(_header) + headerLength);
	}
};