

typedef struct{
	float v_AB[2];
	float K;		// sqrt(2)= 1.4142136f
	float dbuff;
	float qbuff;
	float Ti;
}sogi;

typedef struct{
	float w;
	sogi QSG_d;
	sogi QSG_q;
}dsogi;

dsogi dqsg;//={100.0f*PI_const, {{0.0f, 0.0f}, 1.4142136f, 0.0f, 0.0f, Ts}, {{0.0f, 0.0f}, 1.4142136f, 0.0f, 0.0f, Ts}};

float SOGI_v_AB[2] = {0.0f, 0.0f};

float PLL_w = 100.0f*PI_const;
float PLL_w0 = 100.0f*PI_const;
float PLL_wt = 0.0f;
float PLL_Vdq[2] = {0.0f, 0.0f};
float PLL_LPFbuff_d[2] = {0.0f, 0.0f}; 
float PLL_LPFbuff_q[2] = {0.0f, 0.0f}; 
float PLL_wt_buff = 0.0f;
float PLL_LPFbuff_w[2] = {0.0f, 0.0f}; 


//////// function prototypes

void QSG(float v_in, float w, sogi *s);
void AB_dq(float *AB, float *dq, float wt);



/////// functions

void QSG(float v_in, float w, sogi *s){
	s->v_AB[0]=fw_integ(((v_in-s->v_AB[0])*s->K-s->v_AB[1])*w, &(s->dbuff), s->Ti, 0.0f);
	s->v_AB[1]=w*fw_integ(s->v_AB[0], &(s->qbuff), s->Ti, 0.0f);
}
void AB_dq(float *AB, float *dq, float wt){
	// Conventional conversion into orthogonal components
	*dq=*AB*cos(wt)+*(AB+1)*sin(wt);
	*(dq+1)=-*AB*sin(wt)+*(AB+1)*cos(wt);
}
