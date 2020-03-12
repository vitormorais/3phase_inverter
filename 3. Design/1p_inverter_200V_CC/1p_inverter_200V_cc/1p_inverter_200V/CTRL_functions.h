#define TS_Control 1e-4 // 100us control period
#define PR_Kp 1.5f
#define PR_Kr 6.0f
#define PR_Wi 15.0f*PI
#define PR_W0 100.0f*PI

typedef struct{
	float kp ;
    float kr ;
    float wi ;
    float reference ;
    float output_of_backward_integrator ;
    float output_of_feedback ;
    float output_of_forward_integrator ;
    float last_input_of_forward_integrator ;
}prStruct;

prStruct PR_Test ;





//float I_dq_ref[2]={0.0f, 0.0f};
//float I_AB[2]={0.0f, 0.0f};
float CTRL_Vab[2]={0.0f, 0.0f};
float PR_output = 0.0f;
float VDC_BUS_REF;
float vdc_bus_error;
float vdc_bus_piout;
float vdc_division;
float m_u;
float m_v;

float SOGI_v_AB[2] = {0.0f, 0.0f};

float PLL_w = 100.0f*PI;
float PLL_w0 = 100.0f*PI;
float PLL_wt = 0.0f;
float PLL_Vdq[2] = {0.0f, 0.0f};
float PLL_LPFbuff_d[2] = {0.0f, 0.0f};
float PLL_LPFbuff_q[2] = {0.0f, 0.0f};
float PLL_wt_buff = 0.0f;
float PLL_LPFbuff_w[2] = {0.0f, 0.0f};

float PR_LPFbuff_Iu[2] = {0.0f, 0.0f};
float Iu_filt = 0.0f;

float SOGI_i_AB[2] = {0.0f, 0.0f};
float CTRL_idq[2] = {0.0f, 0.0f};
float CTRL_idq_ref[2] = {0.0f, 0.0f};




///////////  prototypes
void run_DC_ctrl(void);
//void dq_AB(float *dq, float *AB, float wt);
void Init_prStruct(prStruct * s, float, float, float) ;
void set_PR_ref(prStruct * s, float reference);
float Calc_prStruct(prStruct * s, float, float );

//////////  functions

void run_DC_ctrl(void) {

  
}


//void dq_AB(float *dq, float *AB, float wt){
//    // Conventional conversion into orthogonal components
//    float C=cos(wt);
//    float S=sin(wt);
//    *AB=C**dq-S**(dq+1);
//    *(AB+1)=S**dq+C**(dq+1);
//}


void Init_prStruct(prStruct * s, float kp_set, float kr_set, float wi_set){
    s->kp = kp_set ;
    s->kr = kr_set ;
    s->wi = wi_set ;
    s->output_of_feedback = 0.0f ;
    s->output_of_backward_integrator = 0.0f ;
    s->output_of_forward_integrator = 0.0f ;
    s->reference = 0 ;
}
void set_PR_ref(prStruct * s, float reference) {
    s->reference = reference;
}
float Calc_prStruct(prStruct * s, float feedback, float wg){
    float error = s->reference - feedback ;

    float input_of_forward_integrator = 2 * s->wi * s->kr * error - s->output_of_feedback;
    // Forward integrator :
    s->output_of_forward_integrator += TS_Control *  input_of_forward_integrator;

    // Backward integrator:
    s->output_of_backward_integrator += TS_Control * s->output_of_forward_integrator * wg * wg ;

    s->output_of_feedback = s->output_of_backward_integrator + 2 * s->wi * s->output_of_forward_integrator ;

    return s->output_of_forward_integrator + s->kp * error;
}
