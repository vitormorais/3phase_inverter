// v10.3
// Updated in 07/06/2019

// for microProbe
	#define TICKS_PER_SECOND (10000u)
	float Vdc_error=0.0f;

// math constants
	#define PI32 4.7123890f			// 3/2*PI
	#define PI2 6.2831853f			// 2*PI
	#define K23 0.66666667f			// 2/3
	#define Ksqr32 0.86602540f		// sqrt(3)/2
	#define K23sqr3 0.38490018f		// 2/(3*sqrt(3))

// system constants
	#define fs 5000.000f
	#define Ts 0.00010000f
	#define wL 3.3600000f			// =100*pi*10.7e-3 //=100*pi*(1.25e-3*2) //=100*pi*5.2e-3
	#define R 2.5600000f
	#define Kp_PLL 1.370000f
	#define Ki_PLL 163.00000f
	#define Kp_I 2.5600000f
	#define Ki_I 1225.0000f
	#define kd 0.00100000f
	#define td 0.06600000f
	#define Kp_V_DC 0.0003//0.311111f
	#define Ki_V_DC 0.006 //20.1707f
	#define TIMER_DELAY_INIT 1*1000000u		//Scaling the required values by 100000.
	#define TIMER_K1 4*1000000u				//Scaling the required values by 100000.
	#define TIMER_K2 10*1000000u			//Scaling the required values by 100000.
	#define TIMER_OPEN_K1_K2 5*1000000u		//Scaling the required values by 100000.
	#define ma_max 0.970000f				// Maximum modulation index
	const uint32_t chargeTime=2000000;

	#define _1MS 10

// conversion constants

	struct ADC_config {
		float conv;		// 0.00080586f (value* 3.3/4095) uint to real voltage
		float offset;	// 1.6500000f (value* 3.3/2) ac quantity offset
		float toVac;	// 386.814f (2/(15/230*536/(536+3480))*0.58) convert to AC voltage
		float toVdc;	// 0.247897f (value* 800/3.3*3.3/4096) convert to DC bus voltage
		float toIac;	// 29.6875f (value* 50/3.205*(1.9)) convert to AC line currents
	};
	struct ADC_config ADC={0.00080586f, 1.6500000f, 386.814f*0.9f, 0.195313f, 29.6875f};

	// 1st order filters constants
	#define wo_LPF_1 6.28319f		// LPF cutoff frequency- 1 Hz
	#define wo_LPF_10 78.5398f		// LPF cutoff frequency- 12.5 Hz
	#define wo_LPF_50 314.159f		// LPF cutoff frequency- 50 Hz
	#define wo_LPF_100 628.319f		// LPF cutoff frequency- 100 Hz
	#define wo_LPF_500 3455.75f		// LPF cutoff frequency- 550 Hz (3455.75f)
	#define wo_LPF_1500 9424.78f	// LPF cutoff frequency- 1500 Hz
	#define wo_LPF_Iref 314.159f	// LPF cutoff frequency- 50 Hz

	const float K_sync[4]={50000.0f, 0.75f, 10.0f};
	const float PI_K_CC[2]={Kp_I, Ki_I};
	const float HPF_K[4]={kd*td, -kd*td, td, td-Ts};
	// LPF_K_X[3]={k, a0/b1, -b0/b1}
	const float LPF_K_1[3]={1, Ts*wo_LPF_1/(2+Ts*wo_LPF_1), -(Ts*wo_LPF_1-2)/(2+Ts*wo_LPF_1)};
	const float LPF_K_10[3]={1, Ts*wo_LPF_10/(2+Ts*wo_LPF_10), -(Ts*wo_LPF_10-2)/(2+Ts*wo_LPF_10)};
	const float LPF_K_50[3]={1, Ts*wo_LPF_50/(2+Ts*wo_LPF_50), -(Ts*wo_LPF_50-2)/(2+Ts*wo_LPF_50)};
	const float LPF_K_100[3]={1, Ts*wo_LPF_100/(2+Ts*wo_LPF_100), -(Ts*wo_LPF_100-2)/(2+Ts*wo_LPF_100)};
	const float LPF_K_500[3]={1, Ts*wo_LPF_500/(2+Ts*wo_LPF_500), -(Ts*wo_LPF_500-2)/(2+Ts*wo_LPF_500)};
	const float LPF_K_1500[3]={1, Ts*wo_LPF_1500/(2+Ts*wo_LPF_1500), -(Ts*wo_LPF_1500-2)/(2+Ts*wo_LPF_1500)};

