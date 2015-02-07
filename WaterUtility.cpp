#include "global.h"
#include "WaterUtility.h"

using namespace std;

WaterUtility::WaterUtility()
{
}

WaterUtility::~WaterUtility()
{
	zap(costPerMG, numMonths*numFutureYears);
	zap(consumerSurplusPerMG, numMonths);
	zap(reductions, numMonths);
	zap(insurancePayment);
	zap(demandBaseline, terminateYear);
	zap(individualReductions, numAnnualDecisionPeriods);
	zap(restrictionsOnTemplate, 2);
	zap(restrictionsOffTemplate, 2);
	zap(restrictionsOnTemplateROF);
	zap(restrictionsOffTemplateROF);
	zap(restrictionsOn);
	zap(restrictionsOff);
	zap(fractionPricing, numStages);
	zap(totalFailures);
	zap(elasticityIndex);
	zap(PDFIrr, 16); zap(PDFNon, 16);
	zap(CDFIrr, 16); zap(CDFNon, 16);
	zap(demandVariation, numRealizations);
	zap(weeklyTransferTriggers, terminateYear);
	zap(waterPrice_Irr);
	zap(waterPrice_NonIrr);
	zap(waterPrice);
	zap(droughtSurcharges, numTiers+1);
	zap(inflows1DIrr); zap(inflows1DNon);
	zap(demand1DIrr); zap(demand1DNon);
	zap(inflows2DIrr, 78); zap(inflows2DNon, 78);
	zap(demand2DIrr, 18); zap(demand2DNon, 18);
	zap(actualPDF, 16);
	zap(riskOfFailure, 20);
}

void WaterUtility::configure(int nmonths, int nyears, int ntypes, int ntiers, int nstages, int nfutureyears, double failure, int nannualdecisionperiods, int termyear, 
		int volumeIncrements, int nrealizations, int formulation)
{
	numMonths = nmonths;
	numYears = nyears;
	numTypes = ntypes;
	numTiers = ntiers;
	numStages = nstages;
	numAnnualDecisionPeriods = nannualdecisionperiods;
	failurePoint = failure;
	numFutureYears = nfutureyears;
	restrictionStage = 0;
	restrictionReduction = 1;
	currentPDFIndex = -1;
	ROF_resIns = 0;
	terminateYear = termyear;
	numRealizations = nrealizations;
	restrictionIncrement = 1.0/(52.0*double(terminateYear)*double(numRealizations));
	
	// Initialize pointers to null
	waterPrice = NULL; waterPrice_Irr = NULL; waterPrice_NonIrr = NULL;
	droughtSurcharges = NULL;
	weeklyTransferTriggers = NULL;
	inflows1DIrr = NULL, inflows1DNon = NULL, demand1DIrr = NULL, demand1DNon = NULL;
	inflows2DIrr = NULL, inflows2DNon = NULL, demand2DIrr = NULL, demand2DNon = NULL;
	
	// Allocating space for average water price, consumer surplus losses, total restriction reductions, and insurance payments for each stage
	general_2d_allocate(costPerMG, numMonths*nfutureyears, numStages, 0.0);
	general_2d_allocate(consumerSurplusPerMG, numMonths, numStages, 0.0);
	general_2d_allocate(reductions, numMonths, numStages, 0.0);
	general_1d_allocate(insurancePayment, numStages, 0.0);
	general_2d_allocate(demandBaseline, terminateYear, 52, 0.0);
	general_2d_allocate(weeklyTransferTriggers, terminateYear, 52, 0.0);
	
	// total utility water use reduction targets and seasonal restriction triggers
	general_2d_allocate(individualReductions, numAnnualDecisionPeriods, numStages);
	general_2d_allocate(restrictionsOnTemplate, 2, numStages);
	general_2d_allocate(restrictionsOffTemplate, 2, numStages);
	general_1d_allocate(restrictionsOnTemplateROF, numStages);
	general_1d_allocate(restrictionsOffTemplateROF, numStages);
	general_1d_allocate(restrictionsOn, numStages, 0.0);
	general_1d_allocate(restrictionsOff, numStages, 0.0);
	
	// drought surcharges and total failures
	general_2d_allocate(fractionPricing, numStages, numTiers);
	general_1d_allocate(totalFailures, terminateYear, 0.0);
	
	// elasticity indices for each usage tier
	general_1d_allocate(elasticityIndex, numTiers, 0);
	
	general_2d_allocate(demandVariation, numRealizations, 52*terminateYear, 0.0);
	general_2d_allocate(actualPDF, 16, 17, 0.0);
	
	general_2d_allocate(riskOfFailure, 20, 52, 0.0);
	
	if(formulation == 0)
		usesROF = false;
	else
		usesROF = true;
	
}

void WaterUtility::setCapacity(double cap)
{
	Fund.capacity = cap;
}

