#ifndef ReservoirStorage_h
#define ReservoirStorage_h

class ReservoirStorage
{
public:
	//ReservoirStorage();
	double fallsLakeQualityStorage;
	double fallsLakeQualityCapacity;
	double fallsArea;
	
	void initializeReservoirStorage(double durhamCap, double CCRCap, double StQCap, double ULCap, double lakeWBCap, double fallsLakeSupplyCap, double fallsLakeQualityCap, 
										double jordanLakeSupplyCap, double jordanLakeQualityCap, double CaryTreatmentCap, double DurhamCaryCap, double DurhamOWASACap, 
										double RaleighCaryCap, double raleighAllocation, double durhamAllocation, double owasaAllocation, double caryAllocation);
	void initializeReservoirStorageROF(double durhamCap, double CCRCap, double StQCap, double ULCap, double lakeWBCap, double fallsLakeSupplyCap, double fallsLakeQualityCap,
		double durhamStor, double CCRStor, double StQStor, double ULStor, double lakeWBStor, double fallsLakeSupplyStor, double fallsLakeQualityStor, double CaryTreatmentCap, 
		double DurhamCaryCap, double DurhamOWASACap, double RaleighCaryCap);
	void setDemands(double durham, double owasa, double cary, double raleigh, int numberOfDaysInWeek);
	void setSpillover(int week);
	void calcTransfers(double transferDurham, double durhamR, double transferOWASA, double owasaR, double transferRaleigh, double raleighR, double owasaWD);		
	void setInflow(double durham, double ccr, double ul, double stq, double falls, double wb, double clayton, double crabtree, double jordan, double lillington, double ralRet, double durRet, 
						double durRet2, double owasaRet, double evaporationf, double evaporationwb, double evaporation);
	void getReturnRatios(double raleighR, double durhamR);
	void updateStorage(int whatWeek); 
	double getDurhamStorage();
	double getOWASAStorage();
	double getRaleighStorage();
	double getCaryStorage();
	double getDurhamTransfers();
	double getOWASATransfers();
	double getRaleighTransfers();
	double getDurhamSpillage();
	double getOWASASpillage();
	void upgradeCaryTreatmentPlant();
	void upgradeCaryTreatmentPlant2();
	void upgradeDurhamCaryConnection();
	void upgradeDurhamOWASAConnection();
	void openResFiles();

private:
	double durhamInflow;
	double CCRInflow;
	double ULInflow;
	double StQInflow;
	double fallsInflow;
	double wbInflow;
	double claytonInflow;
	double crabtreeInflow;
	double jordanInflow;
	double lillingtonInflow;
	double raleighReturn;
	double durhamReturn;
	double durhamReturn2;
	double owasaReturn;
	double evapF;
	double evapWB;
	double evap;
	double fallsSupplyInflow;
	double fallsQualityInflow;
	double jordanLakeQualityInflow;
	double jordanLakeRaleighSupplyInflow;
	double jordanLakeDurhamSupplyInflow;
	double jordanlakeCarySupplyInflow;
	double jordanLakeOWASASupplyInflow;
	
	
	double durhamStorage;
	double durhamSpillage;
	double OWASAStorage;
	double CCRStorage;
	double ULStorage;
	double StQStorage;
	double lakeWBStorage;
	double fallsLakeSupplyStorage;
	
	double OWASASpillage;
	double CCRSpillage;
	double ULSpillage;
	double StQSpillage;
	double fallsSpillage;
	double lakeWBSpillage;
	double jordanLakeMinRelease;
	
	
	double durhamUse;
	double OWASAUse;
	double CaryUse;
	double raleighUse;
	
	double durhamCapacity;
	double CCRCapacity;
	double StQCapacity;
	double ULCapacity;
	double OWASACapacity;
	double lakeWBCapacity;
	double fallsLakeSupplyCapacity;
	
	double jordanLakeSupplyCapacity;
	double jordanLakeQualityCapacity;
	double jordanLakeSupplyStorage;
	double jordanLakeQualityStorage;
	
	
	double durhamRequest;
	double owasaRequest;
	double raleighRequest;
	double durhamRequestInt;
	double owasaRequestInt;
	double raleighRequestInt;
	double CaryTreatmentCapacity;
	double DurhamCaryCapacity;
	double DurhamOWASACapacity;
	double RaleighCaryCapacity;
	double numdays;
	double claytonGage;
	
	double raleighJordanCapacity;
	double raleighJordanStorage;
	double durhamJordanCapacity;
	double durhamJordanStorage;
	double owasaJordanCapacity;
	double owasaJordanStorage;
	double caryJordanCapacity;
	double caryJordanStorage;
	
	double jordanQualityFraction;
	double jordanRaleighFraction;
	double jordanCaryFraction;
	double jordanDurhamFraction;
	double jordanOWASAFraction;
	double raleighAllocation;
	double caryAllocation;
	double durhamAllocation;
	double owasaAllocation;
	
	

	//calcTransfers variables
	double percentFull_O;
	double percentFull_D;
	double percentFull_R;
	double caryTreatmentBuffer;
	double caryPeakingFactor;
	double caryExtraCapacity;
	double transferLimit;
	double totalRequest;
	ofstream out2;
};

#endif
