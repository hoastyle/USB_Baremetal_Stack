# Event

* Interrupt from PHY
* Event from PHY
* 1ms timer
* button

## Interrupt from PHY
This kind of interrupt will be handled by PORTB_PORTC_PORTD_PORTE_IRQHandler in startup.S.
The actual interrupt handler will be provided by phy driver. In PTN5110, PD_PTN5110IsrFunction will handler this. This function would call **PD_Notify** to set PD_PHY_EVENT_STATE_CHANGE event.

## Event from PHY
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

## 1ms timer
Two main parts:
* timer timeout
* button: Will be discussed in Button section.

Timer timeout first. Handler function is PD_TimerIsrFunction. Timer will set PD_TASK_EVENT_TIME_OUT event when timeout except hard reset. All these event will be handled in PD_PsmProcessState.

## Button
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