void WaterUtility::writeInflowDemandPDF(int inflowR, int UDr, int rows_PDF, int cols_PDF, double size1, double size2, int irrC, int nonC1, int nonC2, TimeSeriesData* Inflows)
{
	// Local variables intermediate calculations
	
	if(shouldAllocate)
	{
		general_1d_allocate(inflows1DIrr, UDr*irrC);
		general_1d_allocate(inflows1DNon, UDr*(nonC1+nonC2));
		general_1d_allocate(demand1DIrr, UDr*irrC);
		general_1d_allocate(demand1DNon, UDr*(nonC1+nonC2));
		
		general_2d_allocate(inflows2DIrr, inflowR, irrC);
		general_2d_allocate(inflows2DNon, inflowR, nonC1+nonC2);
		general_2d_allocate(demand2DIrr, UDr, irrC);
		general_2d_allocate(demand2DNon, UDr, nonC1+nonC2);
	}
	
	for(int y = 0; y < irrC; y++)
	{
		for (int x = 0; x < inflowR; x++)
			inflows2DIrr[x][y] = Inflows->normalizedDataAdj[x][y+16];
		
		for(int x = 0; x <UDr; x++)
			demand2DIrr[x][y] = UD.normalizedData[x][y+16];
	}
	for (int y = 0; y < nonC1; y++)
	{
		for (int x = 0; x < inflowR; x++)
			inflows2DNon[x][y] = Inflows->normalizedDataAdj[x][y];
		
		for (int x = 0; x < UDr; x++)
			demand2DNon[x][y] = UD.normalizedData[x][y];
	}
	for (int y = 0; y < nonC2; y++)
	{
		for (int x = 0; x < inflowR; x++)
			inflows2DNon[x][y+16] = Inflows->normalizedDataAdj[x][y+39];
		
		for (int x = 0; x < UDr; x++)
			demand2DNon[x][y+16] = UD.normalizedData[x][y+39];
	}
	index_array(inflows1DIrr, UDr, irrC, inflowR - UDr, inflows2DIrr);
	index_array(inflows1DNon, UDr, nonC1+nonC2, inflowR - UDr, inflows2DNon);
	index_array(demand1DIrr, UDr, irrC, 0, demand2DIrr);
	index_array(demand1DNon, UDr, nonC1+nonC2, 0, demand2DNon);
	
	if(shouldAllocate)
	{
		general_2d_allocate(PDFIrr, rows_PDF, cols_PDF, 0);
		general_2d_allocate(PDFNon, rows_PDF, cols_PDF, 0);
		general_2d_allocate(CDFIrr, rows_PDF, cols_PDF, 0);
		general_2d_allocate(CDFNon, rows_PDF, cols_PDF, 0);
	}
	
	make_PDF(inflows1DIrr, demand1DIrr, UDr*irrC, PDFIrr, size1, size2);
	make_PDF(inflows1DNon, demand1DNon, UDr*(nonC1+nonC2), PDFNon, size1, size2);
	// Calculate cumulative sums up front (saves time in the main loop)
	int tempSumIrr, tempSumNon;
	for(int i = 0; i < rows_PDF; i++)
	{	
		tempSumIrr = 0; tempSumNon = 0;
		for(int j = 0; j < cols_PDF; j++)
		{
			tempSumIrr += PDFIrr[i][j];
			CDFIrr[i][j] = tempSumIrr;
			tempSumNon += PDFNon[i][j];
			CDFNon[i][j] = tempSumNon;
		}
	}
}

void WaterUtility::setInsurancePayment(double demandBaseline, double inflows, int numRealizations, int terminateYear, int numdays, int week, int month, int year)
{
	int insuranceStage;
	double currentPayment;
	int timeIndex = month-1+(year-1)*12;
	
	insuranceStage = Fund.getInsuranceStage(demandBaseline, inflows, restrictionsOn, restrictionsOff, usesROF, ROF_resIns);
	
	currentPayment = UD.averages[week-1]*numdays*averageUse*(costPerMG[timeIndex][0] - costPerMG[timeIndex][insuranceStage]*reductions[month-1][insuranceStage]);
	
	Fund.setInsurancePayment(currentPayment, insuranceUse);
	insuranceCost += Fund.insurancePayment*insurancePremium/(double(numRealizations)*double(terminateYear));
}

