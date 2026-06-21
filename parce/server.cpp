#include "../webserv.h"

static bool	ft_is_white_spaces(char c)
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
