#include "stdafx.h"
#include "streamParser.h"

namespace JSON
{

	jsonError::jsonError(const std::string& msg) : errorMsg(msg)
	{
	}

	const char* jsonError::what() const throw()
	{
		return errorMsg.c_str();
	}



	details::jsonObject parseJson(const std::string& source)
	{
		try
		{
			return{ source.begin(), source.end() };
		}
		catch (const jsonError& err)
		{
			std::cerr << err.what();
		}
	}

	namespace details {

		jsonObject::jsonObject()
		{
		}

		jsonObject::jsonObject(std::string::const_iterator& it, const std::string::const_iterator& end)
		{
			for (++it; it != end;)
			{
				switch (*it)
				{
				case '"':
				{
					jsonString key{ it, end };
					body.insert(std::make_pair(key, jsonElement{ it, end }));
					break;
				}

				case '}':
				{
					++it;
					return;
				}

				default:
				{
					++it;
					break;
				}
				}
			}
			throw jsonError("Unresolved object");
		}

		jsonElement& jsonObject::operator[](const std::string& key)
		{
			return body[jsonString{ key }];
		}

		size_t jsonObject::size() const
		{
			return body.size();
		}



		jsonString::jsonString(const std::string& msg) : body(msg)
		{
		}

		jsonString::jsonString()
		{
		}

		jsonString::jsonString(std::string::const_iterator& it, const std::string::const_iterator& end)
		{
			for (++it; it != end; ++it)
			{
				switch (*it)
				{
					default:
					{
						body += *it;
						break;
					}

					case '\\':
					{
						++it;
						body += *it;
						break;
					}

					case '"':
					{
						++it;
						return;
					}
				}
			}
			throw jsonError("Unresolved string");
		}

		jsonString::operator std::string() const
		{
			return body;
		}



		jsonArray::jsonArray()
		{
		}

		jsonArray::jsonArray(std::string::const_iterator& it, const std::string::const_iterator& end)
		{
			for (++it; it != end;)
			{
				switch (*it)
				{
				default:
				{
					body.push_back(jsonElement{ it, end });
					break;
				}

				case ',':
				{
					++it;
					break;
				}

				case ']':
				{
					++it;
					return;
				}
				}
			}
			throw jsonError("Unresolved array");
		}

		jsonElement& jsonArray::operator[](int index)
		{
			return body[index];
		}

		size_t jsonArray::size() const
		{
			return body.size();
		}



		jsonElement::jsonElement(std::string::const_iterator& it, const std::string::const_iterator& end)
		{
			for (++it; it != end;)
			{
				switch (*it)
				{
				case '"':
				{
					strValue = { it, end };
					return;
				}

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				{
					intValue = *it - '0';
					for (++it; it != end && isdigit(*it); ++it)
						intValue = 10 * intValue + (*it - '0');
					return;
				}

				case 't':
				case 'f':
				{
					boolValue = *it == 't';
					while (isalpha(*++it))
						;
					return;
				}

				case '{':
				{
					objValue = { it, end };
					return;
				}

				case '[':
				{
					arrValue = { it, end };
					return;
				}

				default:
				{
					++it;
					break;
				}
				}
			}
			throw jsonError("Invalid element");
		}

		jsonElement::jsonElement()
		{
		}

	}

}