void WaterUtility::generateDemandVariation(int numRealizations, int numWeeks, int numYears, TimeSeriesData *Inflows, double demand_variation_multiplier)
{
	
	int totalNumber, partialNumber, counter, index;
	double demandLevel;
	counter = 0;
	partialNumber = 0;
	
	for (int realization = 0; realization < numRealizations; realization++)
	{
		for (int years = 0; years<numYears;years++)
		{
			for (int week = 0; week < numWeeks; week++)
			{	
				index = getPDFIndex(Inflows->simulatedData[realization][week+years*52]); // "inflow" index of the inflow-demand PDF
				if (week > 16 && week < 39)
					totalNumber = PDFIrr[index][16] - 1; // this is the total number of historical streamflows that fall into the current streamflow PDF 'bin'
				else
					totalNumber = PDFNon[index][16] - 1;
					
				if(totalNumber == -1)
					demandLevel = 4;
				else 
				{
					if(totalNumber == 0)
						totalNumber = 1;
						
					partialNumber = rand() % totalNumber + 1; // generate a random demand for this week
					counter = 0;
					
					// Find the first demand level greater than or equal to partialNumber
					if (week > 16 && week < 39)
						while (CDFIrr[index][counter] < partialNumber)
							counter++;
					else
						while (CDFNon[index][counter] < partialNumber)
							counter++;
					
					demandLevel = double(counter);
					
					if (demandLevel > 15) 
						demandLevel = 15;
				}
				
				demandVariation[realization][week+years*52] = ((demandLevel - 8.0)/2.0 + double(rand() % 501)/1000.0)*demand_variation_multiplier;
				if (week>16 && week<39)
				{
					actualPDF[index][counter]+=1;
					actualPDF[index][16] +=1;
				}
				
			}
		}
		
	}
}
void WaterUtility::calculateDemand(int realization, int week, int numdays)
{
	weeklyDemand = numdays*averageUse*(demandVariation[realization][week-1]*UD.standardDeviations[week-1] + UD.averages[week-1]);
}
void WaterUtility::calculateRestrictions(int year, int week, int numdays, int month, int realization)
{
	double baselineCostPerMG, currentCostPerMG;
	
	if(usesROF) // use ROF for owasa and raleigh
	{
		if (restrictionsOn[restrictionStage] < ROF_res) // as long as current storage is below the restriction triggers for a given week, increase stage
			restrictionStage += 1;
		while (restrictionsOff[restrictionStage] > ROF_res) // vice versa for removing restriction stages
			restrictionStage -= 1;
	}
	else
	{
		if (restrictionsOn[restrictionStage] > storageFraction) // as long as current storage is below the restriction triggers for a given week, increase stage
			restrictionStage += 1;
		while (restrictionsOff[restrictionStage] < storageFraction) // vice versa for removing restriction stages
			restrictionStage -= 1;
	}
	
	//calculate restricted and unrestricted demand
	weeklyDemand = numdays*averageUse*(demandVariation[realization][week-1]*UD.standardDeviations[week-1] + UD.averages[week-1]);
	
	baselineCostPerMG = costPerMG[month-1+(year-1)*12][0];
	
	//'Expected' revenue comes from unrestricted demand
	annualRevenue += weeklyDemand*baselineCostPerMG;
	
	// If restrictions are implemented, calculate lost revenue and consumer surplus, update demand, and tally restrictions
	if (restrictionStage != 0)
	{
		//sets the reductions in water use due to water use restrictions
		restrictionReduction = reductions[month-1][restrictionStage];
		restrictedDemand = weeklyDemand*restrictionReduction;
		currentCostPerMG = costPerMG[month-1+(year-1)*12][restrictionStage];
		consumerSurplusLoss = ((restrictedDemand*currentCostPerMG) / (weeklyDemand*baselineCostPerMG)) - 1;
		consumerCount += 1;
		
		// Revenues lost due to restrictions
		demandDeficit = weeklyDemand*baselineCostPerMG - restrictedDemand*currentCostPerMG;
		weeklyDemand = restrictedDemand; // update actual demand under restrictions
		restrictions += restrictionIncrement; // tally restriction frequency
		Fund.subtract(demandDeficit); // dip into the insurance fund to cover demand deficit
	}
	else
	{
		demandDeficit = 0;
		consumerSurplusLoss = 0;
	}
}

void WaterUtility::calcTransferTriggers(int volumeIncrements, int terminateYear)
{	
	int transferLevel = volumeIncrements-1;
	double ROF = 0;	
	
	//Calculating risk-of-failure for transfers - need to find the storage level corresponding with the 'acceptable' risk-of-failure.  Once that storage level is calculated,
	//transfers will be requested up to that point by the utilities.  Storage requests can only be granted to the limits of transfer infrastructure
	
	for(int year = 0; year < terminateYear; year++)
	{
		for(int week = 0; week < 52; week++)
		{	
			//Triggers are translated into a reservoir storage level
			if (TTriggerN == 2)
				weeklyTransferTriggers[year][week] = -1;
			
			else
			{	
				transferLevel = volumeIncrements - 1;
				ROF = 0;
				
				while(ROF < TTriggerI && transferLevel > -1)//Runs through the discritizations of reservoir storage to find the first 'bin' in the given week that has
				{													//a risk-of-failure larger than the transfer trigger
					ROF = riskOfFailure[transferLevel][week];
					transferLevel--;
				}

				weeklyTransferTriggers[year][week] = double(transferLevel + 1)/double(volumeIncrements);
			}
		}
	}
}

