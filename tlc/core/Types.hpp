#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cstdint>


namespace tlc 
{

	using U8 = uint8_t;
	using U16 = uint16_t;
	using U32 = uint32_t;
	using U64 = uint64_t;

	using I8 = int8_t;
	using I16 = int16_t;
	using I32 = int32_t;
	using I64 = int64_t;

	using Size = size_t;

	using Bool = bool;

	using F32 = float;
	using F64 = double;

	using String = std::string;
	using WString = std::wstring;
	using StringView = std::string_view;
	using WStringView = std::wstring_view;

	using CString = const char*;
	using CStringList = std::vector<CString>;

	template<typename T>
	using List = std::vector<T>;

	template<typename Key, typename Value>
	using Map = std::map<Key, Value>;

	template<typename Key, typename Value>
	using UnorderedMap = std::unordered_map<Key, Value>;

	template<typename T>
	using Set = std::set<T>;

	template <typename T, int N>
	using Array = std::array<T, N>;


	template<typename A, typename B>
	struct Pair 
	{
	public:
		A x;
		B y;

		Pair() : x(0), y(0) {}
		Pair(A x, B y) : x(x), y(y) {}
	};

	template<typename A, typename B>
	inline Pair<A, B> MakePair(A x, B y)
	{
		return Pair<A, B>(x, y);
	}
	

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T>
	using WeakRef = std::weak_ptr<T>;

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Raw = T*;

	template<typename T, typename... Args>
	inline Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	inline Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	inline Raw<T> CreateRaw(Args&&... args)
	{
		return new T(std::forward<Args>(args)...);
	}

	template<typename Target, typename Source>
	inline Ref<Target> CastRef(const Ref<Source>& source)
	{
		return std::static_pointer_cast<Target>(source);
	}

	/*
	template<typename T>
	inline void DeleteRaw(Raw<T> ptr)
	{
		delete ptr;
	}
	*/
#define DeleteRaw(x) if (x != nullptr) { delete x; x = nullptr; }
}