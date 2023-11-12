#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <format>

#include "core/Types.hpp"

namespace tlc
{
	enum class LogLevel : U8
	{
		Trace = 0b00000001,
		Debug = 0b00000010,
		Info = 0b00000100,
		Warning = 0b00001000,
		Error = 0b00010000,
		Fatal = 0b00100000,
		All = 0b11111111,
		None = 0b00000000
	};

	inline constexpr LogLevel operator|(LogLevel lhs, LogLevel rhs)
	{
		return static_cast<LogLevel>(static_cast<U8>(lhs) | static_cast<U8>(rhs));
	}

	inline constexpr LogLevel operator&(LogLevel lhs, LogLevel rhs)
	{
		return static_cast<LogLevel>(static_cast<U8>(lhs) & static_cast<U8>(rhs));
	}

	class Logger
	{
	public:
		Logger();
		~Logger() = default;

		Bool AttachFile(const String& file, LogLevel level = LogLevel::All);
		Bool DetachFile(const String& file);

		void Log(LogLevel level, const String& message);

		template<typename... Args>
		inline void Log(LogLevel level, const String& message, Args... args)
		{
			String formattedMessage = std::vformat(message, std::make_format_args(args...));
			Log(level, formattedMessage);
		}

		inline void EnableConsole(bool enable) { m_EnableConsole = enable; }
		Bool IsConsoleEnabled() const { return m_EnableConsole; }

		inline void SetLogLevelFilter(LogLevel level) { m_LogLevelFilter = level; }

		String GetLogLevelString(LogLevel level) const;

		inline static void Init() { if (!s_Instance) s_Instance = CreateScope<Logger>(); }
		inline static Logger* Get() { if (!s_Instance) s_Instance = CreateScope<Logger>(); return s_Instance.get(); }
		inline static void Shutdown() { s_Instance.reset(); }

	private:
		void LogToFile(const String& file, const String& message);
		void LogToConsole(LogLevel level, const String& message);


	private:
		std::vector<std::pair<String, LogLevel>> m_AttachedFiles;
		Bool m_EnableConsole = true;
		LogLevel m_LogLevelFilter = LogLevel::All;

		static Scope<Logger> s_Instance;
	};

	namespace log
	{
		inline void AttachFile(const String& file, LogLevel level = LogLevel::All)
		{
			Logger::Get()->AttachFile(file, level);
		}

		inline void DetachFile(const String& file)
		{
			Logger::Get()->DetachFile(file);
		}

		template <typename... Args>
		inline void Log(LogLevel level, const String& message, Args... args)
		{
			Logger::Get()->Log(level, message, args...);
		}

		template <typename... Args>
		inline void Trace(const String& message, Args... args)
		{
			Logger::Get()->Log(LogLevel::Trace, message, args...);
		}

		template <typename... Args>
		inline void Debug(const String& message, Args... args)
		{
			Logger::Get()->Log(LogLevel::Debug, message, args...);
		}

		template <typename... Args>
		inline void Info(const String& message, Args... args)
		{
			Logger::Get()->Log(LogLevel::Info, message, args...);
		}

		template <typename... Args>
		inline void Warn(const String& message, Args... args)
		{
			Logger::Get()->Log(LogLevel::Warning, message, args...);
		}

		template <typename... Args>
		inline void Error(const String& message, Args... args)
		{
			Logger::Get()->Log(LogLevel::Error, message, args...);
		}

		template <typename... Args>
		inline void Fatal(const String& message, Args... args)
		{
			Logger::Get()->Log(LogLevel::Fatal, message, args...);
			throw std::runtime_error(message);
		}
	}
}