void WaterUtility::clearVariablesForSimulation(int numRealizations, int terminateYear)
{  
	//Initial values for objective variables
	restrictionFreq=0; freqLosses = 0;
	maxFailures = 0; totalCosts = 0;
	totalLosses = 0; consumerFractions = 0;
	week_of_last_restrictions = 0;
	
	// Other variables to reset before running the realizations
	insuranceCost = 0; averageRevenue = 0;
	simLosses = 0;
	maxLosses = 0;
	
	annualCosts.clear();
	for (int x = 0; x < numRealizations; x++)
		annualCosts.push_back(0.0);
	
	zeroes(totalFailures, terminateYear); // keep track of failures per year
	
	//Variables to calculate losses that occur to consumers and water utilities, respectively
	totalConsumerLoss = 0;
	totalUtilityLoss = 0;
	yearlyFailure = 0;
	restrictions = 0;
	consumerCount = 0;
}

void WaterUtility::clearVariablesForRealization(int year)
{
	averageUse = futureDemand[year-1];
	restrictionStage = 0;
	storageFraction = 1;
	annualRevenue = 0;
	restrictions = 0;
	freqLossesint = 0;	
	maxLosses = 0;
	week_of_last_restrictions = 0;
	Fund.initializeRealization();
}

void WaterUtility::fillRestrictionsArray(int season)
{
	if (usesROF)
	{
		for (int stage = 0; stage < numStages; stage++)
		{
			restrictionsOn[stage] = restrictionsOnTemplateROF[stage];
			restrictionsOff[stage] = restrictionsOffTemplateROF[stage];
		}
	}
	else
	{
		for (int stage = 0; stage < numStages; stage++)
		{
			restrictionsOn[stage] = restrictionsOnTemplate[season][stage];
			restrictionsOff[stage] = restrictionsOffTemplate[season][stage];
		}
	}
}

int WaterUtility::getPDFIndex(double inflow)
{
	double counter = -1.0;
	int counterAsInt = -1;
	int loopTrig = 0;
	while (loopTrig == 0)//Finding the inflow condition for joint demand/inflow PDF, owasa
	{
		counter = counter + 1;
		counterAsInt = counterAsInt + 1;
		if (inflow < ((counter/2) - 3.5))
			loopTrig = 1;
	}
	
	if (counterAsInt > 15)//only 16 discritizations to joint PDF
		counterAsInt = 15;
		
	return counterAsInt;
}

void WaterUtility::annualUpdate(int year, int numRealizations, int terminateYear)
{
	bool addInterest = true;
	double thisYearLosses = 0;
	
		
	Fund.add(annualRevenue*annualPayment);
	thisYearLosses = Fund.calcAnnualLosses(annualRevenue, addInterest);
	simLosses += thisYearLosses;
	if(thisYearLosses>maxLosses)
	{
		maxLosses = thisYearLosses;
	}
	
	averageUse = futureDemand[year];
	averageRevenue += annualRevenue/(double(numRealizations)*double(terminateYear));
	annualRevenue = 0;
	
	if (restrictions != 0)
		restrictionFreq += 1.0/(double(numRealizations)*double(terminateYear));
	if (yearlyFailure != 0)
		totalFailures[year-1] += 1/(double(numRealizations));
	
	restrictions = 0;
	yearlyFailure = 0;	
	freqLossesint = 0;
}

void WaterUtility::weeklyUpdate(int numRealizations)
{
	//determine and keep track of reservoir failures
	if (storageFraction < failurePoint)
		yearlyFailure += 1/(52*double(numRealizations));

	//Tallying total transfer volume (resets every year)
	freqLossesint += weeklyTransferVolume;
	
	//Estimate consumer and utility losses to restrictions
	totalUtilityLoss += demandDeficit;
	totalConsumerLoss += consumerSurplusLoss;
}

void WaterUtility::calculateObjectives(int terminateYear, int numRealizations)
{
	int costRiskLevel = int(numRealizations*.99);
	
	//Highest annual failure rate during any one year of the simulation
	for (int year = 0; year < terminateYear; year++)
		if (totalFailures[year] > maxFailures)
			maxFailures = totalFailures[year];
	
	//Average utility costs, including mitigation costs 
	totalCosts = simLosses/(double(numRealizations)*double(terminateYear));
	
	//Adding the mitigation costs to total costs
	double addFunds = (insuranceCost + annualPayment*averageRevenue)/averageRevenue;
	totalCosts += addFunds;
	
	//Worst case scenario (95% VAR) mitigation costs
 	sort(annualCosts.begin(), annualCosts.end());
	totalLosses = annualCosts[costRiskLevel] + addFunds - totalCosts;
	
	//Percent of the financial burden of restrictions felt by the consumer
	if (consumerCount!=0)
	   consumerFractions = totalConsumerLoss/consumerCount;
	else
	   consumerFractions = 0;
}

