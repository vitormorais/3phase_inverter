/*
 * main.c
 *
 *  Created on: 2017 Oct 24 09:50:12
 *  Author: JORGE
 *	Version 3p_THIPWM_v5
 */

#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <probe_scope.h>
#include "init.h"
#include "functions.h"

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

////  vítor
float scope_adc1, scope_adc2, scope_adc3, scope_adc4, scope_adc5, scope_adc6, scope_adc7;


void Update_PWM(void){
	DIGITAL_IO_SetOutputHigh(&P_1_0);

	/// ADC acquisition
	V_abc_adc[0]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_1_V1_1);
	V_abc_adc[1]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_2_V2_1);
	V_abc_adc[2]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_2_V2_3);

	V_abc[0]=(V_abc_adc[0]-1814.820f) * 0.720f;//*ADC.toVac*3.00000f;
	V_abc[1]=(V_abc_adc[1]-1804.70f)  * 0.720f;//*ADC.toVac*3.00000f;
	V_abc[2]=(V_abc_adc[2]-1818.330f) * 0.720f;//*ADC.toVac*3.00000f;

	I_abc_adc[0]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_2_V2_2);
	I_abc_adc[1]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_1_V1_4);
	I_abc_adc[2]=ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_1_V1_3);

	float my_vdc = ADC_MEASUREMENT_ADV_GetResult(&ADC_MEASUREMENT_1_V1_2);
	scope_adc7 = my_vdc;

	scope_adc1 = V_abc_adc[0];
	scope_adc2 = V_abc_adc[1];
	scope_adc3 = V_abc_adc[2];

	scope_adc4 = V_abc[0];
	scope_adc5 = V_abc[1];
	scope_adc6 = V_abc[2];

	// Ensure variables interval
	vDQp_ref[0]=fminf(vDQp_ref[0],1.0f);
	vDQp_ref[0]=fmaxf(vDQp_ref[0],-1.0f);
	vDQp_ref[1]=fminf(vDQp_ref[1],1.0f);
	vDQp_ref[1]=fmaxf(vDQp_ref[1],-1.0f);
	vDQn_ref[0]=fminf(vDQn_ref[0],1.0f);
	vDQn_ref[0]=fmaxf(vDQn_ref[0],-1.0f);
	vDQn_ref[1]=fminf(vDQn_ref[1],1.0f);
	vDQn_ref[1]=fmaxf(vDQn_ref[1],-1.0f);
	Vn=fminf(Vn,1.0f);
	Vn=fmaxf(Vn,0.0f);
	V5=fminf(V5,1.0f);
	V5=fmaxf(V5,0.0f);
	V7=fminf(V7,1.0f);
	V7=fmaxf(V7,0.0f);
	V11=fminf(V11,1.0f);
	V11=fmaxf(V11,0.0f);
	V13=fminf(V13,1.0f);
	V13=fmaxf(V13,0.0f);
	//w=fminf(w,345.5752f);
	//w=fmaxf(w,282.7433f);

// PLL or emulated phase angle
	for (uint8_t i=0; i<2; i++){
		vDQp[i]=vDQp_ref[i]*Vn;
		vDQn[i]=vDQn_ref[i]*Vn;
	}
	vDQ5[0]=V5*Vn;
	vDQ7[0]=V7*Vn;
	vDQ11[0]=V11*Vn;
	vDQ13[0]=V13*Vn;

	wt=fw_integ(w, &wt_buff, Ts, PI2)+wt_jump;
	wt5=fw_integ(5.0f*w, &wt_buff5, Ts, PI2)+wt_jump;
	wt7=fw_integ(7.0f*w, &wt_buff7, Ts, PI2)+wt_jump;
	wt11=fw_integ(11.0f*w, &wt_buff11, Ts, PI2)+wt_jump;
	wt13=fw_integ(13.0f*w, &wt_buff13, Ts, PI2)+wt_jump;

	dq_AB(vDQp, vABp, wt);
	dq_AB(vDQn, vABn, -wt);

	dq_AB(vDQ5, vAB5, wt5);
	dq_AB(vDQ7, vAB7, wt7);
	dq_AB(vDQ11, vAB11, wt11);
	dq_AB(vDQ13, vAB13, wt13);
	AB_abc(vAB5, vabc5);
	AB_abc(vAB7, vabc7);
	AB_abc(vAB11, vabc11);
	AB_abc(vAB13, vabc13);


	vmAB[0]=vABp[0]+vABn[0];
	vmAB[1]=vABp[1]+vABn[1];
	scope1=vmAB[0];
	scope2=vmAB[1];
// Loop during switching
	AB_abc(vmAB, vmabc);
	for (int i=0; i<3; i++){
		// THI, comment line for sinusoidal PWM
		// Vm_abc[i]=1.5f*Vm_abc[i]-K23*Vm_abc[i]*Vm_abc[i]*Vm_abc[i]/Vm_AB2;
		vmabc[i]=vmabc[i]+vabc5[i]+vabc7[i]+vabc11[i]+vabc13[i];
		vmabc[i]=fminf(vmabc[i],0.97f);
		vmabc[i]=fmaxf(vmabc[i],-0.97f);
		// Convert float to uint32 voltage modulator wave
		m_abc[i]=((int32_t)(10000.0f*vmabc[i])>>1)+5000u;
	}
	scopeu1=m_abc[0];
	scopeu2=m_abc[1];
	scopeu3=m_abc[2];
	// update PWM duty cycles
	PWM_SetDutyCycle(&PWM_a,m_abc[0]);
	PWM_SetDutyCycle(&PWM_b,m_abc[1]);
	PWM_SetDutyCycle(&PWM_c,m_abc[2]);

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
  ProbeScope_Init(SystemCoreClock/TICKS_PER_SECOND);
  DIGITAL_IO_SetOutputLow(&ENABLE);
  DIGITAL_IO_SetOutputLow(&FAULT_RST);
  DIGITAL_IO_SetOutputLow(&EN_1);
  DIGITAL_IO_SetOutputLow(&EN_2);
  DIGITAL_IO_SetOutputLow(&EN_3);
  PWM_Stop(&PWM_a);
  PWM_Stop(&PWM_b);
  PWM_Stop(&PWM_c);
  PWM_Start(&PWM_update);
  w=100.0f*PI;

  /* Placeholder for user application code. The while loop below can be replaced
   * with user application code. */
  while(1U)
  {
	  //if (state==1u){		// Initialisation state
		  PWM_Start(&PWM_a);
		  PWM_Start(&PWM_b);
		  PWM_Start(&PWM_c);
		  DIGITAL_IO_SetOutputHigh(&EN_1);
		  DIGITAL_IO_SetOutputHigh(&EN_2);
		  DIGITAL_IO_SetOutputHigh(&EN_3);
	  //}
		  /*
	  // Add state for PLL synchronisation
	  else if (state==0u){	// Operation state
		  DIGITAL_IO_SetOutputLow(&ENABLE);
		  PWM_Stop(&PWM_a);
		  PWM_Stop(&PWM_b);
		  PWM_Stop(&PWM_c);
	  }
	  */
	  ProbeScope_Sampling();
  }
}