// Structures:

	struct Config {
		float Vdc_max;
		float Vdc_min;
		float Vd_max;
		float Vd_min;
		float w_max;
		float w_min;
		float I_max;
		float I_min;
		float S_max; 	// maximum apparent power
	};
	// struct Config conf={Vdc_max, Vdc_min, 1.15pu, 0.85pu, 51.5Hz, 47.5Hz, Imax};
	// struct Config conf={750.0f, 400.0f, 374.06f, 276.47f, 323.59f, 298.45f, 70.0f, -70.0f, 30000.0f};
	struct Config conf={750.0f, 350.0f, 374.06f, 276.47f, 323.59f, 298.45f, 15.0f, -15.0f, 5000.0f};

	// This structure keeps the converter vars after filtering:
	struct ConverterVars {
		float Idq[2];
		float PQ[2];
		float Vdc;
		float Vdq[2];
		float w;
		float wt;
		float temp;
	};
	struct ConverterVars conv={{0.0f, 0.0f},{0.0f, 0.0f}, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f, 0.0f};

	struct LPF1_dq {
		const float K[3]; 	// a0, b1, b0
		float buff[4];		// buffer for last values and filter accumulator of both quantities
	};
	struct LPF1_dq LPF_idq_ref={{1, Ts*wo_LPF_50/(2+Ts*wo_LPF_50), -(Ts*wo_LPF_50-2)/(2+Ts*wo_LPF_50)},{0.0f, 0.0f, 0.0f, 0.0f}};
	struct LPF1_dq LPF_iAB={{1, Ts*wo_LPF_500/(2+Ts*wo_LPF_500), -(Ts*wo_LPF_500-2)/(2+Ts*wo_LPF_500)},{0.0f, 0.0f, 0.0f, 0.0f}};
	struct LPF1_dq LPF_vAB={{1, Ts*wo_LPF_500/(2+Ts*wo_LPF_500), -(Ts*wo_LPF_500-2)/(2+Ts*wo_LPF_500)},{0.0f, 0.0f, 0.0f, 0.0f}};

	// Bilinear discretization:
	struct LPF2 {
			const float K[6]; 	// a0, a1, a2, b0, b1, 1/b2
			float buff[4];		// buffer for last values and filter accumulator of both quantities
		};
	#define LPF_zeta 0.707107f
	struct LPF2 LPF2_K_10={{wo_LPF_10*wo_LPF_10*Ts*Ts, 2*wo_LPF_10*wo_LPF_10*Ts*Ts, wo_LPF_10*wo_LPF_10*Ts*Ts, Ts*Ts*wo_LPF_10*wo_LPF_10+4-4*LPF_zeta*wo_LPF_10*Ts, 2*Ts*Ts*wo_LPF_10*wo_LPF_10-8, 1/(wo_LPF_10*wo_LPF_10*Ts*Ts+4*LPF_zeta*wo_LPF_10*Ts+4)},{0.0f, 0.0f, 0.0f, 0.0f}};
	struct LPF2 LPF2_K_50={{wo_LPF_50*wo_LPF_50*Ts*Ts, 2*wo_LPF_50*wo_LPF_50*Ts*Ts, wo_LPF_50*wo_LPF_50*Ts*Ts, Ts*Ts*wo_LPF_50*wo_LPF_50+4-4*LPF_zeta*wo_LPF_50*Ts, 2*Ts*Ts*wo_LPF_50*wo_LPF_50-8, 1/(wo_LPF_50*wo_LPF_50*Ts*Ts+4*LPF_zeta*wo_LPF_50*Ts+4)},{0.0f, 0.0f, 0.0f, 0.0f}};
	struct LPF2 LPF2_K_100={{wo_LPF_100*wo_LPF_100*Ts*Ts, 2*wo_LPF_100*wo_LPF_100*Ts*Ts, wo_LPF_100*wo_LPF_100*Ts*Ts, Ts*Ts*wo_LPF_100*wo_LPF_100+4-4*LPF_zeta*wo_LPF_100*Ts, 2*Ts*Ts*wo_LPF_100*wo_LPF_100-8, 1/(wo_LPF_100*wo_LPF_100*Ts*Ts+4*LPF_zeta*wo_LPF_100*Ts+4)},{0.0f, 0.0f, 0.0f, 0.0f}};
	struct LPF2 LPF2_K_500={{wo_LPF_500*wo_LPF_500*Ts*Ts, 2*wo_LPF_500*wo_LPF_500*Ts*Ts, wo_LPF_500*wo_LPF_500*Ts*Ts, Ts*Ts*wo_LPF_500*wo_LPF_500+4-4*LPF_zeta*wo_LPF_500*Ts, 2*Ts*Ts*wo_LPF_500*wo_LPF_500-8, 1/(wo_LPF_500*wo_LPF_500*Ts*Ts+4*LPF_zeta*wo_LPF_500*Ts+4)},{0.0f, 0.0f, 0.0f, 0.0f}};
	struct LPF2 LPF2_K_1500={{wo_LPF_1500*wo_LPF_1500*Ts*Ts, 2*wo_LPF_1500*wo_LPF_1500*Ts*Ts, wo_LPF_1500*wo_LPF_1500*Ts*Ts, Ts*Ts*wo_LPF_1500*wo_LPF_1500+4-4*LPF_zeta*wo_LPF_1500*Ts, 2*Ts*Ts*wo_LPF_1500*wo_LPF_1500-8, 1/(wo_LPF_1500*wo_LPF_1500*Ts*Ts+4*LPF_zeta*wo_LPF_1500*Ts+4)},{0.0f, 0.0f, 0.0f, 0.0f}};

	struct PI_tr_p {
		const float K[2]; 	// Kp Ki
		float sat[2];		// output saturation
		float buff[2];		// (x[n]; yi[n])
		uint8_t flag;
	};
	struct PI_tr_p PI_PLL = {{Kp_PLL, Ki_PLL},{942.47780f, -942.47780f},{0.0f, 0.0f},0u};
	struct PI_tr_p PI_Vdc = {{Kp_V_DC, Ki_V_DC},{15, -15},{0.0f, 0.0f},0u};

	struct PI_tr_p PI_d   = {{0.05f, 40.0f},{15, -15},{0.0f, 0.0f},0u};
	struct PI_tr_p PI_q   = {{0.05f, 40.0f},{15, -15},{0.0f, 0.0f},0u};

	struct PI_dq_tr {
		const float K[2]; 	// Kp Ki
		float sat[2];		// output saturation
		float buff[4];		// [e_d[n-1]; ei_d[n-1]; e_q[n-1]; ei_q[n-1]]
		uint8_t flag;
	};
	struct PI_dq_tr PI_cc={{Kp_I, Kp_I*Ki_I},{710.0f, -700.0f},{0.0f, 0.0f, 0.0f, 0.0f},0u};

	struct sogi {
		float v_AB[2];
		float K;		// sqrt(2)= 1.4142136f
		float dbuff;
		float qbuff;
		const float Ti;
	};

	struct dsogi{
		float w;
		struct sogi QSG_d;
		struct sogi QSG_q;
	};
	struct dsogi dqsg={100.0f*PI, {{0.0f, 0.0f}, 1.4142136f, 0.0f, 0.0f, Ts}, {{0.0f, 0.0f}, 1.4142136f, 0.0f, 0.0f, Ts}};
	struct dsogi i_dqsg={100.0f*PI, {{0.0f, 0.0f}, 1.4142136f, 0.0f, 0.0f, Ts}, {{0.0f, 0.0f}, 1.4142136f, 0.0f, 0.0f, Ts}};


