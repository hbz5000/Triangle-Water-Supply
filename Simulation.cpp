#include "global.h"
#include "Simulation.h"

using namespace std;

void Simulation::setNumRealizations(int c_num_rel)
{
	numRealizations = c_num_rel;
	return;
}

void Simulation::setNumYears(int c_terminateYear)
{
	terminateYear = c_terminateYear;
	return;
}

void Simulation::setNumDecisions(int c_num_dec)
{
	num_dec = c_num_dec;
	return;
}
void Simulation::initializeFormulation(int c_num_obj, int c_num_constr)
{
	num_obj = c_num_obj;
	num_constr = c_num_constr;
	return;
}

void Simulation::writeDataLists()
{
	billingMonths = 12;//used to create average water revenues from Durham billing data
	transferCosts = 3000.0/1000000.0;//Price of water transfers ($MM/MG)
	volumeIncrements = 20;
	numFutureYears = 51;
	general_1d_allocate(xreal, num_dec);//Decision variables

	durham.name = "Durham";
	raleigh.name = "Raleigh";
	cary.name = "Cary";
	owasa.name = "OWASA";
	
	general_1d_allocate(totalFallsFailure, terminateYear, 0.0);
	
	// Define Utility Info
	// (months, years, types, tiers, stages, numFutureYears, failurePoint, numAnnualDecisionPeriods)
	durham.configure(billingMonths, 3, 13, 10, 5, numFutureYears, 0.2, 2, terminateYear, volumeIncrements, numRealizations, formulation, 5); 
	cary.configure(billingMonths, 1, 9, 9, 5, numFutureYears, 0.2, 2, terminateYear, volumeIncrements, numRealizations, formulation, 0);
	owasa.configure(billingMonths, 1, 8, 8, 4, numFutureYears, 0.2, 1, terminateYear, volumeIncrements, numRealizations, formulation, 4);
	raleigh.configure(billingMonths, 1, 24, 24, 5, numFutureYears, 0.2, 1, terminateYear, volumeIncrements, numRealizations, formulation, 4);
	
	// Utility Water Prices
	// OWASA: 9 customer classes: 0-4, Residential volumetric tiers 1-5;
	//							  5, Irrigation only use;
	//							  6, Seasonally charged customers
	//							  7, Bulk water rate
	//							  8, Sewer charge
	
	// Irrigation Season (May - October)
	general_1d_allocate(owasa.waterPrice_Irr, 9, 0.0);
	owasa.waterPrice_Irr[0] = 2580;  owasa.waterPrice_Irr[1] = 6260;
	owasa.waterPrice_Irr[2] = 7680;	 owasa.waterPrice_Irr[3] = 10730;
	owasa.waterPrice_Irr[4] = 19400; owasa.waterPrice_Irr[5] = 8340;
	owasa.waterPrice_Irr[6] = 7750;	 owasa.waterPrice_Irr[7] = 5090;
	owasa.waterPrice_Irr[8] = 6350;
	
	// Non-irrigation Season (November - April)
	general_1d_allocate(owasa.waterPrice_NonIrr, 9, 0.0);
	owasa.waterPrice_NonIrr[0] = 2580;	owasa.waterPrice_NonIrr[1] = 6260;
	owasa.waterPrice_NonIrr[2] = 7680;	owasa.waterPrice_NonIrr[3] = 10730;
	owasa.waterPrice_NonIrr[4] = 19400;	owasa.waterPrice_NonIrr[5] = 8340;
	owasa.waterPrice_NonIrr[6] = 4080;	owasa.waterPrice_NonIrr[7] = 5090;
	owasa.waterPrice_NonIrr[8] = 6350;

	// Drought surcharge multipliers (currently in place)
	// Regular Use
	general_2d_allocate(owasa.droughtSurcharges, 9, 4, 0.0);
	
	owasa.droughtSurcharges[0][0] = 1;	owasa.droughtSurcharges[1][0] = 1;
	owasa.droughtSurcharges[2][0] = 1;	owasa.droughtSurcharges[3][0] = 1;
	owasa.droughtSurcharges[4][0] = 1;	owasa.droughtSurcharges[5][0] = 1;
	owasa.droughtSurcharges[6][0] = 1;	owasa.droughtSurcharges[7][0] = 1;
	owasa.droughtSurcharges[8][0] = 1;
	
	// Stage 1 Restrictions
	owasa.droughtSurcharges[0][1] = 1;    owasa.droughtSurcharges[1][1] = 1;
	owasa.droughtSurcharges[2][1] = 1.25; owasa.droughtSurcharges[3][1] = 1.5;
	owasa.droughtSurcharges[4][1] = 2;    owasa.droughtSurcharges[5][1] = 1.15;
	owasa.droughtSurcharges[6][1] = 1.15; owasa.droughtSurcharges[7][1] = 1.15;
	owasa.droughtSurcharges[8][1] = 1;
	
	// Stage 2 Restrictions
	owasa.droughtSurcharges[0][2] = 1;    owasa.droughtSurcharges[1][2] = 1.25;
	owasa.droughtSurcharges[2][2] = 1.5;  owasa.droughtSurcharges[3][2] = 2;
	owasa.droughtSurcharges[4][2] = 3;    owasa.droughtSurcharges[5][2] = 1.25;
	owasa.droughtSurcharges[6][2] = 1.25; owasa.droughtSurcharges[7][2] = 1.25;
	owasa.droughtSurcharges[8][2] = 1;
	
	// Stage 3 Restrictions
	owasa.droughtSurcharges[0][3] = 1;    owasa.droughtSurcharges[1][3] = 1.5;
	owasa.droughtSurcharges[2][3] = 2;    owasa.droughtSurcharges[3][3] = 3;
	owasa.droughtSurcharges[4][3] = 4;    owasa.droughtSurcharges[5][3] = 1.5;
	owasa.droughtSurcharges[6][3] = 1.5;  owasa.droughtSurcharges[7][3] = 1.5;
	owasa.droughtSurcharges[8][3] = 1;
			
			
	// Durham: 11 customer classes 		0-4, Volumetric tiers 1-5, residential consumers
	//								    5-9, Volumetric tiers 6-10, commercial/industrial consumers
	//								    10,	Volumetric sewer charges
	
	general_1d_allocate(durham.waterPrice, 11, 0.0);
	durham.waterPrice[0] = 2299.3;	durham.waterPrice[1] = 3462.3;	durham.waterPrice[2] = 3796.5;
	durham.waterPrice[3] = 4959.5;	durham.waterPrice[4] = 7432.6;	
	durham.waterPrice[5] = 2299.3;	durham.waterPrice[6] = 3462.3;	durham.waterPrice[7] = 3796.5;
	durham.waterPrice[8] = 4959.5;	durham.waterPrice[9] = 7432.6; durham.waterPrice[10] = 4384.7;
			
	// Cary: 9 customer classes 	0-3, Volumetric tiers 1-4, residential consumers
	//								4, Volumetric tier, multi-family consumers
	//								5, Volumetric tier, commercial/industrial consumers
	//								6-7, Irrigation volumetric tiers 1-2, residential consumers
	//								8, Irrigation volumetric tier, commercial/industrial consumers
	//								9,	Volumetric sewer charges
	
	general_1d_allocate(cary.waterPrice, 10, 0.0);
	cary.waterPrice[0] = 3600;	cary.waterPrice[1] = 4080;	cary.waterPrice[2] = 5790;
	cary.waterPrice[3] = 11290;	cary.waterPrice[4] = 4080; cary.waterPrice[5] = 4080;	
	cary.waterPrice[6] = 5790;	cary.waterPrice[7] = 11290;	cary.waterPrice[8] = 6380;
	cary.waterPrice[9] = 8470;
	
	// Raleigh: 25 customer classes 	0-2, volumetric tiers 1-5, inside city limits (Raleigh and Garner)
	//									3, non-residential rates, inside city limits (Raleigh and Garner)
	//									4, irrigation only rates, inside city limits (Raleigh and Garner)
	//									5, Rolesville, volumetric charge inside city limits
	//									6, Knightsdale, volumetric charge inside city limits
	//									7, Wake Forest, volumetric charge inside city limits
	//									8, Wendell, volumetric charge inside city limits
	//									9, Wendell irrigation, volumetric charge inside city limits
	//									10, Zebulon, volumetric charge inside city limits
	//									11, Zebulon reused water, volumetric charge inside city limits
	//									12-14, volumetric tiers 1-5, outside city limits (Raleigh and Garner)
	//									15, non-residential rates, outside city limits (Raleigh and Garner)
	//									16, irrigation only rates, outside city limits (Raleigh and Garner)
	//									17, Rolesville, volumetric charge outside city limits
	//									18, Knightsdale, volumetric charge outside city limits
	//									19, Wake Forest, volumetric charge outside city limits
	//									20, Wendell, volumetric charge outside city limits
	//									21, Wendell irrigation, volumetric charge outside city limits
	//									22, Zebulon, volumetric charge outside city limits
	//									23, Zebulon reused water, volumetric charge outside city limits
	//									24, Volumetric sewer charge
	
	general_1d_allocate(raleigh.waterPrice, 25, 0.0);
	raleigh.waterPrice[0] = 1705;	raleigh.waterPrice[1] = 2842;	raleigh.waterPrice[2] = 3792;
	raleigh.waterPrice[3] = 2206;	raleigh.waterPrice[4] = 3792;	raleigh.waterPrice[5] = 1900;
	raleigh.waterPrice[6] = 2902;	raleigh.waterPrice[7] = 3366;	raleigh.waterPrice[8] = 3546;
	raleigh.waterPrice[9] = 3546;	raleigh.waterPrice[10] = 3762;	raleigh.waterPrice[11] = 1676;
	raleigh.waterPrice[12] = 3411;	raleigh.waterPrice[13] = 5685;	raleigh.waterPrice[14] = 7585;
	raleigh.waterPrice[15] = 4413;	raleigh.waterPrice[16] = 7585;	raleigh.waterPrice[17] = 3628;
	raleigh.waterPrice[18] = 5804;	raleigh.waterPrice[19] = 8303;	raleigh.waterPrice[20] = 7091;
	raleigh.waterPrice[21] = 7091;	raleigh.waterPrice[22] = 7517;	raleigh.waterPrice[23] = 3441;
	raleigh.waterPrice[24] = 2177;
		
	// Fraction of total water use billed as sewer charges
	sewerFractions[0] = .89;	sewerFractions[1] = .91;
	sewerFractions[2] = .84;	sewerFractions[3] = .88;
	sewerFractions[4] = .82;	sewerFractions[5] = .75;
	sewerFractions[6] = .83;	sewerFractions[7] = .84;
	sewerFractions[8] = .87;	sewerFractions[9] = .89;
	sewerFractions[10] = .87;	sewerFractions[11] = .89;
	
	
	// Elasticity values for different customer classes
	// Type 0: Residential, low volume use
	// Type 1: Residential, mid volume use
	// Type 2: Residential, high volume use
	// Type 3: Consumer/Industrial, low volume use
	// Type 4: Consumer/Industrial, high volume use
	elasticity_Of_Demand[0] = -.11; elasticity_Of_Demand[1] = -.58;
	elasticity_Of_Demand[2] = -1.16; elasticity_Of_Demand[3] = -.43;
	elasticity_Of_Demand[4] = -.45;
	
	// Hardcoded elasticity index values for each tier
	durham.elasticityIndex[0] = 0; durham.elasticityIndex[1] = 1;
	durham.elasticityIndex[2] = 1; durham.elasticityIndex[3] = 2;
	durham.elasticityIndex[4] = 2; durham.elasticityIndex[5] = 3;
	durham.elasticityIndex[6] = 3; durham.elasticityIndex[7] = 3;
	durham.elasticityIndex[8] = 4; durham.elasticityIndex[9] = 4;
	
    owasa.elasticityIndex[0] = 0; owasa.elasticityIndex[1] = 1;
	owasa.elasticityIndex[2] = 1; owasa.elasticityIndex[3] = 2;
	owasa.elasticityIndex[4] = 2; owasa.elasticityIndex[5] = 3;
	owasa.elasticityIndex[6] = 3; owasa.elasticityIndex[7] = 4;
	
	cary.elasticityIndex[0] = 0; cary.elasticityIndex[1] = 1;
	cary.elasticityIndex[2] = 2; cary.elasticityIndex[3] = 2;
	cary.elasticityIndex[4] = 1; cary.elasticityIndex[5] = 3;
	cary.elasticityIndex[6] = 3; cary.elasticityIndex[7] = 4;
	cary.elasticityIndex[8] = 3;
	
	raleigh.elasticityIndex[0] = 0; raleigh.elasticityIndex[1] = 1;
	raleigh.elasticityIndex[2] = 2; raleigh.elasticityIndex[3] = 3;
	raleigh.elasticityIndex[4] = 3; raleigh.elasticityIndex[5] = 1;
	raleigh.elasticityIndex[6] = 2; raleigh.elasticityIndex[7] = 1;
	raleigh.elasticityIndex[8] = 1; raleigh.elasticityIndex[9] = 1;
	raleigh.elasticityIndex[10] = 1; raleigh.elasticityIndex[11] = 1;
	raleigh.elasticityIndex[12] = 0; raleigh.elasticityIndex[13] = 1;
	raleigh.elasticityIndex[14] = 2; raleigh.elasticityIndex[15] = 3;
	raleigh.elasticityIndex[16] = 3; raleigh.elasticityIndex[17] = 1;
	raleigh.elasticityIndex[18] = 2; raleigh.elasticityIndex[19] = 1;
	raleigh.elasticityIndex[20] = 1; raleigh.elasticityIndex[21] = 1;
	raleigh.elasticityIndex[22] = 1; raleigh.elasticityIndex[23] = 1;
	
	return;
}

