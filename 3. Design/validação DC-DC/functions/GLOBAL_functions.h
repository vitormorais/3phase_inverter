#define CTRL_FREQ 10000
#define CTRL_PERIOD  1/CTRL_FREQ
float cycle_time = 0;
// math constants
#define PI_const  3.1415926535897932384626433832795028841971693993751f
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
#define Kp_V_DC 0.311111f
#define Ki_V_DC 20.1707f
#define TIMER_DELAY_INIT 1*1000000u		//Scaling the required values by 100000.
#define TIMER_K1 4*1000000u				//Scaling the required values by 100000.
#define TIMER_K2 10*1000000u			//Scaling the required values by 100000.
#define TIMER_OPEN_K1_K2 5*1000000u		//Scaling the required values by 100000.
#define ma_max 0.970000f				// Maximum modulation index
int chargeTime=2000000;

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


////////// adc

float V_DC=0.0f;					// DC bus voltage
float V_abc[3]={0.0f, 0.0f, 0.0f};	// AC voltages
float I_abc[3]={0.0f, 0.0f, 0.0f};	// AC currents
float conv_temp_C = 0.0f;


//////////  BUFFERS
float LPFbuff_vdc[2]={0.0f,0.0f};
//float LPFbuff_vdc


/////////  STRUCTS

typedef struct{
	float Idq[2];
	float PQ[2];
	float Vdc;
	float Vdq[2];
	float w;
	float wt;
	float temp;
}ConverterVars;

ConverterVars conv;//={{0.0f, 0.0f},{0.0f, 0.0f}, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f, 0.0f};

typedef struct{
	float K[2]; 	// Kp Ki
	float sat[2];		// output saturation
	float buff[2];		// (x[n]; yi[n])
	int flag;
}PI_tr_p;

PI_tr_p PI_PLL;//={{Kp_PLL, Ki_PLL},{942.47780f, -942.47780f},{0.0f, 0.0f},0};
PI_tr_p PI_Vdc;//={{Kp_V_DC, Ki_V_DC},{15, -15},{0.0f, 0.0f},0u};


float LPF1(float val, float *LPFbuff, const float LPF_K[3]);
float fw_integ(float val, float *buff, float Ti, float rst);
//float PI_TR_par (float error, float Ti, PI_tr_p *pi);


float LPF1(float val, float *LPFbuff, const float LPF_K[3])  {
	// y[k]= y[k-1]*(-b0/b1)+a0/b1*(x[k]+x[k-1])
	float out = (*(LPF_K+2)) * (*(LPFbuff+1)) + (*LPF_K) * (*(LPF_K+1)) * (val+(*LPFbuff));
	*LPFbuff = val;		// x[k-1]
	*(LPFbuff+1) = out;	// y[k-1]
	return out;
}

float fw_integ(float val, float *buff, float Ti, float rst){
	float out=*buff+val*Ti;
	if (rst!=0.0f){
		if (out>rst){
			out=out-rst+out*Ti;
		}
		else if (out<0.0f){
			out=out+rst-out*Ti;
		}
	}
	*buff=out;
	return out;
}

float PI_TR_par (float error, float Ti, PI_tr_p *pi){
	// bilinear discretization PI in parallel form:
	// to use in ideal form, replace Ki -> Kp*Ki
	float u=0.0f;
	// check if output is not saturated
	if (pi->flag==0u){
		pi->buff[1]+=(error+pi->buff[0])*0.5f*Ti;
	}
	u=pi->K[0]*error+pi->K[1]*pi->buff[1];
	if (u > pi->sat[0]){
		u=pi->sat[0];
		pi->flag=pi->flag|0b00000001;
	}
	else if (u < pi->sat[1]){
		u=pi->sat[1];
		pi->flag=pi->flag|0b00000001;
	}
	else{
		pi->flag=pi->flag&0b1111110;
	}
	pi->buff[0]=error;
	return u;
}