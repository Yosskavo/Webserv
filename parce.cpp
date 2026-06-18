#include "webserv.h"

// TODO: skiping comment's and delete new line
// TODO: Work on brakets {} and a functionality of nested brakest
// TODO: Work on accepting value

bool	ft_is_white_spaces(char c)
{
	if (c == ' ' || (c >= 9 && 13 >= c))
	{
		return (true);
	}
	return (false);
}

bool	ft_getserver(std::vector<std::string> &v_s, std::ifstream &is)
{
	std::stack<char> sc;
	std::string	str = "";
	char		c;

	if (is.eof())
		return (false);
	while (is.get(c))
	{
		if (c == ';' || c == '{' || c == '}')
		{
			if (c == '{')
			{
				sc.push('{');
			}
			else if (c == '}')
			{
				if (sc.size())
					sc.pop();
				else
				 	throw std::runtime_error("a breaket that dont have peer '}'");
			}
			if (str.length())
				v_s.push_back(str);
			str = c;
			v_s.push_back(str);
			str = "";
		}
		else if (c == '#')
		{
			if (str.length())
			{
				v_s.push_back(str);
				str = "";
			}
			while (is.get(c))
			{
				if (c == '\n')
					break ;
			}
		}
		else if (ft_is_white_spaces(c) || c == '\n')
		{
			if (str.length())
			{
				v_s.push_back(str);
				str = "";
			}
		}
		else {
			str += c;
		}
	}
	if (sc.size())
	{
		throw std::runtime_error("A breaket doesn't have an end");
	}
	if (str.length())
		v_s.push_back(str);
	return (true);
}

t_config	ft_parce_config(const char *path_to_config)
{
	t_config		config;
	// std::vector<t_config>		v_config;
	std::ifstream	is( path_to_config );
	// size_t				i = 1;
	std::vector<std::string> v_s;

	if (!is.is_open())
	{
		throw std::runtime_error(std::string("failed to open the file : ") + path_to_config);
	}
	while (ft_getserver(v_s, is))
	{
		for (size_t i = 0; i < v_s.size(); i++)
		{
			std::cout << "the line : " << i << " " << v_s[i] << std::endl;
		}
		v_s.clear();
	}
	is.close();
	return (config);
}
