// Structures:



float aux_var1 = 0.0f;;

////////// adc

float V_DC=0.0f;					// DC bus voltage
float V_abc[3]={0.0f, 0.0f, 0.0f};	// AC voltages
float I_abc[3]={0.0f, 0.0f, 0.0f};	// AC currents
float conv_temp_C = 0.0f;

float SOGI_v_AB[2] = {0.0f, 0.0f};
float SOGI_K = 1.4142136f;		// sqrt(2)= 1.4142136f
float SOGI_dbuff = 0.0f;
float SOGI_qbuff = 0.0f;
float SOGI_Ti = Ts;

float PLL_w = 100.0f*PI_const;
float PLL_w0 = 100.0f*PI_const;
float PLL_wt = 0.0f;
float PLL_Vdq[2] = {0.0f, 0.0f};
float PLL_LPFbuff_d[2] = {0.0f, 0.0f}; 
float PLL_LPFbuff_q[2] = {0.0f, 0.0f}; 
float PLL_wt_buff = 0.0f;
float PLL_LPFbuff_w[2] = {0.0f, 0.0f}; 

float PLL_PI_K[2] = {Kp_PLL, Ki_PLL};
float PLL_PI_sat[2] = {942.47780f, -942.47780f};
float PLL_PI_buff[2] = {0.0f, 0.0f};//{100.0f*PI_const/Ki_PLL, 100.0f*PI_const/Ki_PLL};
int   PLL_PI_flag = 0;

//struct PI_tr_p {
//		const float K[2]; 	// Kp Ki
//		float sat[2];		// output saturation
//		float buff[2];		// (x[n]; yi[n])
//		uint8_t flag;
//	};
//struct PI_tr_p PI_PLL={{Kp_PLL, Ki_PLL},{942.47780f, -942.47780f},{0.0f, 0.0f},0u};



typedef struct{
	float Idq[2];
	float PQ[2];
	float Vdc;
	float Vdq[2];
	float w;
	float wt;
	float temp;
}ConverterVars;

ConverterVars conv={{0.0f, 0.0f},{0.0f, 0.0f}, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f, 0.0f};

typedef struct{
	float v_AB[2];
	float K;		// sqrt(2)= 1.4142136f
	float dbuff;
	float qbuff;
	float Ti;
}sogi;

typedef struct{
	float w;
	struct sogi QSG_d;
	struct sogi QSG_q;
}dsogi;

dsogi dqsg={100.0f*PI_const, {{0.0f, 0.0f}, 1.4142136f, 0.0f, 0.0f, Ts}, {{0.0f, 0.0f}, 1.4142136f, 0.0f, 0.0f, Ts}};


typedef struct{
	float K[2]; 	// Kp Ki
	float sat[2];		// output saturation
	float buff[2];		// (x[n]; yi[n])
	int flag;
}PI_tr_p;

PI_tr_p PI_PLL={{Kp_PLL, Ki_PLL},{942.47780f, -942.47780f},{0.0f, 0.0f},0};
PI_tr_p PI_Vdc={{Kp_V_DC, Ki_V_DC},{15, -15},{0.0f, 0.0f},0u};

typedef struct{
		float K[2]; 	// Kp Ki
		float sat[2];		// output saturation
		float buff[4];		// [e_d[n-1]; ei_d[n-1]; e_q[n-1]; ei_q[n-1]]
		int flag;
}PI_dq_tr;

PI_dq_tr PI_cc={{Kp_I, Kp_I*Ki_I},{710.0f, -700.0f},{0.0f, 0.0f, 0.0f, 0.0f},0};




float LPF_buff_Vdc[2]={0.0f,0.0f};


///   function prototypes

float LPF1(float val, float *LPFbuff, const float LPF_K[3]);
void QSG_psim(float v_in, float w, float *v_AB, float K, float *dbuff, float *qbuff, float Ti);
float fw_integ(float val, float *buff, float Ti, float rst);
void AB_dq_psim(float *AB, float *dq, float wt);
float PI_TR_pll_psim (float error, float Ti);

float LPF1(float val, float *LPFbuff, const float LPF_K[3])  {
// y[k]= y[k-1]*(-b0/b1)+a0/b1*(x[k]+x[k-1])
	float out= (*(LPF_K+2)) * (*(LPFbuff+1)) + (*LPF_K) * (*(LPF_K+1)) * (val+(*LPFbuff));
*LPFbuff = val;		// x[k-1]
*(LPFbuff+1) = out;	// y[k-1]
return out;
}
float fw_integ(float val, float *buff, float Ti, float rst)  {
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

float PI_TR_pll_psim (float error, float Ti){
	// bilinear discretization PI in parallel form:
	// to use in ideal form, replace Ki -> Kp*Ki
	float u=0.0f;
	// check if output is not saturated
	 if (PLL_PI_flag==0){
	 	PLL_PI_buff[1]+=(error+PLL_PI_buff[0])*0.5f*Ti;
	 }
	 u=PLL_PI_K[0]*error+PLL_PI_K[1]*PLL_PI_buff[1];
	 if (u > PLL_PI_sat[0]){
	 	u=PLL_PI_sat[0];
	 	PLL_PI_flag=1;//PLL_PI_flag|0b00000001;
	 }
	 else if (u < PLL_PI_sat[1]){
	 	u=PLL_PI_sat[1];
	 	PLL_PI_flag=1;//PLL_PI_flag|0b00000001;
	 }
	 else{
	 	PLL_PI_flag=0;//PLL_PI_flag&0b1111110;
	 }
	 PLL_PI_buff[0]=error;
	return u;
}

void AB_dq_psim(float *AB, float *dq, float wt){
	// Conventional conversion into orthogonal components
	*dq=*AB*cos(wt)+*(AB+1)*sin(wt);
	*(dq+1)=-*AB*sin(wt)+*(AB+1)*cos(wt);
}

void QSG(float v_in, float w,  sogi *s){
	s->v_AB[0]=fw_integ(((v_in-s->v_AB[0])*s->K-s->v_AB[1])*w, &(s->dbuff), s->Ti, 0.0f);
	s->v_AB[1]=w*fw_integ(s->v_AB[0], &(s->qbuff), s->Ti, 0.0f);
}

void QSG_psim(float v_in, float w, float *v_AB, float K, float *dbuff, float *qbuff, float Ti)  {
	v_AB[0]=fw_integ(((v_in-v_AB[0])*K-v_AB[1])*w, &dbuff, Ti, 0.0f);
	v_AB[1]=w*fw_integ(v_AB[0], &qbuff, Ti, 0.0f);
}
