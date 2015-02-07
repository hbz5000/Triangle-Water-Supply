#include "global.h"
#include "ReservoirStorage.h"

using namespace std;

//ReservoirStorage::ReservoirStorage()
//{
//}

void ReservoirStorage::initializeReservoirStorage(double durhamCap, double CCRCap, double StQCap, double ULCap, double lakeWBCap, double fallsLakeSupplyCap, double fallsLakeQualityCap, 
													double jordanSupplyCap, double jordanQualityCap, double CaryTreatmentCap, double DurhamCaryCap, double DurhamOWASACap, 
													double RaleighCaryCap, double raleighJordanAlloc, double durhamJordanAlloc, double owasaJordanAlloc, double caryJordanAlloc)
{
	//Set reservoir capacities and initial storage volumes
	durhamCapacity=durhamCap;//Durham capacity
	durhamStorage=durhamCap;//Initial storage set to capacity
	CCRCapacity = CCRCap;//OWASA Cane Creek reservoir capacity
	CCRStorage = CCRCap;//Initial storage set to capacity
	ULCapacity = ULCap;//OWASA University Lake reservoir capacity
	ULStorage = ULCap;//Initial storage set to capacity
	StQCapacity = StQCap;//OWASA Stone Quarry reservoir capacity
	StQStorage = StQCap;//Initial storage set to capacity
	OWASACapacity = CCRCap+StQCap+ULCap;//OWASA total capacity
	OWASAStorage = OWASACapacity;//Initial storage set to capacity
	lakeWBCapacity = lakeWBCap;//Raleigh Lakes Wheeler and Benson capacity
	fallsLakeSupplyCapacity = fallsLakeSupplyCap;//Capacity of Falls Lake Water Supply allocation
	fallsLakeQualityCapacity = fallsLakeQualityCap;//Capacity of Falls Lake Water Quality allocation
	lakeWBStorage = lakeWBCap;//Initial storage set to capacity
	fallsLakeSupplyStorage = fallsLakeSupplyCap;//Initial storage set to capacity
	fallsLakeQualityStorage = fallsLakeQualityCap;//Initial storage set to capacity
	jordanLakeSupplyCapacity = jordanSupplyCap;
	jordanLakeQualityCapacity = jordanQualityCap;
	jordanLakeSupplyStorage = jordanSupplyCap;
	jordanLakeQualityStorage = jordanQualityCap;
	CaryTreatmentCapacity = CaryTreatmentCap;//Cary WTP capacity
	DurhamCaryCapacity = DurhamCaryCap;//Capacity of the Durham Cary interconnection
	DurhamOWASACapacity = DurhamOWASACap;//Capacity of the Durham OWASA interconnection
	RaleighCaryCapacity = RaleighCaryCap;//Capacity of the Raleigh Cary interconnection
	durhamRequest = 0;//Durham intial transfer request
	owasaRequest = 0;//OWASA initial transfer request
	raleighRequest = 0;//Raleigh initial transfer request
	raleighAllocation = raleighJordanAlloc;
	caryAllocation = caryJordanAlloc;
	durhamAllocation = durhamJordanAlloc;
	owasaAllocation = owasaJordanAlloc;
	caryJordanCapacity = caryJordanAlloc*jordanLakeSupplyCapacity;
	caryJordanStorage = caryJordanCapacity;
	raleighJordanCapacity = raleighJordanAlloc*jordanLakeSupplyCapacity;
	raleighJordanStorage = raleighJordanCapacity;
	durhamJordanCapacity = durhamJordanAlloc*jordanLakeSupplyCapacity;
	durhamJordanStorage = durhamJordanCapacity;
	owasaJordanCapacity = owasaJordanAlloc*jordanLakeSupplyCapacity;
	owasaJordanStorage = owasaJordanCapacity;
	jordanQualityFraction = 94600.0/(45800.0+94600.0);
	jordanRaleighFraction = raleighAllocation*45800.0/(94600.0+45800.0);
	jordanDurhamFraction = durhamAllocation*45800.0/(94600.0+45800.0);
	jordanCaryFraction = caryAllocation*45800.0/(94600.0+45800.0);
	jordanOWASAFraction = owasaAllocation*45800.0/(94600.0+45800.0);	
	return;
}
void ReservoirStorage::initializeReservoirStorageROF(double durhamCap, double CCRCap, double StQCap, double ULCap, double lakeWBCap, double fallsLakeSupplyCap, double fallsLakeQualityCap, double jordanLakeSupplyCap, double jordanLakeQualityCap, double caryJordanAlloc, double raleighJordanAlloc, double durhamJordanAlloc, double owasaJordanAlloc, double CaryTreatmentCap, double DurhamCaryCap, double DurhamOWASACap, double RaleighCaryCap)
{
	//Same as above, allowing for variable initial reservoir storage
	durhamCapacity=durhamCap;
	CCRCapacity = CCRCap;
	ULCapacity = ULCap;
	StQCapacity = StQCap;
	OWASACapacity = CCRCap+StQCap+ULCap;
	lakeWBCapacity = lakeWBCap;
	fallsLakeSupplyCapacity = fallsLakeSupplyCap;
	fallsLakeQualityCapacity = fallsLakeQualityCap;
	jordanLakeSupplyCapacity = jordanLakeSupplyCap;
	jordanLakeQualityCapacity = jordanLakeQualityCap;
	CaryTreatmentCapacity = CaryTreatmentCap;//Cary WTP capacity
	DurhamCaryCapacity = DurhamCaryCap;//Capacity of the Durham Cary interconnection
	DurhamOWASACapacity = DurhamOWASACap;//Capacity of the Durham OWASA interconnection
	RaleighCaryCapacity = RaleighCaryCap;//Capacity of the Raleigh Cary interconnection
	raleighAllocation = raleighJordanAlloc;
	caryAllocation = caryJordanAlloc;
	durhamAllocation = durhamJordanAlloc;
	owasaAllocation = owasaJordanAlloc;
	caryJordanCapacity = caryJordanAlloc*jordanLakeSupplyCapacity;
	raleighJordanCapacity = raleighJordanAlloc*jordanLakeSupplyCapacity;
	durhamJordanCapacity = durhamJordanAlloc*jordanLakeSupplyCapacity;
	owasaJordanCapacity = owasaJordanAlloc*jordanLakeSupplyCapacity;
	jordanQualityFraction = 94600.0/(45800.0+94600.0);
	jordanRaleighFraction = raleighAllocation*45800.0/(94600.0+45800.0);
	jordanDurhamFraction = durhamAllocation*45800.0/(94600.0+45800.0);
	jordanCaryFraction = caryAllocation*45800.0/(94600.0+45800.0);
	jordanOWASAFraction = owasaAllocation*45800.0/(94600.0+45800.0);

	return;
}

