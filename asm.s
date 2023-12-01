
 .def getPsp
 .def setPsp
 .def setAspPsp
 .def setAspMsp
 .def setTmplUnP
 .def setTmplP
 .def setPendSv
 .def getMsp

 
.thumb
.const

getPsp:
    MRS R0, PSP
    BX LR

getMsp:
    MRS R0, MSP
    BX LR

setPsp:
    MSR PSP, R0
    DSB
    ISB
    BX LR

setAspPsp:
    MRS R0, CONTROL
    MOV R1, #2
    ORR R0, R0, R1
    MSR CONTROL, R0
    DSB
    ISB
    BX LR

setAspMsp:
    MRS R0, CONTROL
    MOV R1, #2
    MVN R1, R1
    AND R0,R0,R1
    MSR CONTROL, R0
    DSB
    ISB
    BX LR

setTmplUnP:
    MRS R0, CONTROL
    MOV R1, #1
    ORR R0, R0, R1
    MSR CONTROL, R0
    DSB
    ISB
    BX LR

setTmplP:
    MRS R0, CONTROL
    MOV R1, #1
    MVN R1, R1
    AND R0,R0,R1
    MSR CONTROL, R0
    DSB
    ISB
    BX LR

setPendSv:
    svc #1
    
;change asp bit in assembly

