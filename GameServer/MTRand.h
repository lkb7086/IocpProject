#pragma once
#include <random>
#include "ThreadLocal.h"

class CMTRand final
{
public:
	CMTRand() = delete;
	~CMTRand() = delete;
	CMTRand(const CMTRand&) = delete;
	CMTRand& operator=(const CMTRand&) = delete;
	CMTRand(CMTRand&&) = delete;
	CMTRand& operator=(CMTRand&&) = delete;

	// mt19937�� ��� �ʴ� 500�ް� �з��� ������ ����
	// random_device�� �ʴ� 1.9�ް� ������ ������ ����
	// random_device�� mt19937���� ��û ������

	// �Ͼ����
	static uint64_t GetRand_uint64(uint64_t minVal, uint64_t maxVal)
	{
		std::uniform_int_distribution<uint64_t> distribution(minVal, maxVal);
		return distribution(*tls_pEngine);
	}

	static float GetRand_float(float minVal, float maxVal)
	{
		std::uniform_real_distribution<float> distribution(minVal, maxVal); // ���ǻ���: 0.0�̻�, 1.0�̸� ����
		return distribution(*tls_pEngine);
	}

	static double GetRand_double(double minVal, double maxVal)
	{
		std::uniform_real_distribution<double> distribution(minVal, maxVal);
		return distribution(*tls_pEngine);
	}
	//------------------------------------------------------------------------------

	// ���� ���� (���, ǥ�� ����)
	static double GetNormalDist(double average, double standard)
	{
		std::normal_distribution<double> distribution{ average, standard };
		return distribution(*tls_pEngine);
	}

	// �������� ����, (Ȯ��(0.0 ~ 1.0)), �����ϸ� true�� �����ϸ� false�� ��ȯ
	static bool GetBernoulliDist(float probability)
	{
		std::bernoulli_distribution distribution{ probability };
		return distribution(*tls_pEngine);
	}

	// ���� ����, (�õ� Ƚ��, Ȯ��(0.0 ~ 1.0)), ������ Ƚ���� ��ȯ
	static int GetBinomialDist(int number, double probability)
	{
		std::binomial_distribution<int> distribution{ number, probability };
		return distribution(*tls_pEngine);
	}

	// ���Ƽ� ����
	static int GetPoissonDist(double average)
	{
		std::poisson_distribution<int> distribution{ average };
		return distribution(*tls_pEngine);
	}

private:
	//static std::random_device s_randDeivce;
	//static std::mt19937_64 s_engine;
};




/*
int g = 0;
for (register int i = 0; i < 1000000; i++)
{
bool b = CMTRand::GetBernoulliDist(0.5);

if (b)
g++;
}
cout << g << endl;
*/