void ReservoirStorage::updateReservoirStorageROF(double storageVolume, double storageIncrements)
{
	//Same as above, allowing for variable initial reservoir storage
	double initialStorage = (storageVolume/storageIncrements);
	durhamStorage=durhamCapacity*initialStorage;
	CCRStorage = CCRCapacity*initialStorage;
	ULStorage = ULCapacity*initialStorage;
	StQStorage = StQCapacity*initialStorage;
	OWASAStorage = OWASACapacity*initialStorage;
	lakeWBStorage = lakeWBCapacity*initialStorage;
	fallsLakeSupplyStorage = fallsLakeSupplyCapacity*initialStorage;
	fallsLakeQualityStorage = fallsLakeQualityCapacity*initialStorage;
	jordanLakeSupplyStorage = jordanLakeSupplyCapacity*initialStorage;
	jordanLakeQualityStorage = jordanLakeQualityCapacity*initialStorage;
	durhamRequest = 0;//Durham intial transfer request
	owasaRequest = 0;//OWASA initial transfer request
	raleighRequest = 0;//Raleigh initial transfer request
	caryJordanStorage = caryJordanCapacity*initialStorage;
	raleighJordanStorage = raleighJordanCapacity*initialStorage;
	durhamJordanStorage = durhamJordanCapacity*initialStorage;
	owasaJordanStorage = owasaJordanCapacity*initialStorage;
	return;
}
void ReservoirStorage::openResFiles()
{
	openFile(out2, "reservoirOutput2.csv");
	return;
}

void ReservoirStorage::setDemands(double durham, double owasa, double raleigh, double cary, int numberOfDaysInTheWeek)
{
	durhamUse = durham;
	OWASAUse = owasa;
	CaryUse = cary;
	raleighUse = raleigh;
	numdays = numberOfDaysInTheWeek;
	return;
}

void ReservoirStorage::getReturnRatios(double raleighR, double durhamR)
{
	raleighReturn = raleighR;
	durhamReturn = durhamR;
	return;
}

