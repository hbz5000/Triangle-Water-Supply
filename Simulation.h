#ifndef Simulation_h
#define Simulation_h

#include "WaterUtility.h"
#include "TimeSeriesData.h"
#include "ReservoirStorage.h"
#include "Dates.h"

class Simulation
{
public:
	~Simulation();
	void importDataFiles();
	void writeDataLists();
	void initializeFormulation(int c_num_obj, int c_num_constr);
	void setNumDecisions(int c_num_dec);

	void setNumRealizations(int c_num_rel);
	void setNumYears(int c_terminateYear);
	
	void preconditionData(double unit_demand_multiplier, double future_demand_multiplier, bool firstTime);
	void realizationLoop();
	void calculation (double *c_xreal, double *c_obj, double *c_constr);
	void calculateWaterPrices();
	void calculateWaterSurcharges();
	
	void generateStreamflows();
	void correlateDemandVariations(double demand_variation_multiplier);
	void createRiskOfFailure(int real, int synthY, double durhAnnDemand, double owasaAnnDemand, double ralAnnDemand, double carAnnDemand);
	void createInfrastructureRisk(int real, int synthY, double durhAnnDemand, double owasaAnnDemand, double ralAnnDemand, double carAnnDemand);
	data_t parameterInput;
	
	WaterUtility durham, owasa, cary, raleigh;
	TimeSeriesData durhamInflows, owasaInflows, fallsInflows, wheelerInflows, crabtreeInflows, claytonInflows, jordanInflows, lillingtonInflows;
	TimeSeriesData durhamOwasaEvap, fallsEvap, wheelerEvap;
	
	int formulation;
	int borgToggle;
	int solutionNumber;
	
private:
	
	//class definitions
	Dates simDates;
	Dates riskOfFailureDates;
	ReservoirStorage systemStorage;
	ReservoirStorage riskOfFailureStorage;

	//for calc water price
	double sewerFractions[12];
	
	//objectives/decisions
	int num_dec, num_obj, num_constr;
	double *xreal;
	
	//data classes for file reading
	data_t littleRiverInflow;
	data_t michieInflow;
	data_t owasaInflow;
	data_t evaporation;
	data_t fallsLakeInflow, lakeWBInflow;
	data_t fallsLakeEvaporation, lakeWheelerEvaporation;
	data_t claytonInflow, crabtreeInflow;
	data_t jordanLakeInflow, lillingtonGaugeInflow;
	
	data_t caryFutureD, raleighFutureD, durhamFutureD, owasaFutureD;
	data_t caryFutureB, raleighFutureB, durhamFutureB, owasaFutureB;
	data_t durhamROF, owasaROF, raleighROF;
	data_t returnRatio;
	
	//durham variables
	double durhamFlowWeekBaseline;
	int durhamLevel, durhamLevelIns;
	
	//owasa variables
	double owasaFlowWeekBaseline;
	int owasaLevel, owasaLevelIns;

	//cary variables
	double caryAllocCapacity;
	double caryFlowWeekBaseline;
	int caryLevel, caryLevelIns;
	
	//raleigh variables
	double raleighFlowWeekBaseline, raleighFlowWeekBaseline2;
	int raleighLevel, raleighLevelIns;
	
	double durhamSpill, OWASASpill, insuranceFallsInflow, insuranceJordanInflow;
	
	//other variables
	int year, month, week, numdays;	
	int inflowC, inflowR;
	int billingMonths;		
	double elasticity_Of_Demand[5];
	int terminateYear;
	double transferCosts;
	int numRealizations;
	int numFutureYears;
	int volumeIncrements, costRiskLevel;
	
	double *anyFailure;
	double anyRestrictionFreq, anyFailureMax;
	
	ofstream out1;
	ofstream out3;
	
};
#endif