void Simulation::importDataFiles()
{	
	//openFile(out1, "reservoirOutput1PSU.csv");
	//openFile(out3, "reservoirOutput.csv");
	//systemStorage.openResFiles();
	// Read in all input files
	string directoryName = "./inputfiles/";
	
	// Weekly inflow data
	// 81 Years: 1930 - 2010
	// 82 Years: 1926 - 2007
	// 83 Years: 1928 - 2010
	// 83 Years: 1929 - 2011 (little river raleigh)
	readFile(michieInflow, directoryName + "updatedMichieInflow.csv", 52, 82);
	readFile(littleRiverInflow, directoryName + "updatedLittleRiverInflow.csv", 52, 82);
	readFile(owasaInflow, directoryName + "updatedOWASAInflow.csv", 52, 82);
	readFile(fallsLakeInflow, directoryName + "updatedFallsLakeInflow.csv", 83, 52);
	readFile(lakeWBInflow, directoryName + "updatedLakeWBInflow.csv", 83, 52);
	readFile(claytonInflow, directoryName + "claytonGageInflow.csv", 83, 52);
	readFile(crabtreeInflow, directoryName + "crabtreeCreekInflow.csv", 83, 52);
	readFile(jordanLakeInflow, directoryName + "updatedJordanLakeInflow.csv", 81, 52);
	readFile(lillingtonGaugeInflow, directoryName + "updatedLillingtonInflow.csv", 81, 52);
	readFile(littleRiverRaleighInflow, directoryName + "updatedLittleRiverRaleighInflow.csv", 83, 52);
	
	// Evaporation data (MG/acre)
	// 82 years (1926 - 2007)
	// 83 years (1928 - 2010)
	readFile(evaporation, directoryName + "updatedEvap.csv", 52, 82); // for Durham and OWASA
	readFile(fallsLakeEvaporation, directoryName + "fallsLakeEvap.csv", 83, 52);
	readFile(lakeWheelerEvaporation, directoryName + "lakeWheelerBensonEvap.csv", 83, 52);

	// U files from Cholesky Factorization - combine with standardized inflows 
	// To make stochastic inflows for weeks 27-52 (1) and weeks 1-26 (2).
	// Each file is an upper triangular matrix, 52x52
	readFile(durhamInflows.stoch1, directoryName + "updatedDurhamStoch1.csv", 52, 52);	
	readFile(durhamInflows.stoch2, directoryName + "updatedDurhamStoch2.csv", 52, 52);
	readFile(owasaInflows.stoch1, directoryName + "updatedOwasaStoch1.csv",  52, 52);
	readFile(owasaInflows.stoch2, directoryName + "updatedOwasaStoch2.csv",  52, 52);
	readFile(fallsInflows.stoch1, directoryName + "updatedFallsStoch1.csv", 52, 52);
	readFile(fallsInflows.stoch2, directoryName + "updatedFallsStoch2.csv", 52, 52);
	readFile(wheelerInflows.stoch1, directoryName + "updatedWheelerStoch1.csv", 52, 52);
	readFile(wheelerInflows.stoch2, directoryName + "updatedWheelerStoch2.csv", 52, 52);
	readFile(crabtreeInflows.stoch1, directoryName + "updatedCrabtreeStoch1.csv", 52, 52);
	readFile(crabtreeInflows.stoch2, directoryName + "updatedCrabtreeStoch2.csv", 52, 52);
	readFile(claytonInflows.stoch1, directoryName + "updatedClaytonStoch1.csv", 52, 52);
	readFile(claytonInflows.stoch2, directoryName + "updatedClaytonStoch2.csv", 52, 52);
	readFile(jordanInflows.stoch1, directoryName + "updatedJordanStoch1.csv", 52, 52);
	readFile(jordanInflows.stoch2, directoryName + "updatedJordanStoch2.csv", 52, 52);
	readFile(lillingtonInflows.stoch1, directoryName + "updatedLillingtonStoch1.csv", 52, 52);
	readFile(lillingtonInflows.stoch2, directoryName + "updatedLillingtonStoch2.csv", 52, 52);
	readFile(littleRiverRaleighInflows.stoch1, directoryName + "updatedLittleRiverRaleighStoch1.csv", 52, 52);
	readFile(littleRiverRaleighInflows.stoch2, directoryName + "updatedLittleRiverRaleighStoch2.csv", 52, 52);
	
	// U file from Cholesky Factorization - combine with standardized evaporation
	// to make stochastic inflows for weeks 1 - 26 (2) and weeks 27 - 52 (1)
	// Upper triangular matrix, 52x52
	readFile(durhamOwasaEvap.stoch1, directoryName + "updatedEvapStoch1.csv", 52, 52);
	readFile(durhamOwasaEvap.stoch2, directoryName + "updatedEvapStoch2.csv", 52, 52);
	readFile(fallsEvap.stoch1, directoryName + "updatedfallsEvapStoch1.csv", 52, 52);
	readFile(fallsEvap.stoch2, directoryName + "updatedfallsEvapStoch2.csv", 52, 52);
	readFile(wheelerEvap.stoch1, directoryName + "updatedwheelerEvapStoch1.csv", 52, 52);
	readFile(wheelerEvap.stoch2, directoryName + "updatedwheelerEvapStoch2.csv", 52, 52);
	
	// 18 years (1990-2007) of weekly demand data (18 x 52)
	// Raleigh does not have enough data, so use Cary's instead
	readFile(durham.UnitDemand, directoryName + "updatedDurhamUnitDemand.csv", 52, 18); 
	readFile(owasa.UnitDemand, directoryName + "updatedOWASAUnitDemand.csv", 52, 18);
	readFile(cary.UnitDemand, directoryName + "updatedCaryUnitDemand.csv", 52, 18);
	readFile(raleigh.UnitDemand, directoryName + "updatedRaleighUnitDemand.csv", 52, 1);

	// Usage data (format varies depending on the utility)
	readFile(owasa.useFractions, directoryName + "updatedOWASARateFractions.csv", 12, 9);
	// Fraction of total use divided between 8 tiers (plus sewer charge as fractions of total water use
	// in the 9th column owasaRateFractions[months][rate tier] - 12x9
	
	readFile(owasa.useFractions_Restrictions, directoryName + "updatedOWASARestRateFractions.csv", 12, 9);
	// Fraction of total use divided between 8 tiers (plus sewer charge as fractions of total water use
	// in the 9th column - when restrictions are occuring

	readFile(durham.usePatterns, directoryName + "updatedDurhamUsePatterns.csv", 468, 6);
	// 3 years (2008-2010) of monthly durham tiered use data	
	// 13 total types: indoor, outdoor * (res., comm., indus., instit., multi fam., irr.) & hydrant
	// 12 monthly values for each list type, repeated for three years
	
	readFile(raleigh.useFractions, directoryName + "raleighRateFractions.csv", 12, 24);
	// Fraction of total use divided between 24 tiers (plus sewer charge as fractions of total water use
	// in the 25th column - when restrictions are occuring
	
	readFile(cary.usePatterns, directoryName + "caryUseFractions.csv", 12, 10);
	//Fraction of total use divided between 9 tiers (plus sewer charge as a fraction of total water use in column 10)

	// 51 years (2010 - 2060) of average daily water use projections
	readFile(caryFutureD, directoryName + "caryFutureDemand.csv", 1, 51);
	readFile(raleighFutureD, directoryName + "raleighFutureDemand.csv", 1, 51);
	readFile(durhamFutureD, directoryName + "durhamFutureDemand.csv", 1, 51);
	readFile(owasaFutureD, directoryName + "owasaFutureDemand.csv", 1, 51);

	// 51 years (2010-2060) of projections of the water use breakdown into 5
	// use types (residential, commercial, industrial, irrigation, and institutional)
	readFile(caryFutureB, directoryName + "caryFutureBreakdown.csv", 5, 51);
	readFile(raleighFutureB, directoryName + "raleighFutureBreakdown.csv", 5, 51);
	readFile(durhamFutureB, directoryName + "durhamFutureBreakdown.csv", 5, 51);
	readFile(owasaFutureB, directoryName + "owasaFutureBreakdown.csv", 5, 51);

	// Percent of water use returned by Durham (row 0) and Raleigh (row 1) as wastewater
	readFile(returnRatio, directoryName + "returnRatio.csv", 2, 52);
	

	// Correct owasaInflow to avoid taking the log of zero			
	for (unsigned int i = 0; i < owasaInflow.size(); i++)
	{
		for (unsigned int j = 0; j < owasaInflow[0].size(); j++)
		{
			if(owasaInflow[i][j] <= 0)
			{
				owasaInflow[i][j] = .01;
			}
		}
	}
	
	for (unsigned int i = 0; i < fallsLakeInflow.size(); i++)
	{
		for (unsigned int j = 0; j < fallsLakeInflow[0].size(); j++)
		{
			if(fallsLakeInflow[i][j] <= 0)
			{
				fallsLakeInflow[i][j] = .01;
			}
		}
	}
	for (unsigned int i = 0; i < lakeWBInflow.size(); i++)
	{
		for (unsigned int j = 0; j < lakeWBInflow[0].size(); j++)
		{
			if(lakeWBInflow[i][j] <= 0)
			{
				lakeWBInflow[i][j] = .01;
			}
		}
	}
	for (unsigned int i = 0; i < claytonInflow.size(); i++)
	{
		for (unsigned int j = 0; j < claytonInflow[0].size(); j++)
		{
			if(claytonInflow[i][j] <= 0)
			{
				claytonInflow[i][j] = .01;
			}
		}
	}
	for (unsigned int i = 0; i < crabtreeInflow.size(); i++)
	{
		for (unsigned int j = 0; j < crabtreeInflow[0].size(); j++)
		{
			if(crabtreeInflow[i][j] <= 0)
			{
				crabtreeInflow[i][j] = .01;
			}
		}
	}
	for (unsigned int i = 0; i < jordanLakeInflow.size(); i++)
	{
		for (unsigned int j = 0; j < jordanLakeInflow[0].size(); j++)
		{
			if(jordanLakeInflow[i][j] <= 0)
			{
				jordanLakeInflow[i][j] = .01;
			}
		}
	}
	for (unsigned int i = 0; i < lillingtonGaugeInflow.size(); i++)
	{
		for (unsigned int j = 0; j < lillingtonGaugeInflow[0].size(); j++)
		{
			if(lillingtonGaugeInflow[i][j] <= 0)
			{
				lillingtonGaugeInflow[i][j] = .01;
			}
		}
	}
}

void Simulation::preconditionData(double unit_demand_multiplier, double future_demand_multiplier, bool firstTime)
{
	int startYear = 0;
	numFutureYears = 51;
	
	durham.shouldAllocate = firstTime;
	owasa.shouldAllocate = firstTime;
	raleigh.shouldAllocate = firstTime;
	cary.shouldAllocate = firstTime;
	
	//this section allows for use of historic record of variable length
	for (int x = 0; x<numFutureYears; x++)
	{
		// Scaling a linear projection -- multiply the slope and subtract off changes to the y-intercept
		cary.futureDemand[x] = caryFutureD[0][x+startYear]*future_demand_multiplier - caryFutureD[0][0]*(future_demand_multiplier-1);
		raleigh.futureDemand[x] = raleighFutureD[0][x+startYear]*future_demand_multiplier - raleighFutureD[0][0]*(future_demand_multiplier-1);
		durham.futureDemand[x] = durhamFutureD[0][x+startYear]*future_demand_multiplier - durhamFutureD[0][0]*(future_demand_multiplier-1);
		owasa.futureDemand[x] = owasaFutureD[0][x+startYear]*future_demand_multiplier - owasaFutureD[0][0]*(future_demand_multiplier-1);

		for (int y = 0; y<5; y++)
		{
			cary.futureDemandBreakdown[y][x] = caryFutureB[y][x+startYear];
			raleigh.futureDemandBreakdown[y][x] = raleighFutureB[y][x+startYear];
			durham.futureDemandBreakdown[y][x] = durhamFutureB[y][x+startYear];
			owasa.futureDemandBreakdown[y][x] = owasaFutureB[y][x+startYear];
		}
	}
	
	// Weekly means and standard deviations are used to 'whiten' the historical data,
	// providing data sets where values are converted to 'standard deviations from the weekly mean'
	// for each week of the historical record
	/// Evaporation ///
	///////////////////
	int evapC = 52; // weeks per year in evaporation files
	int evapR = 78; // historical years of evaporation files
	
	if(firstTime)
	{
		durhamOwasaEvap.allocate(evapR, evapC, terminateYear*52, numRealizations);
		fallsEvap.allocate(evapR, evapC, terminateYear*52, numRealizations);
		wheelerEvap.allocate(evapR, evapC, terminateYear*52, numRealizations);
		
		//this section allows for use of historic record of variable length
		startYear = 0;
		
		for (int row = 0; row<evapR; ++row)
		{
			for (int column = 0; column<evapC; ++column)
			{
				durhamOwasaEvap.rawData[row][column] = evaporation[column][row+4+startYear];// evaporation data from this set starts 3 years ealier than the other two
				fallsEvap.rawData[row][column] = fallsLakeEvaporation[row+1+startYear][column];
				wheelerEvap.rawData[row][column] = lakeWheelerEvaporation[row+1+startYear][column];
			}
		}

		durhamOwasaEvap.calculateNormalizations(evapR, evapC);
		fallsEvap.calculateNormalizations(evapR, evapC);
		wheelerEvap.calculateNormalizations(evapR, evapC);
	}

	/// Demand ///
	int UDc = 52;//weeks per year of historical demand data
	int UDr = 18;//historical years of demand data
	
	if(firstTime)
	{
		durham.UD.allocate(UDr, UDc);
		cary.UD.allocate(UDr, UDc);
		owasa.UD.allocate(UDr, UDc);
		raleigh.UD.allocate(UDr, UDc);
	}
	
	//this section allows for use of historic record of variable length
	startYear = 0;
	
	for (int row = 0; row < UDr; row++)
	{		
		for (int column = 0; column < UDc; column++)
		{
			cary.UD.rawData[row][column] = (cary.UnitDemand[column][row+startYear] - 1)*unit_demand_multiplier + 1;
			owasa.UD.rawData[row][column] = (owasa.UnitDemand[column][row+startYear] - 1)*unit_demand_multiplier + 1;
			durham.UD.rawData[row][column] = (durham.UnitDemand[column][row+startYear] - 1)*unit_demand_multiplier + 1;
		}
	}
	
	//performs mean, standard deviation, and 'whitening' calculations on historical demand datasets
	cary.UD.calculateNormalizations(UDr, UDc);
	durham.UD.calculateNormalizations(UDr, UDc);
	owasa.UD.calculateNormalizations(UDr, UDc);
	
	//only one year of Raleigh demand data exists, Cary standard deviations are used in calcs.
	for (int column = 0; column < UDc; column++)
	{
		raleigh.UD.averages[column] = (raleigh.UnitDemand[column][0] - 1)*unit_demand_multiplier + 1; 
		raleigh.UD.standardDeviations[column] = cary.UD.standardDeviations[column];
	}
	
	/// Inflow ///
	inflowC = 52;//number of weeks in the inflow record
	inflowR = 78;//length of historical streamflow record
	
	if(firstTime)
	{
		durhamInflows.allocate(inflowR, inflowC, terminateYear*52, numRealizations);
		owasaInflows.allocate(inflowR, inflowC, terminateYear*52, numRealizations);
		fallsInflows.allocate(inflowR, inflowC, terminateYear*52, numRealizations);
		wheelerInflows.allocate(inflowR, inflowC, terminateYear*52, numRealizations);
		crabtreeInflows.allocate(inflowR, inflowC, terminateYear*52, numRealizations);
		claytonInflows.allocate(inflowR, inflowC, terminateYear*52, numRealizations);
		jordanInflows.allocate(inflowR, inflowC, terminateYear*52, numRealizations);
		lillingtonInflows.allocate(inflowR, inflowC, terminateYear*52, numRealizations);
		littleRiverRaleighInflows.allocate(inflowR, inflowC, terminateYear*52, numRealizations);

		//Fill out raw data, allowing for use of historic record of variable length
		startYear = 0;
		for (int row = 0; row < inflowR; row++)
		{
			for (int col = 0; col < inflowC; col++)
			{
				durhamInflows.rawData[row][col]		= log(michieInflow[col][row+4+startYear]+littleRiverInflow[col][row+4+startYear]);
				owasaInflows.rawData[row][col]		= log(owasaInflow[col][row+4+startYear]);
				fallsInflows.rawData[row][col]		= log(fallsLakeInflow[row+2+startYear][col]);
				wheelerInflows.rawData[row][col]	= log(lakeWBInflow[row+1+startYear][col]);
				claytonInflows.rawData[row][col]	= log(claytonInflow[row+1+startYear][col]);
				crabtreeInflows.rawData[row][col]	= log(crabtreeInflow[row+1+startYear][col]);
				jordanInflows.rawData[row][col]		= log(jordanLakeInflow[row+startYear][col]);
				lillingtonInflows.rawData[row][col]	= log(lillingtonGaugeInflow[row+startYear][col]);
				littleRiverRaleighInflows.rawData[row][col] = log(littleRiverRaleighInflow[row+1+startYear][col]);
			}
		}
		int streamflowStartYear = inflowR - UDr; 
		durhamInflows.calculateNormalizations(inflowR, inflowC, streamflowStartYear);
		owasaInflows.calculateNormalizations(inflowR, inflowC, streamflowStartYear);
		fallsInflows.calculateNormalizations(inflowR, inflowC, streamflowStartYear);
		wheelerInflows.calculateNormalizations(inflowR, inflowC, streamflowStartYear);
		claytonInflows.calculateNormalizations(inflowR, inflowC, streamflowStartYear);
		crabtreeInflows.calculateNormalizations(inflowR, inflowC, streamflowStartYear);
		jordanInflows.calculateNormalizations(inflowR, inflowC, streamflowStartYear);
		lillingtonInflows.calculateNormalizations(inflowR, inflowC, streamflowStartYear);
		littleRiverRaleighInflows.calculateNormalizations(inflowR, inflowC, streamflowStartYear);
	}
	
	// Creating joint probability density functions between the whitened demand and inflow data for each utility
	// PDF size is 16x16, with an extra numerical count as the last column
	int rows_PDF = 16;
	int cols_PDF = 17;
	double size1 = 16.0;
	double size2 = 16.0;
	int irrC = 23;
	int nonC1 = 16;
	int nonC2 = 13;
	
	durham.writeInflowDemandPDF(inflowR, UDr, rows_PDF, cols_PDF, size1, size2, irrC, nonC1, nonC2, &durhamInflows);
	cary.writeInflowDemandPDF(inflowR, UDr, rows_PDF, cols_PDF, size1, size2, irrC, nonC1, nonC2, &jordanInflows);
	owasa.writeInflowDemandPDF(inflowR, UDr, rows_PDF, cols_PDF, size1, size2, irrC, nonC1, nonC2, &owasaInflows);
	
	// Use Durham's inflow-demand PDF for Raleigh
	if(firstTime)
	{
		general_2d_allocate(raleigh.PDFIrr, rows_PDF, cols_PDF, 0);
		general_2d_allocate(raleigh.PDFNon, rows_PDF, cols_PDF, 0);
		general_2d_allocate(raleigh.CDFIrr, rows_PDF, cols_PDF, 0);
		general_2d_allocate(raleigh.CDFNon, rows_PDF, cols_PDF, 0);
	}
	
	for(int i = 0; i < rows_PDF; i++)
	{
		for(int j = 0; j < cols_PDF; j++)
		{
			raleigh.PDFIrr[i][j] = durham.PDFIrr[i][j];
			raleigh.CDFIrr[i][j] = durham.CDFIrr[i][j];
			raleigh.PDFNon[i][j] = durham.PDFNon[i][j];
			raleigh.CDFNon[i][j] = durham.CDFNon[i][j];
		}
	}
		
	// Set the weekly demand baselines before the main loop (saves time)
	int leapYearCounter = 0, daysInWeek = 7;
	for(int year = 0; year < terminateYear; year++)
	{
		for(int week = 0; week < 52; week++)
		{
			if(week == 51)
			{
				if (leapYearCounter == 2)
					daysInWeek = 9;
				else
					daysInWeek = 8;		
			}
			else
				daysInWeek = 7;
				
			if (week + 1 > 21 && week + 1< 47) // Release requirements for Durham and OWASA change at the same week
			{
				durham.demandBaseline[year][week] = 
					durham.UD.averages[week]*daysInWeek*durham.futureDemand[year] 
					+ durhamOwasaEvap.averages[week]*1069.0+9.05;
				
				owasa.demandBaseline[year][week] = 
					owasa.UD.averages[week]*daysInWeek*owasa.futureDemand[year] 
					+ durhamOwasaEvap.averages[week]*722.0+daysInWeek*1.797;
			}
			else
			{
				durham.demandBaseline[year][week] = 
					durham.UD.averages[week]*daysInWeek*durham.futureDemand[year] 
					+ durhamOwasaEvap.averages[week]*1069.0+daysInWeek*3.877;
				owasa.demandBaseline[year][week] = 
					owasa.UD.averages[week]*daysInWeek*owasa.futureDemand[year] 
					+ durhamOwasaEvap.averages[week]*722.0+daysInWeek*1.797;
			}

			if (week + 1 < 13 || week + 1 > 43) // Release requirement schedule for Raleigh is independent of Durham and OWASA (incomplete calculations
			{
				raleigh.demandBaseline[year][week] = raleigh.UD.averages[week]*daysInWeek*(raleigh.futureDemand[year] - .25);
			}
			else
			{
				raleigh.demandBaseline[year][week] = raleigh.UD.averages[week]*daysInWeek*(raleigh.futureDemand[year]-.25);
			}
			
			cary.demandBaseline[year][week] = cary.UD.averages[week]*daysInWeek*cary.futureDemand[year];
		}
		
		leapYearCounter += 1;
		
		if (leapYearCounter == 4)
			leapYearCounter=0;
	}

	return;
}
void Simulation::correlateDemandVariations(double demand_variation_multiplier)
{
	owasa.generateDemandVariation(52, &owasaInflows, demand_variation_multiplier);
	cary.generateDemandVariation(52, &jordanInflows, demand_variation_multiplier);
	raleigh.generateDemandVariation(52, &durhamInflows, demand_variation_multiplier);
	durham.generateDemandVariation(52, &durhamInflows, demand_variation_multiplier);
	
}
void Simulation::calculateWaterPrices()
{
	durham.calcWaterPrice(elasticity_Of_Demand);
	cary.calcWaterPrice(elasticity_Of_Demand);
	raleigh.calcWaterPrice(elasticity_Of_Demand);
	owasa.calcWaterPrice(elasticity_Of_Demand);
}
void Simulation::calculateWaterSurcharges()
{
	durham.calcSurchargePrice(elasticity_Of_Demand, 0);
	owasa.calcSurchargePrice(elasticity_Of_Demand, 0);
	cary.calcSurchargePrice(elasticity_Of_Demand, 0);
	raleigh.calcSurchargePrice(elasticity_Of_Demand, 0);
}

