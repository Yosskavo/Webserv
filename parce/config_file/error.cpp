#include "../../webserv.h"

void ft_error_pages_get(std::vector< std::string >::iterator &it, std::vector<std::string>::iterator &end, t_config &t)
{
	std::vector<int>	v_error;
	std::string			s_error = "";
	size_t				tmp_error = 0;


	while (*it != ";")
	{
		if (!s_error.empty())
		{
			throw std::runtime_error("In error_page after the path it should have dilimeter");
		}
		if (ft_check_is_number(*it))
		{
			tmp_error = ft_to_number(*it);
			if (tmp_error < 300 || tmp_error >= 600)
				throw std::runtime_error("Invalide error code (the error code should be arround 300-599)");
			v_error.push_back(tmp_error);
		}
		else {
			s_error = *it;
		}
		it++;
		if (it == end)
			throw std::runtime_error("The variable should followed by a value and a dilimeter");
	}
	if (!v_error.size())
	{
		throw std::runtime_error("error_pages should contain a error numbers");
	}
	if (s_error.empty())
	{
		throw std::runtime_error("error_pages should have the path to the error display");
	}
	for (std::vector<int>::iterator iv = v_error.begin(); iv != v_error.end(); iv++)
	{
		t.error_pages[*iv] = s_error;
	}
}
