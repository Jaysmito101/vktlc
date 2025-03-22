#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cstdint>
#include <any>
#include <utility>
#include <stack>
#include <queue>
#include <tuple>


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

	using Any = std::any;

	template<typename Key, typename Value>
	using UnorderedMap = std::unordered_map<Key, Value>;

	template<typename T>
	using Set = std::set<T>;

	template <typename T, int N>
	using Array = std::array<T, N>;

	template <typename... Ts>
	using Tuple = std::tuple<Ts...>;

	namespace internal {
		template<typename T, int N>
		struct GenerateTupleNType {
			template <typename = std::make_index_sequence<N>>
			struct Impl;

			template <size_t... I>
			struct Impl<std::index_sequence<I...>> {
				template <size_t>
				using Wrap = T;

				using Type = Tuple<Wrap<I>...>;
			};

			using Type = typename Impl<>::Type;
		};
	}

	template<typename T, int N>
	using HomoTuple = typename internal::GenerateTupleNType<T, N>::Type;

	template<int N, typename... Ts>
	using NthType = std::tuple_element_t<N, Tuple<Ts...>>;

	template<typename A, typename B>
	using Pair = std::pair<A, B>;

	template<typename A, typename B>
	inline Pair<A, B> MakePair(A a, B b)
	{
		return std::make_pair(a, b);
	}

	template<typename T>
	using Stack = std::stack<T>;

	template<typename T>
	using Queue = std::queue<T>;

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

	template<typename T>
	struct Option {
		T value;
		bool hasValue = false;

		Option() = default;
		Option(T value) : value(value), hasValue(true) {}

		inline operator bool() const { return hasValue; }
		inline T& operator*() { return value; }
		inline const T& operator*() const { return value; }
		inline T* operator->() { return &value; }
		inline const T* operator->() const { return &value; }
	};

	template<typename T, typename Err>
	struct Result {
		T value;
		Err error;
		bool hasValue = false;
		bool hasError = true;

		Result() = default;

		
		inline operator bool() const { return hasValue; }
		inline T& operator*() { return value; }
		inline const T& operator*() const { return value; }
		inline T* operator->() { return &value; }
		inline const T* operator->() const { return &value; }
		inline Err& Error() { return error; }
		inline const Err& Error() const { return error; }
		inline bool IsError() const { return hasError; }
		inline bool IsOk() const { return hasValue; }

	private:
		Result(Option<T> option, Option<Err> error) : value(option.value), error(error.value), hasValue(option.hasValue), hasError(error.hasValue) {}

		template<typename U, typename E>
		friend Result<U, E> Ok(U value);	

		template<typename U, typename E>
		friend Result<U, E> Err(E error);
	};

	template<typename T, typename Err>
	inline Result<T, Err> Ok(T value) { return Result<T, Err>(Option<T>(value), Option<Err>()); }

	template<typename T, typename ErrT>
	inline Result<T, ErrT> Err(ErrT error) { return Result<T, ErrT>(Option<T>(), Option<ErrT>(error)); }
}