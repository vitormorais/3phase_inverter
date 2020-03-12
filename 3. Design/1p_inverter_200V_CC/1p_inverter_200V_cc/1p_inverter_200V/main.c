/*
 * main.c
 *
 *  Created on: 2019 Nov 21 12
 *  Author: VMORAIS
 *	Version 1p_inv
 */

#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <probe_scope.h>
#include "init.h"
#include "functions.h"
#include "CTRL_functions.h"

float scope1, scope2, scope3;
uint32_t scopeu1, scopeu2, scopeu3;
uint32_t counter=1u;
uint32_t fault=0u;

float vDQp_ref[2]={1.0f, 0.0f};
float vDQp[2]={0.0f, 0.0f};
float vABp[2]={0.0f, 0.0f};

float vDQn_ref[2]={0.0f, 0.0f};
float vDQn[2]={0.0f, 0.0f};
float vABn[2]={0.0f, 0.0f};
float Vn=0.5f, V5=0.0f, V7=0.0f, V11=0.0f, V13=0.0f;

float vDQ5[2]={0.0f, 0.0f};
float vAB5[2]={0.0f, 0.0f};
float vabc5[3]={0.0f, 0.0f, 0.0f};
float vDQ7[2]={0.0f, 0.0f};
float vAB7[2]={0.0f, 0.0f};
float vabc7[3]={0.0f, 0.0f, 0.0f};
float vDQ11[2]={0.0f, 0.0f};
float vAB11[2]={0.0f, 0.0f};
float vabc11[3]={0.0f, 0.0f, 0.0f};
float vDQ13[2]={0.0f, 0.0f};
float vAB13[2]={0.0f, 0.0f};
float vabc13[3]={0.0f, 0.0f, 0.0f};

float vmAB[2]={0.0f, 0.0f};

float vmabc[3]={0.0f, 0.0f, 0.0f};

float wt_buff5=0.0f, wt_buff7=0.0f, wt_buff11=0.0f, wt_buff13=0.0f;
float wt5=0.0f, wt7=0.0f, wt11=0.0f, wt13=0.0f;
float wt_jump=0.0f;

////  vĂ­tor
float scope_adc1, scope_adc2, scope_adc3, scope_adc4, scope_adc5, scope_adc6, scope_adc7;
float aux_var1, aux_var2, aux_var3, aux_var4, aux_var5, aux_var6, aux_var7;

float LPFbuff_vdc[2] = {0.0f, 0.0f};
float LPFbuff_temp[2] = {0.0f, 0.0f};
float LPFbuff_v[2] = {0.0f, 0.0f};
float LPFbuff_v2[2] = {0.0f, 0.0f};

float AB_aux[2] = {0.0f, 0.0f};
float dq_aux[2] = {0.0f, 0.0f};
float wt_aux = 0.0f;
float w_aux = 100.0f*PI;
float LPFbuff_w_aux[2] = {0.0f, 0.0f};
float LPFbuff_dq1_aux[2] = {0.0f, 0.0f};
float LPFbuff_dq2_aux[2] = {0.0f, 0.0f};

//// filters ADC AC
float ADC_LPFbuff_VA[2] = {0.0f, 0.0f}; 
float ADC_LPFbuff_VB[2] = {0.0f, 0.0f}; 
float ADC_LPFbuff_VC[2] = {0.0f, 0.0f}; 
float ADC_LPFbuff_IA[2] = {0.0f, 0.0f}; 
float ADC_LPFbuff_IB[2] = {0.0f, 0.0f}; 
float ADC_LPFbuff_IC[2] = {0.0f, 0.0f}; 


int SCADA_enable = 0;			//ON-OFF micrium button
int SCADA_disable = 0;			//ON-OFF micrium button
int SCADA_status_button = 0;    //ON-OFF micrium button
float SCADA_Imax = 15.0f;		//Maximum current
float SCADA_VDCmax = 180.0f;	//Maximum Voltage DCBUS
float SCADA_VDCmin = -10.0f;	//Minimum Voltage DCBUS
float SCADA_Tmax = 60.0f;		//Maximum converter temperature

int SCADA_error_Ia = 0;			//Flag
int SCADA_error_Ib = 0;			//Flag
int SCADA_error_Ic = 0;			//Flag
int SCADA_error_VDCmax = 0;		//Flag
int SCADA_error_VDCmin = 0;		//Flag
int SCADA_error_Tmax = 0;		//Flag
int timer_sm = 0;

