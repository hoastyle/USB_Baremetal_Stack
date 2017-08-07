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

## Layer

# PHY Driver

