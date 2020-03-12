
#include <Stdlib.h>
#include <String.h>
#include <math.h>
#include <Psim.h>

#include "functions/GLOBAL_functions.h"
#include "functions/SM_functions.h"
//#include "functions/CTRL_functions.h"
//#include "functions/PLL_functions.h"

#define PSIM_IN_SCADA 0
#define PSIM_IN_ADC 2
#define PSIM_IN_REFS 9

#define PSIM_OUT_AUX 		0						// 2 saidas auxiliares
#define PSIM_OUT_SCADA 		2						// 6 saidas scada
#define PSIM_OUT_ENABLES	8               		// 5 saidas p/ enables
#define PSIM_OUT_PWM		13						// 6 saidas p/ dcdc
#define PSIM_OUT_DCDC		16						// 6 saidas p/ dcdc
#define PSIM_OUT_CTRL		22						// 8 saidas p/ ctrl

float cycle_time_30k = 0.0f;

//#include "functions.h"

// PLACE GLOBAL VARIABLES OR USER FUNCTIONS HERE...


///   function prototypes, "objects"



float ADC_LPFbuff_VA[2] = {0.0f, 0.0f}; 
float ADC_LPFbuff_VB[2] = {0.0f, 0.0f}; 
float ADC_LPFbuff_VC[2] = {0.0f, 0.0f}; 
float ADC_LPFbuff_IA[2] = {0.0f, 0.0f}; 
float ADC_LPFbuff_IB[2] = {0.0f, 0.0f}; 
float ADC_LPFbuff_IC[2] = {0.0f, 0.0f}; 


//enum ADC_state_machine{PHASE_init, PWM_A_start, PWM_A_wait, PWM_B_start, PWM_B_wait, PWM_C_start, PWM_C_wait, ADC_read_low, ADC_read_high};
//volatile int16_t ADC_state = PHASE_init;
float LPFbuff_k60_Ia_low[2] = {0.0f, 0.0f};
float LPFbuff_k60_Ib_low[2] = {0.0f, 0.0f};
float LPFbuff_k60_Ic_low[2] = {0.0f, 0.0f};
float LPFbuff_k60_Ia_high[2] = {0.0f, 0.0f};
float LPFbuff_k60_Ib_high[2] = {0.0f, 0.0f};
float LPFbuff_k60_Ic_high[2] = {0.0f, 0.0f};
float Ia_low = 0.0f, Ib_low = 0.0f, Ic_low = 0.0f;
float Ia_high = 0.0f, Ib_high = 0.0f, Ic_high = 0.0f;
float I_abc_filt[3] = {0.0f, 0.0f, 0.0f};

float vdc_division = 10.0f;
float Vm_[3] = {0.0f, 0.0f, 0.0f};
float CTRL_i_ref[3] = {0.0f, 0.0f, 0.0f};
float vmabc[3]={0.0f, 0.0f, 0.0f};
int m_abc[3]={0u, 0u, 0u};		// Modulation signal to the PWM

float DC_buff[2] = {0.0f, 0.0f};
float erro = 0.0f, Vdc = 0.0f, Vout = 0.0f;
float Vref = 24.0f;


float VOUT_buff[2] = {0.0f, 0.0f};
float VOUT_filt = 0.0f;
float VOUT_ctrl = 0.0f;

float I_abc_avg[3]={0.0f, 0.0f, 0.0f};	// AC avg currents
float I_abc_rms[3]={0.0f, 0.0f, 0.0f};	// AC rms currents

#define PI_Iabc_lim 5.0f   //Amperes
#define Kp_Iabc 0.002f
#define Ki_Iabc 0.05f
#define Kp_Vout 0.02f
#define Ki_Vout 15.0f