// ENUMS

	enum stateDesignator{init=0, pc1, pc2, idle, running};
	enum button2Designator{disconnect=0, connect=1};
	enum SWButtonDesignator{stop=0, start=1};
	enum timerState{counting=0, finish=1};
	enum controlModes{IdqControl=0, PQControl=1, VbusControl=2, VdqControl=3};
	enum error_bit{ACDC_HARDWARE_ERROR=0x0001, ACDC_BUS_OVERVOLTAGE=0x0002, ACDC_BUS_UNDERVOLTAGE=0x0004, ACDC_GRID_OVERVOLTAGE=0x0008, ACDC_GRID_UNDERVOLTAGE=0x0010, ACDC_GRID_OVERFREQ=0x0020, ACDC_GRID_UNDERFREQ=0x0040, ACDC_GRID_SYNC_LOST=0x0080, ACDC_CURRENT_CONTR_SAT=0x0100, ACDC_VDC_CONTR_SAT=0x0200};

// global vaiables initialization

	// References
	float V_dq_ref[2]={16.2500f, 0.0f};
	float I_dq_ref[2]={0.0f, 0.0f};		// dq current references
	float PQ_ref[2]={0.0f, 0.0f};		// active and reactive power reference
	float V_DC_ref=0.0f;
	float V_DC_ref_f=0.0f;
	
	// State variables
	volatile uint16_t state=init;		// State machine state variable
	volatile uint16_t relayTimerFlag=counting;
	uint8_t mode=IdqControl;
	int8_t ramp=-1;
	uint16_t sync=0u;					// PLL synchronisation flag
	uint16_t B2command=disconnect;
	uint16_t fault_rst=0;				// HW fault reset button
	uint16_t SWcommand=stop;
	uint16_t REARMcommand=stop;
	uint16_t relayTimer;
	uint16_t fault_flag=0u;
	uint32_t error_code=0u;
	uint32_t fan_pwm=0u;				// Fan PWM duty cycle (0-10000)
	uint16_t sync_aux3=0u;				// PLL stability auxiliary variable

	uint16_t termistor_temp=0u;			// ADC termistor value
	uint16_t V_DC_adc=0u;				// ADC DC bus voltage
	uint32_t m_abc[3]={0u, 0u, 0u};		// Modulation signal to the PWM
	uint16_t V_abc_adc[3]={0u, 0u, 0u};	// ADC voltage measurements
	uint16_t I_abc_adc[3]={0u, 0u, 0u};	// ADC current measurements

	float conv_temp_C = 0.0f; 			// Converter temperature based on termistor measurement
	float V_DC=0.0f;					// DC bus voltage
	float V_abc[3]={0.0f, 0.0f, 0.0f};	// AC voltages
	float I_abc[3]={0.0f, 0.0f, 0.0f};	// AC currents
	float V_AB[2]={0.0f, 0.0f};			// alpha beta voltage
	float I_AB[2]={0.0f, 0.0f};			// alpha beta current
	float V_DCf=0.0f;					// DC bus voltage filtered
	float V_ABf[2]={0.0f, 0.0f};		// alpha beta filtered voltage
	float I_ABf[2]={0.0f, 0.0f};		// alpha beta filtered current
	float V_dq[2]={0.0f, 0.0f};			// dq voltage
	float I_dq[2]={0.0f, 0.0f};			// dq current
	float I_dq_ref_f[2]={0.0f, 0.0f};	// dq filtered current references
	float Ie_dq[2]={0.0f, 0.0f};		// dq current error
	float ID_dq[2]={0.0f, 0.0f};		// dq current error derivative
	float PI_sat_CC[2]={0.0f, -0.0f};	// current control PI integral windup
	float Vm_abc[3]={0.0f, 0.0f, 0.0f};	// Modulation voltages
	float Vm_AB[2]={0.0f, 0.0f};		// Modulation alpha beta voltage
	float Vm_dq[2]={30.0f, 0.0f};		// Modulation dq voltage
	float Vm_AB2=0.0f;					// alpha beta squared: A^2+B^2
	float w=100.0f*PI;					// PLL frequency
	float dw=0.0f;						// PLL frequency rate
	float wf=0.0f;						// filtered PLL frequency
	float wt=0.0f;						// PLL angle
	float sync_aux2=0.0f;
	
	// Buffer variables
	float PI_PLL_buff[3]={0.0f, 0.0f, 0.0f}; 	// PLL PI buffer
	float PI_CCd_buff[3]={0.0f, 0.0f, 0.0f};	// PI current control buffer
	float PI_CCq_buff[3]={0.0f, 0.0f, 0.0f};
	float PI_V_DC_buff[3]={0.0f, 0.0f, 0.0f};	// PI DCbus voltage control buffer
	float HPFd_buff[2]={0.0f, 0.0f};
	float HPFq_buff[2]={0.0f, 0.0f};
	float dw_buff=0.0f;
	float wt_buff=0.0f;
	uint16_t sync_aux3_buff=0u;
	float LPF_buff_w0[2]={0.0f, 0.0f};
	float LPF_buff_w[2]={0.0f, 0.0f};
	float LPF_buff_conv_w[2]={0.0f, 0.0f};
	float LPF_buff_conv_wt[2]={0.0f, 0.0f};
	float LPF_buff_sync2[2]={0.0f, 0.0f};
	float LPF_buff_VA[2]={0.0f,0.0f}, LPF_buff_VB[2]={0.0f,0.0f};
	float LPF_buff_IA[2]={0.0f,0.0f}, LPF_buff_IB[2]={0.0f,0.0f};
	// float LPF_buff_Irefd[2]={0.0f,0.0f}, LPF_buff_Irefq[2]={0.0f,0.0f};
	float LPF_buff_Vdc[2]={0.0f,0.0f};
	float LPF_buff_Vdc_ref[2]={0.0f, 0.0f};

