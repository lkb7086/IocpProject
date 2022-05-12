#pragma once

LONG WINAPI my_top_level_filter(__in PEXCEPTION_POINTERS pExceptionPointer);
class CMinidumpHelp final
{
private:
	std::string format_arg_list(const char *fmt, va_list args);
	std::string format_string(const char *fmt, ...);
	std::wstring s2ws(const std::string& s);
	
public:
	std::wstring get_dump_filename();
	CMinidumpHelp();
	~CMinidumpHelp();
	
	void install_self_mini_dump();
};
