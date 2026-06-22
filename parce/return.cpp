#include "../webserv.h"

void ft_return_code_get(std::vector< std::string >::iterator &it, t_location &t)
{
	std::vector<int>	v_return;
	std::string			s_return = "";
	int				tmp_return = 0;


	while (*it != ";")
	{
		if (!s_return.empty())
		{
			throw std::runtime_error("In return after the path it should have dilimeter");
		}
		if (ft_check_is_number(*it))
		{
			tmp_return = ft_to_number(*it);
			if (tmp_return < 0 || tmp_return > 999)
				throw std::runtime_error("Invalide return code (the return code should be arround 0-999)");
			v_return.push_back(tmp_return);
		}
		else {
			s_return = *it;
		}
		it++;
		if (*it == "}")
			throw std::runtime_error("The variable should followed by a value and a dilimeter");
	}
	if (!v_return.size())
	{
		throw std::runtime_error("return should contain a return numbers");
	}
	if (s_return.empty())
	{
		throw std::runtime_error("return should have the path to the return display");
	}
	for (std::vector<int>::iterator iv = v_return.begin(); iv != v_return.end(); iv++)
	{
		t.return_path[*iv] = s_return;
	}
}
