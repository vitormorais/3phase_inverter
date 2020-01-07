
#define _1MS 10
//enum stateDesignator{init=0, idle, running};
int init = 0;
int pc1 = 1;
int pc2 = 2;
int idle = 3;
int running = 4;
int state=init;		// State machine state variable

int SCADA_enable = 0;			//ON-OFF micrium button
int SCADA_disable = 0;			//ON-OFF micrium button
int SCADA_status_button = 0;    //ON-OFF micrium button
float SCADA_Imax = 25.0f;		//Maximum current
float SCADA_VDCmax = 450.0f;	//Maximum Voltage DCBUS
float SCADA_VDCmin = -10.0f;	//Minimum Voltage DCBUS
float SCADA_Tmax = 60.0f;		//Maximum converter temperature

int SCADA_error_Ia = 0;			//Flag
int SCADA_error_Ib = 0;			//Flag
int SCADA_error_Ic = 0;			//Flag
int SCADA_error_VDCmax = 0;		//Flag
int SCADA_error_VDCmin = 0;		//Flag
int SCADA_error_Tmax = 0;		//Flag

int EN_1 = 0, EN_2 = 0, EN_3 = 0; 
int PCR_SW = 0, PCS_SW = 0; 

int timer_sm = 0;




void ConverterStateMachine(void);
int no_fault(void);

void ConverterStateMachine(void) {

	///////////////////   TRANSIÇÕES   /////////////////////
	timer_sm += 1;
	///  switch case INIT
	if ((state == init )  ) {
		if (timer_sm > (20*_1MS)) { 
			state = pc1;
		 	timer_sm = 0;
		}	
	}
	///  switch case PC1
	else if ((state == pc1 )  ) {
	 	if ((timer_sm > (20*_1MS)) && (conv.Vdc > 300.0f)/**/) { 
	 		state = pc2;
	 		timer_sm = 0;
	 	}
	}
	///  switch case PC2
	else if ((state == pc2 )  ) {
	 	if (timer_sm > (50*_1MS) ){ 
	 		state = idle;
	 		timer_sm = 0;
	 	}
	}
	///  switch case IDLE
	else if ((state == idle )  ) {
		if ( (SCADA_enable == 1) && (no_fault() == 0) )  {
			SCADA_enable = 0;
			SCADA_error_Ia = 0;
			SCADA_error_Ib = 0;
			SCADA_error_Ic = 0;
			SCADA_error_VDCmax = 0;
			SCADA_error_VDCmin = 0;
			SCADA_error_Tmax = 0;
			state = running;
		}
	//state = running;
	}
	///  switch case RUNNING
	else if ((state == running ) ) {
		if (SCADA_disable==1)  {
			state = idle;
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
	}
	else {
	//du nothing
	}

	/////////////////////   STATES UPDATE	/////////////////////
	if (state==init){
	EN_1 = 0; //  DIGITAL_IO_SetOutputLow(&EN_1);
	EN_2 = 0; //  DIGITAL_IO_SetOutputLow(&EN_2);
	EN_3 = 0; //  DIGITAL_IO_SetOutputLow(&EN_3);
	PCR_SW = 0;
	PCS_SW = 0;
	//SCADA_status_button = 0;
	}
	else if (state==pc1){
		EN_1 = 0; //  DIGITAL_IO_SetOutputLow(&EN_1);
		EN_2 = 0; //  DIGITAL_IO_SetOutputLow(&EN_2);
		EN_3 = 0; //  DIGITAL_IO_SetOutputLow(&EN_3);
		PCR_SW = 0;
		PCS_SW = 1;
		//SCADA_status_button = 0;
	}
	else if (state==pc2){
		EN_1 = 0; //  DIGITAL_IO_SetOutputLow(&EN_1);
		EN_2 = 0; //  DIGITAL_IO_SetOutputLow(&EN_2);
		EN_3 = 0; //  DIGITAL_IO_SetOutputLow(&EN_3);
		PCR_SW = 1;
		PCS_SW = 1;
		//SCADA_status_button = 0;
	}
	else if (state==idle){
		EN_1 = 0; //  DIGITAL_IO_SetOutputLow(&EN_1);
		EN_2 = 0; //  DIGITAL_IO_SetOutputLow(&EN_2);
		EN_3 = 0; //  DIGITAL_IO_SetOutputLow(&EN_3);
		PCR_SW = 1;
		PCS_SW = 1;
		//SCADA_status_button = 0;
	}
	else if (state==running){
		EN_1 = 1; //  DIGITAL_IO_SetOutputHigh(&EN_1);
		EN_2 = 1; //  DIGITAL_IO_SetOutputHigh(&EN_2);
		EN_3 = 1; //  DIGITAL_IO_SetOutputHigh(&EN_3);
		PCR_SW = 1;
		PCS_SW = 1;
		//SCADA_status_button = 1;
	}
}

int no_fault(void)  {
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