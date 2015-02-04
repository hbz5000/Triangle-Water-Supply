#ifndef InsuranceFund_h
#define InsuranceFund_h

class InsuranceFund
{
public:
	
	void subtract(double losses);
	void add(double payment);
	void initializeRealization();
	double calcAnnualLosses(double annualRevenue, bool addInterest);
	int getInsuranceStage(double demandBaseline, double inflows, double* insuranceOn, double* insuranceOff,
							 bool useInsRisk, double insRisk);
	void setInsurancePayment(double payment, int insuranceLevel);

	double capacity;
	double fundSize, totalDebit;
	double insuranceStorage, insurancePayment, percentLoss, annualLosses;
	int numDebit, insuranceStage;

private:
	double thing;
	
};	

#endif
