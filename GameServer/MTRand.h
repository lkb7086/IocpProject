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

	// mt19937의 경우 초당 500메가 분량의 난수를 생성
	// random_device는 초당 1.9메가 남짓의 난수를 생성
	// random_device는 mt19937보다 엄청 느리다

	// 일양분포
	static uint64_t GetRand_uint64(uint64_t minVal, uint64_t maxVal)
	{
		std::uniform_int_distribution<uint64_t> distribution(minVal, maxVal);
		return distribution(*tls_pEngine);
	}

	static float GetRand_float(float minVal, float maxVal)
	{
		std::uniform_real_distribution<float> distribution(minVal, maxVal); // 주의사항: 0.0이상, 1.0미만 범위
		return distribution(*tls_pEngine);
	}

	static double GetRand_double(double minVal, double maxVal)
	{
		std::uniform_real_distribution<double> distribution(minVal, maxVal);
		return distribution(*tls_pEngine);
	}
	//------------------------------------------------------------------------------

	// 정규 분포 (평균, 표준 편차)
	static double GetNormalDist(double average, double standard)
	{
		std::normal_distribution<double> distribution{ average, standard };
		return distribution(*tls_pEngine);
	}

	// 베르누이 분포, (확률(0.0 ~ 1.0)), 성공하면 true를 실패하면 false를 반환
	static bool GetBernoulliDist(float probability)
	{
		std::bernoulli_distribution distribution{ probability };
		return distribution(*tls_pEngine);
	}

	// 이항 분포, (시도 횟수, 확률(0.0 ~ 1.0)), 성공한 횟수를 반환
	static int GetBinomialDist(int number, double probability)
	{
		std::binomial_distribution<int> distribution{ number, probability };
		return distribution(*tls_pEngine);
	}

	// 포아송 분포
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