void ReservoirStorage::calcTransfers(double transferDurham, double durhamRisk, double transferOWASA, double owasaRisk, double transferRaleigh, double raleighRisk, double OWASAD)
{
	percentFull_O = OWASAStorage/OWASACapacity;//Actual OWASA reservoir storage as a percentage of capacity
	percentFull_D = durhamStorage/durhamCapacity;//Actual Durham reservoir storage as a percentage of capacity
	percentFull_R = (fallsLakeSupplyStorage)/(fallsLakeSupplyCapacity);//Actual Raleigh reservoir storage as a percentage of capacity
	
	//Determines how much water each utility needs to request to reach an acceptable risk-of-failure
	if (percentFull_O<transferOWASA)
		owasaRequestInt = (transferOWASA-percentFull_O)*OWASACapacity;
	else
		owasaRequestInt = 0;
			
	if (percentFull_D<transferDurham)
		durhamRequestInt = (transferDurham-percentFull_D)*durhamCapacity;
	else
		durhamRequestInt = 0;
	
	if (percentFull_R<transferRaleigh)
		raleighRequestInt = (transferRaleigh - percentFull_R)*(fallsLakeSupplyCapacity);
	else
		raleighRequestInt = 0;
	
	
	owasaRequest = owasaRequestInt;
	durhamRequest = durhamRequestInt;
	raleighRequest = raleighRequestInt;
	
	////Cary wants at least a 5 MGD buffer of unused capacity in its treatment plant
	caryTreatmentBuffer = 5.0*numdays;
	
	////Peaking factor at the Cary WTP used to move from daily capacity to average weekly capacity
	caryPeakingFactor = .85;
	
	//Total spare capacity to sell transfers at the Cary WTP
	caryExtraCapacity = (CaryTreatmentCapacity*numdays - caryTreatmentBuffer)*caryPeakingFactor - CaryUse;
	
	////Capacity of the Durham/OWASA interconnection is 7 MGD
	DurhamOWASACapacity = 7*numdays;
	
	//Weekly capacity of the Durham-Cary interconnection
	double DCCap = DurhamCaryCapacity*numdays;
	
	//Weekly capacity of the Raleigh-Cary interconnection
	double RCCap = RaleighCaryCapacity*numdays;
	double raleighExtra = 0;
	double durhamExtra = 0;
	
	/////OWASA must maintain at least 3 MGD production in its own treatment plant
	/////Transfers to OWASA cannot reduce OWASA domestic production below  3 MGD
	double OWASAminimumUse = 3*numdays;
	if (OWASAD-DurhamOWASACapacity<OWASAminimumUse)
	{
		DurhamOWASACapacity = OWASAD-OWASAminimumUse;
	}
	if (DurhamOWASACapacity<0)
	{
		DurhamOWASACapacity = 0;
	}
	
	//No transfers if Cary does not have the extra treatment capacity
	if (caryExtraCapacity<0)
	{
		caryExtraCapacity=0;
		durhamRequest = 0;
		owasaRequest = 0;
		raleighRequest = 0;
	}
	else if ((durhamRequest+owasaRequest+raleighRequest)>caryExtraCapacity)//Adjust requests to take consideration of Cary's transfer capacity
	{
		if ((raleighRisk+owasaRisk+durhamRisk)==0)
		{
			durhamRequest = 0;
			owasaRequest = 0;
			raleighRequest = 0;
		}
		else
		{
			/////If Cary WTP capacity is limiting factor, available capacity is 
			/////proportional to the risk of failure at each utility
			durhamRequest = caryExtraCapacity*durhamRisk/(raleighRisk+owasaRisk+durhamRisk);
			owasaRequest = caryExtraCapacity*owasaRisk/(raleighRisk+owasaRisk+durhamRisk);
			raleighRequest = caryExtraCapacity*raleighRisk/(raleighRisk+owasaRisk+durhamRisk);
		}
	}
	if (raleighRequest>RCCap)//Make sure Raleigh's request does not exceed its interconnection capacity
	{
		raleighExtra = raleighRequest - RCCap;//If Raleigh can not take all of the water allotted to it, the extra water is available to Durham and OWASA
		raleighRequest = RCCap;
		if ((durhamRequest+owasaRequest+raleighExtra) > DCCap)//Make sure Durham can take all the additional water (all water to Durham and OWASA must go through Durham)
		{
			if ((durhamRisk+owasaRisk)==0)
			{
				durhamRequest = 0;
				owasaRequest = 0;
			}
			else
			{
				/////If Durham-Cary interconnection is limiting, available water is divided between Durham & OWASA based on relative risk of failure
				durhamRequest = DCCap*durhamRisk/(durhamRisk+owasaRisk);
				if (durhamRequest>durhamRequestInt)
				{
					durhamExtra = durhamRequest-durhamRequestInt;
					durhamRequest = durhamRequestInt;
				}
				owasaRequest = DCCap*owasaRisk/(durhamRisk+owasaRisk)+durhamExtra;
				if (owasaRequest>owasaRequestInt)
				{
					owasaRequest = owasaRequestInt;
				}
			}
		}
		if (owasaRequest>DurhamOWASACapacity)//Make sure OWASA can take its allottment of water, if not, Durham can hold on to the extra water
		{
			owasaRequest = DurhamOWASACapacity;
			durhamRequest = durhamRequest + owasaRequest - DurhamOWASACapacity;
		}
	}
	else if ((durhamRequest+owasaRequest) > DCCap)//Make sure Durham/OWASA's request does not exceed ints interconnection capacity
	{	
		if ((durhamRisk+owasaRisk)==0)
		{
			durhamExtra = durhamRequest +owasaRequest;
			durhamRequest = 0;
			owasaRequest = 0;
		}
		else
		{
			/////If Durham-Cary interconnection is limiting, available water is divided between Durham & OWASA based on relative risk of failure
			durhamExtra = durhamRequest + owasaRequest - DCCap;
			durhamRequest = DCCap*durhamRisk/(durhamRisk+owasaRisk);
			owasaRequest = DCCap*owasaRisk/(durhamRisk+owasaRisk);
		}
		if (raleighRequest+durhamExtra>RCCap)//Make sure Raleigh can take all the additional water
		{
			raleighRequest = RCCap;
		}
	}
	if (owasaRequest>owasaRequestInt)
	{
		owasaRequest = owasaRequestInt;
	}
	if (raleighRequest>raleighRequestInt)
	{
		raleighRequest = raleighRequestInt;
	}
	if (durhamRequest>durhamRequestInt)
	{
		durhamRequest = durhamRequestInt;
	}	
	return;
}

