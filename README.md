实现PID的几种算法：积分分离，变限积分，和可选择的不完全微分、微分先行。
PID计算过程均使用归一化后的数据，PID三个参数必须均在0~1之间。
通过pid.h里面的宏定义选择具体用那种算法实现。