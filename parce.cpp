#include "webserv.h"

// NOTE: should skip the comment's too

std::vector<std::string>	ft_split(const std::string &s)
{
	size_t	pos = 0;
	size_t	i = 0;
	std::vector<std::string> v_string;

	while (pos != s.length())
	{
		pos = s.find(" ", i);
		if (pos == std::string::npos)
		{
			pos = s.length();
		}
		v_string.push_back(s.substr(i, pos - i));
		i = pos + 1;
	}
	return (v_string);
}

bool		ft_check_while_space(const std::string &s)
{
	for (size_t i = 0; i < s.length(); i++)
	{
		if (s[i] == ' ' || (s[i] >= 9 && s[i] <= 13))
			return (true);
	}
	return (false);
}

t_config	ft_parce_config(const char *path_to_config)
{
	t_config		config;
	std::vector<t_config>		v_config;
	std::stack<char>			s_char;
	std::vector<std::string>		v_string;
	std::ifstream	is( path_to_config );
	std::string		str;
	size_t				i = 1;

	if (!is.is_open())
	{
		throw std::runtime_error(std::string("failed to open the file : ") + str);
	}
	while (std::getline(is, str))
	{
		if (ft_check_while_space(str))
		{
			str.clear();
			continue ;
		}
		v_string = ft_split(str);
		if (v_string[0] == "server")
		{
			if (v_string[v_string.size() - 1] != "{")
			{
				std::stringstream ss;

				ss << "in the line : " << i << ", Block diractive should have beginning '{'";
				throw std::runtime_error(ss.str());
			}
			if (v_string.size() != 2)
			{
				std::stringstream ss;

				ss << "in the line : " << i << " , the block diractive should have only the beginning";
				throw std::runtime_error(ss.str());
			}
			if (s_char.size())
			{
				std::stringstream ss;

				ss << "in the line : " << i << " , the config file have multi biginning '{}'";
				throw std::runtime_error(ss.str());
			}
			s_char.push('{');
		}
		if (v_string[0] == "location")
		{
			if (v_string[v_string.size() - 1] != "{")
			{
				std::stringstream ss;

				ss << "in the line : " << i << ", Block diractive should have beginning '{'";
				throw std::runtime_error(ss.str());
			}
			if (v_string.size() != 3)
			{
				std::stringstream ss;

				ss << "in the line : " << i << " , the block diractive [location] should have a value and a beginning '{'";
				throw std::runtime_error(ss.str());
			}
			if (s_char.size() != 2)
			{
				std::stringstream ss;

				ss << "in the line : " << i << " , the config file have multi biginning '{}'";
				throw std::runtime_error(ss.str());
			}
			s_char.push('{');
		}
		else
		{

		}
		str.clear();
		i++;
	}
	return (config);
}
