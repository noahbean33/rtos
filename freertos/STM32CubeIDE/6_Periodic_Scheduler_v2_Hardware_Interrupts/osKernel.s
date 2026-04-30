			AREA |.text|,CODE,READONLY,ALIGN=2
			THUMB
			PRESERVE8
			EXTERN currentPt
		    EXPORT PendSV_Handler
			EXPORT osSchedularLaunch
			IMPORT osSchedulerRoundRobin

PendSV_Handler   ;save r0,r1,r2,r3,r12,lr,pc,psr  
	CPSID	  I
	PUSH 	  {R4-R11}
	LDR 	  R0,=currentPt
	LDR		  R1,[R0]
	
	STR 	  SP,[R1]
	;LDR 	  R1,[R1,#4]
	;STR 	  R1,[R0]
	
	PUSH	  {R0,LR}
	BL		  osSchedulerRoundRobin
	POP		  {R0,LR}
	LDR		  R1,[R0]     ;R1 = currentPt   i.e. New Thread
	
	LDR 	  SP,[R1]
	
	POP		  {R4-R11}
	CPSIE	  I
	BX		  LR
	

osSchedularLaunch
	LDR		R0,=currentPt
	LDR		R2,[R0]
	
	LDR		SP,[R2]
	
	POP		{R4-R11}
	POP		{R0-R3}
	POP 	{R12}
	ADD		SP,SP,#4
	POP		{LR}
	ADD		SP,SP,#4
	CPSIE    I
	BX		 LR
	ALIGN
	END