void WaterUtility::calcWaterPrice(double elasticity_Of_Demand[])
{	
	// Calculate water use and revenue from each usage type/tier	
	// This function is a bit messy because different utilities
	// have different price calculations
	
	double usage[numStages][numMonths][numTiers];
	double totalUsage[numStages][numMonths];
	
	//Constants for determining proper reductions to each individual consumer type
	double reductionCoefficient[numAnnualDecisionPeriods][numStages];
	double meanUse[numAnnualDecisionPeriods][numStages];
	double **ssq;
	double ssqThreshold = .0001;
	int numberOfIterations = 0;
	int maxIterations = 1000;
	
	int monthsPerDecisionPeriod = numMonths;
	if(numAnnualDecisionPeriods == 2)
		monthsPerDecisionPeriod = 6;
	
	double currentUseValue;
	int durhamTierAdd = 0;
	int season = 0; // 0 = irrigation, 1 = non-irrigation
	
	general_2d_allocate(ssq, numAnnualDecisionPeriods, numStages);
	
	for(int i = 0; i < numAnnualDecisionPeriods; i++)
	{
		for(int j = 0; j < numStages; j++)
		{	
			reductionCoefficient[i][j] = 1 - 0.1*j;
			meanUse[i][j] = 0;
			ssq[i][j] = ssqThreshold*100;
		}
	}
	
	// For OWASA, need to re-calculate the use distribution based on current prices
	// These variables will only be used for OWASA, but they need to be declared unconditionally
	double kValues[numMonths][numTypes];
	double updatedUsage[numStages][numMonths][numTypes];
	if(name.compare("OWASA") == 0)
	{	
		for(int month = 0; month < numMonths; month++)
		{
			for(int tier = 0; tier < numTiers; tier++)
			{
				if(month < 4 || month >= 9)
				{
					//Calculate k Value using equation Q = kP^-e with current prices and water use.
					kValues[month][tier] = useFractions[month][tier]/(pow((waterPrice_NonIrr[tier] + waterPrice_NonIrr[8]*.9), elasticity_Of_Demand[elasticityIndex[tier]]));
					for(int i = 0; i < numStages; i++)
						updatedUsage[i][month][tier] = kValues[month][tier]*pow(((waterPrice_NonIrr[tier]*droughtSurcharges[tier][i] + waterPrice_NonIrr[8]*.9)), elasticity_Of_Demand[elasticityIndex[tier]]);
				}
				else
				{
					kValues[month][tier] = useFractions[month][tier]/(pow((waterPrice_Irr[tier] + waterPrice_Irr[8]*.9), elasticity_Of_Demand[elasticityIndex[tier]]));
					for(int i = 0; i < numStages; i++)
						updatedUsage[i][month][tier] = kValues[month][tier]*pow(((waterPrice_Irr[tier]*droughtSurcharges[tier][i] + waterPrice_Irr[8]*.9)), elasticity_Of_Demand[elasticityIndex[tier]]);
				}
			}
		}
	}
	
	//Iterative solver used to determine the size of reductions to water use for individual user types based on overall reduction targets
	while (anyThresholdExceeded(ssqThreshold, ssq) && numberOfIterations < maxIterations)
	{
		// Reset mean use every iteration
		for(int i = 0; i < numAnnualDecisionPeriods; i++)
			for(int j = 0; j < numStages; j++)
				meanUse[i][j] = 0;
		
		for (int month = 0; month < numMonths; month++)
		{
			if(numAnnualDecisionPeriods == 2 && (month < 4 || month > 9))
				season = 1;
			else
				season = 0;
			
			// Calculate usage at all levels for this restriction pricing
			// (Depends on the utility)
			
			if(name.compare("Durham") == 0)
			{
				for (int tier = 0; tier < 5; tier++)
				{
					// Clear sums from previous iteration
					for(int stage = 0; stage < numStages; stage++)
					{
						usage[stage][month][tier]= 0;
						usage[stage][month][tier + 5]= 0;
					}
					
					// Calculate sums for the current iteration
					for (int year = 0; year < numYears; year++)
					{
						for (int type = 0; type < numTypes; type++)
						{	
							if (type == 0 || type == 6 || type == 4 || type == 10)//residential types
								durhamTierAdd = 0;
							else
								durhamTierAdd = 5;
							
							currentUseValue = usePatterns[month + type*12 + year*12*13][tier];
							
							if (type !=0 && type != 4 && type !=12) //types which do not recieve reductions
							{
								for(int i = 0; i < 4; i++)
									usage[i][month][tier + durhamTierAdd] += currentUseValue*reductionCoefficient[season][i];

								if (type < 7) //use types allowed (at reduced levels) during emergency restrictions. Use from all other types = 0.
									usage[4][month][tier + durhamTierAdd] += currentUseValue*reductionCoefficient[season][4];
							}
							else // if no reductions
							{
								for(int i = 0; i < numStages; i++)
									usage[i][month][tier + durhamTierAdd] += currentUseValue;
							}
						}
					}
				}
			}
			else if(name.compare("Cary") == 0)
			{
				for (int tier = 0; tier < numTiers; tier++)
				{
					for(int stage = 0; stage < numStages; stage++)
						usage[stage][month][tier]= 0;
					
					currentUseValue = usePatterns[month][tier];
					usage[0][month][tier] += currentUseValue;
					
					if(tier > 5) // irrigation uses: reduced in stage 1 & 2, not allowed in stage 3 & 4
					{
						for(int i = 1; i < 3; i++)
							usage[i][month][tier] += currentUseValue*reductionCoefficient[season][i];
					}
					else if(tier == 5) // commercial/industrial use, reduced in all stages
					{
						for(int i = 1; i < numStages; i++)
							usage[i][month][tier] += currentUseValue*reductionCoefficient[season][i];
					}
					else if(tier == 4) // no changes to mulit-family residential use in stage 1 and 2, reduced use in stages 3 & 4
					{
						for(int i = 1; i < 3; i++)
							usage[i][month][tier] += currentUseValue;
						for(int i = 3; i < 5; i++)
							usage[i][month][tier] += currentUseValue*reductionCoefficient[season][i];
					}
					else if(tier > 0 && tier < 4)// no changes to residential use in stage 1 and 2, in stages 3 and 4 no use past tier 1 use levels
					{
						for(int i = 1; i < 3; i++)
							usage[i][month][tier] += currentUseValue;
					}
					else // if tier == 0
					{
						for(int i = 1; i < 4; i++)
							usage[i][month][tier] += currentUseValue;
						usage[4][month][tier] += currentUseValue*.85;
					}
				}
			}
			else if(name.compare("Raleigh") == 0)
			{
				for (int tier = 0; tier < numTiers; tier++)
				{
					for(int stage = 0; stage < numStages; stage++)
						usage[stage][month][tier] = 0;
					
					currentUseValue = useFractions[month][tier];
					usage[0][month][tier] += currentUseValue;
					
					if ((tier >= 1 && tier <= 3) || (tier >= 13 && tier <= 15)) //restriction-applicable user types
					{
						for(int i = 1; i < numStages; i++)
							usage[i][month][tier] += currentUseValue*reductionCoefficient[season][i];
					}
					else if (tier == 4 || tier == 16) // no water use during emergency restrictions (stage 4)
					{
						for(int i = 1; i < 4; i++)
							usage[i][month][tier] += currentUseValue*reductionCoefficient[season][i];
					}
					else // uses that are never restricted
					{
						for(int i = 1; i < numStages; i++)
							usage[i][month][tier] += currentUseValue;
					}
				}
			}
			else if(name.compare("OWASA") == 0)
			{
				for(int tier = 0; tier < numTypes; tier++)
				{
					for(int stage = 0; stage < numStages; stage++)
						usage[stage][month][tier] = 0;
					
					usage[0][month][tier] += updatedUsage[0][month][tier];
					
					if(tier != 0) // restrictions not applied to lowest-tier customers
					{
						for(int i = 1; i < numStages; i++)
							usage[i][month][tier] += updatedUsage[i][month][tier]*reductionCoefficient[season][i];
					}
					else
					{
						for(int i = 1; i < numStages; i++)
							usage[i][month][tier] += updatedUsage[i][month][tier];
					}

				}
			}
			else
			{
				cerr << "Error: Price calculations not defined for utility " << name << ". See the file WaterUtility.cpp for examples. Exiting..." << endl;
				exit(1);
			}
			
			// Calculate aggregated use for this month over all restriction stages
			for(int stage = 0; stage < numStages; stage++)
			{
				totalUsage[stage][month]= 0.0;
				
				for (int tier = 0; tier < numTiers; tier++)
				{
					totalUsage[stage][month] += usage[stage][month][tier];//total use, unrestricted
				}
				
				reductions[month][stage] = totalUsage[stage][month]/totalUsage[0][month];//reductions, unrestricted
				meanUse[season][stage] += reductions[month][stage]/monthsPerDecisionPeriod;
			}
		}
		
		// Have restrictions worked? Recalculate restriction values for each stage
		for(int i = 0; i < numAnnualDecisionPeriods; i++)
		{
			for(int j = 1; j < numStages; j++)
			{
				ssq[i][j] = pow(meanUse[i][j] - individualReductions[i][j], 2);
				reductionCoefficient[i][j] = reductionCoefficient[i][j]*(.9+.1*(individualReductions[i][j])/meanUse[i][j]);
			}
		}
		
		numberOfIterations++;
	}
	
	// End of iterative solution. Move usage data into permanent matrices.
	general_2d_allocate(data2, numMonths, numTiers, 0.0);
	general_2d_allocate(data3, numMonths, numTiers, 0.0);
	general_2d_allocate(data31, numMonths, numTiers, 0.0);
	general_2d_allocate(data32, numMonths, numTiers, 0.0);
	general_2d_allocate(data4, numMonths, numTiers, 0.0);
	
	for (int month = 0; month < numMonths; month++)
	{
		for (int tier = 0; tier < numTiers; tier++)
		{
			//Percentages of the total unrestrained use, by price tier
			data2[month][tier] = usage[0][month][tier]/totalUsage[0][month];//unrestrained (normal)
			data3[month][tier] = usage[1][month][tier]/totalUsage[0][month];//stage 1
			data31[month][tier] = usage[2][month][tier]/totalUsage[0][month];//stage 2
			data32[month][tier] = usage[3][month][tier]/totalUsage[0][month];//stage 3
			data4[month][tier] = usage[4][month][tier]/totalUsage[0][month];//stage 4
		}
	}
	
	zap(ssq, numAnnualDecisionPeriods);
	
	return;
}