void ReservoirStorage::setInflow(double durham, double ccr, double ul, double stq, double falls, double wb, double clayton, double crabtree, double jordan, double lillington,
									double ralRet, double durRet, double durRet2, double owasaRet, double evaporationf, double evaporationwb, double evaporation)
{
	durhamInflow = durham;//Inflows to Durham Reservoirs
	CCRInflow = ccr;//Inflows to Cane Creek Reservoir
	ULInflow = ul;//Infow to University Lake
	StQInflow = stq;//Inflow to Stone Quarry
	fallsInflow = falls;//Inflow to Falls Lake
	wbInflow = wb;//Inflow to Wheeler Benson
	claytonInflow = clayton;//Inflow to clayton gauge
	crabtreeInflow = crabtree;//Inflow to Crabtree Creek
	jordanInflow = jordan;//Inflow to Jordan Lake
	lillingtonInflow = lillington;//Inflows to lillington Gauge
	raleighReturn = ralRet;//Raleigh WW Returns downstream of Falls Lake
	durhamReturn = durRet;//Durham WW Returns to the Neuse Basin
	durhamReturn2 = durRet2;//Durham WW Returns to the Cape Fear Basin
	owasaReturn = owasaRet;//OWASA WW Returns to Cape Fear Basin
	evapF = evaporationf;//Evaporation from Falls Lake
	evapWB = evaporationwb;//Evaporation from Wheeler and Benson
	evap = evaporation;//Evaporation from Durham and OWASA
	return;
}

void ReservoirStorage::setSpillover(int week)
{
	if (week <21 || week>47)//Spillage rules for Durham Reservoir at Little River
		durhamSpillage = 3.877*numdays;
	else
		durhamSpillage = 9.05;
	return;
}

