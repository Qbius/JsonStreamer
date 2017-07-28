#ifndef STREAM_PARSER_H
#define STREAM_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace JSON
{

	namespace details
	{

		class jsonObject;
		class jsonElement;
		class jsonString;
		class jsonArray;

	}

	details::jsonObject parseJson(const std::string& source);

	class jsonError : public std::exception
	{
		std::string errorMsg;

	public:

		jsonError(const std::string& msg);
		const char* what() const throw();
	};

	namespace details
	{

		class jsonObject
		{
			std::map<jsonString, jsonElement> body;

			friend jsonElement;
			friend jsonObject JSON::parseJson(const std::string&);

			jsonObject();
			jsonObject(std::string::const_iterator& it, const std::string::const_iterator& end);

		public:

			jsonElement& operator[](const std::string& key);
			size_t size() const;
		};


		class jsonString
		{
			std::string body;

			friend jsonObject;
			friend jsonElement;
			friend jsonArray;

			jsonString();
			jsonString(const std::string& msg);
			jsonString(std::string::const_iterator& it, const std::string::const_iterator& end);

		public:

			operator std::string() const;
			inline bool operator<(const jsonString& rhs) const
			{
				return body < rhs.body;
			}
		};


		class jsonArray
		{
			std::vector<jsonElement> body;

			friend jsonElement;

			jsonArray();
			jsonArray(std::string::const_iterator& it, const std::string::const_iterator& end);

		public:

			jsonElement& operator[](int index);
			size_t size() const;
		};


		class jsonElement
		{
			jsonString strValue;
			int intValue;
			bool boolValue;
			jsonObject objValue;
			jsonArray arrValue;

			friend jsonObject;
			friend jsonArray;

			jsonElement(std::string::const_iterator& it, const std::string::const_iterator& end);

		public:

			jsonElement();

			template<typename T>
			const T& get() const;

			template<>
			const std::string& get<std::string>() const
			{
				return strValue;
			}

			template<>
			const int& get<int>() const
			{
				return intValue;
			}

			template<>
			const bool& get<bool>() const
			{
				return boolValue;
			}

			template<>
			const jsonObject& get<jsonObject>() const
			{
				return objValue;
			}

			template<>
			const jsonArray& get<jsonArray>() const
			{
				return arrValue;
			}
		};

	}

}

#endif
