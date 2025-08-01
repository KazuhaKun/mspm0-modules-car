/*
 * test.h
 *
 *  测试模块头文件
 */

#ifndef TEST_TEST_H_
#define TEST_TEST_H_

// 核心测试函数
void Test_Square_Movement_Hybrid(void);          // 混合模式正方形循迹
void Test_Square_Movement_Hybrid_With_Laps(int laps); // 指定圈数的混合模式正方形循迹
void Test_Square_Movement_Hybrid_Key_Control(void); // 通过按键控制圈数的混合模式正方形循迹

#endif /* TEST_TEST_H_ */