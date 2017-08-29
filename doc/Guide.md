# Code Process

## function in main.c

* Board Initialization
* PD_AppInit()
* task pd_port_task
* PD_DemoInit(& pd_app_t g_PDAppInstance)
* PD_DemoTaskFun(& pd_app_t gPDAppInstance)

## Board Initialization

* Init pins
* Init clock
* Init I2C, init I2C interrupt for arm core
* Init console for debug
* Init 1ms timer

## PD_AppInit

* TCPC board gpio interupt initialization
* PD_InstanceInit()
* PD_PowerBoardReset()
* g_PDAppInstance Initialization, including PDO and some other info

# Framework

## Basic Data Structure

* Basic Data Structure
* Data Structure Relationship

pd_app_t: for application or demo
pd_instance_t: for pd

* 每个instance对应什么？

pd_ptn5110_ctrl_pin_t
pd_instance_config_t, 包含power config, interface config, phy type config, irq config, product type config, 该config在PD_InstanceInit()中被使用。
pd_power_port_config_t, 表示power的配置，使用宏控制选择不同的power config, 如DRP...

dpmStateMachine

## Layer

* Application
* PD Stack
* Phy Driver

Application

PD Stack 和 App的接口是PD Stack层的usb_pd_policy.c PD_DpmAppCallback函数。
PD Stack中的PD_DpmAppCallback, 调用通过PD_InstanceInit初始化的pdInstance->pdCallback，该函数在APP层pd_app.c实现为PD_DpmDemoAppCallback. DPM callback有对应的DPM callback event.
而PD_DpmDemoAppCallback会调用以下两个函数
* pd_app.c : PD_DpmConnectCallback
* pd_command_interface.c : PD_DpmAppCommandCallback
根据不同的DPM callback event调用实现在pd_command_app.c中的event handler.

PD_DpmAppCallback在Psm状态机中被调用。

另外两个重要的接口是PD_Control以及PD_PhyControl (PD Stack中调用phy).

PD Stack 和 PHY Driver的接口是定义在pd/usb_pd_interface.c中的pd_phy_api_interface_t结构体，具体在下节中说明, phy中的接口只由pd stack调用。PD_PhyControl就是其中之一，对应pd_phy_control_t event.

PD_Control在PD Stack中实现，对应pd_control_t event，由App层调用。

# PHY Driver

两个部分

* 公共
* 特定type phy
	* hal
	* connect
	* msg
	* interface

公共部分在pd/usb_pd_phy.h中定义，包括相关的结构体，枚举体等等。
特定部分在pd/specific_phy_driver中定义，并定义相应接口文件，如specific_phy_driver_interface.c

## Interface
usb_pd_ptn5110_interface.c: 接口文件，其中定义的function作为整个PHY Driver的接口API。
PHY Driver的接口API，在pd/usb_pd_interface.c中复制给pd_phy_api_interface_t结构体，该结构体则是PHY Driver对于上层软件的entry.

```
struct pd_phy_api_interface_t {
	pdPhyInit;
	pdPhyDeinit;
	pdPhySend;
	pdPhyReceive;
	pdPhyControl;
}
```

## Question
* What's the role of hal?
* Why use register cache?

# State Machine
## Stack State Machine

## Connect State Machine
核心变量：
* TypeCState_t pdInstance->curConnectState
* connected: pd_connect_state_t not stable, connected, disconnected
* connectStatus:
* pdInstance->connectResult:

PD_ConnectStateMachine
PD_ConnectSetupNewState

## Policy Engine State Machine

Shield's alert pin will connect to PORTE-31 (J2-2) of FL27 board, J2-2 of FL27 board is connect with PORTE-31 of FL27 MCU, this pin was 使用PORT E 31(对应NVIC 31中断)作为shield board的中断。

PE:
PD_PsmReset

Protocol:

# Source Changer Code Analysis
## main
* timer init
* app init
	* capacilities
	* board gpio interrupt for PTN5110
	* power reset by control phy
* task pd_port_task
* no task of demo

## pd_port_task
* PD_StackStateMachine

