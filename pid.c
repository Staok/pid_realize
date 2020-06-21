#include "pid.h"

/*定义用户参数，一个器件定义一套PID参数，并初始化参数！*/
struct pid_struct pid = 
{						/*参数初始化*/
	.SetSpeed = 		0.0,			/*MaxSpeed ~ MinSpeed*/
	.ActualSpeed = 		0.0,			
	.MaxSpeed = 		300.0, 			/*定义最大、最小速度，必须分布在0的两边*/
	.MinSpeed = 		-300.0,
	.pid_realize = 		PID_Realize, 	/*定义本套参数的PID计算函数*/
	.Kp = 				0.7, 			/*定义比例、积分、微分系数：0~1.0*/
	.Ki = 				0.2,
	.Kd = 				0.05,
	.isoKi_Max = 		0.5, 			/*积分分离的上限：0~1.0*/
	.isoKi_Min = 		0.2, 			/*积分分离的下限：0~1.0*/
	.output = 			0.0,
	.err = 				0.0,
	.err_last = 		0.0,
	.integral = 		0.0
	
};
struct pid_struct Moto1PID; /*Moto1PID结构体的初始化略*/
//struct pid_struct Moto2PID;

/*定义用户PID计算函数，一个器件定义一个PID计算函数*/  
/*注意！这个函数有待PC实验，用输出值再给设定值，循环跑一哦试试*/
float PID_Realize(void)
{
	float index; //定义积分项系数：0~1.0
	float setSpeed,realSpeed; /*记录归一化之后的设定值和实际值*/
	float absErr; /*记录误差的绝对值*/
	
	static float Ud,Ud_last,realSpeed_last;/*微分先行相关的参数*/
	const float Td = pid.Kd / pid.Kp;
	const float Ts = 1.0,gama = 0.1;
	float temp,c1,c2,c3;
	
	/*检查是否超范围*/
	if(pid.SetSpeed > pid.MaxSpeed){pid.SetSpeed = pid.MaxSpeed;}
	if(pid.SetSpeed < pid.MinSpeed){pid.SetSpeed = pid.MinSpeed;}
	//if(pid.ActualSpeed > pid.MaxSpeed){pid.ActualSpeed = pid.MaxSpeed;} 注释掉！
	//if(pid.ActualSpeed < pid.MinSpeed){pid.ActualSpeed = pid.MinSpeed;}
	
	/*设置速度归一化，都归到 -1.0~1.0 */
	if(pid.SetSpeed >= 0){setSpeed = pid.SetSpeed / pid.MaxSpeed;}
		else{setSpeed = (-pid.SetSpeed) / pid.MinSpeed;}
	/*实际速度归一化后可以 >1或者<-1*/
	if(pid.ActualSpeed >= 0){realSpeed = pid.ActualSpeed / pid.MaxSpeed;}
		else{realSpeed = (-pid.ActualSpeed) / pid.MinSpeed;}
	
	/*计算误差*/
	pid.err=setSpeed-realSpeed;
	/*抗积分饱和过程*/
	if(pid.ActualSpeed > (pid.MaxSpeed * 0.7)) /*这个时候积分项只进行负误差的累加*/
	{
		/*变积分过程*/
		if(pid.err >= 0) absErr = pid.err; else absErr = -pid.err; /*取绝对值*/
		if(absErr > pid.isoKi_Max) 
		{
			index = 0.0; /*大误差时，不使用积分项*/
		}else if(absErr < pid.isoKi_Min){ 
			index = 1.0; /*误差小于积分分离下限时，使用完全积分项*/
			if(pid.err < 0) pid.integral += pid.err; /*只进行负误差的累加*/
		}else{		/*误差介于积分分离的上下限之间，使用变积分*/
			index = (pid.isoKi_Max - absErr) / (pid.isoKi_Max - pid.isoKi_Min); 
			if(pid.err < 0) pid.integral += pid.err; /*只进行负误差的累加*/
		}
	}else if(pid.ActualSpeed < (pid.MinSpeed * 0.7)) /*这个时候积分项只进行正误差的累加*/
	{
		if(pid.err >= 0) absErr = pid.err; else absErr = -pid.err; 
		if(absErr > pid.isoKi_Max) 
		{
			index = 0.0; 
		}else if(absErr < pid.isoKi_Min){ 
			index = 1.0; 
			if(pid.err > 0) pid.integral += pid.err; /*只进行正误差的累加*/
		}else{		
			index = (pid.isoKi_Max - absErr) / (pid.isoKi_Max - pid.isoKi_Min); 
			if(pid.err > 0) pid.integral += pid.err; /*只进行正误差的累加*/
		}
	}else
	{											
		if(pid.err >= 0) absErr = pid.err; else absErr = -pid.err; 
		if(absErr > pid.isoKi_Max) 
		{
			index = 0.0; 
		}else if(absErr < pid.isoKi_Min){ 
			index = 1.0; 
			pid.integral += pid.err; 
		}else{		
			index = (pid.isoKi_Max - absErr) / (pid.isoKi_Max - pid.isoKi_Min); 
			pid.integral += pid.err; 
		}
	}
	
	/*计算PID*/
	#if UseSpecialDifferentiation
		#if !UseIncompleteDifferentiationOrUseDifferentialPreceding
			/*使用不完全微分，即微分项加一个一阶滤波：降低微分项的高频震荡对输出的影响*/
			pid.output = pid.Kp * pid.err + index * pid.Ki * pid.integral + FirstOrderLPF(pid.Kd * (pid.err - pid.err_last));
		#else
			/*使用微分先行：适合于给定指令频繁升降的场合，避免震荡，改善动态性能*/
			/*借鉴 https://wenku.baidu.com/view/c1f88556f01dc281e53af02c.html 程序写的基本对 吧~*/
			temp = gama * Td + Ts; c3 = Td / temp; c2 = (Td + Ts) / temp; c1 = gama * c3;
			Ud = c1 * Ud_last + c2 * realSpeed - c3 * realSpeed_last;
			pid.output = pid.Kp * pid.err + index * pid.Ki * pid.integral + Ud;
			Ud_last = Ud; realSpeed_last = realSpeed;
		#endif
	
	#else
		pid.output = pid.Kp * pid.err + index * pid.Ki * pid.integral + pid.Kd * (pid.err - pid.err_last);
	#endif
	
	pid.err_last = pid.err;
	
	/*防超范围*/
	if(pid.output > 1)  {pid.output = 1.0;}
	if(pid.output < -1) {pid.output = -1.0;}
	
	/*返回一个 pid.MaxSpeed ~ pid.MinSpeed之间的数*/
	if(pid.output >= 0){return pid.output * pid.MaxSpeed;}else{return pid.output * (-pid.MinSpeed);}
}

/*Moto1PID_Realize函数的实现略*/
/*或者就用一个pid实现函数，传入参数改为pid结构体指针！！*/

/*调用PID计算函数*/
//output = (pid.pid_realize)();

/*一阶低通滤波器：本次滤波结果=(1-a)*本次采样值+a*上次滤波结果，a越大滤波程度越高，a必须在0~1之间*/
/*a与一阶低通的时间常数T关系：a = T/(T+1),T越大滤波程度越高*/
#define LPF_A 0.69
float FirstOrderLPF(float NewValue)
{
	static float LastValue,output;
	output = (1.0 - LPF_A) * NewValue + LPF_A * LastValue;
	LastValue = NewValue;
	return output;
}