void Simulation::generateStreamflows()
{
	int **randomInflowNumber;
	general_2d_allocate(randomInflowNumber, numRealizations*(terminateYear+1), 52, 0);

	for (int x = 0; x<(numRealizations*(terminateYear+1)); x++)
	{
		for (int y = 0; y<52; y++)
		{
			randomInflowNumber[x][y] = rand() % inflowR;
		}
	}
	
	durhamInflows.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, true);
	owasaInflows.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, true);
	fallsInflows.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, true);
	wheelerInflows.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, true);
	claytonInflows.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, true);
	crabtreeInflows.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, true);
	jordanInflows.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, true);
	lillingtonInflows.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, true);
	littleRiverRaleighInflows.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, true);
	
	durhamOwasaEvap.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, false);
	fallsEvap.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, false);
	wheelerEvap.generateSynthetic(randomInflowNumber, terminateYear, numRealizations, false);

	zap(randomInflowNumber, numRealizations*(terminateYear+1));
	
	return;
}

void Simulation::calculation(double *c_xreal, double *c_obj, double *c_constr)
{	
	// Decision variable assignment
	if (borgToggle ==3)
	{
		for (int i = 0; i < num_dec; i++)
		{
			//xreal[i] = parameterInput[solutionNumber][i];
		}
		xreal[0] = 0.1;
		xreal[1] = 0.1;
		xreal[2] = 0.1;
		xreal[3] = 0.1;
		xreal[4] = 0.02;
		xreal[5] = 0.02;
		xreal[6] = 0.02;
		xreal[7] = 0.1;
		xreal[8] = 0.1;
		xreal[9] = 0.1;
		xreal[10] = 0.5;
		xreal[11] = 0.00;
		xreal[12] = 0.00;
		xreal[13] = 0.00;
		xreal[14] = 0.00;
		xreal[15] = 0.02;
		xreal[16] = 0.02;
		xreal[17] = 0.02;
		xreal[18] = 0.02;
		xreal[19] = .0;
		xreal[20] = .0;
		xreal[21] = .0;
		xreal[22] = .0;
		xreal[23] = .02;
		xreal[24] = .02;
		xreal[25] = .02;
		xreal[26] = .02;
		xreal[27] = .01;
		xreal[28] = .02;
		xreal[29] = .03;
		xreal[30] = .04;
		xreal[31] = .05;
		xreal[32] = .06;
		xreal[33] = .07;
		xreal[34] = .08;
		xreal[35] = .09;
		xreal[36] = .1;
		xreal[37] = .11;
		xreal[38] = .12;
		xreal[39] = .13;
		xreal[40] = .14;
		xreal[41] = .15;
		xreal[42] = .16;
		xreal[43] = .17;
		xreal[44] = .18;
		xreal[45] = .19;
		xreal[46] = 25.0;
		xreal[47] = 30.0;
		xreal[48] = 35.0;
		xreal[49] = .25;
		xreal[50] = .25;
		xreal[51] = .25;
		xreal[52] = 2000.0;
	}
	else
	{
		for (int i = 0; i < num_dec; i++)
		{
			xreal[i] = c_xreal[i];
		}
	}
			
		
	calculateWaterPrices();
	calculateWaterSurcharges();
	
	// Consumer reductions factor (0/1 is irrigation/non, and 0-4 is the restriction stage
	durham.individualReductions[0][0] = 1;
	durham.individualReductions[0][1] = .85;
	durham.individualReductions[0][2] = .7;
	durham.individualReductions[0][3] = .6;
	durham.individualReductions[0][4] = .45;
	durham.individualReductions[1][0] = 1;
	durham.individualReductions[1][1] = .93;
	durham.individualReductions[1][2] = .88;
	durham.individualReductions[1][3] = .83;
	durham.individualReductions[1][4] = .7;
	
	owasa.individualReductions[0][0] = 1;
	owasa.individualReductions[0][1] = .9;
	owasa.individualReductions[0][2] = .85;
	owasa.individualReductions[0][3] = .8;
	
	raleigh.individualReductions[0][0] = 1;
	raleigh.individualReductions[0][1] = .923;
	raleigh.individualReductions[0][2] = .84615;
	raleigh.individualReductions[0][3] = .53846;
	raleigh.individualReductions[0][4] = .384615;
	
	//Cary reductions by stage
	cary.individualReductions[0][0] = 1;
	cary.individualReductions[0][1] = .87;
	cary.individualReductions[0][2] = .68;
	cary.individualReductions[0][3] = .62;
	cary.individualReductions[0][4] = .54;
	cary.individualReductions[1][0] = 1;
	cary.individualReductions[1][1] = .94;
	cary.individualReductions[1][2] = .9;
	cary.individualReductions[1][3] = .83;
	cary.individualReductions[1][4] = .71;
	
	durham.insurancePremium = 1.2; 
	cary.insurancePremium = 1.2;
	raleigh.insurancePremium = 1.2;
	owasa.insurancePremium = 1.2;
	
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 5; j++) {
			if(durham.individualReductions[i][j] > 1) durham.individualReductions[i][j] = 1;
			if(cary.individualReductions[i][j] > 1) cary.individualReductions[i][j] = 1;
			
			if(i == 0 && raleigh.individualReductions[i][j] > 1) raleigh.individualReductions[i][j] = 1;
			if(i == 0 && j < 4 && owasa.individualReductions[i][j] > 1) owasa.individualReductions[i][j] = 1;
		}
	}	
	
	//Water use restrictions:  Durham and Raleigh have 4 stages, OWASA has three
	// 			Two decision variables per utility:  storage levels (Durham and Raleigh) or risk of failure (OWASA) which causes stage 1 restrictions
	//												 1 trigger for May-October (irrigation season), 1 trigger from November-April (non-irrigation season)
	//			Other stages of restrictions are triggered based on their relative value compared to stage 1 restrictions as currently practiced (stage 2 begins when storage
	//			is drawn down 20% farther than stage 1, etc).
	//			Triggers to turn off restricitons are different than the triggers to turn them on, and they also are based on their relative value compared to the triggers activating 
	// 			restrictions
	durham.restrictionsOnTemplate[0][0] = xreal[0];//Durham stage 1 restriction trigger, irrigation season
	durham.restrictionsOnTemplate[0][1] = xreal[0] - .2;//Durham stage 2 restriction trigger, irrigation season
	durham.restrictionsOnTemplate[0][2] = xreal[0] - .35;//Durham stage 3 restriction trigger, irrigation season
	durham.restrictionsOnTemplate[0][3] = xreal[0] - .45;//Durham stage 4 restriction trigger, irrigation season
	durham.restrictionsOnTemplate[0][4] = -1;
	durham.restrictionsOnTemplate[1][0] = xreal[1];//Durham stage 1 restriction trigger, non-irrigation season
	durham.restrictionsOnTemplate[1][1] = xreal[1] - .05;//Durham stage 2 restriction trigger, non-irrigation season
	durham.restrictionsOnTemplate[1][2] = xreal[1] - .1;//Durham stage 3 restriction trigger, non-irrigation season
	durham.restrictionsOnTemplate[1][3] = xreal[1] - .2;//Durham stage 1 restriction trigger, non-irrigation season
	durham.restrictionsOnTemplate[1][4] = -1;
	owasa.restrictionsOnTemplate[0][0] = xreal[2];//OWASA stage 1 restriction trigger (ROF), irrigation season
	owasa.restrictionsOnTemplate[0][1] = xreal[2] - .2;//OWASA stage 2 restriction trigger (ROF), irrigation season
	owasa.restrictionsOnTemplate[0][2] = xreal[2] - .35;//OWASA stage 3 restriction trigger (ROF), irrigation season
	owasa.restrictionsOnTemplate[0][3] = -1;
	owasa.restrictionsOnTemplate[1][0] = xreal[3];//OWASA stage 1 restriction trigger (ROF), non-irrigation season
	owasa.restrictionsOnTemplate[1][1] = xreal[3] - .05;//OWASA stage 2 restriction trigger (ROF), non-irrigation season
	owasa.restrictionsOnTemplate[1][2] = xreal[3] - .1;//OWASA stage 3 restriction trigger (ROF), non-irrigation season
	owasa.restrictionsOnTemplate[1][3] = -1;
	raleigh.restrictionsOnTemplate[0][0] = xreal[3];//Raleigh stage 1 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOnTemplate[0][1] = xreal[3] - .2;//Raleigh stage 2 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOnTemplate[0][2] = xreal[3] - .35;//Raleigh stage 3 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOnTemplate[0][3] = xreal[3] - .45;//Raleigh stage 4 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOnTemplate[0][4] = -1;
	raleigh.restrictionsOnTemplate[1][0] = xreal[4];//Raleigh stage 1 restriction trigger (ROF), non-irrigation season
	raleigh.restrictionsOnTemplate[1][1] = xreal[4] - .05;//Raleigh stage 1 restriction trigger (ROF), non-irrigation season
	raleigh.restrictionsOnTemplate[1][2] = xreal[4] - .1;//Raleigh stage 1 restriction trigger (ROF), non-irrigation season
	raleigh.restrictionsOnTemplate[1][3] = xreal[4] - 0.2;//Raleigh stage 1 restriction trigger (ROF), non-irrigation season
	raleigh.restrictionsOnTemplate[1][4] = -1;
	cary.restrictionsOnTemplate[0][0] = xreal[5];//Cary stage 1 restriction trigger, irrigation season
	cary.restrictionsOnTemplate[0][1] = xreal[5] - .2;//Cary stage 2 restriction trigger, irrigation season
	cary.restrictionsOnTemplate[0][2] = xreal[5] - .35;//Cary stage 3 restriction trigger, irrigation season
	cary.restrictionsOnTemplate[0][3] = xreal[5] - .45;//Cary stage 4 restriction trigger, irrigation season
	cary.restrictionsOnTemplate[0][4] = -1;
	cary.restrictionsOnTemplate[1][0] = xreal[6];//Cary stage 1 restriction trigger, non-irrigation season
	cary.restrictionsOnTemplate[1][1] = xreal[6] - .05;//Cary stage 2 restriction trigger, non-irrigation season
	cary.restrictionsOnTemplate[1][2] = xreal[6] - .1;//Cary stage 3 restriction trigger, non-irrigation season
	cary.restrictionsOnTemplate[1][3] = xreal[6] - .2;//Cary stage 1 restriction trigger, non-irrigation season
	cary.restrictionsOnTemplate[1][4] = -1;
	
	durham.restrictionsOnTemplateROF[0] = xreal[0];//Durham stage 1 restriction trigger, irrigation season
	durham.restrictionsOnTemplateROF[1] = xreal[0] +.15;//Durham stage 2 restriction trigger, irrigation season
	durham.restrictionsOnTemplateROF[2] = xreal[0] +.35;//Durham stage 3 restriction trigger, irrigation season
	durham.restrictionsOnTemplateROF[3] = xreal[0] +.6;//Durham stage 4 restriction trigger, irrigation season
	durham.restrictionsOnTemplateROF[4] = 1.1;
	owasa.restrictionsOnTemplateROF[0] = xreal[1];//OWASA stage 1 restriction trigger (ROF), irrigation season
	owasa.restrictionsOnTemplateROF[1] = xreal[1] +.15;//OWASA stage 2 restriction trigger (ROF), irrigation season
	owasa.restrictionsOnTemplateROF[2] = xreal[1] +.35;//OWASA stage 3 restriction trigger (ROF), irrigation season
	owasa.restrictionsOnTemplateROF[3] = 1.1;
	raleigh.restrictionsOnTemplateROF[0] = xreal[2];//Raleigh stage 1 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOnTemplateROF[1] = xreal[2] +.15;//Raleigh stage 2 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOnTemplateROF[2] = xreal[2] +.35;//Raleigh stage 3 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOnTemplateROF[3] = xreal[2] +.6;//Raleigh stage 4 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOnTemplateROF[4] = 1.1;
	cary.restrictionsOnTemplateROF[0] = xreal[3];//Cary stage 1 restriction trigger, irrigation season
	cary.restrictionsOnTemplateROF[1] = xreal[3] +.15;//Cary stage 2 restriction trigger, irrigation season
	cary.restrictionsOnTemplateROF[2] = xreal[3] +.35;//Cary stage 3 restriction trigger, irrigation season
	cary.restrictionsOnTemplateROF[3] = xreal[3] +.6;//Cary stage 4 restriction trigger, irrigation season
	cary.restrictionsOnTemplateROF[4] = 1.1;
	
	
	for (int x = 0; x < 2;x++)//Make sure that triggers do not take values outside of [0, 1] (relationships are additive)
	{
		for(int y = 0; y < 4; y++)
		{
			if(durham.restrictionsOnTemplate[x][y] < 0)
				durham.restrictionsOnTemplate[x][y] = 0;
			if(cary.restrictionsOnTemplate[x][y] < 0)
				cary.restrictionsOnTemplate[x][y] = 0;
			if(raleigh.restrictionsOnTemplate[x][y] < 0)
				raleigh.restrictionsOnTemplate[x][y] = 0;
		}
		for(int y = 0; y < 3; y++)
		{
			if(owasa.restrictionsOnTemplate[x][y] < 0)
				owasa.restrictionsOnTemplate[x][y] = 0;
		}
	}
	for(int y = 0; y < 4; y++)
	{
		if(durham.restrictionsOnTemplateROF[y] > 1)
			durham.restrictionsOnTemplateROF[y] = 1;
		if(cary.restrictionsOnTemplateROF[y] > 1)
			cary.restrictionsOnTemplateROF[y] = 1;
		if(raleigh.restrictionsOnTemplateROF[y] > 1)
			raleigh.restrictionsOnTemplateROF[y] = 1;
	}
	for(int y = 0; y < 3; y++)
	{
		if(owasa.restrictionsOnTemplateROF[y] > 1)
			owasa.restrictionsOnTemplateROF[y] = 1;
	}
	durham.restrictionsOffTemplate[0][1] = .95;//Durham stage 1 restriction trigger off, irrigation season
	durham.restrictionsOffTemplate[0][2] = durham.restrictionsOnTemplate[0][2]+.05;//Durham stage 2 restriction trigger off, irrigation season
	durham.restrictionsOffTemplate[0][3] = durham.restrictionsOnTemplate[0][3]+.05;//Durham stage 3 restriction trigger off, irrigation season
	durham.restrictionsOffTemplate[0][4] = durham.restrictionsOnTemplate[0][4]+.1;//Durham stage 4 restriction trigger off, irrigation season
	durham.restrictionsOffTemplate[0][0] = 1.1;
	durham.restrictionsOffTemplate[1][1] = .95;//Durham stage 1 restriction trigger off, non-irrigation season
	durham.restrictionsOffTemplate[1][2] = durham.restrictionsOnTemplate[1][2]+.2;//Durham stage 2 restriction trigger off, non-irrigation season
	durham.restrictionsOffTemplate[1][3] = durham.restrictionsOnTemplate[1][3]+.15;//Durham stage 3 restriction trigger off, non-irrigation season
	durham.restrictionsOffTemplate[1][4] = durham.restrictionsOnTemplate[1][4]+.1;//Durham stage 4 restriction trigger off, non-irrigation season
	durham.restrictionsOffTemplate[1][0] = 1.1;
	owasa.restrictionsOffTemplate[0][1] = .95;//OWASA stage 1 restriction trigger off, irrigation season
	owasa.restrictionsOffTemplate[0][2] = owasa.restrictionsOnTemplate[0][2]+.05;//OWASA stage 2 restriction trigger off, irrigation season
	owasa.restrictionsOffTemplate[0][3] = owasa.restrictionsOnTemplate[0][3]+.05;//OWASA stage 3 restriction trigger off, irrigation season
	owasa.restrictionsOffTemplate[0][0] = 1.1;
	owasa.restrictionsOffTemplate[1][1] = .95;//OWASA stage 1 restriction trigger off, non-irrigation season
	owasa.restrictionsOffTemplate[1][2] = owasa.restrictionsOnTemplate[1][2]+.05;//OWASA stage 2 restriction trigger off, non-irrigation season
	owasa.restrictionsOffTemplate[1][3] = owasa.restrictionsOnTemplate[1][3]+.05;//OWASA stage 3 restriction trigger off, non-irrigation season
	owasa.restrictionsOffTemplate[1][0] = 1.1;
	raleigh.restrictionsOffTemplate[0][1] = .95;//Raleigh stage 1 restriction trigger off, irrigation season
	raleigh.restrictionsOffTemplate[0][2] = raleigh.restrictionsOnTemplate[0][1]+.05;//Raleigh stage 2 restriction trigger off, irrigation season
	raleigh.restrictionsOffTemplate[0][3] = raleigh.restrictionsOnTemplate[0][2]+.05;
	raleigh.restrictionsOffTemplate[0][4] = raleigh.restrictionsOnTemplate[0][3]+.1;
	raleigh.restrictionsOffTemplate[0][0] = 1.1;
	raleigh.restrictionsOffTemplate[1][1] = 95;//Raleigh stage 1 restriction trigger off, non-irrigation season
	raleigh.restrictionsOffTemplate[1][2] = raleigh.restrictionsOnTemplate[1][1]+.2;//Raleigh stage 2 restriction trigger off, non-irrigation season
	raleigh.restrictionsOffTemplate[1][3] = raleigh.restrictionsOnTemplate[1][2]+.15;//Raleigh stage 3 restriction trigger off, non-irrigation season
	raleigh.restrictionsOffTemplate[1][4] = raleigh.restrictionsOnTemplate[1][3]+.1;//Raleigh stage 4 restriction trigger off, non-irrigation season
	raleigh.restrictionsOffTemplate[1][0] = 1.1;
	cary.restrictionsOffTemplate[0][1] = .95;//Cary stage 1 restriction trigger off, irrigation season
	cary.restrictionsOffTemplate[0][2] = cary.restrictionsOnTemplate[0][2]+.05;//Cary stage 2 restriction trigger off, irrigation season
	cary.restrictionsOffTemplate[0][3] = cary.restrictionsOnTemplate[0][3]+.05;//Cary stage 3 restriction trigger off, irrigation season
	cary.restrictionsOffTemplate[0][4] = cary.restrictionsOnTemplate[0][4]+.1;//Cary stage 4 restriction trigger off, irrigation season
	cary.restrictionsOffTemplate[0][0] = 1.1;
	cary.restrictionsOffTemplate[1][1] = .95;//Cary stage 1 restriction trigger off, non-irrigation season
	cary.restrictionsOffTemplate[1][2] = cary.restrictionsOnTemplate[1][2]+.2;//Cary stage 2 restriction trigger off, non-irrigation season
	cary.restrictionsOffTemplate[1][3] = cary.restrictionsOnTemplate[1][3]+.15;//Cary stage 3 restriction trigger off, non-irrigation season
	cary.restrictionsOffTemplate[1][4] = cary.restrictionsOnTemplate[1][4]+.1;//Cary stage 4 restriction trigger off, non-irrigation season
	cary.restrictionsOffTemplate[1][0] = 1.1;
	
	durham.restrictionsOffTemplateROF[1] = .02;//Durham stage 1 restriction trigger, irrigation season
	durham.restrictionsOffTemplateROF[2] = durham.restrictionsOnTemplateROF[0];//Durham stage 2 restriction trigger, irrigation season
	durham.restrictionsOffTemplateROF[3] = durham.restrictionsOnTemplateROF[1];//Durham stage 3 restriction trigger, irrigation season
	durham.restrictionsOffTemplateROF[4] = durham.restrictionsOnTemplateROF[2];//Durham stage 4 restriction trigger, irrigation season
	durham.restrictionsOffTemplateROF[0] = -1;
	owasa.restrictionsOffTemplateROF[1] = .02;//OWASA stage 1 restriction trigger (ROF), irrigation season
	owasa.restrictionsOffTemplateROF[2] = owasa.restrictionsOnTemplateROF[0];//OWASA stage 2 restriction trigger (ROF), irrigation season
	owasa.restrictionsOffTemplateROF[3] = owasa.restrictionsOnTemplateROF[1];//OWASA stage 3 restriction trigger (ROF), irrigation season
	owasa.restrictionsOffTemplateROF[0] = -1;
	raleigh.restrictionsOffTemplateROF[1] = .02;//Raleigh stage 1 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOffTemplateROF[2] = raleigh.restrictionsOnTemplateROF[0];//Raleigh stage 2 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOffTemplateROF[3] = raleigh.restrictionsOnTemplateROF[1];//Raleigh stage 3 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOffTemplateROF[4] = raleigh.restrictionsOffTemplateROF[2];//Raleigh stage 4 restriction trigger (ROF), irrigation season
	raleigh.restrictionsOffTemplateROF[0] = -1;
	cary.restrictionsOffTemplateROF[1] = .02;//Cary stage 1 restriction trigger, irrigation season
	cary.restrictionsOffTemplateROF[2] = cary.restrictionsOnTemplateROF[0];//Cary stage 2 restriction trigger, irrigation season
	cary.restrictionsOffTemplateROF[3] = cary.restrictionsOnTemplateROF[1];//Cary stage 3 restriction trigger, irrigation season
	cary.restrictionsOffTemplateROF[4] = cary.restrictionsOnTemplateROF[2];//Cary stage 4 restriction trigger, irrigation season
	cary.restrictionsOffTemplateROF[0] = -1;
	
	
	for (int x = 0; x < 2;x++)//Make sure that triggers do not take out-of-range values (relationships are additive)
	{
		for(int y = 1; y < 5; y++)
		{
			if(durham.restrictionsOffTemplate[x][y] > 1)
				durham.restrictionsOffTemplate[x][y] = 1;
			if(cary.restrictionsOffTemplate[x][y] > 1)
				cary.restrictionsOffTemplate[x][y] = 1;
			if(raleigh.restrictionsOffTemplate[x][y] > 1)
				raleigh.restrictionsOffTemplate[x][y] = 1;
			if(y < 4 && owasa.restrictionsOffTemplate[x][y] > 1)
				owasa.restrictionsOffTemplate[x][y] = 1;
		}
	}
	for(int y = 1; y < 5; y++)
	{
		if(durham.restrictionsOffTemplateROF[y] < 0)
			durham.restrictionsOffTemplateROF[y] = 0;
		if(cary.restrictionsOffTemplateROF[y] < 0)
			cary.restrictionsOffTemplateROF[y] = 0;
		if(raleigh.restrictionsOffTemplateROF[y] < 0)
			raleigh.restrictionsOffTemplateROF[y] = 0;
		if(y < 4 && owasa.restrictionsOffTemplateROF[y] < 0)
			owasa.restrictionsOffTemplateROF[y] = 0;
	}
	
	//Water transfer triggers - Risk of failure, one value per utility
	if (formulation == 0)//No transfers, triggers are set to 200% ROF
	{
		durham.TTriggerN = 2;
		durham.TTriggerI = 2;
		owasa.TTriggerN = 2;
		owasa.TTriggerI = 2;
		raleigh.TTriggerI = 2;
		raleigh.TTriggerN = 2;
		durham.jordanLakeAlloc = 0;
		raleigh.jordanLakeAlloc = 0;
		owasa.jordanLakeAlloc = 0;
		cary.jordanLakeAlloc = .32;
	}
	else
	{
		durham.TTriggerN = xreal[4];
		durham.TTriggerI = xreal[4];
		owasa.TTriggerN = xreal[5];
		owasa.TTriggerI = xreal[5];
		raleigh.TTriggerI = xreal[6];
		raleigh.TTriggerN = xreal[6];
		durham.jordanLakeAlloc = xreal[7];
		raleigh.jordanLakeAlloc = xreal[8];
		owasa.jordanLakeAlloc = xreal[9];
		cary.jordanLakeAlloc = xreal[10];
	}

	//Mitigation variables
	//Annual payment - size of the annual contribution to a contingency fund, as a percentage of expected total annual volumetric revenue
	durham.annualPayment = xreal[11];
	owasa.annualPayment = xreal[12];
	raleigh.annualPayment = xreal[13];
	cary.annualPayment = xreal[14];
	
	//Insurance purchases - what stage of restrictions should be covered by insurance from 1 (all) to 5 (none)
	durham.insuranceUse = xreal[15];
	owasa.insuranceUse = xreal[16];
	raleigh.insuranceUse = xreal[17];
	cary.insuranceUse = xreal[18];
	
	durham.insurancePayment = xreal[19];
	owasa.insurancePayment = xreal[20];
	raleigh.insurancePayment = xreal[21];
	cary.insurancePayment = xreal[22];
	
	
	//Infrastructure triggers - what forward-risk-of-failure threshold is used to trigger new infrastructure?
	
	durham.infTrigger = xreal[23];
	owasa.infTrigger = xreal[24];
	raleigh.infTrigger = xreal[25];
	cary.infTrigger = xreal[26];
	
	////Infrastructure rankings - what order should new infrastructure be built once it is triggered?
	///OWASA infrastructure
	owasa.infMatrix[0][0] = xreal[27];////University Lake expansion
	owasa.infMatrix[0][2] = 17;
	owasa.infMatrix[0][3] = 107.0;
	owasa.infMatrix[1][0] = xreal[28];////Cane Creek expansion
	owasa.infMatrix[1][2] = 17;
	owasa.infMatrix[1][4] = 127.0;
	owasa.infMatrix[2][0] = xreal[29];////Quarry reservoir expansion (shallow)
	owasa.infMatrix[2][2] = 23;
	owasa.infMatrix[2][4] = 1.4;
	owasa.infMatrix[3][0] = xreal[30];////Quarry reservoir expansion (deep)
	owasa.infMatrix[3][2] = 23;
	owasa.infMatrix[3][4] = 1.4;
	///Durham infrastructure
	durham.infMatrix[0][0] = xreal[31];////Teer quarry expansion
	durham.infMatrix[0][2] = 7;
	durham.infMatrix[0][4] = 22.6;
	durham.infMatrix[1][0] = xreal[32];////Reclaimed water (low)
	durham.infMatrix[1][2] = 7;
	durham.infMatrix[1][4] = 27.5;
	durham.infMatrix[2][0] = xreal[33];////Reclaimed water (high)
	durham.infMatrix[2][2] = 17;
	durham.infMatrix[2][4] = 104.4;
	durham.infMatrix[3][0] = xreal[34];////Lake Michie expansion (low)
	durham.infMatrix[3][2] = 17;
	durham.infMatrix[3][4] = 158.3;
	durham.infMatrix[4][0] = xreal[35];////Lake Michie expansion (high)
	durham.infMatrix[4][2] = 17;
	durham.infMatrix[4][4] = 203.3;
	///Raleigh infrastructure
	raleigh.infMatrix[0][0] = xreal[36];////Little River Reservoir
	raleigh.infMatrix[0][2] = 17;
	raleigh.infMatrix[0][4] = 263.0;
	raleigh.infMatrix[1][0] = xreal[37];////Richland Creek Quarry
	raleigh.infMatrix[1][2] = 17;
	raleigh.infMatrix[1][4] = 265.0;
	raleigh.infMatrix[2][0] = xreal[38];////Neuse River Intake
	raleigh.infMatrix[2][2] = 17;
	raleigh.infMatrix[2][4] = 225.5;
	raleigh.infMatrix[3][0] = xreal[39];////Reallocate Falls Lake
	raleigh.infMatrix[3][2] = 12;
	raleigh.infMatrix[3][4] = 68.2;
	
	if(formulation == 2)
	{
	/////All utilities have the potential to trigger the Western Wake Treatment plant
		///OWASA infrastructure
		owasa.infMatrix[4][0] = xreal[40];////Western Wake Treatment plant
		owasa.infMatrix[4][2] = 12;
		owasa.infMatrix[4][4] = 243.3*xreal[49];
		owasa.infMatrix[5][0] = xreal[41];////Western Wake Treatment plant
		owasa.infMatrix[5][2] = 27;
		owasa.infMatrix[5][4] = 316.8*xreal[49];
		///Durham infrastructure
		durham.infMatrix[5][0] = xreal[42];/////Western Wake Treatment plant
		durham.infMatrix[5][2] = 12;
		durham.infMatrix[5][4] = 243.3*xreal[50];
		durham.infMatrix[6][0] = xreal[43];////Western Wake Treatment plant
		durham.infMatrix[6][2] = 27;
		durham.infMatrix[6][4] = 316.8*xreal[50];
		///Raleigh infrastructure
		raleigh.infMatrix[4][0] = xreal[44];//Western Wake Treatment plant
		raleigh.infMatrix[4][2] = 12;
		raleigh.infMatrix[4][4] = 243.3*xreal[51];
		raleigh.infMatrix[5][0] = xreal[45];////Western Wake Treatment plant
		raleigh.infMatrix[5][2] = 27;
		raleigh.infMatrix[5][4] = 316.8*xreal[51];
	}
	caryUpgrades[0] = xreal[46];
	caryUpgrades[1] = xreal[47];
	caryUpgrades[2] = xreal[48];
	caryUpgrades[3] = 999.0;
	owasa.westernWakeTreatmentFrac = xreal[49];
	durham.westernWakeTreatmentFrac = xreal[50];
	raleigh.westernWakeTreatmentFrac = xreal[51];
	fallsLakeReallocation = xreal[52];
		
	//Drought surcharges - 2 variables for each utility, one for residential customers and one for commercial/industrial/irrigation customers
	if (formulation<6)
	{
		for(int x = 0; x<5; x++)
		{
			for (int y = 0; y<10;y++)
			{
				durham.fractionPricing[x][y] = 1;
			}
			for (int y = 0; y<24;y++)
			{
				raleigh.fractionPricing[x][y] = 1;
			}
			for (int y = 0; y<9;y++)
			{
				cary.fractionPricing[x][y] = 1;
			}
		}
		for(int x = 0;x<4;x++)
		{
			for (int y = 0; y<8; y++)
			{
				owasa.fractionPricing[x][y] = 1;
			}
		}
	}
	
	//Takes water use distributions calculated in calc_Water_Price functions and applies drought surcharge to calculate
	//the average revenue recieved from the sale of 1 MG of water, the consumer surplus lost from water use reductions/cost increases,
	//and overall reductions from restrictions and price increases
	//(Only do it once at the beginning. If formulation is 5, redo it every simulation).
	if(formulation == 6)
	{
		durham.calcSurchargePrice(elasticity_Of_Demand, 1);
		owasa.calcSurchargePrice(elasticity_Of_Demand, 1);
		cary.calcSurchargePrice(elasticity_Of_Demand, 1);
		raleigh.calcSurchargePrice(elasticity_Of_Demand, 1);
	}
	
	
	// Initialize variables back to 0 to start the next simulation
	durham.clearVariablesForSimulation();
	owasa.clearVariablesForSimulation();
	cary.clearVariablesForSimulation();
	raleigh.clearVariablesForSimulation();
	
	durham.setCapacity(6349.0);
	owasa.setCapacity(3558.0);
	raleigh.setCapacity(14700.0+2789.66);
	cary.setCapacity(cary.jordanLakeAlloc*14924.0);
	
	//Actual reservoir simulation loop
	realizationLoop();
	
	//Writing objective variables
	if(borgToggle == 0)//Using MOEA, with overall objectives determined by the largest individual utility value for the given objective
	{	
		// The first three objectives (reliability, restriction frequency, and total cost) are used in all formulations
		c_obj[0] = maxValue(durham.maxFailures, owasa.maxFailures, raleigh.maxFailures, cary.maxFailures);
		c_obj[1] = durham.expectedNPC + owasa.expectedNPC + raleigh.expectedNPC + cary.expectedNPC;
		c_obj[2] = maxValue(durham.peakDebt, owasa.peakDebt, raleigh.peakDebt, cary.peakDebt);
		c_obj[3] = maxValue(durham.totalLosses, owasa.totalLosses, raleigh.totalLosses, cary.totalLosses);
		c_obj[4] = maxFallsFailure;

		if (formulation > 0) // For higher formulations, include average transfer volume objective
			c_obj[5] = xreal[7]+xreal[8]+xreal[9]+xreal[10];
		
	}
	
	else if(borgToggle == 3) //Mode to allow for manual input of parameters and individual objective function evaluation
	{
		// RefSet Reruns - want the 6 objectives from O0
		
		// The first three objectives (reliability, restriction frequency, and total cost) are used in all formulations
		/*c_obj[0] = maxValue(durham.maxFailures, owasa.maxFailures, raleigh.maxFailures, cary.maxFailures);
		c_obj[1] = maxValue(durham.restrictionFreq, owasa.restrictionFreq, raleigh.restrictionFreq, cary.restrictionFreq);
		c_obj[2] = maxValue(durham.totalCosts, owasa.totalCosts, raleigh.totalCosts, cary.totalCosts);
		c_obj[3] = durham.jordanLakeAlloc + owasa.jordanLakeAlloc + raleigh.jordanLakeAlloc + cary.jordanLakeAlloc;
		c_obj[4] = maxValue(durham.totalLosses, owasa.totalLosses, raleigh.totalLosses, cary.totalLosses);
		c_obj[5] = maxValue(durham.consumerFractions, owasa.consumerFractions, raleigh.consumerFractions, cary.consumerFractions);*/
		
		c_obj[0] = durham.maxFailures;
		c_obj[1] = durham.expectedNPC;
		c_obj[2] = durham.peakDebt;
		c_obj[3] = durham.totalLosses;
		c_obj[4] = xreal[7];
		
		c_obj[5] = owasa.maxFailures;
		c_obj[6] = owasa.expectedNPC;
		c_obj[7] = owasa.peakDebt;
		c_obj[8] = owasa.totalLosses;
		c_obj[9] = xreal[8];
		
		c_obj[10] = raleigh.maxFailures;
		c_obj[11] = raleigh.expectedNPC;
		c_obj[12] = raleigh.peakDebt;
		c_obj[13] = raleigh.totalLosses;
		c_obj[14] = xreal[9];
		
		c_obj[15] = cary.maxFailures;
		c_obj[16] = cary.expectedNPC;
		c_obj[17] = cary.peakDebt;
		c_obj[18] = cary.totalLosses;
		c_obj[19] = xreal[10];
		
		c_obj[20] = maxFallsFailure;
		
	}
	
	// If optimizing, also calculate the constraints
	// JDH 11/12: Turning off constraints for now
	/*
	if(borgToggle == 0 || borgToggle == 1)
	{
		if(num_constr >= 3)
		{
			c_constr[0] = -1*c_obj[0]/.02 + 1;
			c_constr[1] = -1*c_obj[1]/.50 + 1;
			c_constr[2] = -1*c_obj[2]/.25 + 1;
		}
		if(num_constr >= 4)
		{
			c_constr[3] = -1*c_obj[3] + 1;
		}
		if(num_constr >= 5)
		{
			if (c_obj[2]>0)
				c_constr[4] = -1*c_obj[4]/c_obj[2] + 2;
			else
				c_constr[4] = 0;
		}
		if(num_constr >= 6)
		{
			c_constr[5] = 0;
		}
		
		for (int x = 0; x < num_constr;x++)
		{
			if (c_constr[x] > 0)
				c_constr[x] = 0;
		}
	}*/
	
	return;
}
void Simulation::createRiskOfFailure(int realization, int synthYear, double durhamDemandValue, double owasaDemandValue, double raleighDemandValue, double caryDemandValue)
{
	// Risk of failure measures the risk, given a certain week of the year and storage volume, of reservoirs running out of water using historical 
	// streamflows.  This function goes through every combination of storage (discritized at 5% intervals) and week-of-year and runs through 52 weeks of 
	// streamflow levels 78 seperate times, beginning at the same week of the year in each of the first 78 years of historical streamflow record.  At each
	// distinct storage/week point, a percentage is calculating, showing the probability that reservoir levels will drop below 20% of total storage capacity
	// at least once over the next 52 weeks.  Demand is updated for every year with demand growth estimates, such that a distinct risk-of-failure is calculated
	// in each future year.  52(weeks per year) X 20 (storage discritizations) X 51 (max number of future years) distinct risk values are calculated.
	double durhamROFDemand = 0.0;
	double owasaROFDemand = 0.0;
	double raleighROFDemand = 0.0;
	double caryROFDemand = 0.0;
	double durhamROFInflow = 0.0;
	double raleighROFInflow = 0.0;
	double owasaROFInflow = 0.0;
	double wbROFInflow = 0.0;
	double claytonROFInflow = 0.0;
	double crabtreeROFInflow = 0.0;
	double jordanROFInflow = 0.0;
	double lillingtonROFInflow = 0.0;
	double littleRiverRaleighROFInflow = 0.0;
	double ROFevap = 0.0;
	double fallsROFevap = 0.0;
	double wbROFevap = 0.0;
	double oROFs = 0.0;
	double dROFs = 0.0;
	double rROFs = 0.0;
	double cROFs = 0.0;
	int oROFstage = 0.0;
	int dROFstage = 0.0;
	int rROFstage = 0.0;
	int cROFstage = 0.0;
	int counter = 0;
	int syntheticIndex = 0;
	double thisTimeO =0.0;
	double thisTimeR = 0.0;
	double thisTimeD = 0.0;
	double thisTimeC = 0.0;
	int histYear = 77;
	int yearROF;
	int monthROF;
	int weekROF;
	int numdaysROF;
	int insurancePriceCounter;
	for(int x = 0; x< 52; x++)
	{
		for(int y = 0; y < volumeIncrements; y++)
		{
			owasa.probReach[y][x] = 0.0;
			durham.probReach[y][x] = 0.0;
			raleigh.probReach[y][x] = 0.0;
			cary.probReach[y][x] = 0.0;
		}
	}
	
	for (int startingWeek = 1;  startingWeek<53; startingWeek++)//determines the week of the year from which to calculate risk
	{
		for (int startingVolume = 0; startingVolume<volumeIncrements; startingVolume++)// determines the storage volume from which to calculate risk
		{
			owasa.riskOfFailure[startingVolume][startingWeek-1] = 0.0;
			durham.riskOfFailure[startingVolume][startingWeek-1] = 0.0;
			raleigh.riskOfFailure[startingVolume][startingWeek-1] = 0.0;
			cary.riskOfFailure[startingVolume][startingWeek-1] = 0.0;
			
			

			for (int histRealizations = 0; histRealizations<histYear; histRealizations++)// determines the year of the historical streamflow record to be used in calculations
			{
				counter = 0;
				thisTimeO=0;
				thisTimeD = 0;
				thisTimeR = 0;
				thisTimeC = 0;
				riskOfFailureDates.initializeDates(4,1,startingWeek,7,0);//each simulation is run independently (52 week intervals, then the slate is cleaned)
				riskOfFailureStorageROF.updateReservoirStorageROF( double(startingVolume), double(volumeIncrements));
										
					
				while (counter<52)
				{	
					//Retrieving date information
					yearROF = riskOfFailureDates.getYear();
					monthROF = riskOfFailureDates.getMonth();
					weekROF = riskOfFailureDates.getWeek();
					numdaysROF = riskOfFailureDates.getDays();
						
					//Demand calcs
					//While seasonal trends and future growth are incorporated, no demand uncertainty is used (weekly averages only)
					durhamROFDemand = durham.UD.averages[weekROF-1]*numdays*durhamDemandValue;//Durham demands
					owasaROFDemand = owasa.UD.averages[weekROF-1]*numdays*owasaDemandValue;//OWASA demands
					raleighROFDemand = raleigh.UD.averages[weekROF-1]*numdays*raleighDemandValue;//Raleigh Demands
					caryROFDemand = raleigh.UD.averages[weekROF-1]*numdays*caryDemandValue;//Raleigh Demands
					
					riskOfFailureStorageROF.setDemands(durhamROFDemand, owasaROFDemand, raleighROFDemand, caryROFDemand, numdaysROF);//passes demands to storage calcs
					
					//Inflow Calcs
					durhamROFInflow = michieInflow[weekROF-1][histRealizations+yearROF]+littleRiverInflow[weekROF-1][histRealizations+yearROF-3];//Durham Inflows
					raleighROFInflow = fallsLakeInflow[histRealizations+yearROF-3][weekROF-1];//Raleigh
					owasaROFInflow = owasaInflow[weekROF-1][histRealizations+yearROF];//OWASA
					wbROFInflow = lakeWBInflow[histRealizations+yearROF-2][weekROF-1];//Wheeler&Benson
					claytonROFInflow = claytonInflow[histRealizations+yearROF-2][weekROF-1];//Clayton gauge
					crabtreeROFInflow = crabtreeInflow[histRealizations+yearROF-2][weekROF-1];// Crabtree creek
					jordanROFInflow = jordanLakeInflow[histRealizations+yearROF-4][weekROF-1];
					lillingtonROFInflow = lillingtonGaugeInflow[histRealizations+yearROF-4][weekROF-1];
					littleRiverRaleighROFInflow = littleRiverRaleighInflow[histRealizations+yearROF-3][weekROF-1];
					//Evap Calcs
					ROFevap = evaporation[weekROF-1][histRealizations+yearROF-4];//Durham and OWASA evap
					fallsROFevap = fallsLakeEvaporation[histRealizations+yearROF-3][weekROF-1];// Falls Lake evap
					wbROFevap= lakeWheelerEvaporation[histRealizations+yearROF-4][weekROF-1];//Wheeler Benson evap

					riskOfFailureStorageROF.setInflow(durhamROFInflow, //passes inflows to storage calcs
						31.4*owasaROFInflow,
						28.7*owasaROFInflow, 
						1.2*owasaROFInflow, 
						raleighROFInflow, 
						wbROFInflow, 
						claytonROFInflow, 
						crabtreeROFInflow, jordanROFInflow, lillingtonROFInflow, 
						raleighROFDemand*returnRatio[1][weekROF-1], 
						durhamROFDemand*returnRatio[0][weekROF-1], durhamROFDemand*returnRatio[0][weekROF-1],owasaROFDemand*returnRatio[0][weekROF-1],
						fallsROFevap, 
						wbROFevap, 
						ROFevap, littleRiverRaleighROFInflow);
					riskOfFailureStorageROF.setSpillover(weekROF-1);//reservoir releases to meet downstream needs and/or reservoir capacity
					riskOfFailureStorageROF.updateStorage(weekROF-1);//storage calcs
					
					oROFs = riskOfFailureStorageROF.getOWASAStorage();//retrieve overall storage, OWASA
					dROFs = riskOfFailureStorageROF.getDurhamStorage();//retrieve overall storage, Durham
					rROFs = riskOfFailureStorageROF.getRaleighStorage();//retrieve overall storage, Raleigh
					cROFs = riskOfFailureStorageROF.getCaryStorage();
					if(startingWeek == 13&&startingVolume == volumeIncrements - 1)
					{
						insurancePriceCounter = startingWeek - 1 + counter;
						if(insurancePriceCounter > 51)
						{
							insurancePriceCounter -= 52;
						}
						oROFstage = int(oROFs*volumeIncrements);
						dROFstage = int(dROFs*volumeIncrements);
						rROFstage = int(rROFs*volumeIncrements);
						cROFstage = int(cROFs*volumeIncrements);
						for(int currentInc = oROFstage; currentInc<volumeIncrements; currentInc++)
						{
							owasa.probReach[currentInc][insurancePriceCounter] += 1.0/(double(histYear+synthYear));
						}
						for(int currentInc = dROFstage; currentInc<volumeIncrements; currentInc++)
						{
							durham.probReach[currentInc][insurancePriceCounter] += 1.0/(double(histYear+synthYear));
						}
						for(int currentInc = rROFstage; currentInc<volumeIncrements; currentInc++)
						{
							raleigh.probReach[currentInc][insurancePriceCounter] += 1.0/(double(histYear+synthYear));
						}
						for(int currentInc = cROFstage; currentInc<volumeIncrements; currentInc++)
						{
							cary.probReach[currentInc][insurancePriceCounter] += 1.0/(double(histYear+synthYear));
						}
					}
					if (oROFs<.2)
					{
						thisTimeO = 1;//Mark OWASA failure (not a count, there is no difference between 1 week of failure and 52)
					}
					if (dROFs<.2)
					{
						thisTimeD = 1;//Mark Durham failure (not a count, there is no difference between 1 week of failure and 52)
					}
					if (rROFs<.2)
					{
						thisTimeR = 1;//Mark Raleigh failure (not a count, there is no difference between 1 week of failure and 52)
					}
					if (cROFs<.2)
					{
						thisTimeC = 1;
					}
					riskOfFailureDates.increase();//increase week by one
					counter++;
				}
					
				if (thisTimeO==1)//Fill out the risk matricies (every historical year that causes at least one week of failure results in an increase in risk of 1.28% (1/78)
				{
					owasa.riskOfFailure[startingVolume][startingWeek-1]+= 1.0;
				}
				if (thisTimeD==1)
				{
					durham.riskOfFailure[startingVolume][startingWeek-1]+= 1.0;
				}
				if (thisTimeR==1)
				{
					raleigh.riskOfFailure[startingVolume][startingWeek-1]+= 1.0;
				}
				if (thisTimeC==1)
				{
					cary.riskOfFailure[startingVolume][startingWeek-1]+= 1.0;
				}
			}
			for (int synthRealizations = 3; synthRealizations<synthYear; synthRealizations++)// determines the year of the historical streamflow record to be used in calculations
			{
				counter = 0;
				thisTimeO=0;
				thisTimeD = 0;
				thisTimeR = 0;
				thisTimeC = 0;
				riskOfFailureDates.initializeDates(1,1,startingWeek,7,0);//each simulation is run independently (52 week intervals, then the slate is cleaned)
				riskOfFailureStorageROF.updateReservoirStorageROF( double(startingVolume), double(volumeIncrements));
										
					
				while (counter<52)
				{	
					//Retrieving date information
					yearROF = riskOfFailureDates.getYear();
					monthROF = riskOfFailureDates.getMonth();
					weekROF = riskOfFailureDates.getWeek();
					numdaysROF = riskOfFailureDates.getDays();
						
					//Demand calcs
					//While seasonal trends and future growth are incorporated, no demand uncertainty is used (weekly averages only)
					durhamROFDemand = durham.UD.averages[weekROF-1]*numdays*durhamDemandValue;//Durham demands
					owasaROFDemand = owasa.UD.averages[weekROF-1]*numdays*owasaDemandValue;//OWASA demands
					raleighROFDemand = raleigh.UD.averages[weekROF-1]*numdays*raleighDemandValue;//Raleigh Demands
					caryROFDemand = raleigh.UD.averages[weekROF-1]*numdays*caryDemandValue;//Raleigh Demands
	
					riskOfFailureStorageROF.setDemands(durhamROFDemand, owasaROFDemand, raleighROFDemand, caryROFDemand, numdaysROF);//passes demands to storage calcs
					
					//Inflow Calcs
					syntheticIndex = (yearROF-1+synthRealizations)*52+weekROF-1;
					durhamROFInflow = durhamInflows.synthetic[realization][syntheticIndex];//Durham Inflows
					raleighROFInflow = owasaInflows.synthetic[realization][syntheticIndex];//Raleigh
					owasaROFInflow = fallsInflows.synthetic[realization][syntheticIndex];//OWASA
					wbROFInflow = wheelerInflows.synthetic[realization][syntheticIndex];//Wheeler&Benson
					claytonROFInflow = claytonInflows.synthetic[realization][syntheticIndex];//Clayton gauge
					crabtreeROFInflow = crabtreeInflows.synthetic[realization][syntheticIndex];// Crabtree creek
					jordanROFInflow = jordanInflows.synthetic[realization][syntheticIndex];
					lillingtonROFInflow = lillingtonInflows.synthetic[realization][syntheticIndex];
					littleRiverRaleighROFInflow = littleRiverRaleighInflows.synthetic[realization][syntheticIndex];
					//Evap Calcs
					ROFevap = evaporation[weekROF-1][synthRealizations];//Durham and OWASA evap
					fallsROFevap = fallsLakeEvaporation[synthRealizations][weekROF-1];// Falls Lake evap
					wbROFevap= lakeWheelerEvaporation[synthRealizations][weekROF-1];//Wheeler Benson evap
					
					riskOfFailureStorageROF.setInflow(durhamROFInflow, //passes inflows to storage calcs
						31.4*owasaROFInflow,
						28.7*owasaROFInflow, 
						1.2*owasaROFInflow, 
						raleighROFInflow, 
						wbROFInflow, 
						claytonROFInflow, 
						crabtreeROFInflow, jordanROFInflow, lillingtonROFInflow, 
						raleighROFDemand*returnRatio[1][weekROF-1], 
						durhamROFDemand*returnRatio[0][weekROF-1], durhamROFDemand*returnRatio[0][weekROF-1],owasaROFDemand*returnRatio[0][weekROF-1],
						fallsROFevap, 
						wbROFevap, 
						ROFevap, littleRiverRaleighROFInflow);
					riskOfFailureStorageROF.setSpillover(week-1);//reservoir releases to meet downstream needs and/or reservoir capacity
					riskOfFailureStorageROF.updateStorage(week-1);//storage calcs
					
					oROFs = riskOfFailureStorageROF.getOWASAStorage();//retrieve overall storage, OWASA
					dROFs = riskOfFailureStorageROF.getDurhamStorage();//retrieve overall storage, Durham
					rROFs = riskOfFailureStorageROF.getRaleighStorage();//retrieve overall storage, Raleigh
					cROFs = riskOfFailureStorageROF.getCaryStorage();
					if(startingWeek == 13&&startingVolume == volumeIncrements - 1)
					{
						insurancePriceCounter = startingWeek - 1 + counter;
						if(insurancePriceCounter > 51)
						{
							insurancePriceCounter -= 52;
						}
						oROFstage = int(oROFs*volumeIncrements);
						dROFstage = int(dROFs*volumeIncrements);
						rROFstage = int(rROFs*volumeIncrements);
						cROFstage = int(cROFs*volumeIncrements);
						for(int currentInc = oROFstage; currentInc<volumeIncrements; currentInc++)
						{
							owasa.probReach[currentInc][insurancePriceCounter] += 1.0/(double(histYear+synthYear));
						}
						for(int currentInc = dROFstage; currentInc<volumeIncrements; currentInc++)
						{
							durham.probReach[currentInc][insurancePriceCounter] += 1.0/(double(histYear+synthYear));
						}
						for(int currentInc = rROFstage; currentInc<volumeIncrements; currentInc++)
						{
							raleigh.probReach[currentInc][insurancePriceCounter] += 1.0/(double(histYear+synthYear));
						}
						for(int currentInc = cROFstage; currentInc<volumeIncrements; currentInc++)
						{
							cary.probReach[currentInc][insurancePriceCounter] += 1.0/(double(histYear+synthYear));
						}
					}
							
					if (oROFs<.2)
					{
						thisTimeO = 1;//Mark OWASA failure (not a count, there is no difference between 1 week of failure and 52)
					}
					if (dROFs<.2)
					{
						thisTimeD = 1;//Mark Durham failure (not a count, there is no difference between 1 week of failure and 52)
					}
					if (rROFs<.2)
					{
						thisTimeR = 1;//Mark Raleigh failure (not a count, there is no difference between 1 week of failure and 52)
					}
					if (cROFs<.2)
					{
						thisTimeC = 1;
					}
					riskOfFailureDates.increase();//increase week by one
					counter++;
				}
				if (thisTimeO==1)//Fill out the risk matricies (every historical year that causes at least one week of failure results in an increase in risk of 1.28% (1/78)
				{
					owasa.riskOfFailure[startingVolume][startingWeek-1]+= 1.0;
				}
				if (thisTimeD==1)
				{
					durham.riskOfFailure[startingVolume][startingWeek-1]+= 1.0;
				}
				if (thisTimeR==1)
				{
					raleigh.riskOfFailure[startingVolume][startingWeek-1]+= 1.0;
				}
				if (thisTimeC==1)
				{
					cary.riskOfFailure[startingVolume][startingWeek-1]+= 1.0;
				}
				
			}
			owasa.riskOfFailure[startingVolume][startingWeek-1] = owasa.riskOfFailure[startingVolume][startingWeek-1]/(double(histYear+synthYear));
			
			
			durham.riskOfFailure[startingVolume][startingWeek-1] = durham.riskOfFailure[startingVolume][startingWeek-1]/(double(histYear+synthYear));

			raleigh.riskOfFailure[startingVolume][startingWeek-1] = raleigh.riskOfFailure[startingVolume][startingWeek-1]/(double(histYear+synthYear));
			
			cary.riskOfFailure[startingVolume][startingWeek-1] = cary.riskOfFailure[startingVolume][startingWeek-1]/(double(histYear+synthYear));
			
		}
	}
return;
}

