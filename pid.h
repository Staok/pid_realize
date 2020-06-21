#ifndef _PID_H
#define _PID_H

/*功课：
1.比例环节 Kp，作用是加快系统的响应速度，提高系统的调节精度，副作用是会导致超调;
2.积分环节 Ki，作用是消除稳态误差，副作用是导致积分饱和现象;
3.微分环节 Kd，作用是改善系统的动态性能，副作用是延长系统的调节时间。

规范：
1.计算过程全部用归一化的数值，负值代表反转，正值代表正转
2.设定速度变量“.SetSpeed”和PID三个参数全局可变；
3.获取的实际速度“.ActualSpeed”在定时器中断中实时获取
4.PID计算函数在定时器中断中紧接着获取完速度后调用，计算PID结果后立马执行结果
*/

/*如果不使用“不完全微分”或者“微分先行”，选择0，否则选择1*/
#define UseSpecialDifferentiation	1
/*是否使用不完全微分：降低微分项的高频震荡对输出的影响*/
/*是否使用微分先行：适合于给定指令频繁升降的场合，避免震荡，改善动态性能*/
/*选择0使用前者，选择1使用后者*/
#define UseIncompleteDifferentiationOrUseDifferentialPreceding	1 


/*一阶低通滤波器*/
float FirstOrderLPF(float NewValue);
/*实现基本：抗积分饱和 + 变积分 + 不完全微分 + 微分先行 的PID*/
struct pid_struct{
	float SetSpeed; 		//定义设定值：MaxSpeed ~ MinSpeed
	float ActualSpeed; 		//定义实际值：MaxSpeed ~ MinSpeed
	float MaxSpeed,MinSpeed;//定义最大、最小速度，用于归一化：执行器接受的最小值~执行器接受的最大值
							//最大、小速度一般都关于0对称，MaxSpeed必须大于0，MinSpeed必须小于0
	float (*pid_realize)(void);  //pid计算函数
	float Kp,Ki,Kd; 		//定义比例、积分、微分系数：0~1.0
	float isoKi_Max;			//定义积分分离的上限
	float isoKi_Min;			//定义积分分离的下限
	float output; 			//定义计算结果，执行器的输入：-1.0~1.0
	float err; 				//定义偏差值
	float err_last; 		//定义上一个偏差值
	float integral; 		//定义积分值
};

/*定义用户参数，一个器件定义一套PID参数*/
extern struct pid_struct pid;
extern struct pid_struct Moto1PID;
//extern pid_struct Moto2PID;

/*定义用户PID计算函数，一个器件定义一个PID计算函数*/
float PID_Realize(void); /*PID计算*/
float Moto1PID_Realize(void);


#endif