PI_tr_p PI_Ia;		//  = {{0.05f, 40.0f},{PI_Iabc_lim, -PI_Iabc_lim},{0.0f, 0.0f},0u};
PI_tr_p PI_Ib;		// = {{0.05f, 40.0f},{PI_Iabc_lim, -PI_Iabc_lim},{0.0f, 0.0f},0u};
PI_tr_p PI_Ic;		//  = {{0.05f, 40.0f},{PI_Iabc_lim, -PI_Iabc_lim},{0.0f, 0.0f},0u};

PI_tr_p PI_Vout;	//   = {{0.05f, 40.0f},{100, -100},{0.0f, 0.0f},0u};

int adc_updown = 0.0f;
#define Ts60 0.000016666667f
//#define wo_LPF_100 628.319f		// LPF cutoff frequency- 12.5 Hz
float LPF_K60_1500[3]={1, Ts60*wo_LPF_1500/(2+Ts60*wo_LPF_1500), -(Ts60*wo_LPF_1500-2)/(2+Ts60*wo_LPF_1500)};


/////////////////////////////////////////////////////////////////////
// FUNCTION: SimulationStep
//   This function runs at every time step.
//double t: (read only) time
//double delt: (read only) time step as in Simulation control
//double *in: (read only) zero based array of input values. in[0] is the first node, in[1] second input...
//double *out: (write only) zero based array of output values. out[0] is the first node, out[1] second output...
//int *pnError: (write only)  assign  *pnError = 1;  if there is an error and set the error message in szErrorMsg
//    strcpy(szErrorMsg, "Error message here..."); 
// DO NOT CHANGE THE NAME OR PARAMETERS OF THIS FUNCTION
void SimulationStep(
	double t, double delt, double *in, double *out,
	int *pnError, char * szErrorMsg,
	void ** reserved_UserData, int reserved_ThreadIndex, void * reserved_AppPtr)
{
// ENTER YOUR CODE HERE...

SCADA_enable = in[PSIM_IN_SCADA];
SCADA_disable = in[PSIM_IN_SCADA+1];

if(cycle_time_30k >= (float) CTRL_PERIOD/6) {
	
	/////////////////   PERIODIC CODE 60khz
	// if (adc_updown == 0.0f) {
	// I_abc[0] = in[PSIM_IN_ADC+3];  // 0.0f;
	// I_abc[1] = in[PSIM_IN_ADC+4];  // 0.0f;
	// I_abc[2] = in[PSIM_IN_ADC+5];  // 0.0f;

	// // Ia_low = LPF1(I_abc[0], LPFbuff_k60_Ia_low, LPF_K60_1500);
	// // Ib_low = LPF1(I_abc[1], LPFbuff_k60_Ib_low, LPF_K60_1500);
	// // Ic_low = LPF1(I_abc[2], LPFbuff_k60_Ic_low, LPF_K60_1500);

	// Ia_low = (I_abc[0] < Ia_low) ? I_abc[0] : Ia_low + 0.01f;
	// Ib_low = (I_abc[1] < Ib_low) ? I_abc[1] : Ib_low + 0.01f;
	// Ic_low = (I_abc[2] < Ic_low) ? I_abc[2] : Ic_low + 0.01f;

	// adc_updown = 1.0f;
	// //out[PSIM_OUT_AUX] = 1;
	// }
	// else {
	I_abc[0] = in[PSIM_IN_ADC+3];  // 0.0f;
	I_abc[1] = in[PSIM_IN_ADC+4];  // 0.0f;
	I_abc[2] = in[PSIM_IN_ADC+5];  // 0.0f;

	// Ia_high = LPF1(I_abc[0], LPFbuff_k60_Ia_high, LPF_K60_1500);
	// Ib_high = LPF1(I_abc[1], LPFbuff_k60_Ib_high, LPF_K60_1500);
	// Ic_high = LPF1(I_abc[2], LPFbuff_k60_Ic_high, LPF_K60_1500);

	Ia_high = (I_abc[0] > Ia_high) ? I_abc[0] : Ia_high - 0.002f;
	Ib_high = (I_abc[1] > Ib_high) ? I_abc[1] : Ib_high - 0.002f;
	Ic_high = (I_abc[2] > Ic_high) ? I_abc[2] : Ic_high - 0.002f;
	Ia_low = (I_abc[0] < Ia_low) ? I_abc[0] : Ia_low + 0.002f;
	Ib_low = (I_abc[1] < Ib_low) ? I_abc[1] : Ib_low + 0.002f;
	Ic_low = (I_abc[2] < Ic_low) ? I_abc[2] : Ic_low + 0.002f;

	// adc_updown = 0.0f;
	// //out[PSIM_OUT_AUX] = 1;
	// } 

	cycle_time_30k=0;
}
else
{
	//out[PSIM_OUT_AUX]=0;
	cycle_time_30k += delt;
}
/////////////////////////////////////////////////////////////////
if(cycle_time >= (float) CTRL_PERIOD) {
	/////////////////   PERIODIC CODE
	///// acquisition ADC's
	V_abc[0] = in[PSIM_IN_ADC];  // 0.0f;
	V_abc[1] = in[PSIM_IN_ADC+1];  // 0.0f;
	V_abc[2] = in[PSIM_IN_ADC+2];  // 0.0f;
	// I_abc[0] = in[PSIM_IN_ADC+3];  // 0.0f;
	// I_abc[1] = in[PSIM_IN_ADC+4];  // 0.0f;
	// I_abc[2] = in[PSIM_IN_ADC+5];  // 0.0f;
	V_DC = in[PSIM_IN_ADC+6];  // 0.0f;
	conv.Vdc = LPF1(V_DC, LPFbuff_vdc, LPF_K_500);//V_DC;
	conv_temp_C = 30.0f;  ///  no input var in PSIM
	conv.temp = conv_temp_C;
	V_abc[0] = LPF1(V_abc[0], ADC_LPFbuff_VA, LPF_K_1500);
	V_abc[1] = LPF1(V_abc[1], ADC_LPFbuff_VB, LPF_K_1500);
	V_abc[2] = LPF1(V_abc[2], ADC_LPFbuff_VC, LPF_K_1500);

	// I_abc[0] = LPF1(I_abc[0], ADC_LPFbuff_IA, LPF_K_1500);
	// I_abc[1] = LPF1(I_abc[1], ADC_LPFbuff_IB, LPF_K_1500);
	// I_abc[2] = LPF1(I_abc[2], ADC_LPFbuff_IC, LPF_K_1500);
	I_abc_avg[0]= (Ia_high+Ia_low) * 0.5f;
	I_abc_avg[1]= (Ib_high+Ib_low) * 0.5f;
	I_abc_avg[2]= (Ic_high+Ic_low) * 0.5f;
	
	I_abc_rms[0]= (Ia_high+Ia_low) * 0.577350269f;
	I_abc_rms[1]= (Ib_high+Ib_low) * 0.577350269f;
	I_abc_rms[2]= (Ic_high+Ic_low) * 0.577350269f;

	I_abc_filt[0]	=	Ia_high;//(	Ia_high + Ia_low 	) * 0.5f;
	I_abc_filt[1]	=	Ib_high;//(	Ib_high + Ib_low 	) * 0.5f;
	I_abc_filt[2]	=	Ic_high;//(	Ic_high + Ic_low 	) * 0.5f;

	I_abc_filt[0] = LPF1(I_abc_filt[0], ADC_LPFbuff_IA, LPF_K_10);
	I_abc_filt[1] = LPF1(I_abc_filt[1], ADC_LPFbuff_IB, LPF_K_10);
	I_abc_filt[2] = LPF1(I_abc_filt[2], ADC_LPFbuff_IC, LPF_K_10);
	
	//// end acquisition ADC's

	// Vdc = V_abc[0];
	// Vout = LPF1(Vdc, DC_buff, LPF_K_100);

	// ///////////////////////////////////////   BEGIN CONTROL


	 
	
	 CTRL_i_ref[0] = (I_abc_filt[0]+I_abc_filt[1]+I_abc_filt[2])/3;
	 CTRL_i_ref[1] = CTRL_i_ref[0];
	 CTRL_i_ref[2] = CTRL_i_ref[0];
	// CTRL_i_ref[0] = 0.0f;
	
	// /////////////// início PI corrente
	VOUT_filt = LPF1(V_abc[0], VOUT_buff, LPF_K_10);
	erro = (state == running) ?  (Vref - V_abc[0]) : 0;

	VOUT_ctrl= -PI_TR_par(erro, Ts, &PI_Vout);
	VOUT_ctrl *= -0.333333f;

	if (state == running) {
		
		Vm_[0] = VOUT_filt +  PI_TR_par(CTRL_i_ref[0] - I_abc_filt[0], Ts, &PI_Ia) + VOUT_ctrl;
		Vm_[1] = VOUT_filt +  PI_TR_par(CTRL_i_ref[1] - I_abc_filt[1], Ts, &PI_Ia) + VOUT_ctrl;
		Vm_[2] = VOUT_filt +  PI_TR_par(CTRL_i_ref[2] - I_abc_filt[2], Ts, &PI_Ia) + VOUT_ctrl;
	}
	else  {
		Vm_[0] = VOUT_filt + 0;
		Vm_[1] = VOUT_filt + 0;
		Vm_[2] = VOUT_filt + 0;
	}
	// /////////////// fim PI corrente

	 vdc_division = conv.Vdc;//(V_DC < 0.10) ? 0.10 : V_DC;   					//permitir divisão por 0
	 vdc_division *= 0.5;
	 //vdc_division = 1.0f / vdc_division; 			 //evitar calcular 3x uma divisão
	
	 // for (int i=0; i<3; i++){
	 // 	vmabc[i] = 0.5;//Vm_[i]*vdc_division;
	 // }
	vmabc[0] = -1+Vm_[0]/vdc_division;
	vmabc[1] = -1+Vm_[1]/vdc_division;
	vmabc[2] = -1+Vm_[2]/vdc_division;

	/////  state machine
	ConverterStateMachine();
	/////  end state machine

	/////////////////  END PERIODIC CODE
out[PSIM_OUT_AUX+1] = 1;
cycle_time=0;
}
else
{
	out[PSIM_OUT_AUX+1]=0;
	cycle_time += delt;
}
///////////////////////////////////////////////////////////////////////
out[PSIM_OUT_AUX+1]=state;

out[PSIM_OUT_SCADA+0] = SCADA_error_Ia;			//Flag
out[PSIM_OUT_SCADA+1] = SCADA_error_Ib;			//Flag
out[PSIM_OUT_SCADA+2] = SCADA_error_Ic;			//Flag
out[PSIM_OUT_SCADA+3] = SCADA_error_VDCmax;		//Flag
out[PSIM_OUT_SCADA+4] = SCADA_error_VDCmin;		//Flag
out[PSIM_OUT_SCADA+5] = SCADA_error_Tmax;		//Flag

out[PSIM_OUT_ENABLES+0] = EN_1;
out[PSIM_OUT_ENABLES+1] = EN_2;
out[PSIM_OUT_ENABLES+2] = EN_3;
out[PSIM_OUT_ENABLES+3] = 0;
out[PSIM_OUT_ENABLES+4] = 0;

out[PSIM_OUT_PWM+0] = vmabc[0];
out[PSIM_OUT_PWM+1] = vmabc[1];
out[PSIM_OUT_PWM+2] = vmabc[2];

out[PSIM_OUT_DCDC+0] = Vm_[0] ;
out[PSIM_OUT_DCDC+1] = Vm_[1] ;
out[PSIM_OUT_DCDC+2] = Vm_[2] ;
out[PSIM_OUT_DCDC+3] = CTRL_i_ref[0];
out[PSIM_OUT_DCDC+4] = CTRL_i_ref[1];
out[PSIM_OUT_DCDC+5] = CTRL_i_ref[2];

out[PSIM_OUT_CTRL+0] = I_abc_filt[0];
out[PSIM_OUT_CTRL+1] = I_abc_filt[1];
out[PSIM_OUT_CTRL+2] = I_abc_filt[2];
out[PSIM_OUT_CTRL+3] = Ia_low;
out[PSIM_OUT_CTRL+4] = Ia_high;
out[PSIM_OUT_CTRL+5] = VOUT_filt; 
out[PSIM_OUT_CTRL+6] = VOUT_ctrl;
out[PSIM_OUT_CTRL+7] = erro;

}




