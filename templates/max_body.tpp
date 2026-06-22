template <typename T>
	void ft_get_max_body(std::string &s, T &t)
	{
		size_t	size;
		char	*c;
		short	p;

		size = std::strtoul(s.c_str(), &c, 10);
		if (*c == '\0')
			p = 0;
		else if (*c == 'K' || *c == 'k')
		{
			p = 1;
			if (size > MAX_KILO)
				throw std::runtime_error("A possible overflow could happen in client_max_body_size given variable");
		}
		else if (*c == 'M' || *c == 'm')
		{
			p = 2;
			if (size > MAX_MIGA)
				throw std::runtime_error("A possible overflow could happen in client_max_body_size given variable");
		}
		else if  (*c == 'G' || *c == 'g')
		{
			p = 3;
			if (size > MAX_GIGA)
				throw std::runtime_error("A possible overflow could happen in client_max_body_size given variable");
		}
		else
			throw std::runtime_error("Invalide character in max body");
		t.client_max_body_size = size * std::pow(2, p * 10);
	}
