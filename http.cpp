#include "tcp.h"

void ft_parse_the_http_request(std::string & s)
{
	std::vector<std::string> v_s;
	std::string				tmp;
	std::vector<std::vector<std::string> > v_v_s;
	std::vector<std::string> v_s_e;
	size_t i = 0;
	bool flag = true;
	size_t j = 0;

	while (flag)
	{
		i = s.find("\r\n", i);
		if (i == std::string::npos)
		{
			flag = false;
			i = s.length();
		}
		tmp = s.substr(j, i - j);
		v_s_e.push_back(tmp);
		j = i += 2;
	}
	for (std::vector<std::string>::iterator it = v_s_e.begin(); it != v_s_e.end(); it++)
	{
		std::cout << "==> " << *it << std::endl;
	}
	std::cout << "-------------------" << std::endl;
	for (std::vector<std::string>::iterator it = v_s_e.begin(); it != v_s_e.end(); it++)
	{
		i = 0;
		j = 0;
		flag = true;
		std::cout << "|=> " << *it << std::endl;
		while (flag)
		{
			i = it->find(" ", i);
			if (i == std::string::npos)
			{
				i = it->length();
				flag = false;
			}
			tmp = it->substr(j, i - j);
			std::cout << "/==> " << tmp << std::endl;
			v_s.push_back(tmp);
			j = i += 1;
		}
		v_v_s.push_back(v_s);
		v_s.clear();
	}

	std::cout << "here 3" << std::endl;
	for (std::vector<std::vector<std::string> >::iterator it = v_v_s.begin(); it != v_v_s.end(); it++)
	{
		for (std::vector<std::string>::iterator ij = it->begin(); it->end() != ij; ij++)
		{
			std::cout << "--> " << *ij << std::endl;
		}
	}
}
