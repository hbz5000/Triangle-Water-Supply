#ifndef WaterUtility_h
#define WaterUtility_h

#include "global.h"
#include "TimeSeriesData.h"
#include "InsuranceFund.h"

class WaterUtility
{
	public:
	
	WaterUtility();
	~WaterUtility();
	
	string name;
	
	data_t UnitDemand;
	data_t FutureDemand;
	data_t FutureBreakdown;
	data_t usePatterns, useFractions, useFractions_Restrictions;
	data_t riskOfFailure;
	double *inflows1DIrr , *inflows1DNon, *demand1DIrr, *demand1DNon;
	double **inflows2DIrr, **inflows2DNon, **demand2DIrr, **demand2DNon;
	double **actualPDF;
	
	TimeSeriesData UD; // unit demand
	InsuranceFund Fund;
	
	// Objective Values to Minimize
	double restrictionFreq, maxFailures, freqLosses, totalCosts, totalLosses, consumerFractions;
	
	// Running sum of restrictions implemented each year
	double restrictions, averageRevenue, annualRevenue;
	
	// Sum of mitigation costs (to be added to total costs)
	double addFunds;
	
	// Restriction triggers on/off. Row: 0/1 for Irrigation/Non-Irrigation. Column: Restriction tier (0-stages) 
	double **restrictionsOnTemplate, **restrictionsOffTemplate;
	double *restrictionsOnTemplateROF, *restrictionsOffTemplateROF;
	
	// Transfer trigger decision variables
	double TTriggerI, TTriggerN;
	// Transfer trigger, translated into reservoir storage volume
	double TTrigger;
	
	// Storage amount (fraction of full)
	double storageFraction;
	
	// Restriction stage (0 means no restrictions)
	int restrictionStage, insuranceUse;
	double restrictionReduction;
	
	// Usage tier info. Cost per million gallons. Usage reductions during restrictions.
	double **reductions;
	double **costPerMG, **consumerSurplusPerMG;
	
	double *insurancePayment;
	double *restrictionsOn, *restrictionsOff;
	double **data2, **data3, **data31, **data32, **data4;
	double **fractionPricing;
	double *totalFailures;
	double **individualReductions;
	
	// Demand PDF (fixed number of discrete buckets)
	int **PDFIrr, **PDFNon, **CDFIrr, **CDFNon;
	
	int numMonths, numYears, numTypes, numTiers, numStages, numAnnualDecisionPeriods, numFutureYears, terminateYear, numRealizations;
	int *elasticityIndex;
	
	// Give user the option of a single price list, or both for irrigation/non-irrigation seasons
	double *waterPrice, *waterPrice_Irr, *waterPrice_NonIrr;
	double **droughtSurcharges;
	double **demandBaseline, **demandVariation;
	double **weeklyTransferTriggers;
	double averageUse;
	double futureDemand[51];
	double futureDemandBreakdown[5][51];
	
	double jordanLakeAlloc, annualPayment, insuranceCost, simLosses, maxLosses, totalConsumerLoss, totalUtilityLoss, failurePoint, freqLossesint;
	double yearlyFailure;
	vector <double> annualCosts;
	
	double weeklyDemand, restrictedDemand, consumerSurplusLoss, demandDeficit, weeklyTransferVolume;
	double ROF_res, ROF_resIns;
	double restrictionIncrement;
	double consumerCount;
	
	int week_of_last_restrictions;
	double insurancePremium;
	bool shouldAllocate;
	
	//~~~~~~~~~~~~~~~~ Public functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void calcWaterPrice(double elasticity_Of_Demand[]);
	void calcSurchargePrice(double elasticity_Of_Demand[], int constant);
	void configure(int nmonths, int nyears, int ntypes, int ntiers, int nstages, int nfutureyears, double failure, int nannualdecisionperiods, int terminateYear,
		int volumeIncrements, int numRealizations, int formulation);
	void clearVariablesForSimulation(int numRealizations, int terminateYear);
	void clearVariablesForRealization(int year);
	void fillRestrictionsArray(int season);
	int getPDFIndex(double inflow);
	void annualUpdate(int year, int numRealizations, int terminateYear);
	void weeklyUpdate(int numRealizations);
	void calculateObjectives(int terminateYear, int numRealizations);
	void writeInflowDemandPDF(int InflowR, int UDr, int rows_PDF, int cols_PDF, double size1, double size2, int irrC, int nonC1, int nonC2, TimeSeriesData* Inflows);
	void generateDemandVariation(int numRealizations, int numWeeks, int numYears, TimeSeriesData *Inflows, double demand_variation_multiplier);
	void calculateRestrictions(int year, int week, int numdays, int month, int realization);
	void calcTransferTriggers(int volumeIncrements, int terminateYear);
    void setCapacity(double cap);
	void setInsurancePayment(double demandBaseline, double inflows, int numRealizations, int terminateYear, int numdays, int week, int month, int year);
	void calculateDemand(int realizations, int week, int numdays);
	
	//~~~~~~~~~~~~~~~~ Private class members ~~~~~~~~~~~~~~~~~~~~~~~
	private:
	
	int currentPDFIndex;
	bool usesROF;
	
	bool anyThresholdExceeded(double threshold, double **ssq);
	bool tierIsResidential(int tier);
	void getCurrentUtilityPrices(int month, int tier, int stage, double &currentPrice, double &currentSewerPrice, double &currentSewerFraction, double &droughtMultiplier);
};

#endif