void WaterUtility::calcSurchargePrice(double elasticity_Of_Demand[], int constant)
{
	double kValues[numMonths][numTiers];
	double finalFractions[numStages][numMonths][numTiers];
	double totalResidentialUse[numStages][numMonths], totalCommercialUse[numStages][numMonths];
	double currentElasticity, currentPrice, currentSewerPrice, currentSewerFraction = 0.9;
	double droughtMultiplier = 1;
	
	if(constant == 0)
	{
		for (int tier = 0; tier < numTiers; tier++)
		{
			for (int stage = 0; stage < numStages; stage++)
			{
				fractionPricing[stage][tier] = 1;
			}
		}
	}

	// Calculate new water use distributions after drought surcharges using Q = kP^-e
	for (int month = 0; month < numMonths; month++)
	{
		for(int stage = 1; stage < numStages; stage++)	
			consumerSurplusPerMG[month][stage] = 0; // clear from previous simulation
					
		for(int tier = 0; tier < numTiers; tier++)
		{
			// Set current variables for each utility (no "stage" here)
			currentElasticity = elasticity_Of_Demand[elasticityIndex[tier]];
			getCurrentUtilityPrices(month, tier, 1, currentPrice, currentSewerPrice, currentSewerFraction, droughtMultiplier);
			
			// Calculate k-values and finalFractions
			// Note: Cary kValues used raleigh.data2 in the original version for the kValue, but not for finalfraction. Was this deliberate?
			kValues[month][tier] = data2[month][tier]/(pow((currentPrice*droughtMultiplier + currentSewerPrice*currentSewerFraction), currentElasticity));
			finalFractions[0][month][tier] = data2[month][tier];
			
			for(int stage = 1; stage < numStages; stage++)
			{
				getCurrentUtilityPrices(month, tier, stage, currentPrice, currentSewerPrice, currentSewerFraction, droughtMultiplier); // update where "stage" is needed
				finalFractions[stage][month][tier] = kValues[month][tier]*pow((currentPrice*droughtMultiplier*fractionPricing[stage][tier] + currentSewerPrice*currentSewerFraction), currentElasticity);
			}

			// Reset final final fraction data if it is unreasonable
			if(finalFractions[1][month][tier] > data3[month][tier])
				finalFractions[1][month][tier] = data3[month][tier];
	
			if(finalFractions[2][month][tier] > data31[month][tier])
				finalFractions[2][month][tier] = data31[month][tier];
				
			if(finalFractions[3][month][tier] > data32[month][tier])
				finalFractions[3][month][tier] = data32[month][tier];
			
			if(numStages > 4 && finalFractions[4][month][tier] > data4[month][tier])
				finalFractions[4][month][tier] = data4[month][tier];
				
			//Total consumer surplus losses are calculated ($MM/MG of total use) for reductions in use made during each stage of water use restrictions
			for(int stage = 1; stage < numStages; stage++)
			{		
				//Note: In the original version, Cary's calculation added to Raleigh's consumerSurplusPerMG. Was this deliberate?
				if (kValues[month][tier] != 0)		
					consumerSurplusPerMG[month][stage] += (pow((1/kValues[month][tier]), (1/(currentElasticity)))/(1+(1/currentElasticity)))
						*(pow(data2[month][tier], (1+1/currentElasticity)) - pow(finalFractions[stage][month][tier],(1+1/currentElasticity)))
						+(currentPrice*fractionPricing[stage][tier] - currentPrice)*finalFractions[stage][month][tier]
						-(data2[month][tier] - finalFractions[stage][month][tier])*(currentPrice + currentSewerFraction*currentSewerPrice);
			}
		}
		
		// Determine overall use reductions and final water use distribution
		for(int stage = 0; stage < numStages; stage++)
		{
			totalResidentialUse[stage][month] = 0;
			totalCommercialUse[stage][month] = 0;
		}
			
		// Sum the total use across tiers
		for (int tier = 0; tier < numTiers; tier++)
		{
			for(int stage = 0; stage < numStages; stage++)
			{
				if(tierIsResidential(tier))
					totalResidentialUse[stage][month] += finalFractions[stage][month][tier];
				else
					totalCommercialUse[stage][month] += finalFractions[stage][month][tier];
			}
		}
		// Recalculate the final fractions for each tier
		for (int tier = 0; tier < numTiers; tier++)
		{
			for(int stage = 0; stage < numStages; stage++)
			{
				if(tierIsResidential(tier))
					finalFractions[stage][month][tier] = finalFractions[stage][month][tier]/totalResidentialUse[stage][month];
				else
					finalFractions[stage][month][tier] = finalFractions[stage][month][tier]/totalCommercialUse[stage][month];
			}
		}
		
		// Reductions are a fraction of unrestricted use
		for(int stage = 1; stage < numStages; stage++)
			reductions[month][stage] = (totalResidentialUse[stage][month] + totalCommercialUse[stage][month]) / (totalResidentialUse[0][month] + totalCommercialUse[0][month]);
	}
	
	// Calculate future costs over the simulation period
	for (int year = 0; year < numFutureYears; year++)
	{
		for(int month = 0; month < numMonths; month++)
		{
			for(int stage = 0; stage < numStages; stage++)
				costPerMG[month + year*numMonths][stage] = 0; // clear from previous simulation
			
			for (int tier = 0; tier < numTiers; tier++)
			{
				for(int stage = 0; stage < numStages; stage++)
				{
					// fill in current variables depending on which utility it is
					getCurrentUtilityPrices(month, tier, stage, currentPrice, currentSewerPrice, currentSewerFraction, droughtMultiplier);
					
					if(tierIsResidential(tier))
						costPerMG[month + year*numMonths][stage] += finalFractions[stage][month][tier]
							*(currentPrice*droughtMultiplier*fractionPricing[stage][tier] + currentSewerFraction*currentSewerPrice)*futureDemandBreakdown[0][year];
					else
						costPerMG[month + year*numMonths][stage] += finalFractions[stage][month][tier]
							*(currentPrice*droughtMultiplier*fractionPricing[stage][tier] + currentSewerFraction*currentSewerPrice)
							*(futureDemandBreakdown[1][year] + futureDemandBreakdown[2][year] + futureDemandBreakdown[3][year]);
				}
			}
		}
	}
	
	zap(data2, numMonths);
	zap(data3, numMonths);
	zap(data31, numMonths);
	zap(data32, numMonths);
	zap(data4, numMonths);
	
}