float SCADA_Id_ref = 0;
float SCADA_Iq_ref = 0;
float SCADA_Vdc_ref = 200;


uint8_t no_fault(void) {
	if ( (I_abc[0] > SCADA_Imax) || (I_abc[0] < -SCADA_Imax) ||
			(I_abc[1] > SCADA_Imax) || (I_abc[1] < -SCADA_Imax) ||
			(I_abc[2] > SCADA_Imax) || (I_abc[2] < -SCADA_Imax) ||
			(conv.Vdc > SCADA_VDCmax) ||
			(conv.Vdc < SCADA_VDCmin) ||
			(conv.temp > SCADA_Tmax)) {
		return 1;
	}
	else {
		return 0;
	}

}

void ConverterStateMachine(void){

	timer_sm += 1;
	switch(state){
	case init:
		if ((SCADA_enable==1) && (timer_sm > (500*_1MS))) {
			state = pc1;
			timer_sm = 0;
		}
		break;
	case pc1:
		if ((timer_sm > (500*_1MS)) && (conv.Vdc > 120.0f)/**/) {
			state = pc2;
			timer_sm = 0;
		}
		if (SCADA_disable==1) {
			//SCADA_enable = 0;
			state = init;
			timer_sm = 0;
		}
		break;
	case pc2:
		if (timer_sm > (500*_1MS) ){
			state = idle;
			timer_sm = 0;
		}
		if (SCADA_disable==1) {
			//SCADA_enable = 0;
			state = init;
			timer_sm = 0;
		}
		break;
	case idle:
		if (SCADA_enable==1 && no_fault()==0 ) {
			//SCADA_enable = 0;
			SCADA_error_Ia = 0;
			SCADA_error_Ib = 0;
			SCADA_error_Ic = 0;
			SCADA_error_VDCmax = 0;
			SCADA_error_VDCmin = 0;
			SCADA_error_Tmax = 0;
			state = running;
		}
		if (SCADA_disable==1) {
			//SCADA_enable = 0;
			state = init;
			timer_sm = 0;
		}
		break;
	case running:
		if (SCADA_disable==1) {
			//SCADA_enable = 0;
			state = init;
		}
		//FALHAS
		if ((I_abc[0] > SCADA_Imax) || (I_abc[0] < -SCADA_Imax)) {
			state = init;
			SCADA_error_Ia = 1;
		}
		if ((I_abc[1] > SCADA_Imax) || (I_abc[1] < -SCADA_Imax)) {
			state = init;
			SCADA_error_Ib = 1;
		}
		if ((I_abc[2] > SCADA_Imax) || (I_abc[2] < -SCADA_Imax)) {
			state = init;
			SCADA_error_Ic = 1;
		}
		if ((conv.Vdc > SCADA_VDCmax)) {
			state = init;
			SCADA_error_VDCmax = 1;
		}
		if ((conv.Vdc < SCADA_VDCmin)) {
			state = init;
			SCADA_error_VDCmin = 1;
		}
		if ((conv.temp > SCADA_Tmax)) {
			state = init;
			SCADA_error_Tmax = 1;
		}
		break;
	}


	if (state==init){
		DIGITAL_IO_SetOutputLow(&EN_1);
		DIGITAL_IO_SetOutputLow(&EN_2);
		DIGITAL_IO_SetOutputLow(&EN_3);
		DIGITAL_IO_SetOutputLow(&SSR1);//PCR_SW = 0;
		DIGITAL_IO_SetOutputLow(&SSR2);//PCS_SW = 0;
		SCADA_status_button = 0;
	}
	else if (state==pc1){
		DIGITAL_IO_SetOutputLow(&EN_1);
		DIGITAL_IO_SetOutputLow(&EN_2);
		DIGITAL_IO_SetOutputLow(&EN_3);
		DIGITAL_IO_SetOutputLow(&SSR1);//PCR_SW = 0;
		DIGITAL_IO_SetOutputHigh(&SSR2);//PCS_SW = 1;
		SCADA_status_button = 0;
	}
	else if (state==pc2){
		DIGITAL_IO_SetOutputLow(&EN_1);
		DIGITAL_IO_SetOutputLow(&EN_2);
		DIGITAL_IO_SetOutputLow(&EN_3);
		DIGITAL_IO_SetOutputHigh(&SSR1);//PCR_SW = 1;
		DIGITAL_IO_SetOutputHigh(&SSR2);//PCS_SW = 1;
		SCADA_status_button = 0;
	}
	else if (state==idle){
		DIGITAL_IO_SetOutputLow(&EN_1);
		DIGITAL_IO_SetOutputLow(&EN_2);
		DIGITAL_IO_SetOutputLow(&EN_3);
		DIGITAL_IO_SetOutputHigh(&SSR1);//PCR_SW = 1;
		DIGITAL_IO_SetOutputHigh(&SSR2);//PCS_SW = 1;
		SCADA_status_button = 0;
	}
	else if (state==running){
		DIGITAL_IO_SetOutputHigh(&EN_1);
		DIGITAL_IO_SetOutputHigh(&EN_2);
		DIGITAL_IO_SetOutputHigh(&EN_3);
		DIGITAL_IO_SetOutputHigh(&SSR1);//PCR_SW = 1;
		DIGITAL_IO_SetOutputHigh(&SSR2);//PCS_SW = 1;
		SCADA_status_button = 1;
	}

}


