
#include <Stdlib.h>
#include <String.h>
#include <math.h>
#include <Psim.h>

#include "functions/GLOBAL_functions.h"
#include "functions/SM_functions.h"
#include "functions/CTRL_functions.h"
#include "functions/PLL_functions.h"

#define PSIM_IN_SCADA 0
#define PSIM_IN_ADC 2
#define PSIM_IN_REFS 9

#define PSIM_OUT_AUX 		0						// 2 saidas auxiliares
#define PSIM_OUT_SCADA 		2						// 6 saidas scada
#define PSIM_OUT_ENABLES	8               		// 5 saidas p/ enables
#define PSIM_OUT_PLL		13						// 6 saidas p/ pll
#define PSIM_OUT_CTRL		19						// 8 saidas p/ ctrl


//#include "functions.h"

// PLACE GLOBAL VARIABLES OR USER FUNCTIONS HERE...




///   function prototypes, "objects"

float PR_LPFbuff_Iu[2] = {0.0f, 0.0f}; 
float Iu_filt = 0.0f;



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


/////////////////////////////////////////////////////////////////
if(cycle_time >= (float) CTRL_PERIOD) {
	/////////////////   PERIODIC CODE
	///// acquisition ADC's
	V_abc[0] = in[PSIM_IN_ADC];  // 0.0f;
	V_abc[1] = in[PSIM_IN_ADC+1];  // 0.0f;
	V_abc[2] = in[PSIM_IN_ADC+2];  // 0.0f;
	I_abc[0] = in[PSIM_IN_ADC+3];  // 0.0f;
	I_abc[1] = in[PSIM_IN_ADC+4];  // 0.0f;
	I_abc[2] = in[PSIM_IN_ADC+5];  // 0.0f;
	V_DC = in[PSIM_IN_ADC+6];  // 0.0f;
	conv.Vdc = LPF1(V_DC, LPFbuff_vdc, LPF_K_100);//V_DC;
	conv_temp_C = 30.0f;  ///  no input var in PSIM
	conv.temp = conv_temp_C;
	///// end acquisition ADC's

	///// begin PLL
	QSG(V_abc[0], dqsg.w, &(dqsg.QSG_d));
	SOGI_v_AB[0] = dqsg.QSG_d.v_AB[0];
	SOGI_v_AB[1] = dqsg.QSG_d.v_AB[1];//*(w_aux/(100.0f*PI));
	SOGI_v_AB[1] *= (PLL_w / PLL_w0);

	AB_dq(SOGI_v_AB, PLL_Vdq, PLL_wt);
	PLL_Vdq[0] = LPF1(PLL_Vdq[0], PLL_LPFbuff_d, LPF_K_500);
	PLL_Vdq[1] = LPF1(PLL_Vdq[1], PLL_LPFbuff_q, LPF_K_500);
	conv.Vdq[0] = PLL_Vdq[0];
	conv.Vdq[1] = PLL_Vdq[1];

	PLL_w = PI_TR_par(PLL_Vdq[1], Ts, &PI_PLL) + PLL_w0;
	PLL_wt = fw_integ(PLL_w, &PLL_wt_buff, Ts, PI2);
	PLL_w = LPF1(PLL_w, PLL_LPFbuff_w, LPF_K_10);
	conv.wt = PLL_wt + ((PLL_w*PLL_w - PLL_w0*PLL_w0)/(1.4142136f*PLL_w0*PLL_w));
	///// end PLL

	///// begin DC_ctrl
	//run_DC_ctrl();
    VDC_BUS_REF = 400.0f;
    if (state == running) {
    	vdc_bus_error = ((VDC_BUS_REF)-(conv.Vdc));
        //vdc_bus_error = ((VDC_BUS_REF*VDC_BUS_REF)-(conv.Vdc*conv.Vdc));
    }
    else  {
        vdc_bus_error = 0;
    }

    I_dq_ref[0] = -PI_TR_par(vdc_bus_error, Ts, &PI_Vdc);//10; //activa
    I_dq_ref[1] = 0;  //reactiva
    dq_AB(I_dq_ref, I_AB, conv.wt);

	///// end DC_ctrl

	///// begin PR_ctrl
	if (state != running) {
		set_PR_ref(&PR_Test, 0);
	}
	set_PR_ref(&PR_Test, I_AB[0]);
	Iu_filt = LPF1(I_abc[0], PR_LPFbuff_Iu, LPF_K_500);
	PR_output = Calc_prStruct(&PR_Test, Iu_filt, 100.0f*PI_const) ;
	if (state != running) {
		PR_output = 0;
	}

	PR_output = (PR_output >  20.0f) ?  20.0f : PR_output;
	PR_output = (PR_output < -20.0f) ? -20.0f : PR_output;

	vdc_division = (conv.Vdc < 1) ? 1 : conv.Vdc;
	m_u = (PR_output + V_abc[0]) / (vdc_division);
	m_v = -m_u;
	///// end PR_ctrl

	/////  state machine
	ConverterStateMachine();
	/////  end state machine

	/////////////////  END PERIODIC CODE
out[PSIM_OUT_AUX] = 1;
cycle_time=0;
}
else
{
	out[PSIM_OUT_AUX]=0;
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
out[PSIM_OUT_ENABLES+3] = PCR_SW;
out[PSIM_OUT_ENABLES+4] = PCS_SW;

out[PSIM_OUT_PLL+0] = SOGI_v_AB[0];
out[PSIM_OUT_PLL+1] = SOGI_v_AB[1];
out[PSIM_OUT_PLL+2] = PLL_Vdq[0];
out[PSIM_OUT_PLL+3] = PLL_Vdq[1];
out[PSIM_OUT_PLL+4] = PLL_w;
out[PSIM_OUT_PLL+5] = PLL_wt;

out[PSIM_OUT_CTRL+0] = in[9];
out[PSIM_OUT_CTRL+1] = 0;
out[PSIM_OUT_CTRL+2] = I_dq_ref[0];
out[PSIM_OUT_CTRL+3] = I_dq_ref[1];
out[PSIM_OUT_CTRL+4] = I_AB[0];
out[PSIM_OUT_CTRL+5] = PR_output;
out[PSIM_OUT_CTRL+6] = m_u;
out[PSIM_OUT_CTRL+7] = m_v;
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

Init_prStruct(&PR_Test, PR_Kp, PR_Kr, PR_Wi);
//dqsg = {100.0f*PI_const, {{0.0f, 0.0f}, 1.4142136f, 0.0f, 0.0f, Ts}, {{0.0f, 0.0f}, 1.4142136f, 0.0f, 0.0f, Ts}};
dqsg.w = 100.0f*PI_const;
dqsg.QSG_d.K = 1.4142136f;
dqsg.QSG_d.Ti = Ts;
dqsg.QSG_q.K = 1.4142136f;
dqsg.QSG_q.Ti = Ts;
//PI_PLL = {{Kp_PLL, Ki_PLL},{942.47780f, -942.47780f},{0.0f, 0.0f},0};
PI_PLL.K[0] = Kp_PLL;
PI_PLL.K[1] = Ki_PLL;
PI_PLL.sat[0] = 942.47780f;
PI_PLL.sat[1] = -942.47780f;

//PI_Vdc = {{Kp_V_DC, Ki_V_DC},{15, -15},{0.0f, 0.0f},0u};
PI_Vdc.K[0] = Kp_V_DC;
PI_Vdc.K[1] = Ki_V_DC;
PI_Vdc.sat[0] = 15.0f;
PI_Vdc.sat[1] = -15.0f;

PLL_LPFbuff_w[0] = PLL_w0;
PLL_LPFbuff_w[1] = PLL_w0;

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


