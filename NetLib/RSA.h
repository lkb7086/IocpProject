#pragma once

class CRSA
{
private:
	int p, q, n, t, flag, e[100], d[100], temp[100], j, m[100], en[100], i;
	char* msg;

public:
	CRSA(char* pMsg, int length);

	int Prime(int);
	void Ce();
	int Cd(int x);
	void Encrypt();
	void Decrypt();
};