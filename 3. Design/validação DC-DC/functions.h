// v10.3
// Updated in 07/06/2019

// Needs to be validated!
float HPF1(float val, float *HPF_buff, const float *HPF_K){
	float res=(*(HPF_K+3)**HPF_buff+*HPF_K*val+*(HPF_K+1)**(HPF_buff+1))/(*HPF_K);
	*HPF_buff=res;
	*(HPF_buff+1)=val;
	return res;
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

float tr_integ(float val, float *buff, float Ti){
	float out=(buff[0]+val)*0.5f*Ti+buff[1];
	buff[0]=val;
	if (out>2.0f*PI){
		out=out-2.0f*PI;
	}
	else if (out<0.0f){
		out=out+2.0f*PI;
	}
	buff[1]=out;
	return out;
}

float deriv(float val, float *buff, float Ti){
	float out=(val-*buff)/Ti;
	*buff=val;
	return out;
}

void abc_AB(float *abc, float *AB){
	// Conventional conversion into orthogonal components
	*AB=K23*(*abc-0.5f**(abc+1)-0.5f**(abc+2));
	*(AB+1)=K23*(Ksqr32**(abc+1)-Ksqr32**(abc+2));
}

void abc_AB_Dy(float *abc, float *AB){
	// Conversion into orthogonal components converting triangle to star
	*AB=K23sqr3*(Ksqr32**abc-Ksqr32**(abc+2));
	*(AB+1)=K23sqr3*(-0.5f**abc+*(abc+1)-0.5f**(abc+2));
}

void AB_dq(float *AB, float *dq, float wt){
	// Conventional conversion into orthogonal components
	*dq=*AB*arm_cos_f32(wt)+*(AB+1)*arm_sin_f32(wt);
	*(dq+1)=-*AB*arm_sin_f32(wt)+*(AB+1)*arm_cos_f32(wt);
}

void dq_AB(float *dq, float *AB, float wt){
	// Conventional conversion into orthogonal components
	float C=arm_cos_f32(wt);
	float S=arm_sin_f32(wt);
	*AB=C**dq-S**(dq+1);
	*(AB+1)=S**dq+C**(dq+1);
}

void AB_abc(float *AB, float *abc){
	// Conventional conversion into orthogonal components
	*abc=*AB;
	*(abc+1)=-0.5f**(AB)+Ksqr32**(AB+1);
	*(abc+2)=-0.5f**(AB)-Ksqr32**(AB+1);
}

float LPF1(float val, float *LPFbuff, const float LPF_K[3]){
	// y[k]= y[k-1]*(-b0/b1)+a0/b1*(x[k]+x[k-1])
	float out= (*(LPF_K+2)) * (*(LPFbuff+1)) + (*LPF_K) * (*(LPF_K+1)) * (val+(*LPFbuff));
	*LPFbuff = val;		// x[k-1]
	*(LPFbuff+1) = out;	// y[k-1]
	return out;
}

float LPF2(float val, struct LPF2 *filter){
	// y[k]=(a2*x[k]+a1*x[k-1]+a2*x[k-2]-(b1*y[k-1]+b0*y[k-2]))*1/b2
	float out=(val*filter->K[2]+filter->buff[0]*filter->K[1]+filter->buff[1]*filter->K[0]-(filter->buff[2]*filter->K[4]+filter->buff[3]*filter->K[3]))*filter->K[5];
	filter->buff[1]=filter->buff[0];	// x[k-2]
	filter->buff[0]=val;				// x[k-1]
	filter->buff[3]=filter->buff[2];	// y[k-2]
	filter->buff[2]=out;				// y[k-1]
	return out;
}

void LPF1_dq (float *x_dq, struct LPF1_dq *lpf, float *xf_dq){

	// d component
	xf_dq[0]=lpf->K[2]*lpf->buff[1]+lpf->K[0]*lpf->K[1]*(x_dq[0]+lpf->buff[0]);
	lpf->buff[0]=x_dq[0];	// x[k-1]
	lpf->buff[1]=xf_dq[0];	// y[k-1]

	// q component
	xf_dq[1]=lpf->K[2]*lpf->buff[3]+lpf->K[0]*lpf->K[1]*(x_dq[1]+lpf->buff[2]);
	lpf->buff[2]=x_dq[1];	// x[k-1]
	lpf->buff[3]=xf_dq[1];	// y[k-1]
}

int16_t conv2DAC(float value){
	float aux = value;//fmax(value,1.0f);
	aux=(aux>1.0f)?1.0f:aux;    //fmax()
	aux=(aux<-1.0f)?-1.0f:aux; //fmin()
	return ((int16_t)(aux*2047.0f));
}

float PI_FW(float val, float *PI_buff, const float *PI_K){
	float res=*PI_buff+*PI_K*val+*PI_K**(PI_buff+1);
	*PI_buff=res;
	*(PI_buff+1)=val;
	return res;
}

float PI_TR(float val, float *PIbuff, float Ti, const float PI_K[2], const float sat[2], uint16_t *flag){
	// bilinear discretization PI in ideal form
	// to change it to parallel form replace Ki with Ki/Kp
	float error=val;
	float integ_error=*PIbuff+(error+*(PIbuff+1))*0.5f*Ti;
	// saturation of integral accumulator:
	if (integ_error>0){
		integ_error=fminf(integ_error,sat[0]);		// 7853.98=(pi/4)/Ti
		if (integ_error<sat[0])
			*flag=0u;
		else
			*flag=1u;
	}
	else{
		integ_error=fmaxf(integ_error,sat[1]);
		if (integ_error>sat[1])
			*flag=0u;
		else
			*flag=1u;
	}
	*PIbuff=integ_error;
	*(PIbuff+1)=error;
	return PI_K[0]*(error+PI_K[1]*integ_error);
}

float PI_TR_p (float error, float Ti, struct PI_tr_p *pi){
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

void PI_TR_dq (float *dq_error, float Ti, struct PI_dq_tr *pi, float *u_dq){
	// bilinear discretization PI in parallel form:
	// to use in ideal form, replace Ki -> Kp*Ki
	if (pi->flag==0u){
		pi->buff[1]+=(dq_error[0]+pi->buff[0])*0.5f*Ti;	// d integral component
		pi->buff[3]+=(dq_error[1]+pi->buff[2])*0.5f*Ti;	// q integral component
	}

	u_dq[0]=pi->K[0]*dq_error[0]+pi->K[1]*pi->buff[1];	// d component
	u_dq[1]=pi->K[0]*dq_error[1]+pi->K[1]*pi->buff[3];	// q component

	// saturations d
	if(u_dq[0]>pi->sat[0]){
		// pi->buff[2]=u_dq[0]-pi->sat[0];
		u_dq[0]=pi->sat[0];
		pi->flag=pi->flag|0b00000010;
	}
	else if(u_dq[0]<pi->sat[1]){
		// pi->buff[2]=u_dq[0]-pi->sat[1];
		pi->flag=pi->flag|0b00000010;
	}
	else{
		// pi->buff[2]=0;
		pi->flag=pi->flag&0b1111101;
	}

	// saturations q
	if(u_dq[1]>pi->sat[0]){
		// pi->buff[5]=u_dq[1]-pi->sat[0];
		u_dq[1]=pi->sat[0];
		pi->flag=pi->flag|0b00000001;
	}
	else if(u_dq[1]<pi->sat[1]){
		// pi->buff[5]=u_dq[1]-pi->sat[1];
		u_dq[1]=pi->sat[1];
		pi->flag=pi->flag|0b00000001;
	}
	else{
		// pi->buff[5]=0;
		pi->flag=pi->flag&0b1111110;
	}

	/*
	PI->buff[0]	// last measured error
	PI->buff[1]	// integral error accumulator
	PI->buff[2]	// back calculation
	*/
	pi->buff[0]=dq_error[0]; 	// e[k-1]
	pi->buff[2]=dq_error[1];	// e[k-1]
}

uint8_t abs_sat(float *V, float LIM){
	if(*V**V+*(V+1)**(V+1)>LIM*LIM){
		float V_abs=sqrtf(*V**V+*(V+1)**(V+1));
		*V=*V*LIM/V_abs;
		*(V+1)=*(V+1)*LIM/V_abs;
		return 1u;
	}
	else
		return 0u;
}

uint8_t sat2D(float* array, float lim){
	float mag2[2]={array[0]*array[0], array[1]*array[1]};
	float frac[2];
	float sqrt_temp[2];
	if ((mag2[0]+mag2[1])>(lim*lim)){
		frac[0]=mag2[0]/(mag2[0]+mag2[1]);
		frac[1]=mag2[1]/(mag2[0]+mag2[1]);
		arm_sqrt_f32((lim*lim*frac[0]), sqrt_temp);
		arm_sqrt_f32((lim*lim*frac[1]), &sqrt_temp[1]);
		array[0]=sqrt_temp[0]*array[0]/fabs(array[0]);
		array[1]=sqrt_temp[1]*array[1]/fabs(array[1]);
		return 1u;
	}
	else
		return 0u;
}

void QSG(float v_in, float w, struct sogi *s){
	s->v_AB[0]=fw_integ(((v_in-s->v_AB[0])*s->K-s->v_AB[1])*w, &(s->dbuff), s->Ti, 0.0f);
	s->v_AB[1]=w*fw_integ(s->v_AB[0], &(s->qbuff), s->Ti, 0.0f);
}

void DSOGI_filter(float *v_AB, struct dsogi *s, float *v_ABp){
	QSG(v_AB[0], s->w, &(s->QSG_d));
	QSG(v_AB[1], s->w, &(s->QSG_q));
	v_ABp[0]=0.5f*(s->QSG_d.v_AB[0]-s->QSG_q.v_AB[1]);
	v_ABp[1]=0.5f*(s->QSG_d.v_AB[1]+s->QSG_q.v_AB[0]);
}

void FFDSOGI_filter(float *v_AB, struct dsogi *s, float w, float *v_ABp){
	QSG(v_AB[0], s->w, &(s->QSG_d));
	QSG(v_AB[1], s->w, &(s->QSG_q));
	v_ABp[0]=0.5f*(s->QSG_d.v_AB[0]-s->QSG_q.v_AB[1]*(w/s->w));
	v_ABp[1]=0.5f*(s->QSG_d.v_AB[1]*(w/s->w)+s->QSG_q.v_AB[0]);
}



/*
void check_errors(void){
	if (DIGITAL_IO_GetInput(&FAULT_IN)==1u){
		error_code||ACDC_HARDWARE_ERROR;
	}
	if (V_DCf > conf.Vdc_max){
		error_code||ACDC_BUS_OVERVOLTAGE;
	}
	else if (V_DCf < conf.Vdc_min){
		error_code||ACDC_BUS_UNDERVOLTAGE;
	}
	if (V_dq[0] > conf.Vd_max){
		error_code||ACDC_GRID_OVERVOLTAGE;
	}
	else if (V_dq[0] < conf.Vd_min){
		error_code||ACDC_GRID_UNDERVOLTAGE;
	}
	if (wf > conf.w_max){
		error_code||ACDC_GRID_OVERFREQ;
	}
	else if (wf < conf.w_min){
		error_code||ACDC_GRID_UNDERFREQ;
	}
	if (sync==0u){
		error_code||ACDC_GRID_SYNC_LOST;
	}
	if (PI_cc.flag==1u){
		error_code||ACDC_CURRENT_CONTR_SAT;
	}
	if (PI_Vdc.flag==1u){
		error_code||ACDC_VDC_CONTR_SAT;
	}
}
*/
void update_config(){
	PI_Vdc.sat[0]=conf.I_max;
	PI_Vdc.sat[1]=conf.I_min;
}
