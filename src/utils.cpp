#include "utils.h"
#include "mutex.h"

namespace utils
{
	uint32_t get_tick_count()
	{
	  struct timeval tv;
	  if (gettimeofday(&tv, NULL)) return 0;
	  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	}

	FILE *log_file = NULL;

	void log(const char *fmt, ...)
	{
		CS_SCOPE(mutex::mlog);

#ifdef ANDROID
		va_list lst;
		va_start(lst, fmt);
		__android_log_vprint(ANDROID_LOG_DEBUG, "AB", fmt, lst);
		va_end(lst);
#else
		char str[256];
		va_list lst;
		va_start(lst, fmt);
		vsprintf(str, fmt, lst);
		va_end(lst);
		printf(str);
		printf("\n");
		pspDebugScreenPrintf(str);
		pspDebugScreenPrintf("\n");
#endif
		if (!log_file)
		{
#ifdef ANDROID
			char str[1024];
			sprintf(str, "%s/cleo/cleo.log", getenv("EXTERNAL_STORAGE"));
#else
			char str[] = "ms0:/PSP/PLUGINS/cleo/cleo.log";
#endif
			remove(str);
			log_file = fopen(str, "wt");
		}
		if (log_file)
		{
			struct timeval tv;
			struct timezone tz;
			memset(&tv, 0, sizeof(tv));
			memset(&tz, 0, sizeof(tz));
			if (gettimeofday(&tv, &tz) == 0)
			{			
				tv.tv_sec -= tz.tz_minuteswest * 60;
				uint32_t h1 = tv.tv_sec / 3600;
				uint32_t h = h1 % 24;
				uint32_t m = (tv.tv_sec % 3600) / 60;
				uint32_t s = tv.tv_sec - h1 * 3600 - m * 60;
				fprintf(log_file, "[%s%02d:%02d:%02d] ", tz.tz_minuteswest ? "" : "UTC|", h, m, s);
			}
			va_list lst;
			va_start(lst, fmt);
			vfprintf(log_file, fmt, lst);
			va_end(lst);			
			fprintf(log_file, "\n");
			fflush(log_file);
		}
	}

	bool string_compare(const std::string &left, const std::string &right)
	{
	   for (std::string::const_iterator lit = left.begin(), rit = right.begin(); lit != left.end() && rit != right.end(); ++lit, ++rit)
	      if (tolower(*lit) < tolower(*rit))
	         return true;
	      else if (tolower(*lit) > tolower(*rit))
	         return false;
	   if (left.size() < right.size())
	      return true;
	   return false;
	}

	bool list_files_in_dir(std::string dir, std::vector<std::string> &files)
	{
#ifdef ANDROID
	    DIR *dp;
	    struct dirent *dirp;
	    if ((dp = opendir(dir.c_str())) == NULL) return false;
	    while ((dirp = readdir(dp)) != NULL)
	        files.push_back(std::string(dirp->d_name));
	    closedir(dp);
#else		
		int fd = sceIoDopen(dir.c_str());
		if (fd < 0) return false;
		SceIoDirent dirp;
		memset(&dirp, 0, sizeof(dirp));
		while (sceIoDread(fd, &dirp) > 0)
			if((dirp.d_stat.st_attr & FIO_SO_IFDIR) == 0)
				files.push_back(std::string(dirp.d_name));
		sceIoDclose(fd);
#endif
	    if (files.size())
	    	sort(files.begin(), files.end(), string_compare);
	    return true;
	}

	uint8_t *load_binary_file(std::string filename, uint32_t &size)
	{
#ifdef ANDROID
		FILE *file = fopen(filename.c_str(), "rb");
#else
		FILE *file = fopen(filename.c_str(), "r");
#endif
		if (!file) return NULL;
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);
		uint8_t *buf = cast<ptr>(malloc(size));
		fread(buf, 1, size, file);
		fclose(file);
		return buf;
	}

}


