# 1 Layer

* Application
* PD Stack
* Phy Driver

## Application

PD Stack 和 App的接口是PD Stack层的usb_pd_policy.c PD_DpmAppCallback函数。
PD Stack中的PD_DpmAppCallback, 调用通过PD_InstanceInit初始化的pdInstance->pdCallback，该函数在APP层pd_app.c实现为PD_DpmDemoAppCallback. DPM callback有对应的DPM callback event.而PD_DpmDemoAppCallback会调用以下两个函数:
* pd_app.c : PD_DpmConnectCallback
* pd_command_interface.c : PD_DpmAppCommandCallback
根据不同的DPM callback event调用实现在pd_command_app.c中的event handler.

PD_DpmAppCallback在Psm状态机中被调用.

另外两个重要的接口是PD_Control以及PD_PhyControl (PD Stack中调用phy).

PD Stack 和 PHY Driver的接口是定义在pd/usb_pd_interface.c中的pd_phy_api_interface_t结构体，具体在下节
中说明, phy中的接口只由pd stack调用。PD_PhyControl就是其中之一，对应pd_phy_control_t event.

PD_Command在**PD Stack**中实现，case 中的pd_command_t 对应AMS command，由App层调用.
PD_Control在**PD Stack**中实现，对应pd_control_t event，由App层调用. 其中部分会调用PD_PhyControl.

PD_Command
实现: 首先根据command不同设置对应需要修改的变量 (pdInstance，因为上下文的所有信息都通过该结构体保存，所以该结构体异常的大). 设置相应变量之后，通过PD_DpmSendMsg设置TASK EVENT: PD_TASK_EVENT_DPM_MSG 以及 dpmMsgBits(具体command的标识).
应用场景: 通过App层的输入选择调用(按键驱动以及命令驱动，pd_app_demo.c) 以及 根据DPM Event Callback调用

PD_Control
实现: 对应pd_control_t event, 主要是获取信息以及通过PD_PhyControl进行一些控制操作.
应用场景: 其中大部分被PD_DpmAppCommandCallback中的DPM callback event调用, 另外一部分用于获取信息, 帮助DPM做判断.

## PD Stack

## Phy Driver
主要实现pd_phy_api_interface_t中的五个函数以及Alert interrupt handler.
```
static const pd_phy_api_interface_t s_PTN5110Interface = {                                 
    PDPTN5110_Init, PDPTN5110_Deinit, PDPTN5110_Send, PDPTN5110_Receive, PDPTN5110_Control,
};
```

其中，Control函数重要实现phy event的处理函数.

Question: phy driver中有状态机么?


# 2 Event

* Interrupt from PHY
* Event from PHY
* 1ms timer
* button

## 2.1 Interrupt from PHY
This kind of interrupt will be handled by PORTB_PORTC_PORTD_PORTE_IRQHandler in startup.S.
The actual interrupt handler will be provided by phy driver. In PTN5110, PD_PTN5110IsrFunction will handler this. This function would call **PD_Notify** to set PD_PHY_EVENT_STATE_CHANGE event.

## 2.2 Event from PHY
Set event by **PD_Notify** function.

Event state in PHY:
```
typedef enum _pd_notify_event_           
{
    PD_PHY_EVENT_STATE_CHANGE,
    PD_PHY_EVENT_SEND_COMPLETE,          
    PD_PHY_EVENT_RECEIVE_COMPLETE,       
    PD_PHY_EVENT_HARD_RESET_RECEIVED,    
    PD_PHY_EVENT_VBUS_STATE_CHANGE,      
    PD_PHY_EVENT_FR_SWAP_SINGAL_RECEIVED,
    PD_PHY_EVENT_REQUEST_STACK_RESET,    
    PD_PHY_EVENT_VCONN_PROTECTION_FAULT, 
    PD_PHY_EVENT_FAULT_RECOVERY,         
} pd_phy_notify_event_t;                 
```

通过FreeRTOS event group设置task event.

-> task event

```
typedef enum _pd_task_event_type
{           
    PD_TASK_EVENT_RECEIVED_HARD_RESET = 0x01u,
    PD_TASK_EVENT_PD_MSG = 0x02u,              
    PD_TASK_EVENT_DPM_MSG = 0x04u,             
    PD_TASK_EVENT_SEND_DONE = 0x08u,           
    PD_TASK_EVENT_TIME_OUT = 0x10u,            
    PD_TASK_EVENT_PHY_STATE_CHAGNE = 0x20u,    
    PD_TASK_EVENT_OTHER = 0x40u,               
    PD_TASK_EVENT_FR_SWAP_SINGAL = 0x80u,      
    PD_TASK_EVENT_TYPEC_STATE_PROCESS = 0x100u,
} pd_task_event_type_t;                        
```

## 2.3 1ms timer
Two main parts:
* timer timeout
* button: Will be discussed in Button section.

If timer timeout, timeout handler function which is PD_TimerIsrFunction will be called. Timer will set PD_TASK_EVENT_TIME_OUT event when timeout except hard reset by PD_TimerCallback. All these event will be handled in PD_PsmProcessState.

## 2.4 Button
Button interrupt will be handled by 1ms timer interrupt handler (PD_Demo1msIsrProcessSW). The handler will poll the status of button and set corresponding vars according to it. 

Demo Task will take care button event. It will send appropriate command according to button status.
Command which was mentioned here is just set the msg bit of event group. PD_PsmProcessState will also handle this.

# Path

* state machine task
* button task 

# Input and Output
```
PD_DpmAppCallback(pd instance, event, param, done)
	-> pdInstance->pdCallback
		-> PD_DpmDemoAppCallback
			-> PD_DpmConnectCallback
			-> PD_DpmAppCommandCallback
				-> command event callback, such as PD_DpmDataRoleSwapRequestCallback
					-> PD_DemoPrint(app instance, output string)
```
在PD_DemoPrint()中，printProvider作为printBuffer的index，每写入则printProvider + 1

# State Machine
* connection initialization
