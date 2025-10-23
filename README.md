# TI-RSLK-SmartCar

**基于 TI-RSLK 套件与 MSP432 Cortex-M4 的智能循迹/避障/防撞小车**
Smart line-tracking, obstacle-avoidance & bump-protection car built on **TI RSLK Kit** and **MSP432P401R**.

> 📄 本仓库包含：若干独立示例工程（GPIO/电机/PWM/传感器/中断等）、完整综合工程（`test_final`）、以及通用外设驱动源码（`RSLK_base/inc`）。

---

## ✨ 功能特性

* 线迹循迹：红外反射阵列识别黑色轨道
* 避障功能：红外测距触发减速/停车/绕行
* 防撞保护：机械防撞开关（Bump）触发紧急停止
* 电机控制：PWM 变速、启动、反转与刹停
* 模块化代码：GPIO/Timer/PWM/UART/ADC/IRDistance/Reflectance/Tachometer 等可复用组件

---

## 🧰 硬件与工具

* **MCU**: TI MSP432P401R（Cortex-M4F）
* **Kit**: TI-RSLK 学习套件（红外循迹阵列、红外测距、减速电机、驱动板、机械碰撞开关等）
* **IDE**: TI Code Composer Studio (CCS) 10+
* **SDK/Lib**: 驱动与示例已随工程提供（见 `RSLK_base/inc`）


## 📦 仓库结构

```
.
├── LICENSE
├── 实验报告.pdf
├── RSLK_base/               # 通用外设驱动与示例（强烈推荐阅读此处源码）
│   ├── inc/                 # 驱动/中间件源码（ADC14, PWM, Motor, Reflectance, IRDistance, Bump, UART, SysTick...）
│   ├── Lab06_GPIO/          # GPIO/Led/按键/反射阵列等实验
│   ├── Lab08_Track_Maze/    # 迷宫/循迹实验（含 LCD 显示示例）
│   ├── Lab12_Motors/        # 电机与 PWM 基础实验
│   ├── Lab13_UART_TTS/      # UART 通讯示例
│   ├── Lab14_EdgeInterrupts # 边沿中断/防撞开关示例
│   └── ...                  # 其它子工程
├── test_sensor/             # 传感器综合测试（IR 测距、反射阵列）
├── test_Motor/              # 电机与 PWM 单元测试
├── test_3_track/            # 三路循迹基础示例
├── test_bizhang/            # 避障逻辑示例（含 BumpInt）
├── test_all/                # 多模块联调
└── test_final/              # ✅ 综合工程：循迹 + 避障 + 防撞（推荐从这里上板验证）
```

## 🚀 快速上手

1. **克隆项目**

   ```bash
   git clone https://github.com/LittleWang6770/TI-RSLK-SmartCar.git
   ```

2. **导入 CCS 工程**

   * 打开 **Code Composer Studio** → `File` → `Import...` → `Code Composer Studio` → `CCS Projects`
   * 选择仓库根目录，勾选需要的子工程（建议先导入 `test_final` 和 `RSLK_base/inc`）

3. **连接硬件与供电**

   * MSP432 LaunchPad 通过 USB 连接
   * 确保电机驱动板与电机电源连接正确，极性无误

4. **编译与下载**

   * 在 CCS 中选择工程 → `Build` → `Debug`/`Release` → `Debug`/`Load`

5. **运行与观察**

   * 上电后，车辆应能在黑线轨道上循迹
   * 放置障碍物时触发避障；碰撞开关触发时立即刹停


## ⚙️ 配置与标定

### 1) 反射式循迹传感器（黑线校准）

* 打开 `Reflectance.c / Reflectance.h`，根据底板与场地反射率适当调整阈值（如 `REF_THRESHOLD`）
* 推荐流程：白底读取 → 黑线读取 → 取中值或自适应阈值
* 巡线 PID 可在 `test_final/main.c`（或相关控制文件）中调整：

  * `KP`（比例）：纠偏速度
  * `KD`（微分）：抑制振荡
  * `KI`（积分）：长期偏差（通常较小或关闭）

### 2) 红外测距（避障阈值）

* 在 `IRDistance.c / IRDistance.h` 调整距离换算曲线与阈值（如 `OBSTACLE_CM = 15`）

### 3) 电机 PWM（速度/方向）

* `Motor.c / Motor.h` 中设定左右轮 `PWM` 范围与方向翻转（若左右轮安装互换请注意正反转配置）
* 需要更平滑启停可在加减速逻辑中增加斜坡（ramp）

### 4) 防撞开关（Bump / BumpInt）

* 若使用中断触发，确保引脚模式与中断优先级正确（见 `BumpInt.c`）
* 发生碰撞时应立即刹停并进入安全态


## 🧪 推荐编译顺序

1. `test_Motor`：确认电机正反转与速度调节正常
2. `test_sensor`：确认 IR 测距与反射阵列读数合理
3. `test_bizhang`：验证避障与碰撞逻辑
4. `test_3_track` 或 `RSLK_base/Lab08_Track_Maze`：验证基础循迹
5. `test_final`：综合联调（循迹 + 避障 + 防撞）


## 💡 Troubleshooting 常见问题

* **下载失败/无法识别板卡**：
  安装/更新 XDS110 驱动；检查 USB 线与供电；在 CCS `Project Properties → General → Connection` 选择正确的仿真器与目标器件（MSP432P401R）。

* **电机抖动/无力**：
  检查电机供电与地线共地；调低 PWM 或增加加减速斜坡；确认电机驱动板电流能力足够。

* **循迹漂移严重**：
  重新标定黑白阈值；适当降低车速；调小 `KP` 或增大 `KD`；检查传感器离地高度。

* **避障误触发**：
  调整 IR 测距过滤与阈值；避免强光直射；加简单时间去抖。

---

## 🧾 License & Attribution 许可与致谢

* 本仓库使用 **MIT License**（见 `LICENSE`）。
* 部分底层驱动/学习示例参考 TI RSLK/Valvano 教学资料与库文件（见 `RSLK_base/inc`），在此致谢原作者与 TI 教学团队。

## Tips
- 本项目年代久远，难免有些功能纰漏，仅供参考，具体代码实现请以最新版实验要求为准
- 不懂的地方建议优先求助AI工具，包括但不限于：ChatGPT，豆包，元宝，DeepSeek等，作者工作繁忙，不提供任何售后服务