void ReservoirStorage::updateStorage(int week)
{	
/////////Durham Reservoir (Little River Reservoir and Lake Michie) Water Balance///////////////////////////////////
	
	durhamStorage = durhamStorage + durhamInflow + durhamRequest - durhamUse - evap*1069 - durhamSpillage;
	//durhamStorage = Little River Reservoir/Lake Michie Storage
	//durhamInflow = new inflows to Little River Reservoir/Lake Michie
	//durhamRequest = transfers to Durham from Lake Jordan
	//durhamUse = durham municipal use
	//evap = evaporation; 1069 is the surface area (acres) of the two reservoirs (evaporation is in MG/acre)
	//durhamSpillage = mandatory releases for environmental flows
	
	//Boundry conditions
	if (durhamStorage>durhamCapacity)
	{
		durhamSpillage+=durhamStorage - durhamCapacity;
		durhamStorage=durhamCapacity;
		//water greater than combined reservoir capacity is added to the releases for environmental flows
	}
	
	if (durhamStorage<0)
		durhamStorage = 0;
		//non negativity constraint
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////Raleigh Reservoir (Falls Lake, Lake Wheeler, and Lake Benson - Lake Wheeler and Benson treated as one)//////

	//Determining production at each of Raleigh's 2 WTP (one for Falls Lake, one for Lake Wheeler and Lake Benson
	double raleighDemand1, raleighDemand2;//Demand from WTP drawing from Lakes Wheeler and Benson (1) and WTP drawing from Falls Lake (2)
	raleighDemand1 = .25*numdays;////WTP serving Lake Wheeler and Benson - note: this model was calibrated to data before Raleigh started
								//fully using Lake Wheelr and Benson, and in the future there will be more production from this WTP
	raleighDemand2 = raleighUse-raleighDemand1;//Demand from WTP drawing from Falls Lake
	
	if (raleighDemand2<36*numdays)//Treatment plant must run at at least 36 MGD
	{
		raleighDemand2 = 36*numdays;
		raleighDemand1 = raleighUse - raleighDemand2;//remaining demand comes from other WTP (Lake Wheeler and Benson)
	}

	
	
	////Determining Spillage at Lake Wheeler and Lake Benson
	if (lakeWBStorage <.3*lakeWBCapacity)////environmental flow requirements are determined by storage in reservoir
	{
		lakeWBSpillage = numdays/1.547;////1 ccf
	}
	else if (lakeWBStorage<.6*lakeWBCapacity)
	{
		lakeWBSpillage = 2*numdays/1.547;////2 ccf
	}
	else
	{
		lakeWBSpillage = 3*numdays/1.547;////3 ccf
	}
	
	//////Lake Wheeler and Benson Water Balance
	lakeWBStorage += wbInflow - raleighDemand1 - lakeWBSpillage - (lakeWBStorage*.3675*evapWB);
	////surface area at Lake Wheeler and Benson = to 0.3675*Total Storage (evapWB in MG/acre) 
	
	////boundry conditions, anything over capacity is added to environmental flow releases
	if (lakeWBStorage >lakeWBCapacity)
	{
		lakeWBSpillage = lakeWBStorage - lakeWBCapacity;
		lakeWBStorage = lakeWBCapacity;
	}
	
		//Determining releases from Falls Lake to meet environmental flow requirements
	//flows have to hit two targets, minimum releases from the reservoir and minimum flows at a downstream gauge
	if (week< 13 || week>43)//environmental requirement changes from April-Oct and Nov-Mar
	{
		fallsSpillage = numdays*60/1.547;//Mandatory minimum releases at Falls Lake Reservoir = 60 cfs (1.547 cfs to 1 MGD)
		if ( (fallsSpillage + raleighReturn + claytonInflow + crabtreeInflow) < 130*numdays)
			//Mandatory Minimum Flows at Clayton Gauge includes falls lake releases, new flows from downstream watershed, and wastewater returns
		{
			fallsSpillage = 130*numdays - raleighReturn - claytonInflow - crabtreeInflow;////clayton gauge targets
		}
	}
	else
	{
		fallsSpillage = 100*numdays/1.547;//Mandatory minimum releases at Falls Lake Reservoir = 100 cs (1.547 cfs to 1 MGD)
		if ( (fallsSpillage + raleighReturn + claytonInflow + crabtreeInflow) < 180*numdays)//Mandatory Minimum Flows at Clayton Gauge
		{
			fallsSpillage = 180*numdays - raleighReturn - claytonInflow - crabtreeInflow;
		}
	}
	claytonGage = fallsSpillage + raleighReturn + claytonInflow + crabtreeInflow;//totalFlows at Clayton Gauge

	
	////determining the area of Falls Lake for evaporation requirements
	////surface area roughly scales with storage (5734 is dead storage)
	if ((fallsLakeSupplyStorage+fallsLakeQualityStorage+5734)<29000)
	{
		fallsArea = .32*(fallsLakeSupplyStorage+fallsLakeQualityStorage+5734);
	}
	else
	{
		fallsArea = .28*(fallsLakeSupplyStorage+fallsLakeQualityStorage+5734);
	}

	////Inflows are divided between the water storage supply and water quality supply proportionatly (14.7BG supply storage, 20 BG quality storage)
	////Durham reservoir releases and wastewater returns are added to Falls Lake inflows
	fallsSupplyInflow = (fallsInflow + durhamSpillage - fallsArea*evapF + durhamReturn)*(14.7/34.7) +raleighRequest;
	fallsQualityInflow = (fallsInflow + durhamSpillage - fallsArea*evapF + durhamReturn)*(20/34.7);

	////Environmental Releases come from the water quality storage portion of Falls Lake
	fallsLakeQualityStorage += fallsQualityInflow - fallsSpillage;
	
	//boundry conditions on the water quality storage in Falls Lake
	if (fallsLakeQualityStorage > fallsLakeQualityCapacity)
	{
		////If excess water quality storage exists, it is allocated to the water supply storage portion of Falls Lake
		fallsSupplyInflow += fallsLakeQualityStorage - fallsLakeQualityCapacity;
		fallsQualityInflow += fallsLakeQualityCapacity - fallsLakeQualityStorage;
		fallsLakeQualityStorage = fallsLakeQualityCapacity;
	}
	if (fallsLakeQualityStorage<0)
	{
		fallsLakeQualityStorage = 0;
	}
	
	////Municipal water demand comes from the water suply storage portion of Falls Lake
	fallsLakeSupplyStorage += fallsSupplyInflow - raleighDemand2;
	
	/////boundry conditions on water supply storage in Falls Lake (don't need to keep track of excess spillage from Falls Lake)
	if (fallsLakeSupplyStorage>fallsLakeSupplyCapacity)
	{
		fallsLakeSupplyStorage = fallsLakeSupplyCapacity;
	}
	if (fallsLakeSupplyStorage<0)
	{
		fallsLakeSupplyStorage = 0;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	////////OWASA Reservoirs///////////////////////////////////////////////////////////////////////////////////////////////
	
	////OWASA transfers are added to Cane Creek Reservoir Storage
	CCRStorage+=owasaRequest;
	
	//Spillage calculated from Cane Creek Reservoir - based on inflow to CCR
	if (CCRInflow > 1.797*7)
		OWASASpillage = 1.797*7;
	else if (CCRInflow < .1422*7)
		OWASASpillage = .1422*7;
	else
		OWASASpillage = CCRInflow;
		
	//No environmental flow requirements for Stone Quarry, spillage is based on capacity constraints
	if (StQStorage +StQInflow - 10*evap>StQCapacity)
	////Stone quarry has a surface area of 10 acres
	{
		StQSpillage = StQStorage + StQInflow -10*evap - StQCapacity;
		StQStorage = StQCapacity;
	}
	else
	{
		StQStorage = StQStorage + StQInflow - 10*evap;
		StQSpillage = 0;
	}
	
	//OWASA has three reservoirs, and they use water from their different reservoirs depending on storage levels
	if ((ULStorage + ULInflow + StQSpillage - 212*evap - OWASAUse) > .75*ULCapacity)
	/////University Lake Inflows include Stone Quarry Spillage
	/////Surface area of University Lake is 212 acres
	{
		////All use comes from University Lake
		ULStorage = ULStorage + ULInflow + StQSpillage - 212*evap - OWASAUse;
		if (ULStorage > ULCapacity)
		{
			OWASASpillage += ULStorage - ULCapacity;
			ULStorage = ULCapacity;
		}
		
		////No use from Cane Creek Reservoir (Cane Creek Reservoir surface area is 500 acres)
		CCRStorage = CCRStorage +CCRInflow - 500*evap - OWASASpillage;
		if (CCRStorage > CCRCapacity)
		{
			OWASASpillage += CCRStorage - CCRCapacity;
			CCRStorage = CCRCapacity;
		}
	}
	else if ((CCRCapacity - (CCRStorage + CCRInflow -500*evap - OWASASpillage))< .25*ULCapacity)
	{
		if (OWASAUse<70)
		{
			////No water use comes from University Lake
			ULStorage = ULStorage + ULInflow + StQSpillage - 212*evap;
			if (ULStorage > ULCapacity)
			{
				OWASASpillage += ULStorage - ULCapacity;
				ULStorage = ULCapacity;
			}
		
			////All water use come from Cane Creek Reservoir
			CCRStorage = CCRStorage +CCRInflow - 500*evap - OWASASpillage - OWASAUse;
			if (CCRStorage > CCRCapacity)
			{
				OWASASpillage += CCRStorage - CCRCapacity;
				CCRStorage = CCRCapacity;
			}
		}
		else
		{
			////First 70 MG comes from Cane Creek Reservoir, remainder comes from University Lake
			ULStorage = ULStorage + ULInflow + StQSpillage - 212*evap - (OWASAUse - 70);
			if (ULStorage > ULCapacity)
			{
				OWASASpillage += ULStorage - ULCapacity;
				ULStorage = ULCapacity;
			}
		
			CCRStorage = CCRStorage +CCRInflow - 500*evap - OWASASpillage - 70;
			if (CCRStorage > CCRCapacity)
			{
				OWASASpillage += CCRStorage - CCRCapacity;
				CCRStorage = CCRCapacity;
			}
		}
	}
	else if ((ULStorage + ULInflow + StQSpillage - evap*212)> .5*OWASAUse)
	{
		////half of the water use comes from University Lake
		ULStorage = ULStorage + ULInflow + StQSpillage - 212*evap - .5*OWASAUse;
		if (ULStorage > ULCapacity)
		{
			OWASASpillage += ULStorage - ULCapacity;
			ULStorage = ULCapacity;
		}
		
		////half of the water use comes from cane Creek Reservoir
		CCRStorage = CCRStorage +CCRInflow - 500*evap - OWASASpillage - .5*OWASAUse;
		if (CCRStorage > CCRCapacity)
		{
			OWASASpillage += CCRStorage - CCRCapacity;
			CCRStorage = CCRCapacity;
		}
	}
	else if ((CCRStorage + CCRInflow -evap*500 - OWASASpillage)>OWASAUse)
	{
		ULStorage = ULStorage + ULInflow + StQSpillage - 212*evap;
		if (ULStorage > ULCapacity)
		{
			OWASASpillage += ULStorage - ULCapacity;
			ULStorage = ULCapacity;
		}
		////all water use comes from Cane Creek Reservoir
		CCRStorage = CCRStorage +CCRInflow - 500*evap - OWASASpillage - OWASAUse;
		if (CCRStorage > CCRCapacity)
		{
			OWASASpillage += CCRStorage - CCRCapacity;
			CCRStorage = CCRCapacity;	
		}
	}
	else
	{
		////water needs not met, Stone Quarry emptied into Cane Creek
		ULStorage = ULStorage + ULInflow + StQSpillage - evap*212;
		CCRStorage = CCRStorage + CCRInflow - 500*evap - OWASASpillage;
		CCRStorage = CCRStorage + StQStorage + ULStorage;
		ULStorage = 0;
		StQStorage = 0;
		CCRStorage = CCRStorage - OWASAUse;
	}
	if (CCRStorage<0)
	{
		CCRStorage = 0;
	}
	if (ULStorage<0)
	{
		ULStorage = 0;
	}
	if (StQStorage<0)
	{
		StQStorage = 0;
	}
	//////aggregating OWASA Storage
	OWASAStorage = CCRStorage + ULStorage + StQStorage;
	////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	///////Jordan Lake//////////////////////////////////////////////////////////////////////////////////
	
	/////Determining Jordan Lake environmental releases - from Jordan Lake Water Quality Pool
	/////Jordan Lake environmental flows must hit minimum reservoir releases and minimum downstream flow targets
	if (jordanLakeQualityStorage/jordanLakeQualityCapacity>.4)
	{
		jordanLakeMinRelease = 25.8527*numdays;
		////releases at reservoir must be at least 25.8 MGD
		
		/////downstream flow requirements are based on current storage in Jordan Lake Quality Pool
		if (jordanLakeQualityStorage/jordanLakeQualityCapacity>.8)
		{
			if(jordanLakeMinRelease+lillingtonInflow<387.7901*numdays)
			{
				jordanLakeMinRelease = 387.7901*numdays-lillingtonInflow;
				//////flows at downstream gauge must be at least 387.8 MGD
			}
		}
		else if(jordanLakeQualityStorage/jordanLakeQualityCapacity>.6)
		{
			if(jordanLakeMinRelease+lillingtonInflow<290.8426*numdays)
			{
				jordanLakeMinRelease = 290.8426*numdays-lillingtonInflow;
				//////flows at downstream gauge must be at least 387.8 MGD
			}
		}
		else
		{
			if(jordanLakeMinRelease+lillingtonInflow<193.8951*numdays)
			{
				jordanLakeMinRelease = 193.8951*numdays-lillingtonInflow;
				//////flows at downstream gauge must be at least 387.8 MGD
			}
		}
	}
	////during extremely low storage conditions, downstream flow requirements are abandoned and 
	////minimum releases are decreased
	else if (jordanLakeQualityStorage/jordanLakeQualityCapacity>.2)
	{
		jordanLakeMinRelease = 129.2634*numdays;
	}
	else
	{
		jordanLakeMinRelease = 64.6317*numdays;
	}
	
	
	//////apportioning inflows to each Jordan Lake pool (water quality and individual utility supply storage)
	/////surface area of Jordan Lake is 13940 acres
	jordanLakeQualityStorage += (jordanInflow+durhamReturn2+owasaReturn+OWASASpillage-evap*13940)*jordanQualityFraction-jordanLakeMinRelease;
	raleighJordanStorage += (jordanInflow+durhamReturn2+owasaReturn+OWASASpillage-evap*13940)*jordanRaleighFraction - raleighRequest;
	durhamJordanStorage += (jordanInflow+durhamReturn2+owasaReturn+OWASASpillage-evap*13940)*jordanDurhamFraction - durhamRequest;
	owasaJordanStorage += (jordanInflow+durhamReturn2+owasaReturn+OWASASpillage-evap*13940)*jordanOWASAFraction - owasaRequest;
	caryJordanStorage += (jordanInflow+durhamReturn2+owasaReturn+OWASASpillage-evap*13940)*jordanCaryFraction - CaryUse;
	
	/////if water quality storage exceeds quality pool capacity, excess water is apportioned to the individual supply storage pools
	if (jordanLakeQualityStorage>jordanLakeQualityCapacity)
	{
		raleighJordanStorage+=raleighAllocation*(jordanLakeQualityStorage-jordanLakeQualityCapacity);
		durhamJordanStorage+=durhamAllocation*(jordanLakeQualityStorage-jordanLakeQualityCapacity);
		owasaJordanStorage+=owasaAllocation*(jordanLakeQualityStorage-jordanLakeQualityCapacity);
		caryJordanStorage+=caryAllocation*(jordanLakeQualityStorage-jordanLakeQualityCapacity);
		jordanLakeQualityStorage=jordanLakeQualityCapacity;
	}
	if (jordanLakeQualityStorage<0)
	{
		jordanLakeQualityStorage = 0;
	}
	
	////if an individual utility's supply storage is below zero, they cannot withdraw water for transfers
	if(raleighJordanStorage<0)
	{
		fallsLakeSupplyStorage+=raleighJordanStorage;
		raleighRequest+=raleighJordanStorage;
		if (fallsLakeSupplyStorage<0)
		{
			fallsLakeSupplyStorage = 0;
		}
		if (raleighRequest<0)
		{
			raleighRequest = 0;
		}
		raleighJordanStorage = 0;
	}
	////no need to calculate Jordan lake spillage
	if(raleighJordanStorage>raleighJordanCapacity)
	{
		raleighJordanStorage = raleighJordanCapacity;
	}
	if(durhamJordanStorage<0)
	{
		durhamStorage+=durhamJordanStorage;
		durhamRequest+=durhamJordanStorage;
		if (durhamRequest<0)
		{
			durhamRequest = 0;
		}
		if (durhamStorage<0)
		{
			durhamStorage = 0;
		}
		durhamJordanStorage = 0;
	}
	if(durhamJordanStorage>durhamJordanCapacity)
	{
		durhamJordanStorage = durhamJordanCapacity;
	}
	if(owasaJordanStorage<0)
	{
		CCRStorage+=owasaJordanStorage;
		OWASAStorage+=owasaJordanStorage;
		owasaRequest+=owasaJordanStorage;
		if (owasaRequest<0)
		{
			owasaRequest = 0;
		}
		if (CCRStorage<0)
		{
			CCRStorage = 0;
		}
		if (OWASAStorage <0)
		{
			OWASAStorage = 0;
		}
		owasaJordanStorage = 0;
	}
	if(owasaJordanStorage>owasaJordanCapacity)
	{
		owasaJordanStorage = owasaJordanCapacity;
	}
	if(caryJordanStorage<0)
	{
		caryJordanStorage = 0;
	}
	if(caryJordanStorage>caryJordanCapacity)
	{
		caryJordanStorage = caryJordanCapacity;
	}
	return;
}
double ReservoirStorage::getDurhamStorage()
{
	return durhamStorage/durhamCapacity;
}
double ReservoirStorage::getOWASAStorage()
{
	return OWASAStorage/OWASACapacity;
}
double ReservoirStorage::getRaleighStorage()
{
	return (fallsLakeSupplyStorage)/(fallsLakeSupplyCapacity);
}
double ReservoirStorage::getCaryStorage()
{
	return (caryJordanStorage)/(caryJordanCapacity);
}
double ReservoirStorage::getDurhamTransfers()
{
	return durhamRequest;
}
double ReservoirStorage::getOWASATransfers()
{
	return owasaRequest;
}
double ReservoirStorage::getRaleighTransfers()
{
	return raleighRequest;
}
double ReservoirStorage::getOWASASpillage()
{
	return OWASASpillage;
}
double ReservoirStorage::getDurhamSpillage()
{
	return durhamSpillage;
}
void ReservoirStorage::upgradeCaryTreatmentPlant()
{
	CaryTreatmentCapacity = 56;
}
void ReservoirStorage::upgradeCaryTreatmentPlant2()
{
	CaryTreatmentCapacity = 64;
}
void ReservoirStorage::upgradeDurhamCaryConnection()
{
	DurhamCaryCapacity = 23;
}
void ReservoirStorage::upgradeDurhamOWASAConnection()
{
	DurhamOWASACapacity = 16;
	

}
