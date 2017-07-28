#ifndef STREAM_SERIALIZER_H
#define STREAM_SERIALIZER_H

#include <ostream>

namespace JSON
{
	namespace details
	{
		const int IS_IN_OBJECT = 0x01;
		const int IS_NEW_INSTANCE = 0x02;

		inline int streamFlag()
		{
			static int streamFlag = std::ios_base::xalloc();
			return streamFlag;
		}

		template<typename T, typename U = T>
		class smanip
		{
		public:
			std::ostream& (*f)(std::ostream&, U);
			T arg;

			smanip(std::ostream& (*f)(std::ostream&, U), U arg)
				: f(f), arg(arg)
			{
			}
		};

		template <typename T, typename U = T>
		std::ostream& operator<<(std::ostream& st, const smanip<T, U>& s)
		{
			return s.f(st, s.arg);
		}

		class streamProperString : public smanip<const char*>
		{
		public:
			streamProperString(const char* v) : smanip<const char*>(present, v)
			{
			}

		private:
			static std::ostream& present(std::ostream& st, const char* v)
			{
				for (auto it = v; *it != '\0'; ++it)
				{
					if (*it == '\\' || *it == '\"' || *it == '\'')
						st << '\\';

					st << *it;
				}
				return st;
			}
		};

		class streamIfStateT
		{
		public:
			const char* msg;
			int flag;

			streamIfStateT(const char* msg, int flag) : msg(msg), flag(flag)
			{
			}
		};

		std::ostream& operator<<(std::ostream& st, const streamIfStateT& s)
		{
			return st << (((st.iword(streamFlag()) & s.flag) == s.flag) ? s.msg : "");
		}

		streamIfStateT streamIfState(const char* msg, int flag)
		{
			return streamIfStateT(msg, flag);
		}

		streamIfStateT streamIfState(const char* msg, int flag, int flag2)
		{
			return streamIfStateT(msg, flag | flag2);
		}

		template<typename ... TArgs>
		streamIfStateT streamIfState(const char* msg, int flag, int flag2, TArgs ... args)
		{
			return streamIfStateT(streamIfState(msg, flag | flag2, args));
		}

		class setTrue : public smanip<int>
		{
		public:
			setTrue(int flag) : smanip<int>(present, flag)
			{
			}

		private:
			static std::ostream& present(std::ostream& st, int flag)
			{
				st.iword(streamFlag()) |= flag;
				return st;
			}
		};

		class setFalse : public smanip<int>
		{
		public:
			setFalse(int flag) : smanip<int>(present, flag)
			{
			}

		private:
			static std::ostream& present(std::ostream& st, int flag)
			{
				st.iword(streamFlag()) &= ~flag;
				return st;
			}
		};

		class setOther : public smanip<int>
		{
		public:
			setOther(int flag) : smanip<int>(present, flag)
			{
			}

		private:
			static std::ostream& present(std::ostream& st, int flag)
			{
				st.iword(streamFlag()) ^= flag;
				return st;
			}
		};

		class keyT : public smanip<const char*>
		{
		public:
			keyT(const char* v) : smanip<const char*>(present, v)
			{
			}

		private:
			static std::ostream& present(std::ostream& st, const char* v)
			{
				return st << streamIfState(",", IS_NEW_INSTANCE) << setTrue(IS_IN_OBJECT) << setTrue(IS_NEW_INSTANCE) << "\"" << streamProperString(v) << "\"" << ":";
			}
		};

		template<typename T>
		class valueT : public smanip<T>
		{
		public:
			valueT(T v) : smanip<T>(present, v)
			{
			}

		private:
			static std::ostream& present(std::ostream& st, T v)
			{
				return st << setOther(IS_IN_OBJECT) << streamIfState(",", IS_IN_OBJECT, IS_NEW_INSTANCE) << setOther(IS_IN_OBJECT) << setTrue(IS_NEW_INSTANCE) << v;
			}
		};

		template<>
		class valueT<const char*> : public smanip<const char*>
		{
		public:
			valueT(const char* v) : smanip<const char*>(present, v)
			{
			}

		private:
			static std::ostream& present(std::ostream& st, const char* v)
			{
				return st << setOther(IS_IN_OBJECT) << streamIfState(",", IS_IN_OBJECT, IS_NEW_INSTANCE) << setOther(IS_IN_OBJECT) << setTrue(IS_NEW_INSTANCE) << "\"" << streamProperString(v) << "\"";
			}
		};

		template<>
		class valueT<std::string> : public smanip<std::string, const std::string&>
		{
		public:
			valueT(const std::string& v) : smanip<std::string, const std::string&>(present, v)
			{
			}

		private:
			static std::ostream& present(std::ostream& st, const std::string& v)
			{
				return st << setOther(IS_IN_OBJECT) << streamIfState(",", IS_IN_OBJECT, IS_NEW_INSTANCE) << setOther(IS_IN_OBJECT) << setTrue(IS_NEW_INSTANCE) << "\"" << streamProperString(v.c_str()) << "\"";
			}
		};

		template<>
		class valueT<bool> : public smanip<bool>
		{
		public:
			valueT(bool v) : smanip<bool>(present, v)
			{
			}

		private:
			static std::ostream& present(std::ostream& st, bool v)
			{
				return st << setOther(IS_IN_OBJECT) << streamIfState(",", IS_IN_OBJECT, IS_NEW_INSTANCE) << setOther(IS_IN_OBJECT) << setTrue(IS_NEW_INSTANCE) << (v ? "true" : "false");
			}
		};
	}

	inline std::ostream& begin_object(std::ostream& st)
	{
		using namespace details;
		return st << setOther(IS_IN_OBJECT) << streamIfState(",", IS_IN_OBJECT, IS_NEW_INSTANCE) << setOther(IS_IN_OBJECT) << setFalse(IS_NEW_INSTANCE) << "{";
	}

	inline std::ostream& end_object(std::ostream& st)
	{
		using namespace details;
		return st << setFalse(IS_IN_OBJECT) << setTrue(IS_NEW_INSTANCE) << "}";
	}

	inline std::ostream& begin_array(std::ostream& st)
	{

		using namespace details;
		return st << setOther(IS_IN_OBJECT) << streamIfState(",", IS_IN_OBJECT, IS_NEW_INSTANCE) << setOther(IS_IN_OBJECT) << setFalse(IS_NEW_INSTANCE) << "[" << setFalse(IS_IN_OBJECT) << setFalse(IS_NEW_INSTANCE);
	}

	inline std::ostream& end_array(std::ostream& st)
	{
		using namespace details;
		return st << setTrue(IS_NEW_INSTANCE) << "]";
	}

	inline std::ostream& reset_json(std::ostream& st)
	{
		using namespace details;
		return st << setFalse(IS_NEW_INSTANCE) << setFalse(IS_IN_OBJECT);
	}

	inline details::keyT key(const char* v)
	{
		return details::keyT(v);
	}

	inline details::keyT key(const std::string& v)
	{
		return details::keyT(v.c_str());
	}

	template<typename T>
	details::valueT<T> value(T v)
	{
		return details::valueT<T>(v);
	}
}

#endif
