#include <cstring>
#include <boost/lexical_cast.hpp>
#include "packet_header.h"

const std::array<std::pair<PacketHeader::Type, boost::regex>, 6> PacketHeader::regexes{{
		{ PacketHeader::Type::CLIENT, boost::regex("CLIENT ([0-9]+)") },
		{ PacketHeader::Type::UPLOAD, boost::regex("UPLOAD ([0-9]+)") },
		{ PacketHeader::Type::DATA, boost::regex("DATA ([0-9]+) ([0-9]+) ([0-9]+)") },
		{ PacketHeader::Type::ACK, boost::regex("ACK ([0-9]+) ([0-9]+)") },
		{ PacketHeader::Type::RETRANSMIT, boost::regex("RETRANSMIT ([0-9]+)") },
		{ PacketHeader::Type::KEEPALIVE, boost::regex("KEEPALIVE") }
}};

PacketHeader::PacketHeader(Type type, unsigned param1, unsigned param2, unsigned param3)
	: type(type)
{
	params[1] = param1;
	params[2] = param2;
	params[3] = param3;
}

PacketHeader::PacketHeader()
{
}

PacketHeader PacketHeader::parse(const char* buffer, const unsigned &size)
{
	PacketHeader header;
	const char* end = (const char*) memchr(buffer, '\n', size);
	if(end == NULL)
	{
		header.type = Type::UNKNOWN;
		return header;
	}
	for(auto reg : regexes)
	{
		boost::cmatch match_results;
		if(boost::regex_match(buffer, end, match_results, reg.second))
		{
			header.type = reg.first;
			header.size = end - buffer + 1;
			for(unsigned i = 1; i < match_results.size(); i++)
				header.params[i] = boost::lexical_cast<unsigned>(match_results[i]);
			return header;
		}
	}
	header.type = Type::UNKNOWN;
	return header;
}

void PacketHeader::build(char* buffer)
{
	switch(type)
	{
		case Type::CLIENT:
			sprintf(buffer, "CLIENT %u\n", params[1]);
			break;
		case Type::UPLOAD:
			sprintf(buffer, "UPLOAD %u\n", params[1]);
			break;
		case Type::DATA:
			sprintf(buffer, "DATA %u %u %u\n", params[1], params[2], params[3]);
			break;
		case Type::ACK:
			sprintf(buffer, "ACK %u %u\n", params[1], params[2]);
			break;
		case Type::RETRANSMIT:
			sprintf(buffer, "RETRANSMIT %u\n", params[1]);
			break;
		case Type::KEEPALIVE:
			sprintf(buffer, "KEEPALIVE\n");
			break;
		default:
			break;
	}
	size = strlen(buffer);
}

unsigned PacketHeader::get_header_size()
{
	return size;
}

PacketHeader::Type PacketHeader::get_type()
{
	return type;
}

unsigned PacketHeader::get_param(unsigned index)
{
	return params[index];
}


