#include "../../webserv.h"

void ft_check_headers(t_string_split_http &s)
{
	for (std::map<std::string, std::string>::iterator it = s.header.begin(); it != s.header.end(); it++)
	{
		if (!( it->first.size() ) && !(it->second.size()))
		{
			s.error = MISSING_KEY_VALUE_IN_HEADER;
			return ;
		}
		else if (!(it->first.size()))
		{
			s.error = MISSING_KEY_IN_HEADER;
			return ;
		}
		else if (!(it->second.size()))
		{
			s.error = MISSING_VALUE_IN_HEADER;
			return ;
		}
	}
}
