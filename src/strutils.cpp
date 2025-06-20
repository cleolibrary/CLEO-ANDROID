#include "strutils.h"

namespace strutils
{
	std::string ansi_from_wstr(uint16_t *w)
	{
		std::string res;
		while (*w)
		{
			res += static_cast<char>(*w);
			w++;
		}
		return res;
	}

	wide_string wstr_from_ansi(const char *a)
	{
		wide_string res;
		while (*a)
		{
			res += static_cast<char>(*a);
			a++;
		}
		return res;
	}

	void wstr_from_ansi(uint16_t *w, const char *a)
	{
		int i = 0;
		while (a[i])
		{
			w[i] = a[i];
			i++;
		}
		w[i] = 0;
	}

	std::string str_to_lower(std::string str)
	{
		for (int i = 0; i < (int)str.size(); i++)
			str[i] = tolower(str[i]);
		return str;
	}

	uint32_t str_hash(std::string astr)
	{
		const char *str = astr.c_str();
		uint32_t len = strlen(str);
		if (!len) return 0;
		uint32_t hash, i;
		for (hash = i = 0; i < len; ++i)
		{
			hash += str[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;
	}

	std::string str_replace(std::string subject, const std::string& search, const std::string& replace)
	{
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos)
		{
			 subject.replace(pos, search.length(), replace);
			 pos += replace.length();
		}
		return subject;
	}

	std::string replace_slashes(std::string str)
	{
		std::string res;
		char prev_char = 0;
		for (int i = 0; i < (int)str.size(); i++)
		{
			char c = str[i];
			if (c == '\\') c = '/';
			if (c == '/' && prev_char == '/')
				continue;
			res += (prev_char = c);
		}
		return res;
	}

	std::string path_normalize(std::string path, eTrailingSlashPolicy tsp)
	{
		if (path.size())
		{
			path = replace_slashes(path);
			if (path.size())
			{
				bool has_trailing_slash = path[path.size() - 1] == '/';
				switch (tsp)
				{
				case tspMake:
					if (!has_trailing_slash)
						path += '/';
					break;
				case tspClear:
					if (has_trailing_slash)
						path = path.substr(0, path.size() - 1);
					break;
				}
			}
		}
		return path;
	}

	std::string get_ext(std::string path, bool lower)
	{
		size_t pos = path.find_last_of(".");
		return pos != std::string::npos && pos < path.size() - 1 ?
				lower ? str_to_lower(path.substr(pos + 1)) : path.substr(pos + 1) :
				"";
	}
}