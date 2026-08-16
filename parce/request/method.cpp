#include "../../server.hpp"

bool	ft_method(t_request & req, const std::string &tmp)
{
	size_t	pos;
	std::vector<std::string> tmp_vect;

	tmp_vect = ft_split(tmp, " ");
	if (tmp_vect.size() != 3)
		return (false);
	req.method = tmp_vect[0];
	req.version = tmp_vect[2];
	pos = tmp_vect[1].find("?");
	req.target = tmp_vect[1].substr(0, pos);
	if (pos != std::string::npos)
		req.query_string = tmp_vect[1].substr(pos + 1);
	return (true);
}