/////////////////////////////////////////////////////////////////////
// FUNCTION: SimulationBegin
//   Initialization function. This function runs once at the beginning of simulation
//   For parameter sweep or AC sweep simulation, this function runs at the beginning of each simulation cycle.
//   Use this function to initialize static or global variables.
//const char *szId: (read only) Name of the C-block 
//int nInputCount: (read only) Number of input nodes
//int nOutputCount: (read only) Number of output nodes
//int nParameterCount: (read only) Number of parameters is always zero for C-Blocks.  Ignore nParameterCount and pszParameters
//int *pnError: (write only)  assign  *pnError = 1;  if there is an error and set the error message in szErrorMsg
//    strcpy(szErrorMsg, "Error message here..."); 
// DO NOT CHANGE THE NAME OR PARAMETERS OF THIS FUNCTION
void SimulationBegin(
	const char *szId, int nInputCount, int nOutputCount,
	int nParameterCount, const char ** pszParameters,
	int *pnError, char * szErrorMsg,
	void ** reserved_UserData, int reserved_ThreadIndex, void * reserved_AppPtr)
{
// ENTER INITIALIZATION CODE HERE...

//PI_Ia   = {{0.05f, 40.0f},{PI_Iabc_lim, -PI_Iabc_lim},{0.0f, 0.0f},0u};
PI_Ia.K[0] = Kp_Iabc;
PI_Ia.K[1] = Ki_Iabc;
PI_Ia.sat[0] = 100.0f;
PI_Ia.sat[1] = -100.0f;

//PI_Ib   = {{0.05f, 40.0f},{PI_Iabc_lim, -PI_Iabc_lim},{0.0f, 0.0f},0u};
PI_Ib.K[0] = Kp_Iabc;
PI_Ib.K[1] = Ki_Iabc;
PI_Ib.sat[0] = 100.0f;
PI_Ib.sat[1] = -100.0f;

//PI_Ic   = {{0.05f, 40.0f},{PI_Iabc_lim, -PI_Iabc_lim},{0.0f, 0.0f},0u};
PI_Ic.K[0] = Kp_Iabc;
PI_Ic.K[1] = Ki_Iabc;
PI_Ic.sat[0] = 100.0f;
PI_Ic.sat[1] = -100.0f;

//PI_Vout   = {{0.05f, 40.0f},{100, -100},{0.0f, 0.0f},0u};
PI_Vout.K[0] = Kp_Vout;
PI_Vout.K[1] = Ki_Vout;
PI_Vout.sat[0] = 150.0f;
PI_Vout.sat[1] = -150.0f;

vdc_division = 10.0f;

}


/////////////////////////////////////////////////////////////////////
// FUNCTION: SimulationEnd
//   Termination function. This function runs once at the end of simulation
//   For parameter sweep or AC sweep simulation, this function runs at the end of each simulation cycle.
//   Use this function to de-allocate any allocated memory or to save the result of simulation in an alternate file.
// Ignore all parameters for C-block 
// DO NOT CHANGE THE NAME OR PARAMETERS OF THIS FUNCTION
void SimulationEnd(const char *szId, void ** reserved_UserData, int reserved_ThreadIndex, void * reserved_AppPtr)
{




}