void Update_PWM(void){
	DIGITAL_IO_SetOutputHigh(&P_1_0);
	///////////////////   BEGGINING OF PERIODIC CONTROL INTERRUPTION


	/// ADC acquisition  - AC Voltages
	V_abc_adc[0]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_1_V1_1);
	V_abc_adc[1]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_2_V2_1);
	V_abc_adc[2]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_2_V2_3);

	V_abc[0]=(V_abc_adc[0]-(1800.0f + 12.98f)) * 0.720f;
	V_abc[1]=(V_abc_adc[1]-(1800.0f + 2.69f)) * 0.720f;
	V_abc[2]=(V_abc_adc[2]-(1800.0f + 16.69f)) * 0.720f;

	V_abc[0] = LPF1(V_abc[0], ADC_LPFbuff_VA, LPF_K_1500);
	V_abc[1] = LPF1(V_abc[1], ADC_LPFbuff_VB, LPF_K_1500);
	V_abc[2] = LPF1(V_abc[2], ADC_LPFbuff_VC, LPF_K_1500);

	/// ADC acquisition  - AC Currents
	I_abc_adc[0]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_2_V2_2);
	I_abc_adc[1]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_1_V1_4);
	I_abc_adc[2]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_1_V1_3);

	I_abc[0]=(I_abc_adc[0]-(1800.0f - 18.18f)) * 0.022f;
	I_abc[1]=(I_abc_adc[1]-(1800.0f - 15.91f)) * 0.022f;
	I_abc[2]=(I_abc_adc[2]-(1800.0f - 19.55f)) * 0.022f;

	I_abc[0] = LPF1(I_abc[0], ADC_LPFbuff_IA, LPF_K_1500);
	I_abc[1] = LPF1(I_abc[1], ADC_LPFbuff_IB, LPF_K_1500);
	I_abc[2] = LPF1(I_abc[2], ADC_LPFbuff_IC, LPF_K_1500);

	/// ADC acquisition  - DC BUS Voltage
	V_DC_adc = ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_1_V1_2);
	V_DC = (V_DC_adc-(1800.0f + 9.7f)) * -0.58f;

	conv.Vdc = LPF1(V_DC, LPFbuff_vdc, LPF_K_100);

	/// ADC acquisition  - Converter temperature
	termistor_temp = ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_3_V3_1);
	float conv_temperature = termistor_temp; //auxiliar variable to avoid multiple
	conv_temp_C =       (conv_temperature*conv_temperature*-0.00016973) +  //x^2
			(conv_temperature*0.69969095) +  //x^1
			-653.10731030;  //x^0 */
	conv_temp_C += 7.0f;  //compensação para temperatura da junção

	conv.temp = LPF1(conv_temp_C, LPFbuff_temp, LPF_K_1);


	///  micrium scope variables
	scope_adc7 = V_DC;
	scope_adc1 = I_abc[0]; scope_adc2 = I_abc[1]; scope_adc3 = I_abc[2];
	scope_adc4 = V_abc[0]; scope_adc5 = V_abc[1]; scope_adc6 = V_abc[2];

	////////////////////////////////////////   PLACE CONVERTER CONTROL HERE


	///// begin PLL
	QSG(V_abc[2], dqsg.w, &(dqsg.QSG_d));
	SOGI_v_AB[0] = dqsg.QSG_d.v_AB[0];
	SOGI_v_AB[1] = dqsg.QSG_d.v_AB[1];//*(w_aux/(100.0f*PI));
	SOGI_v_AB[1] *= (PLL_w / PLL_w0);

	AB_dq(SOGI_v_AB, PLL_Vdq, PLL_wt);
	PLL_Vdq[0] = LPF1(PLL_Vdq[0], PLL_LPFbuff_d, LPF_K_500);
	PLL_Vdq[1] = LPF1(PLL_Vdq[1], PLL_LPFbuff_q, LPF_K_500);
	conv.Vdq[0] = PLL_Vdq[0];
	conv.Vdq[1] = PLL_Vdq[1];

	PLL_w = PI_TR_p(PLL_Vdq[1], Ts, &PI_PLL) + PLL_w0;
	PLL_wt = fw_integ(PLL_w, &PLL_wt_buff, Ts, PI2);
	PLL_w = LPF1(PLL_w, PLL_LPFbuff_w, LPF_K_10);
	conv.wt = PLL_wt + ((PLL_w*PLL_w - PLL_w0*PLL_w0)/(1.4142136f*PLL_w0*PLL_w));
	///// end PLL

	///// begin CTRL: Idq transform
	i_dqsg.w = dqsg.w;
	QSG(I_abc[0], i_dqsg.w, &(i_dqsg.QSG_d));
	SOGI_i_AB[0] =  i_dqsg.QSG_d.v_AB[0];
	SOGI_i_AB[1] =  i_dqsg.QSG_d.v_AB[1];
	AB_dq(SOGI_i_AB, CTRL_idq, PLL_wt);
	///// end CTRL: Idq transform
	
	/////////////////////   switch MODO controlo barramento
