#include "StdAfx.h"

thread_local CSerializer* tls_pSer = nullptr;
thread_local std::mt19937_64* tls_pEngine = nullptr;