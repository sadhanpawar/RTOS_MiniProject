/*
 * Faults.h
 *
 *  Created on: Sep 14, 2023
 *      Author: sadhanpawar
 */

#ifndef FAULTS_H_
#define FAULTS_H_


extern void MpuFaultIntHandler(void);
extern void BusFaultIntHandler(void); 
extern void UsageFaultIntHandler(void);
extern void PendSvIntHandler(void);

#endif /* FAULTS_H_ */