/*     VDC_BUS_REF = 200.0f; //SCADA_Vdc_ref; 							//recebe referência do micrium/CAN
     //vdc_bus_error = ((VDC_BUS_REF)-(conv.Vdc));						//calcula erro para PI: tensão  
     vdc_bus_error = ((VDC_BUS_REF*VDC_BUS_REF)-(conv.Vdc*conv.Vdc));	//calcula erro para PI: ENERGIA dc_bus
     vdc_bus_error = (state == running) ? vdc_bus_error : 0;			//hold ao erro se não estiver em running
     vdc_bus_piout = -PI_TR_p(vdc_bus_error, Ts, &PI_Vdc); 				//PI CTRL tensão barramento DC
     I_dq_ref[0] = vdc_bus_piout;										//passa referência corrente activa para transf Idq->IAB 

    CTRL_idq_ref[1] = (state == running) ? SCADA_Iq_ref : 0;			//recebe referência do micrium/CAN
    CTRL_idq_ref[1] = (CTRL_idq_ref[1] < -5) ? -5 : CTRL_idq_ref[1]; 	//saturação
    CTRL_idq_ref[1] = (CTRL_idq_ref[1] > 5)  ?  5 : CTRL_idq_ref[1]; 	//saturação
    //CTRL_idq_ref[1] = (state == running) ? 0 : 0;					
     I_dq_ref[1] = PI_TR_p(CTRL_idq_ref[1] - CTRL_idq[1], Ts, &PI_q);	//PI CTRL corrente reactiva
*/
    /////////////////////   switch MODO controlo PQ
    CTRL_idq_ref[0] = (state == running) ? SCADA_Id_ref : 0;   			//recebe referência do micrium/CAN
    CTRL_idq_ref[0] = (CTRL_idq_ref[0] < 0) ? 0 : CTRL_idq_ref[0];  	//saturação !!!!!!!!!!!!  se barramento=rectificador -> min=0
    CTRL_idq_ref[0] = (CTRL_idq_ref[0] > 5) ? 5 : CTRL_idq_ref[0];  	//saturação
    //CTRL_idq_ref[0] = (state == running) ? 0 : 0;

    CTRL_idq_ref[1] = (state == running) ? SCADA_Iq_ref : 0;			//recebe referência do micrium/CAN
    CTRL_idq_ref[1] = (CTRL_idq_ref[1] < -5) ? -5 : CTRL_idq_ref[1]; 	//saturação
    CTRL_idq_ref[1] = (CTRL_idq_ref[1] > 5)  ?  5 : CTRL_idq_ref[1]; 	//saturação
    //CTRL_idq_ref[1] = (state == running) ? 0 : 0;
	I_dq_ref[0] = PI_TR_p(CTRL_idq_ref[0] - CTRL_idq[0], Ts, &PI_d);	//PI CTRL corrente activa
	I_dq_ref[1] = PI_TR_p(CTRL_idq_ref[1] - CTRL_idq[1], Ts, &PI_q);  	//PI CTRL corrente reactiva

	/////////////////////   END switch

	///// begin Idq to AB references transform
	dq_AB(I_dq_ref, I_AB, conv.wt);										//transf Idq->IAB
    I_AB[0] = (state == running) ? I_AB[0] : 0;							//passa referência para PR
    ///// end Idq to AB references transform

	///// begin PR_ctrl
	set_PR_ref(&PR_Test, I_AB[0]);										//Atribui Ialfa como referêcia p/ PR
	Iu_filt = LPF1(I_abc[0], PR_LPFbuff_Iu, LPF_K_500);					//Filtragem !!!!!!!!!  Ja feita acima
	PR_output = Calc_prStruct(&PR_Test, Iu_filt, 100.0f*PI) ;			//Corre PR CTRL

	PR_output = (state == running) ? PR_output : 0;						//passa referência para modulação
	PR_output = (PR_output >  15.0f) ?  15.0f : PR_output;				//satura saída
	PR_output = (PR_output < -15.0f) ? -15.0f : PR_output;				//satura saída

	//PR_output = 0.0f;
	vdc_division = (conv.Vdc < 10) ? 10 : conv.Vdc;   					//permitir divisão por 0
	vdc_division *= 2.0f;  // ganho do transformador					//!!!!!!!!!  UMA VEZ QUE MEDIDA Vac É NO PRIMÁRIO DO TRANSFORMADOR, ENTÃO É COLOCADO AQUI UMA "ESTIMATIVA" DO GANHO

	m_u = (PR_output + SOGI_v_AB[0]) / (vdc_division);					//MODULAÇÃO: soma Va_PLL a PR_out
	m_v = -m_u;															//Gera a referência p/ segundo braço

	vmabc[0] = m_u * 1.0f;												//A RETIRAR
	vmabc[1] = m_v * 1.0f;												//A RETIRAR


	for (int i=0; i<3; i++){
		vmabc[i]=fminf(vmabc[i],0.97f);
		vmabc[i]=fmaxf(vmabc[i],-0.97f);
		// Convert float to uint32 voltage modulator wave
		m_abc[i]=((int32_t)(10000.0f*vmabc[i])>>1)+5000u;
	}

	///  micrium scope variables
	scopeu1=m_abc[0];
	scopeu2=m_abc[1];
	scopeu3=m_abc[2];

	// update PWM duty cycles
	PWM_SetDutyCycle(&PWM_a,m_abc[0]);
	PWM_SetDutyCycle(&PWM_b,m_abc[1]);
	PWM_SetDutyCycle(&PWM_c,m_abc[2]);

	ConverterStateMachine();

	///////////////////   END OF PERIODIC CONTROL INTERRUPTION
	DIGITAL_IO_SetOutputLow(&P_1_0);
}

