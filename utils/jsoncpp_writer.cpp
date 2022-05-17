#include "jsoncpp_writer.h"
/*

JsonWriter::JsonWriter(const Json::Value& json, bool is_indent)
	:_json(json),
    _is_indent(is_indent)
{}

void JsonWriter::to_string(std::string& out)
{
	switch (_json.type())
	{
	case Json::nullValue:
		break;
	case Json::intValue:
		pushValue(Json::valueToString(_json.asLargestInt()));
		break;
	case Json::uintValue:
		pushValue(Json::valueToString(_json.asLargestUInt()));
		break;
	case Json::realValue:
		pushValue(Json::valueToString(_json.asDouble()));
		break;
	case Json::stringValue:
		pushValue(_json.asString());
		break;
	case Json::booleanValue:
		pushValue(Json::valueToString(_json.asBool()));
		break;
	case Json::arrayValue:
		writeArrayValue(_json);
		break;
	}
}

void JsonWriter::pushValue(const std::string& str)
{
	ss << str;
}

void JsonWriter::writeArrayValue2(Json::Value const& value)
{
    unsigned size = value.size();
    if (size == 0)
    {
        pushValue("[]");
    }
    writeWithIndent("[");
    for (unsigned i = 0; i < size; ++i)
    {
        Json::ValueType tt = value[i].type();
        if (tt == Json::objectValue)
        {

        }
        else if (tt == Json::arrayValue)
        {
        }
        else if (tt == Json::nullValue)
        {
        }
        else
        {
            pushValue(value[i].asString());
        }
    }
    writeWithIndent("]");
}

void JsonWriter::writeIndent()
{
    ss << "\n";
}

void JsonWriter::writeWithIndent(std::string const& value)
{}

*/