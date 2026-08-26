#include "../server/server.hpp"

std::string	ft_trim(std::string s, char c)
{
	size_t	start_pos = 0;
	size_t	end_pos = s.length();

	while (s[start_pos] == c)
	{
		start_pos++;
	}
	if (start_pos == end_pos)
		return ("");
	do {
		end_pos--;
	} while (s[end_pos] == c);
	return (s.substr(start_pos, end_pos - start_pos + 1));
}
