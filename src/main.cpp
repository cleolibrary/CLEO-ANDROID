#include "core.h"
#include "utils.h"

#define EXPORT __attribute__ ((visibility ("default")))

extern "C" {

	EXPORT char ver_date[] = VERSION_DATE;
	EXPORT uint32_t ver_code = VERSION_CODE;
	EXPORT char ver_code_str[] = VERSION_CODE_STR;

	EXPORT jint loader_JNI_OnLoad(JavaVM *aVm, void *aReserved)
	{
		#define SET_CHAR(buf, str, num) \
			buf[num] = str[num];	\
			if (!str[num]) break;

		#define  SET_10_CHARS(buf, str, from) \
			SET_CHAR(buf, str, from + 0) \
			SET_CHAR(buf, str, from + 1) \
			SET_CHAR(buf, str, from + 2) \
			SET_CHAR(buf, str, from + 3) \
			SET_CHAR(buf, str, from + 4) \
			SET_CHAR(buf, str, from + 5) \
			SET_CHAR(buf, str, from + 6) \
			SET_CHAR(buf, str, from + 7) \
			SET_CHAR(buf, str, from + 8) \
			SET_CHAR(buf, str, from + 9)

		#define	SET_STRING(buf, str) \
			for (;;) \
			{ \
				SET_10_CHARS(buf, str, 0)  \
				SET_10_CHARS(buf, str, 10) \
				SET_10_CHARS(buf, str, 20) \
				SET_10_CHARS(buf, str, 30) \
				SET_10_CHARS(buf, str, 40) \
				SET_10_CHARS(buf, str, 50) \
				SET_10_CHARS(buf, str, 60) \
			}

		#define DEF_STRING(name, str) \
			char name[strlen(str) + 8]; \
			SET_STRING(name, str);

		DEF_STRING(__java_io_FileInputStream, "java/io/FileInputStream");
		DEF_STRING(__init, "<init>");
		DEF_STRING(__java_lang_String__ret_void, "(Ljava/lang/String;)V");
		DEF_STRING(__proc_self_maps, "/proc/self/maps");
		DEF_STRING(__read, "read");
		DEF_STRING(__ret_int, "()I");
		DEF_STRING(__close, "close");
		DEF_STRING(__ret_void, "()V");
		DEF_STRING(__java_lang_System, "java/lang/System");
		DEF_STRING(__load, "load");

		JNIEnv *env;
		aVm->GetEnv((void **)&env, JNI_VERSION_1_4);
		// get lib path from maps
		jstring js_libname = NULL;
		// create stream
		jclass jcl_FileInputStream = env->FindClass(__java_io_FileInputStream);
		jmethodID jm_FileInputStream_new = env->GetMethodID(jcl_FileInputStream, __init, __java_lang_String__ret_void);
		jobject jobj_FileInputStream = env->functions->NewObject(env, jcl_FileInputStream, jm_FileInputStream_new, env->NewStringUTF(__proc_self_maps));
		// read file
		jmethodID jm_FileInputStream_read = env->GetMethodID(jcl_FileInputStream, __read, __ret_int);
		while (true)
		{
			char buf[1024];
			int offset = 0;
			// read one line
			bool read_end = false;
			while (true)
			{
				int byte = env->functions->CallIntMethod(env, jobj_FileInputStream, jm_FileInputStream_read);
				buf[offset] = (char)byte;
				if (byte == -1)
				{
					read_end = true;
					buf[offset] = 0;
					break;
				}
				if (buf[offset] == '\r' || buf[offset] == '\n')
				{
					buf[offset] = 0;
					break;
				}
				offset++;			
			}
			// check empty line and eof
			if (!offset)
			{
				if (read_end)
					break;
				else
					continue;
			}
			// get libname
			char libname[1024];
			bool libname_start = false;
			int libname_offset = 0;
			for (int i = 0; i < offset; i++)
			{
				if (buf[i] == '/')
					libname_start = true;
				if (libname_start && buf[i])
					libname[libname_offset++] = buf[i];
			}
			libname[libname_offset] = 0;
			// check libname
			if (libname_offset > 16)
			{
				bool bRs = false, bSo = false;
				// has "com.rockstar"
				for (int i = 0; i < libname_offset; i++)
					if (libname[i] == 'c' && libname[i+1] == 'o' && libname[i+2] == 'm' && libname[i+3] == '.' &&
						libname[i+4] == 'r' && libname[i+5] == 'o' && libname[i+6] == 'c' && libname[i+7] == 'k' &&
						libname[i+8] == 's' && libname[i+9] == 't' && libname[i+10] == 'a' && libname[i+11] == 'r')
					{
						bRs = true;
						break;
					}
				// ends with ".so"
				if (libname[libname_offset - 3] == '.' &&
					libname[libname_offset - 2] == 's' &&
					libname[libname_offset - 1] == 'o')
					bSo = true;

				if (bRs && bSo)
				{
					// utils::log("%s", libname);
					for (int i = libname_offset - 1; i >= 0; i--)
						if (libname[i] == '/')
						{
							libname[i+1] = 'l';	libname[i+2] = 'i';	libname[i+3] = 'b';
							libname[i+4] = 'c';	libname[i+5] = 'l';	libname[i+6] = 'e';	libname[i+7] = 'o';
							libname[i+8] = '.';	libname[i+9] = 's';	libname[i+10] = 'o';
							libname[i+11] = 0;
							break;
						}
					js_libname = env->NewStringUTF(libname);
					break;
				}
			}
		}
		//utils::log("libname %s", env->GetStringUTFChars(js_libname, &isCopy));
		// close stream
		jmethodID jm_FileInputStream_close = env->GetMethodID(jcl_FileInputStream, __close, __ret_void);
		env->functions->CallVoidMethod(env, jobj_FileInputStream, jm_FileInputStream_close);
		// load library
		if (js_libname)
		{
			jclass clazz = env->FindClass(__java_lang_System);
			jmethodID loadLibrary = env->GetStaticMethodID(clazz, __load, __java_lang_String__ret_void);
			env->functions->CallStaticVoidMethod(env, clazz, loadLibrary, js_libname);
		}
		// orig JNI_OnLoad
		extern void *loader_getOrigJniProcAddr();
		return ((jint (*)(JavaVM *, void *))loader_getOrigJniProcAddr())(aVm, aReserved);
	}

	EXPORT void *loader_getOrigJniProcAddr()
	{
		__asm
		("								;\
			mov		R0, PC				;\
			b 		.LRET				;\
			.word 	0xABCDEFAA			;\
			.LRET:						;\
			sub		R0, #4				;\
			push	{R1}				;\
			ldr		R1, [R0, #4]		;\
			add		R0, R1				;\
			pop		{R1}				;\
			bx		LR					;\
		");
	}

	EXPORT void loader_end()
	{
	}

	jint JNI_OnLoad(JavaVM *aVm, void *aReserved)
	{
		//JNIEnv *env;
		//aVm->GetEnv((void **)&env, JNI_VERSION_1_4);
		//utils::log("plugin");
		core::initialize();
		return JNI_VERSION_1_4;
	}
}