#include "global.h"
#include "InsuranceFund.h"

using namespace std;

void InsuranceFund::initializeRealization()
{
	fundSize = 0;
	totalDebit = 0;
	numDebit = 0;
	percentLoss = 0;
	insuranceStorage = 1;
	insuranceStage = 0;
}
void InsuranceFund::subtract(double losses)
{
	fundSize -= losses;
}
void InsuranceFund::add(double payment)
{
	fundSize += payment;
}

int InsuranceFund::getInsuranceStage(double demandBaseline, double inflows, double* insuranceOn, double* insuranceOff,
										bool useInsRisk, double insRisk)
{
	insuranceStorage += (inflows - demandBaseline)/capacity;//update the estimated insurance storage

	if (insuranceStorage > 1)//limit storage between 0 and 100% of capacity
		insuranceStorage = 1;
	else if (insuranceStorage < 0)
		insuranceStorage = 0;

	// use the estimated insurance storage and triggers to determine the insurance payout in each week
	if(useInsRisk)
	{
		if (insuranceOn[insuranceStage] < insRisk)
			insuranceStage += 1;
		while (insuranceOff[insuranceStage] > insRisk)
			insuranceStage -= 1;
	}
	else
	{
		if (insuranceOn[insuranceStage] > insuranceStorage)
			insuranceStage += 1;
		while (insuranceOff[insuranceStage] < insuranceStorage)
			insuranceStage -= 1;
	}
	
	return insuranceStage;
}

void InsuranceFund::setInsurancePayment(double payment, int insuranceLevel)
{
	if(insuranceStage >= insuranceLevel)
		insurancePayment = payment;
	else
		insurancePayment = 0;
	
	fundSize += insurancePayment;
}

double InsuranceFund::calcAnnualLosses(double annualRevenue, bool addInterest)
{
	if (fundSize < 0)
	{
		numDebit++;
		annualLosses = fundSize*-1/annualRevenue;
		fundSize = 0;
	}
	else
		annualLosses = 0;
	
	if(addInterest)
		fundSize *= 1.05; // 5% annual fund growth
	
	return annualLosses;
}
