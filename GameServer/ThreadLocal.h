#pragma once
#define thread_local __declspec(thread)

extern thread_local CSerializer* tls_pSer;
extern thread_local std::mt19937_64* tls_pEngine;