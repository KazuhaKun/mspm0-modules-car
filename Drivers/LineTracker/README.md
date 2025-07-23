# 循迹传感器模块使用说明

## 概述
本循迹传感器模块基于MSPM0G3507微控制器，使用7路TRM循迹传感器阵列，支持智能小车的巡线功能。

## 硬件配置
根据 `ti_msp_dl_config.h` 的配置，7个传感器对应的GPIO引脚如下：

| 传感器编号 | GPIO端口 | GPIO引脚 | 物理引脚 | 位置描述 |
|-----------|---------|---------|---------|----------|
| 0 | GPIOB | PIN_19 | 16 | 最左边 |
| 1 | GPIOB | PIN_18 | 15 | 左二 |
| 2 | GPIOB | PIN_17 | 14 | 左三 |
| 3 | GPIOA | PIN_16 | 9  | 中间 |
| 4 | GPIOA | PIN_14 | 7  | 右三 |
| 5 | GPIOB | PIN_20 | 19 | 右二 |
| 6 | GPIOA | PIN_25 | 26 | 最右边 |

## 传感器工作原理
- **检测逻辑**: 0 = 检测到黑线，1 = 白色背景
- **位置计算**: 使用加权平均算法，范围 -30 到 +30
  - 负值表示线偏向左侧
  - 正值表示线偏向右侧
  - 0表示线在中间位置

## 主要功能

### 1. 基础函数
```c
// 初始化循迹传感器
void LineTracker_Init(void);

// 读取所有传感器的值
void LineTracker_ReadSensors(void);

// 获取线的位置
int16_t LineTracker_GetLinePosition(void);

// 获取当前线状态
LineState_t LineTracker_GetLineState(void);
```

### 2. 线状态枚举
```c
typedef enum {
    LINE_STATE_ON_LINE = 0,         // 在线上
    LINE_STATE_LEFT_TURN,           // 需要左转
    LINE_STATE_RIGHT_TURN,          // 需要右转
    LINE_STATE_SHARP_LEFT,          // 急转左
    LINE_STATE_SHARP_RIGHT,         // 急转右
    LINE_STATE_NO_LINE,             // 没有检测到线
    LINE_STATE_ALL_LINE,            // 全部传感器检测到线
    LINE_STATE_UNKNOWN              // 未知状态
} LineState_t;
```

### 3. 数据结构
```c
typedef struct {
    uint8_t sensorValue[7];         // 原始传感器值
    uint8_t sensorBits;             // 传感器位图表示
    int16_t linePosition;           // 线的位置（-30到30）
    LineState_t lineState;          // 当前线状态
    uint8_t activeSensorCount;      // 激活的传感器数量
    bool lineDetected;              // 是否检测到线
} LineTracker_t;
```

## 使用方法

### 1. 在main.c中启用循迹测试
```c
int main(void)
{
    // 系统初始化
    SYSCFG_DL_init();
    SysTick_Init();
    OLED_Init();

    // 循迹测试初始化
    Test_LineTrackerInit();

    // 主循环
    while (1) 
    {
        Test_LineTrackerLoop();  // 循迹传感器测试
    }
}
```

### 2. 基本循迹控制逻辑
```c
// 读取传感器
LineTracker_ReadSensors();

// 获取位置偏差
int16_t position = LineTracker_GetLinePosition();

// 根据位置计算电机速度
int16_t leftSpeed = baseSpeed - (position * kp);
int16_t rightSpeed = baseSpeed + (position * kp);

// 根据状态调整控制策略
switch (LineTracker_GetLineState()) {
    case LINE_STATE_ON_LINE:
        // 正常巡线
        break;
    case LINE_STATE_LEFT_TURN:
        // 左转控制
        break;
    case LINE_STATE_RIGHT_TURN:
        // 右转控制
        break;
    // ... 其他状态
}
```

### 3. OLED显示功能

测试模式下，OLED会显示：
- 第一行（Y=0）：标题 "LineTracker"
- 第二行（Y=2）：7个传感器状态 (0或1)
- 第三行（Y=4）：位置值和激活传感器数量
- 第四行（Y=6）：当前状态文字

注意：OLED的Y坐标只能使用 0, 2, 4, 6 来显示16号字体，或者 0, 4 来显示8号字体。

## 控制参数调整

### 1. PID参数
可以在 `linetracker_test.c` 中调整以下参数：
- `LINE_FOLLOW_BASE_SPEED`: 基础速度
- `LINE_FOLLOW_KP`: 比例系数
- `LINE_FOLLOW_MAX_SPEED`: 最大速度
- `LINE_FOLLOW_MIN_SPEED`: 最小速度

### 2. 传感器权重
可以在 `linetracker.h` 中调整传感器权重：
```c
#define SENSOR_WEIGHT_0     -30     // 最左边传感器
#define SENSOR_WEIGHT_1     -20
// ... 其他权重
```

## 注意事项

1. **GPIO配置**: 确保在SysConfig中正确配置了TRM相关的GPIO引脚
2. **传感器安装**: 传感器阵列应该正确安装，确保0号传感器在最左边
3. **线宽适配**: 根据实际黑线宽度调整传感器间距和权重
4. **环境光**: 避免强光直射影响传感器工作
5. **定时读取**: 建议以100ms左右的频率读取传感器数据

## 文件结构
```
Drivers/LineTracker/
├── linetracker.h           # 头文件
├── linetracker.c           # 主要实现
├── linetracker_test.c      # 测试和示例代码
└── README.md              # 本说明文档
```

## 示例输出
串口调试输出示例：
```
LineTracker状态:
传感器值: 0 0 1 1 1 0 0 
位图: 0b0011100 (0x1C)
线位置: 0
激活传感器数: 3
线状态: 在线上
检测到线: 是
------------------------
```

<!-- 
## 调试功能（已注释）

### 1. 串口打印
调用 `LineTracker_PrintStatus()` 可以在串口输出详细的传感器状态信息。

### 2. 单独传感器测试
使用 `LineTracker_IndividualSensorTest()` 可以测试每个传感器的工作状态。
-->