int main(void)
{
	DAVE_STATUS_t status;
	status = DAVE_Init();           /* Initialization of DAVE APPs  */
	if(status != DAVE_STATUS_SUCCESS)
	{
		/* Placeholder for error handler code. The while loop below can be replaced
		 * with an user error handler. */
		XMC_DEBUG("DAVE APPs initialisation failed\n");
		while(1U){}
	}
	ProbeScope_Init(SystemCoreClock/TICKS_PER_SECOND);   /// for micrium uScope


	/////////////   DISABLE OUPUTS
	DIGITAL_IO_SetOutputLow(&ENABLE);
	DIGITAL_IO_SetOutputLow(&FAULT_RST);

	DIGITAL_IO_SetOutputLow(&EN_1);
	DIGITAL_IO_SetOutputLow(&EN_2);
	DIGITAL_IO_SetOutputLow(&EN_3);

	PWM_Stop(&PWM_a);
	PWM_Stop(&PWM_b);
	PWM_Stop(&PWM_c);

	////////////   ENABLE OUTPUTS/TIMERS
	PWM_Start(&PWM_update);
	PWM_Start(&PWM_a);
	PWM_Start(&PWM_b);
	PWM_Start(&PWM_c);



	////////////  SET GLOBAL VARS HERE
	Init_prStruct(&PR_Test, PR_Kp, PR_Kr, PR_Wi);

	while(1U)
	{
		///////////   IMPLEMENT STATE MACHINE HERE
		//ConverterStateMachine();
		//////////   END OF STATE MACHINE
		ProbeScope_Sampling();    /// for micrium uScope
	}
}