// Private helper functions for the WaterUtility Class

void WaterUtility::getCurrentUtilityPrices(int month, int tier, int stage, double &currentPrice, double &currentSewerPrice, double &currentSewerFraction, double &droughtMultiplier)
{
	if(name.compare("Durham") != 0 && name.compare("Cary") != 0 && name.compare("Raleigh") != 0 && name.compare("OWASA") != 0)
	{
		cerr << "Error: function calcSurchargePrice (in WaterUtility.cpp) is not defined for utility " << name << ". Exiting..." << endl;
		exit(1);
	}
			
	if(name.compare("OWASA") == 0)
	{
		if(month < 4 || month >= 9)
		{
			currentPrice = waterPrice_NonIrr[tier];
			currentSewerPrice = waterPrice_NonIrr[8];
		}
		else
		{
			currentPrice = waterPrice_Irr[tier];
			currentSewerPrice = waterPrice_Irr[8];
		}
		droughtMultiplier = droughtSurcharges[tier][stage];
	}
	else
	{
		currentPrice = waterPrice[tier];
		currentSewerPrice = waterPrice[numTiers]; // last price is sewer price
	}
	
	if(name.compare("Cary") == 0)
		currentSewerFraction = usePatterns[month][9];
	else
		currentSewerFraction = 0.9;
}

bool WaterUtility::anyThresholdExceeded(double threshold, double **ssq)
{
	for(int i = 0; i < numAnnualDecisionPeriods; i++)
	{
		for(int j = 1; j < numStages; j++)
			if(ssq[i][j] > threshold)
				return true;
	}
	return false;
}

bool WaterUtility::tierIsResidential(int tier)
{
	if((name.compare("OWASA") == 0 || name.compare("Durham") == 0) && tier < 5)
		return true;
	else if(name.compare("Cary") == 0 && (tier != 5 && tier != 8))
		return true;
	else if(name.compare("Raleigh") == 0 && tier != 3)
		return true;
	else
		return false;
}