void Simulation::createInfrastructure()
{
	int owasaConstruction;
	int durhamConstruction;
	int raleighConstruction;
	for(int x = 0; x<owasa.infrastructureCount; x++)
	{
		owasaConstruction = owasa.buildInfrastructure(x);
		if(owasaConstruction == 1)
		{
			switch(x)
			{
				case 0:
					systemStorage.buildULexp();
					riskOfFailureStorageROF.buildULexp();
					owasa.addCapacity(2550.0);
					break;
				case 1:
					systemStorage.buildCCexp();
					riskOfFailureStorageROF.buildCCexp();
					owasa.addCapacity(3000.0);
					break;
				case 2:
					systemStorage.buildSQlow();
					riskOfFailureStorageROF.buildSQlow();
					owasa.addCapacity(1500.0);
					break;
				case 3:
					systemStorage.buildSQhigh();
					riskOfFailureStorageROF.buildSQhigh();
					owasa.addCapacity(2200.0);
					break;
				case 4:
					systemStorage.buildWWWTPlow(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					riskOfFailureStorageROF.buildWWWTPlow(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					wwWTPInsuranceTrigger = 1.0;
					break;
				case 5:
					systemStorage.buildWWWTPhigh(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					riskOfFailureStorageROF.buildWWWTPhigh(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					wwWTPInsuranceTrigger = 1.0;
					break;
			}
		}
	}
	for(int x = 0; x<durham.infrastructureCount; x++)
	{
		durhamConstruction = durham.buildInfrastructure(x);
		if(durhamConstruction == 1)
		{
			switch(x)
			{
				case 0:
					systemStorage.buildTeerQuarry();
					riskOfFailureStorageROF.buildTeerQuarry();
					break;
				case 1:
					systemStorage.buildReclaimedLow();
					riskOfFailureStorageROF.buildReclaimedLow();
					durhamReclaimedInsuranceTrigger = 1.0;
					break;
				case 2:
					systemStorage.buildReclaimedHigh();
					riskOfFailureStorageROF.buildReclaimedHigh();
					durhamReclaimedInsuranceTrigger = 1.0;
					break;
				case 3:
					systemStorage.buildMichieLow();
					riskOfFailureStorageROF.buildMichieLow();
					durham.setCapacity(8849.0);
				case 4:
					systemStorage.buildMichieHigh();
					riskOfFailureStorageROF.buildMichieHigh();
					durham.setCapacity(14049.0);
					break;
				case 5:
					systemStorage.buildWWWTPlow(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					riskOfFailureStorageROF.buildWWWTPlow(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					wwWTPInsuranceTrigger = 1.0;
					break;
				case 6:
					systemStorage.buildWWWTPhigh(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					riskOfFailureStorageROF.buildWWWTPhigh(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					wwWTPInsuranceTrigger = 1.0;
					break;
			}
		}
	}
	for(int x = 0; x< raleigh.infrastructureCount; x++)
	{
		raleighConstruction = raleigh.buildInfrastructure(x);
		if(raleighConstruction == 1)
		{
			switch(x)
			{
				case 0:
					systemStorage.buildLittleRiverRal();
					riskOfFailureStorageROF.buildLittleRiverRal();
					raleigh.addCapacity(3700.0);
					littleRiverRalInsuranceTrigger = 1.0;
					break;
				case 1:
					systemStorage.buildRalQuarry();
					riskOfFailureStorageROF.buildRalQuarry();
					break;
				case 2:
					systemStorage.buildNeuseIntake();
					riskOfFailureStorageROF.buildNeuseIntake();
					ralIntakeInsuranceTrigger = 1.0;
					break;
				case 3:
					systemStorage.reallocateFallsLake(fallsLakeReallocation);
					riskOfFailureStorageROF.reallocateFallsLake(fallsLakeReallocation);
					raleigh.addCapacity(fallsLakeReallocation);
					raleigh.addInsStorage(fallsLakeReallocation);
					break;
				case 4:
					systemStorage.buildWWWTPlow(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					riskOfFailureStorageROF.buildWWWTPlow(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					wwWTPInsuranceTrigger = 1.0;
					break;
				case 5:
					systemStorage.buildWWWTPhigh(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					riskOfFailureStorageROF.buildWWWTPhigh(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
					wwWTPInsuranceTrigger = 1.0;
					break;
			}
		}
	}
	return;
}

	
void Simulation::createInfrastructureRisk(int realization, int synthYear, double durhamDemandValue, double owasaDemandValue, double raleighDemandValue, double caryDemandValue)
{
	// Risk of failure measures the risk, given a certain week of the year and storage volume, of reservoirs running out of water using historical 
	// streamflows.  This function goes through every combination of storage (discritized at 5% intervals) and week-of-year and runs through 52 weeks of 
	// streamflow levels 78 seperate times, beginning at the same week of the year in each of the first 78 years of historical streamflow record.  At each
	// distinct storage/week point, a percentage is calculating, showing the probability that reservoir levels will drop below 20% of total storage capacity
	// at least once over the next 52 weeks.  Demand is updated for every year with demand growth estimates, such that a distinct risk-of-failure is calculated
	// in each future year.  52(weeks per year) X 20 (storage discritizations) X 51 (max number of future years) distinct risk values are calculated.
	double durhamROFDemand = 0.0;
	double owasaROFDemand = 0.0;
	double raleighROFDemand = 0.0;
	double caryROFDemand = 0.0;
	double durhamROFInflow = 0.0;
	double raleighROFInflow = 0.0;
	double owasaROFInflow = 0.0;
	double wbROFInflow = 0.0;
	double claytonROFInflow = 0.0;
	double crabtreeROFInflow = 0.0;
	double jordanROFInflow = 0.0;
	double lillingtonROFInflow = 0.0;
	double littleRiverRaleighROFInflow = 0.0;
	double ROFevap = 0.0;
	double fallsROFevap = 0.0;
	double wbROFevap = 0.0;
	double oROFs = 0.0;
	double dROFs = 0.0;
	double rROFs = 0.0;
	double cROFs = 0.0;
	int counter = 0;
	int syntheticIndex = 0;
	double thisTimeO = 0.0;
	double thisTimeR = 0.0;
	double thisTimeD = 0.0;
	double thisTimeC = 0.0;
	int histYear = 77;
	durham.infRisk = 0.0;
	raleigh.infRisk = 0.0;
	cary.infRisk = 0.0;
	owasa.infRisk = 0.0;
	int yearROF;
	int monthROF;
	int weekROF;
	int numdaysROF;
	for (int histRealizations = 0; histRealizations<histYear; histRealizations++)// determines the year of the historical streamflow record to be used in calculations
	{
		counter = 0;
		thisTimeO=0;
		thisTimeD = 0;
		thisTimeR = 0;
		thisTimeC = 0;
		riskOfFailureDates.initializeDates(4,1,1,7,0);//each simulation is run independently (52 week intervals, then the slate is cleaned)
		riskOfFailureStorageInf.updateReservoirStorageROF( 1.0, 1.0);								
					
		while (counter<78)
		{	
			//Retrieving date information
			yearROF = riskOfFailureDates.getYear();
			monthROF = riskOfFailureDates.getMonth();
			weekROF = riskOfFailureDates.getWeek();
			numdaysROF = riskOfFailureDates.getDays();
						
			//Demand calcs
			//While seasonal trends and future growth are incorporated, no demand uncertainty is used (weekly averages only)
			durhamROFDemand = durham.UD.averages[weekROF-1]*numdays*durhamDemandValue;//Durham demands
			owasaROFDemand = owasa.UD.averages[weekROF-1]*numdays*owasaDemandValue;//OWASA demands
			raleighROFDemand = raleigh.UD.averages[weekROF-1]*numdays*raleighDemandValue;//Raleigh Demands
			caryROFDemand = raleigh.UD.averages[weekROF-1]*numdays*caryDemandValue;//Raleigh Demands
					
			riskOfFailureStorageInf.setDemands(durhamROFDemand, owasaROFDemand, raleighROFDemand, caryROFDemand, numdaysROF);//passes demands to storage calcs
			
			//Inflow Calcs
			durhamROFInflow = michieInflow[weekROF-1][histRealizations+yearROF]+littleRiverInflow[weekROF-1][histRealizations+yearROF-3];//Durham Inflows
			raleighROFInflow = fallsLakeInflow[histRealizations+yearROF-3][weekROF-1];//Raleigh
			owasaROFInflow = owasaInflow[weekROF-1][histRealizations+yearROF];//OWASA
			wbROFInflow = lakeWBInflow[histRealizations+yearROF-2][weekROF-1];//Wheeler&Benson
			claytonROFInflow = claytonInflow[histRealizations+yearROF-2][weekROF-1];//Clayton gauge
			crabtreeROFInflow = crabtreeInflow[histRealizations+yearROF-2][weekROF-1];// Crabtree creek
			jordanROFInflow = jordanLakeInflow[histRealizations+yearROF-4][weekROF-1];
			lillingtonROFInflow = lillingtonGaugeInflow[histRealizations+yearROF-4][weekROF-1];
			littleRiverRaleighROFInflow = littleRiverRaleighInflow[histRealizations+yearROF-3][weekROF-1];
			//Evap Calcs
			ROFevap = evaporation[week-1][histRealizations+yearROF-4];//Durham and OWASA evap
			fallsROFevap = fallsLakeEvaporation[histRealizations+yearROF-3][weekROF-1];// Falls Lake evap
			wbROFevap= lakeWheelerEvaporation[histRealizations+yearROF-4][weekROF-1];//Wheeler Benson evap

			riskOfFailureStorageInf.setInflow(durhamROFInflow, //passes inflows to storage calcs
				31.4*owasaROFInflow,
				28.7*owasaROFInflow, 
				1.2*owasaROFInflow, 
				raleighROFInflow, 
				wbROFInflow, 
				claytonROFInflow, 
				crabtreeROFInflow, jordanROFInflow, lillingtonROFInflow, 
				raleighROFDemand*returnRatio[1][weekROF-1], 
				durhamROFDemand*returnRatio[0][weekROF-1], durhamROFDemand*returnRatio[0][weekROF-1],owasaROFDemand*returnRatio[0][weekROF-1],
				fallsROFevap, 
				wbROFevap, 
				ROFevap, littleRiverRaleighROFInflow);
			riskOfFailureStorageInf.setSpillover(weekROF-1);//reservoir releases to meet downstream needs and/or reservoir capacity
			riskOfFailureStorageInf.updateStorage(weekROF-1);//storage calcs
					
			oROFs = riskOfFailureStorageInf.getOWASAStorage();//retrieve overall storage, OWASA
			dROFs = riskOfFailureStorageInf.getDurhamStorage();//retrieve overall storage, Durham
			rROFs = riskOfFailureStorageInf.getRaleighStorage();//retrieve overall storage, Raleigh
			cROFs = riskOfFailureStorageInf.getCaryStorage();
			if (oROFs<.2)
			{
				thisTimeO = 1;//Mark OWASA failure (not a count, there is no difference between 1 week of failure and 52)
			}
			if (dROFs<.2)
			{
				thisTimeD = 1;//Mark Durham failure (not a count, there is no difference between 1 week of failure and 52)
			}
			if (rROFs<.2)
			{
				thisTimeR = 1;//Mark Raleigh failure (not a count, there is no difference between 1 week of failure and 52)
			}
			if (cROFs<.2)
			{
				thisTimeC = 1;
			}
			riskOfFailureDates.increase();//increase week by one
			counter++;
		}
					
		if (thisTimeO==1)//Fill out the risk matricies (every historical year that causes at least one week of failure results in an increase in risk of 1.28% (1/78)
		{
			owasa.infRisk += 1.0;
		}
		if (thisTimeD==1)
		{
			durham.infRisk += 1.0;
		}
		if (thisTimeR==1)
		{
			raleigh.infRisk += 1.0;
		}
		if (thisTimeC==1)
		{
			cary.infRisk += 1.0;
		}
	}
	for (int synthRealizations = 3; synthRealizations<synthYear; synthRealizations++)// determines the year of the historical streamflow record to be used in calculations
	{
		counter = 0;
		thisTimeO=0;
		thisTimeD = 0;
		thisTimeR = 0;
		thisTimeC = 0;
		riskOfFailureDates.initializeDates(1,1,1,7,0);//each simulation is run independently (52 week intervals, then the slate is cleaned)
		riskOfFailureStorageInf.updateReservoirStorageROF( 1.0, 1.0);
										
					
		while (counter<52)
		{	
			//Retrieving date information
			yearROF = riskOfFailureDates.getYear();
			monthROF = riskOfFailureDates.getMonth();
			weekROF = riskOfFailureDates.getWeek();
			numdaysROF = riskOfFailureDates.getDays();
						
			//Demand calcs
			//While seasonal trends and future growth are incorporated, no demand uncertainty is used (weekly averages only)
			durhamROFDemand = durham.UD.averages[weekROF-1]*numdays*durhamDemandValue;//Durham demands
			owasaROFDemand = owasa.UD.averages[weekROF-1]*numdays*owasaDemandValue;//OWASA demands
			raleighROFDemand = raleigh.UD.averages[weekROF-1]*numdays*raleighDemandValue;//Raleigh Demands
			caryROFDemand = raleigh.UD.averages[weekROF-1]*numdays*caryDemandValue;//Raleigh Demands
	
			riskOfFailureStorageInf.setDemands(durhamROFDemand, owasaROFDemand, raleighROFDemand, caryROFDemand, numdaysROF);//passes demands to storage calcs
					
			//Inflow Calcs
			syntheticIndex = (yearROF-1+synthRealizations)*52+weekROF-1;
			durhamROFInflow = durhamInflows.synthetic[realization][syntheticIndex];//Durham Inflows
			raleighROFInflow = owasaInflows.synthetic[realization][syntheticIndex];//Raleigh
			owasaROFInflow = fallsInflows.synthetic[realization][syntheticIndex];//OWASA
			wbROFInflow = wheelerInflows.synthetic[realization][syntheticIndex];//Wheeler&Benson
			claytonROFInflow = claytonInflows.synthetic[realization][syntheticIndex];//Clayton gauge
			crabtreeROFInflow = crabtreeInflows.synthetic[realization][syntheticIndex];// Crabtree creek
			jordanROFInflow = jordanInflows.synthetic[realization][syntheticIndex];
			lillingtonROFInflow = lillingtonInflows.synthetic[realization][syntheticIndex];
			littleRiverRaleighROFInflow = littleRiverRaleighInflows.synthetic[realization][syntheticIndex];
			//Evap Calcs
			ROFevap = evaporation[weekROF-1][synthRealizations];//Durham and OWASA evap
			fallsROFevap = fallsLakeEvaporation[synthRealizations][weekROF-1];// Falls Lake evap
			wbROFevap= lakeWheelerEvaporation[synthRealizations][weekROF-1];//Wheeler Benson evap

			riskOfFailureStorageInf.setInflow(durhamROFInflow, //passes inflows to storage calcs
				31.4*owasaROFInflow,
				28.7*owasaROFInflow, 
				1.2*owasaROFInflow, 
				raleighROFInflow, 
				wbROFInflow, 
				claytonROFInflow, 
				crabtreeROFInflow, jordanROFInflow, lillingtonROFInflow, 
				raleighROFDemand*returnRatio[1][weekROF-1], 
				durhamROFDemand*returnRatio[0][weekROF-1], durhamROFDemand*returnRatio[0][weekROF-1],owasaROFDemand*returnRatio[0][weekROF-1],
				fallsROFevap, 
				wbROFevap, 
				ROFevap, littleRiverRaleighROFInflow);
			riskOfFailureStorageInf.setSpillover(weekROF-1);//reservoir releases to meet downstream needs and/or reservoir capacity
			riskOfFailureStorageInf.updateStorage(weekROF-1);//storage calcs
					
			oROFs = riskOfFailureStorageInf.getOWASAStorage();//retrieve overall storage, OWASA
			dROFs = riskOfFailureStorageInf.getDurhamStorage();//retrieve overall storage, Durham
			rROFs = riskOfFailureStorageInf.getRaleighStorage();//retrieve overall storage, Raleigh
			cROFs = riskOfFailureStorageInf.getCaryStorage();
			if (oROFs<.2)
			{
				thisTimeO = 1;//Mark OWASA failure (not a count, there is no difference between 1 week of failure and 52)
			}
			if (dROFs<.2)
			{
				thisTimeD = 1;//Mark Durham failure (not a count, there is no difference between 1 week of failure and 52)
			}
			if (rROFs<.2)
			{
				thisTimeR = 1;//Mark Raleigh failure (not a count, there is no difference between 1 week of failure and 52)
			}
			if (cROFs<.2)
			{
				thisTimeC = 1;
			}
			riskOfFailureDates.increase();//increase week by one
			counter++;
		}	
			
		if (thisTimeO==1)//Fill out the risk matricies (every historical year that causes at least one week of failure results in an increase in risk of 1.28% (1/78)
		{
			owasa.infRisk += 1.0;
		}
		if (thisTimeD==1)
		{
			durham.infRisk += 1.0;
		}
		if (thisTimeR==1)
		{
			raleigh.infRisk += 1.0;
		}
		if (thisTimeC==1)
		{
			cary.infRisk += 1.0;
		}
		
	}
	owasa.infRisk = owasa.infRisk/(double(histYear+synthYear));
	durham.infRisk = durham.infRisk/(double(histYear+synthYear));
	raleigh.infRisk = raleigh.infRisk/(double(histYear+synthYear));
	cary.infRisk = cary.infRisk/(double(histYear+synthYear));
	return;
}
void Simulation::triggerInfrastructure(int realization)
{
	int owasaIndex;
	int durhamIndex;
	int raleighIndex;
	if(owasa.infRisk>owasa.infTrigger)
	{
		owasaIndex = owasa.startNewInfrastructure(year);
		switch(owasaIndex)
		{
			
			case 0:
				owasa.addDebt(year, realization, owasa.infMatrix[0][4], bondLength, bondRate);
				owasa.infMatrix[0][3] = 1.0;
				riskOfFailureStorageInf.buildULexp();
				break;
			case 1:
				owasa.addDebt(year, realization, owasa.infMatrix[1][4], bondLength, bondRate);
				owasa.infMatrix[1][3] = 1.0;
				riskOfFailureStorageInf.buildCCexp();
				break;
			case 2:
				owasa.addDebt(year, realization, owasa.infMatrix[2][4], bondLength, bondRate);
				owasa.infMatrix[2][3] = 1.0;
				riskOfFailureStorageInf.buildSQlow();
				break;
			case 3:
				if(owasa.infMatrix[2][1]<1.0)
				{
					owasa.addDebt(year, realization, owasa.infMatrix[3][4], bondLength, bondRate);
				}
				else
				{
					owasa.addDebt(year, realization, owasa.infMatrix[3][4]-owasa.infMatrix[2][4], bondLength, bondRate);
				}
				owasa.infMatrix[3][3] = 1.0;
				owasa.infMatrix[2][1] = 2.0;
				riskOfFailureStorageInf.buildSQhigh();
				
				break;
			case 4:
				owasa.addDebt(year, realization, owasa.infMatrix[4][4], bondLength, bondRate);
				durham.addDebt(year, realization, durham.infMatrix[5][4], bondLength, bondRate);
				raleigh.addDebt(year, realization, raleigh.infMatrix[4][4], bondLength, bondRate);
				owasa.infMatrix[4][3] = 1.0;
				durham.infMatrix[5][1] = 2.0;
				raleigh.infMatrix[4][1] = 2.0;
				riskOfFailureStorageInf.buildWWWTPlow(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
				break;
			case 5:
				if(owasa.infMatrix[4][1]<1.0)
				{
					owasa.addDebt(year, realization, owasa.infMatrix[5][4], bondLength, bondRate);
					durham.addDebt(year, realization, durham.infMatrix[6][4], bondLength, bondRate);
					raleigh.addDebt(year, realization, raleigh.infMatrix[5][4], bondLength, bondRate);
				}
				else
				{
					owasa.addDebt(year, realization, owasa.infMatrix[5][4] - owasa.infMatrix[4][4], bondLength, bondRate);
					durham.addDebt(year, realization, durham.infMatrix[6][4] - durham.infMatrix[5][4], bondLength, bondRate);
					raleigh.addDebt(year, realization, raleigh.infMatrix[5][4] - raleigh.infMatrix[4][4], bondLength, bondRate);
				}		
				owasa.infMatrix[5][3] = 1.0;
				durham.infMatrix[5][1] = 2.0;
				raleigh.infMatrix[4][1] = 2.0;
				owasa.infMatrix[4][1]  = 2.0;
				durham.infMatrix[6][1] = 2.0;
				raleigh.infMatrix[5][1] = 2.0;
				riskOfFailureStorageInf.buildWWWTPhigh(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
				break;
		}

	}
	if(durham.infRisk>durham.infTrigger)
	{
		durhamIndex = durham.startNewInfrastructure(year);
		switch(durhamIndex)
		{
			case 0:
				durham.addDebt(year, realization, durham.infMatrix[0][4], bondLength, bondRate);
				durham.infMatrix[0][3] = 1.0;
				riskOfFailureStorageInf.buildTeerQuarry();
				owasa.addDebt(year, realization, durham.infMatrix[3][4], bondLength, bondRate);
				break;
			case 1:
				durham.addDebt(year, realization, durham.infMatrix[1][4], bondLength, bondRate);
				durham.infMatrix[1][3] = 1.0;
				riskOfFailureStorageInf.buildReclaimedLow();
				break;
			case 2:
				if(durham.infMatrix[1][1]<1.0)
				{
					durham.addDebt(year, realization, durham.infMatrix[2][4], bondLength, bondRate);
				}
				else
				{
					durham.addDebt(year, realization, durham.infMatrix[2][4] - durham.infMatrix[1][4], bondLength, bondRate);
				}
				durham.infMatrix[2][3] = 1.0;
				durham.infMatrix[1][1] = 2.0;
				riskOfFailureStorageInf.buildReclaimedHigh();
				break;
			case 3:
				durham.addDebt(year, realization, durham.infMatrix[3][4], bondLength, bondRate);
				durham.infMatrix[3][3] = 1.0;
				riskOfFailureStorageInf.buildMichieLow();
			case 4:
				if(durham.infMatrix[3][1]<1.0)
				{
					durham.addDebt(year, realization, durham.infMatrix[4][4], bondLength, bondRate);
				}
				else
				{
					durham.addDebt(year, realization, durham.infMatrix[4][4] - durham.infMatrix[3][4], bondLength, bondRate);
				}
				durham.infMatrix[4][3] = 1.0;
				durham.infMatrix[3][1] = 2.0;
				riskOfFailureStorageInf.buildMichieHigh();
				break;
			case 5:
				owasa.addDebt(year, realization, owasa.infMatrix[4][4], bondLength, bondRate);
				durham.addDebt(year, realization, durham.infMatrix[5][4], bondLength, bondRate);
				raleigh.addDebt(year, realization, raleigh.infMatrix[4][4], bondLength, bondRate);
				durham.infMatrix[5][3] = 1.0;
				owasa.infMatrix[4][1] = 2.0;
				raleigh.infMatrix[4][1] = 2.0;
				riskOfFailureStorageInf.buildWWWTPlow(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
				break;
			case 6:
				if(owasa.infMatrix[4][1]<1.0)
				{
					owasa.addDebt(year, realization, owasa.infMatrix[5][4], bondLength, bondRate);
					durham.addDebt(year, realization, durham.infMatrix[6][4], bondLength, bondRate);
					raleigh.addDebt(year, realization, raleigh.infMatrix[5][4], bondLength, bondRate);
				}
				else
				{
					owasa.addDebt(year, realization, owasa.infMatrix[5][4] - owasa.infMatrix[4][4], bondLength, bondRate);
					durham.addDebt(year, realization, durham.infMatrix[6][4] - durham.infMatrix[5][4], bondLength, bondRate);
					raleigh.addDebt(year, realization, raleigh.infMatrix[5][4] - raleigh.infMatrix[4][4], bondLength, bondRate);
				}		
				durham.infMatrix[6][3] = 1.0;
				durham.infMatrix[5][1] = 2.0;
				raleigh.infMatrix[4][1] = 2.0;
				owasa.infMatrix[4][1]  = 2.0;
				owasa.infMatrix[5][1] = 2.0;
				raleigh.infMatrix[5][1] = 2.0;
				riskOfFailureStorageInf.buildWWWTPhigh(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
				break;
		}
	}
	if(raleigh.infRisk>raleigh.infTrigger)
	{
		raleighIndex = raleigh.startNewInfrastructure(year);
		switch(raleighIndex)
		{
			case 0:
				raleigh.addDebt(year, realization, raleigh.infMatrix[0][4], bondLength, bondRate);
				raleigh.infMatrix[0][3] = 1.0;
				riskOfFailureStorageInf.buildLittleRiverRal();
				break;
			case 1:
				raleigh.addDebt(year, realization, raleigh.infMatrix[1][4], bondLength, bondRate);
				raleigh.infMatrix[1][3] = 1.0;
				riskOfFailureStorageInf.buildRalQuarry();
				break;
			case 2:
				raleigh.addDebt(year, realization, raleigh.infMatrix[2][4], bondLength, bondRate);
				raleigh.infMatrix[2][3] = 1.0;
				riskOfFailureStorageInf.buildNeuseIntake();
				break;
			case 3:
				raleigh.addDebt(year, realization, raleigh.infMatrix[3][4], bondLength, bondRate);
				raleigh.infMatrix[3][3] = 1.0;
				riskOfFailureStorageInf.reallocateFallsLake(fallsLakeReallocation);
				break;
			case 4:
				owasa.addDebt(year, realization, owasa.infMatrix[4][4], bondLength, bondRate);
				durham.addDebt(year, realization, durham.infMatrix[5][4], bondLength, bondRate);
				raleigh.addDebt(year, realization, raleigh.infMatrix[4][4], bondLength, bondRate);
				raleigh.infMatrix[4][3] = 1.0;
				owasa.infMatrix[4][1] = 2.0;
				durham.infMatrix[5][1] = 2.0;
				riskOfFailureStorageInf.buildWWWTPlow(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
				break;
			case 5:
				if(owasa.infMatrix[4][1]<1.0)
				{
					owasa.addDebt(year, realization, owasa.infMatrix[5][4], bondLength, bondRate);
					durham.addDebt(year, realization, durham.infMatrix[6][4], bondLength, bondRate);
					raleigh.addDebt(year, realization, raleigh.infMatrix[5][4], bondLength, bondRate);
				}
				else
				{
					owasa.addDebt(year, realization, owasa.infMatrix[5][4] - owasa.infMatrix[4][4], bondLength, bondRate);
					durham.addDebt(year, realization, durham.infMatrix[6][4] - durham.infMatrix[5][4], bondLength, bondRate);
					raleigh.addDebt(year, realization, raleigh.infMatrix[5][4] - raleigh.infMatrix[4][4], bondLength, bondRate);
				}		
				raleigh.infMatrix[5][3] = 1.0;
				durham.infMatrix[5][1] = 2.0;
				raleigh.infMatrix[4][1] = 2.0;
				owasa.infMatrix[4][1]  = 2.0;
				owasa.infMatrix[5][1] = 2.0;
				raleigh.infMatrix[5][1] = 2.0;
				riskOfFailureStorageInf.buildWWWTPhigh(owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac);
				break;
		}
	}
return;
}

void Simulation::realizationLoop()
{	
	double durhamActualInflow, owasaActualInflow, fallsActualInflow, wbActualInflow, claytonActualInflow;
	double crabtreeActualInflow, jordanActualInflow, lillingtonActualInflow,actualEvap, actualFallsEvap, actualWBEvap;
	double littleRiverRaleighActualInflow;
	int season = 1, syntheticIndex = 0; // week 1 is non-irrigation season (season = 1)
	double raleighBaselineMultiplier = 40434.0*.32*(14700.0/34700.0);
	double durham_res_supply_capacity = 6349.0;
	double cane_creek_supply_capacity = 2909.0;
	double stone_quarry_supply_capacity = 200.0;
	double university_lake_supply_capacity = 449.0;
	double lake_wheeler_benson_supply_capacity = 2789.66;
	double falls_lake_supply_capacity = 14700.0;
	double falls_lake_wq_capacity = 34700.0 - falls_lake_supply_capacity;
	double jordan_lake_supply_capacity = 14924.0;
	double jordan_lake_wq_capacity = (14924.0 - jordan_lake_supply_capacity) + 30825.0;
	double cary_treatment_capacity = 56.0;
	double durham_cary_capacity = 10.0;
	double durham_owasa_capacity = 7.0;
	double raleigh_cary_capacity = 10.8;
	double raleigh_durham_capacity = 10.0;
	double teer_quarry_supply_capacity = 1315.0;
	double teer_quarry_intake_capacity = 0.0;
	double teer_quarry_outflow_capacity = 0.0;
	double little_river_raleigh_supply_capacity = 0.0;
	double western_wake_treatment_capacity = 0.0;
	double durham_reclaimation_capacity = 0.0;
	double raleigh_quarry_capacity = 4000.0;
	double raleigh_quarry_intake_capacity = 0.0;
	double raleigh_quarry_outflow_capacity = 0.0;
	double raleigh_intake_capacity = 0.0;
	double cary_quarry_capacity = 0.0;
	double cary_quarry_intake_capacity = 0.0;
	double cary_quarry_outflow_capacity = 0.0;
	bondRate = 0.05;
	bondLength = 30;
	fallsFailurePoint = 0.2;
	fallsFailure = 0.0;
	zeroes(totalFallsFailure, terminateYear);
	maxFallsFailure = 0.0;
	
	
	for (int realization = 0; realization < numRealizations; realization++)
	{
		
		// Initialize demand and reservoir storage objects (year, month, week, daysPerWeek, leapYearCounter)
		simDates.initializeDates(4,1,1,7,0);
		
		systemStorage.initializeReservoirStorage(durham_res_supply_capacity, 
													cane_creek_supply_capacity, stone_quarry_supply_capacity,university_lake_supply_capacity, lake_wheeler_benson_supply_capacity, falls_lake_supply_capacity, falls_lake_wq_capacity,
													jordan_lake_supply_capacity, jordan_lake_wq_capacity, cary_treatment_capacity, durham_cary_capacity, durham_owasa_capacity, raleigh_cary_capacity, raleigh_durham_capacity, raleigh.jordanLakeAlloc, durham.jordanLakeAlloc, owasa.jordanLakeAlloc, cary.jordanLakeAlloc, teer_quarry_supply_capacity, teer_quarry_intake_capacity, teer_quarry_outflow_capacity, little_river_raleigh_supply_capacity, western_wake_treatment_capacity, durham_reclaimation_capacity, raleigh_quarry_capacity, raleigh_quarry_intake_capacity, raleigh_quarry_outflow_capacity, raleigh_intake_capacity, cary_quarry_capacity, cary_quarry_intake_capacity, cary_quarry_outflow_capacity, owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac); //infrastructure included in the model
		riskOfFailureStorageInf.initializeReservoirStorageROF(durham_res_supply_capacity, 
																cane_creek_supply_capacity,	stone_quarry_supply_capacity,				university_lake_supply_capacity, lake_wheeler_benson_supply_capacity,falls_lake_supply_capacity, falls_lake_wq_capacity, jordan_lake_supply_capacity, jordan_lake_wq_capacity, cary_treatment_capacity, durham_cary_capacity, durham_owasa_capacity, raleigh_cary_capacity, raleigh_durham_capacity,raleigh.jordanLakeAlloc, durham.jordanLakeAlloc, owasa.jordanLakeAlloc, cary.jordanLakeAlloc, teer_quarry_supply_capacity, teer_quarry_intake_capacity, teer_quarry_outflow_capacity, little_river_raleigh_supply_capacity, western_wake_treatment_capacity, durham_reclaimation_capacity, raleigh_quarry_capacity, raleigh_quarry_intake_capacity, raleigh_quarry_outflow_capacity, raleigh_intake_capacity, cary_quarry_capacity, cary_quarry_intake_capacity, cary_quarry_outflow_capacity, owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac); 
		riskOfFailureStorageROF.initializeReservoirStorageROF(durham_res_supply_capacity, 
																cane_creek_supply_capacity,	stone_quarry_supply_capacity,					university_lake_supply_capacity, lake_wheeler_benson_supply_capacity,			falls_lake_supply_capacity, falls_lake_wq_capacity,					jordan_lake_supply_capacity, jordan_lake_wq_capacity,					cary_treatment_capacity, durham_cary_capacity, durham_owasa_capacity, raleigh_cary_capacity, raleigh_durham_capacity, raleigh.jordanLakeAlloc, durham.jordanLakeAlloc, owasa.jordanLakeAlloc, cary.jordanLakeAlloc, teer_quarry_supply_capacity, teer_quarry_intake_capacity, teer_quarry_outflow_capacity, little_river_raleigh_supply_capacity, western_wake_treatment_capacity, durham_reclaimation_capacity, raleigh_quarry_capacity, raleigh_quarry_intake_capacity, raleigh_quarry_outflow_capacity, raleigh_intake_capacity, cary_quarry_capacity, cary_quarry_intake_capacity, cary_quarry_outflow_capacity, owasa.westernWakeTreatmentFrac, durham.westernWakeTreatmentFrac, raleigh.westernWakeTreatmentFrac); 
		
		year = simDates.getYear();//passes the dates from the simDates class to the main simulation
		month = simDates.getMonth();
		week = simDates.getWeek();
		numdays = simDates.getDays();
				
		durham.clearVariablesForRealization(year);
		owasa.clearVariablesForRealization(year);
		raleigh.clearVariablesForRealization(year);
		cary.clearVariablesForRealization(year);
		createRiskOfFailure(realization, year-1, durham.averageUse, owasa.averageUse, raleigh.averageUse, cary.averageUse);
		durham.priceInsurance(year, realization);
		owasa.priceInsurance(year, realization);
		raleigh.priceInsurance(year, realization);
		cary.priceInsurance(year, realization);
		int caryWTPcounter = 0;
		double durhamReclaimedCap = durham_reclaimation_capacity;
		double durhamTreatmentCap = western_wake_treatment_capacity*durham.westernWakeTreatmentFrac;
		double owasaTreatmentCap = western_wake_treatment_capacity*owasa.westernWakeTreatmentFrac;
		double raleighTreatmentCap = western_wake_treatment_capacity*raleigh.westernWakeTreatmentFrac;
		double raleighIntakeCap = raleigh_intake_capacity;
		while (year-1<(terminateYear))
		{
			syntheticIndex = (year-1)*52+week-1;
			
			// update on/off triggers for each restriction stage
			if (week > 16 && week < 39) // Irrigation season
				season = 0;
			else
				season = 1;
				
			durham.fillRestrictionsArray(season);
			owasa.fillRestrictionsArray(season);
			cary.fillRestrictionsArray(season);
			raleigh.fillRestrictionsArray(season);
			
			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			
			// Finding OWASA's current risk of failure (for restrictions, ROF calcs for transfers are performed later for all utilities)
			owasaLevel = int(volumeIncrements*owasa.storageFraction);
			if (owasaLevel > 19) owasaLevel = 19;
			else if (owasaLevel < 0) owasaLevel = 0;
			owasa.ROF_res = owasa.riskOfFailure[owasaLevel][week-1];
			
			durhamLevel = int(volumeIncrements*durham.storageFraction);
			if (durhamLevel > 19) durhamLevel = 19;
			else if (durhamLevel < 0) durhamLevel = 0;
			durham.ROF_res = durham.riskOfFailure[durhamLevel][week-1];
			
			raleighLevel = int(volumeIncrements*raleigh.storageFraction);
			if (raleighLevel > 19) raleighLevel = 19;
			else if (raleighLevel < 0) raleighLevel = 0;
			raleigh.ROF_res = raleigh.riskOfFailure[raleighLevel][week-1];
			
			caryLevel = int(volumeIncrements*cary.storageFraction);
			if (caryLevel > 19) caryLevel = 19;
			else if (caryLevel < 0) caryLevel = 0;
			cary.ROF_res = cary.riskOfFailure[caryLevel][week-1];
			

			// Use the demand PDF to estimate uncertain demand, also finds current level of restrictions (and revenue losses from them)
			// Raleigh uses Durham's data for the inflow-demand PDF
			durham.calculateDemand(realization, week, numdays);
			owasa.calculateDemand(realization, week, numdays);
			cary.calculateDemand(realization, week, numdays);
			raleigh.calculateDemand(realization, week, numdays);
			
			durham.calculateRestrictions(year, week, numdays, month, realization);
			owasa.calculateRestrictions(year, week, numdays, month, realization);
			cary.calculateRestrictions(year, week, numdays, month, realization);
			raleigh.calculateRestrictions(year, week, numdays, month, realization);
			
			systemStorage.setDemands(durham.weeklyDemand, owasa.weeklyDemand, raleigh.weeklyDemand, cary.weeklyDemand, numdays);//pass weekly demand values to reservoir storage update function
			
			//Translate inflows and evap from weekly variations to a physical volume
			durhamActualInflow = durhamInflows.synthetic[realization][syntheticIndex];
			owasaActualInflow = owasaInflows.synthetic[realization][syntheticIndex];
			fallsActualInflow = fallsInflows.synthetic[realization][syntheticIndex];
			wbActualInflow = wheelerInflows.synthetic[realization][syntheticIndex];
			claytonActualInflow = claytonInflows.synthetic[realization][syntheticIndex];
			crabtreeActualInflow = crabtreeInflows.synthetic[realization][syntheticIndex];
			jordanActualInflow = jordanInflows.synthetic[realization][syntheticIndex];
			lillingtonActualInflow = lillingtonInflows.synthetic[realization][syntheticIndex];
			littleRiverRaleighActualInflow = littleRiverRaleighInflows.synthetic[realization][syntheticIndex];
		
			actualEvap = durhamOwasaEvap.synthetic[realization][syntheticIndex];
			actualFallsEvap = fallsEvap.synthetic[realization][syntheticIndex];
			actualWBEvap = wheelerEvap.synthetic[realization][syntheticIndex];
			
			//Pass along inflow values
			systemStorage.setInflow(durhamActualInflow, 31.4*owasaActualInflow,
				28.7*owasaActualInflow, 1.2*owasaActualInflow, fallsActualInflow, wbActualInflow, claytonActualInflow, crabtreeActualInflow, jordanActualInflow, lillingtonActualInflow,
							raleigh.weeklyDemand*returnRatio[1][week-1], durham.weeklyDemand*returnRatio[0][week-1], durham.weeklyDemand*(returnRatio[1][week-1]-returnRatio[0][week-1]),
							owasa.weeklyDemand*returnRatio[1][week-1],actualFallsEvap, actualWBEvap, actualEvap, littleRiverRaleighActualInflow);
							
			if(formulation > 0) 
			{
				durham.calcTransferTriggers(week);
				owasa.calcTransferTriggers(week);
				raleigh.calcTransferTriggers(week);
				durham.TTrigger = durham.weeklyTransferTriggers;
				owasa.TTrigger = owasa.weeklyTransferTriggers;
				raleigh.TTrigger = raleigh.weeklyTransferTriggers;
				
				//Transfer requests are granted based on the limitations of infrastructure
				systemStorage.calcTransfers(durham.TTrigger,durham.ROF_res, owasa.TTrigger, owasa.ROF_res, raleigh.TTrigger, raleigh.ROF_res, owasa.weeklyDemand);
			}
			//Update reservoir storage levels
			systemStorage.setSpillover(week-1);
			systemStorage.updateStorage(week-1);
			//Retrieve the weekly transfers
			durham.weeklyTransferVolume = systemStorage.getDurhamTransfers();
			owasa.weeklyTransferVolume = systemStorage.getOWASATransfers();
			raleigh.weeklyTransferVolume = systemStorage.getRaleighTransfers();
			// Use the mitigation fund to calculate water transfer payments (sent to Cary)
			if(durham.weeklyTransferVolume>0.0)
			{
				durham.Fund.subtract(durham.weeklyTransferVolume*transferCosts);
			}
			if(owasa.weeklyTransferVolume>0.0)
			{
				owasa.Fund.subtract(owasa.weeklyTransferVolume*transferCosts);
			}
			if(raleigh.weeklyTransferVolume>0.0)
			{
				raleigh.Fund.subtract(raleigh.weeklyTransferVolume*transferCosts);
			}
			if((durham.weeklyTransferVolume+owasa.weeklyTransferVolume+raleigh.weeklyTransferVolume)>0.0)
			{
				cary.Fund.add((durham.weeklyTransferVolume + owasa.weeklyTransferVolume + raleigh.weeklyTransferVolume)*transferCosts);
			}
			// Get current weekly demand baseline values for each utility
			durhamFlowWeekBaseline = durham.demandBaseline[year-1][week-1] - durhamReclaimedInsuranceTrigger*durhamReclaimedCap*numdays - wwWTPInsuranceTrigger*durhamTreatmentCap*numdays;
			owasaFlowWeekBaseline = owasa.demandBaseline[year-1][week-1]  - wwWTPInsuranceTrigger*owasaTreatmentCap*numdays;
			raleighFlowWeekBaseline = raleigh.demandBaseline[year-1][week-1] + fallsEvap.averages[week-1]*raleigh.Fund.insuranceStorage*raleighBaselineMultiplier - wwWTPInsuranceTrigger*raleighTreatmentCap*numdays - ralIntakeInsuranceTrigger*raleighIntakeCap*numdays;
			caryFlowWeekBaseline = cary.demandBaseline[year-1][week-1];
			owasaLevelIns = int(volumeIncrements*owasa.Fund.insuranceStorage);
			if (owasaLevelIns>19)
			{
				owasaLevelIns = 19;
			}
			owasa.ROF_resIns = owasa.riskOfFailure[owasaLevelIns][week-1];

			raleighLevelIns = int(volumeIncrements*raleigh.Fund.insuranceStorage);
			if(raleighLevelIns >19)
			{
				raleighLevelIns = 19;
			}
			raleigh.ROF_resIns = raleigh.riskOfFailure[raleighLevelIns][week-1];
				
			durhamLevelIns = int(volumeIncrements*durham.Fund.insuranceStorage);
			if(durhamLevelIns >19)
			{
				durhamLevelIns = 19;
			}
			durham.ROF_resIns = durham.riskOfFailure[durhamLevelIns][week-1];
				
			caryLevelIns = int(volumeIncrements*cary.Fund.insuranceStorage);
			if(caryLevelIns >19)
			{
				caryLevelIns = 19;
			}
			cary.ROF_resIns = cary.riskOfFailure[caryLevelIns][week-1];
			durhamSpill = systemStorage.getDurhamSpillage();
			OWASASpill = systemStorage.getOWASASpillage();
			insuranceFallsInflow = (fallsActualInflow + durhamSpill + durham.weeklyDemand*returnRatio[0][week-1]-systemStorage.fallsArea*actualFallsEvap)*(14700.0/34700.0) + raleigh.weeklyTransferVolume;
			insuranceJordanInflow = (OWASASpill + owasa.weeklyDemand*returnRatio[1][week-1] + durham.weeklyDemand*(returnRatio[1][week-1] - returnRatio[0][week-1]) +
					jordanActualInflow-actualEvap*13900)*cary.jordanLakeAlloc*(45800.0/(94600.0+45800.0));
				
				//Determine insurance payout (goes directly to the mitigation fund)
			durham.setInsurancePayment(durhamFlowWeekBaseline, durhamActualInflow + durham.weeklyTransferVolume);
			owasa.setInsurancePayment(owasaFlowWeekBaseline, owasaActualInflow*61.3 + owasa.weeklyTransferVolume);
			raleigh.setInsurancePayment(raleighFlowWeekBaseline, insuranceFallsInflow + littleRiverRalInsuranceTrigger*littleRiverRaleighActualInflow);
			cary.setInsurancePayment(caryFlowWeekBaseline, insuranceJordanInflow);
						
			//retrieve updated storage levels
			owasa.storageFraction = systemStorage.getOWASAStorage();
			durham.storageFraction = systemStorage.getDurhamStorage();
			raleigh.storageFraction = systemStorage.getRaleighStorage();
			cary.storageFraction = systemStorage.getCaryStorage();
			//cout << "Realization " << realization << ", Week " << week << ": (D) " << durham.storageFraction << " (O) " << owasa.storageFraction << " (R) " << raleigh.storageFraction << " (C) " << cary.storageFraction << endl;
			
			//update timestep
			simDates.increase();
			year = simDates.getYear();
			month = simDates.getMonth();
			week = simDates.getWeek();
			numdays = simDates.getDays();
			durham.weeklyUpdate();
			owasa.weeklyUpdate();
			raleigh.weeklyUpdate();
			cary.weeklyUpdate();
			updateFallsQuality();
			if (week == 1)
			{
				createInfrastructure();
				createInfrastructureRisk(realization, year-2, durham.averageUse, owasa.averageUse, raleigh.averageUse, cary.averageUse);
				triggerInfrastructure(realization);
				createRiskOfFailure(realization, year-2, durham.averageUse, owasa.averageUse, raleigh.averageUse, cary.averageUse);
				durham.annualUpdate(year-1, realization);
				owasa.annualUpdate(year-1, realization);
				cary.annualUpdate(year-1, realization);
				raleigh.annualUpdate(year-1, realization);

				//Upgrade Cary WTP to 56 MGD in 2016
				if (caryUpgrades[caryWTPcounter]<cary.averageUse)
				{
					systemStorage.upgradeCaryTreatmentPlant(caryWTPcounter);
					caryWTPcounter++;
				}
				totalFallsFailure[year-2] = fallsFailure;
				fallsFailure = 0.0;
				durhamReclaimedCap = systemStorage.getDurhamReclaimedCap();
				durhamTreatmentCap = systemStorage.getDurhamTreatment();
				owasaTreatmentCap = systemStorage.getOWASATreatment();
				raleighTreatmentCap = systemStorage.getRaleighTreatment();
				raleighIntakeCap = systemStorage.getRaleighIntake();

				
			}
			
		} // End weekly loop
		// Saving the average annual cost from each 15-year realization
		
	} //end realization loop
	durham.calculateObjectives();
	owasa.calculateObjectives();
	cary.calculateObjectives();
	raleigh.calculateObjectives();
	for(int x = 0; x< terminateYear; x++)
	{
		if(totalFallsFailure[x]>maxFallsFailure)
		{
			maxFallsFailure = totalFallsFailure[x];
		}
	}
	
	return;
}
void Simulation::updateFallsQuality()
{
	double qualityStorage = 0.0;
	qualityStorage = systemStorage.getFallsQuality();
	if(qualityStorage < fallsFailurePoint)
	{
		fallsFailure = 1.0/double(numRealizations);
	}
	return;
}
Simulation::~Simulation()
{
	zap(xreal);
}

