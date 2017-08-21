# Event

* Interrupt from PHY
* 1ms timer
* button

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


