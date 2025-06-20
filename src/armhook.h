#pragma once

#include "common.h"

namespace armhook
{
#ifdef ANDROID

	// initialize
	void init(ptr nearSpace, uint32_t nearSize);
	// replace call
	void replace_thumb_call(ptr addr, ptr func_to);
	// templated proc
	template <typename T>
	void replace_thumb_call(uint32_t addr, T func_to)
	{
		replace_thumb_call(cast<ptr>(addr), cast<ptr>(func_to));
	}
	// common hook proc
	void hook_thumb_func(ptr func, uint32_t startSize, ptr func_to, ptr *func_orig);
	// templated proc
	template <typename T>
	void hook_thumb_func(T func, uint32_t startSize, T func_to, T *func_orig)
	{
		hook_thumb_func(cast<ptr>(func),
						startSize,
						cast<ptr>(func_to),
						cast<ptr *>(func_orig));
	}

	// replace call for arm
	void replace_arm_call(ptr addr, ptr func_to);
	// templated proc for arm
	template <typename T>
	void replace_arm_call(uint32_t addr, T func_to)
	{
		replace_arm_call(cast<ptr>(addr), cast<ptr>(func_to));
	}
	// common hook proc for arm
	void hook_arm_func(ptr func, uint32_t startSize, ptr func_to, ptr *func_orig);
	// templated proc for arm
	template <typename T>
	void hook_arm_func(T func, uint32_t startSize, T func_to, T *func_orig)
	{
		hook_arm_func(cast<ptr>(func),
					  startSize,
					  cast<ptr>(func_to),
					  cast<ptr *>(func_orig));
	}

#else

	// initialize
	void init();
	// replace call
	void replace_mips_call(ptr addr, ptr func_to);
	// templated proc
	template <typename T1, typename T2>
	void replace_mips_call(T1 addr, T2 func_to)
	{
		replace_mips_call(cast<ptr>(addr), cast<ptr>(func_to));
	}
	// common hook proc
	void hook_mips_func(ptr func, uint32_t startSize, ptr func_to, ptr *func_orig);
	// templated proc
	template <typename T>
	void hook_mips_func(T func, uint32_t startSize, T func_to, T *func_orig)
	{
		hook_mips_func(cast<ptr>(func),
					   startSize,
					   cast<ptr>(func_to),
					   cast<ptr *>(func_orig));
	}
	// common find func calls
	std::vector<ptr> find_mips_func_calls(ptr func);
	// templated find func calls
	template <typename T>
	std::vector<ptr> find_mips_func_calls(T func)
	{
		return find_mips_func_calls(cast<ptr>(func));
	}
	// common find func calls in another func
	std::vector<ptr> find_mips_func_calls_in_func(ptr func, ptr func_in);
	template <typename T1, typename T2>
	// templated find func calls in another func
	std::vector<ptr> find_mips_func_calls_in_func(T1 func, T2 func_in)
	{
		return find_mips_func_calls_in_func(cast<ptr>(func), cast<ptr>(func_in));
	}

#endif
}
