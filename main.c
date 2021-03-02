/*******************************************************************************
* Copyright(C) 2008, 2017 Renesas Electronics Corporation
* RENESAS ELECTRONICS CONFIDENTIAL AND PROPRIETARY
* This program must be used solely for the purpose for which
* it was furnished by Renesas Electronics Corporation. No part of this
* program may be reproduced or disclosed to others, in any
* form, without the prior written permission of Renesas Electronics
* Corporation.
*******************************************************************************
*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corp. and is only 
* intended for use with Renesas products. No other uses are authorized.
*
* This software is owned by Renesas Electronics Corp. and is protected under 
* all applicable laws, including copyright laws.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES 
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, 
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
* PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY 
* DISCLAIMED.
*
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORP. NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES 
* FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS 
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*
* http://www.renesas.com/disclaimer
*""FILE COMMENT""***************************************************************
* System Name	: RAJ240090/100 sample program
* File Name		: main.c
* Contents		: main function source file
* Device		: RAJ240090/100
* Compiler		: CC-RL (V.4.01)
* Note			: 
*""FILE COMMENT END""**********************************************************/

/***********************************************************************
** Include files
***********************************************************************/
#include "define.h"																			// union/define definition
#include "afe.h"																			// AFE register(Window 0)
#include "afe1.h"																			// AFE register(Window 1)
#include "afe2.h"																			// AFE register(Window 2)
#include "kousei.h"																			// Initial calibration
#include "mcu.h"																			// mcu.c
#include "ram.h"																			// ram.c
#include "smbus.h"																			// smbus.c
#include "smbus_ex.h"																		// smbus_ex.c
#include "dataflash.h"																		// dataflash.c
#include "iodefine.h"
#include "datcalc_spl.h"																	// datcalc_spl.c
#include "datcalc.h"																		// datcalc.c
#include "mode.h"																			// mode.c
#include "cell.h"																			// cell.c

/***********************************************************************
** Announcement of internal function prototype
***********************************************************************/
void main(void);
void SlowMain(void);
#ifdef Clock_SW
void OCO2LOCO(void);
void LOCO2OCO(void);
#endif


#pragma	inline_asm	FUD_Mode
static void FUD_Mode(void)
{
	BR !!0x01CC00;
}


/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: 1sec interval timer initial processing 
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void Timer1sec_init(void)
 *--------------------------------------------------------------------------
* Function			: 1sec timer initialize. (Timer array unit0 channel0)
 *					: 
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void Timer1sec_init(void)
{
	/* Stop channel0 */
	TT0L_bit.no0 = 1;																			// Stop TM00

	TPS0 |= 0b0000000000000111;															// CKm0 : Clock=62.5kHz(8MHz/128)
			//			  |||+- PRSm00 : 1
			//			  ||+-- PRSm01 : 1
			//			  |+--- PRSm02 : 1
			//			  +---- PRSm03 : 0

	/* Mask channel0 interrupt */
	TMMK00 = 1;																			// disable INTTM00 interrupt
	TMIF00 = 0;																			// clear INTTM00 interrupt flag
	/* Set INTTM00 top priority */
	TMPR100 = 0;																		// TM00 Interrupt top priority
	TMPR000 = 0;
	/* Channel0 used as interval timer */
	TMR00 = 0x0000;																		// Select CKm0
	TDR00 = 62500;																		// 62.5kHz * 62500 = 1sec

	TMIF00 = 0;																			// clear INTTM00 interrupt flag
	TMMK00 = 0;																			// enable INTTM00 interrupt
	TS0L_bit.no0 = 1;																			// Start TM00
}



/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: 1msec interval timer initial processing 
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void Timer1msec_init(void)
 *--------------------------------------------------------------------------
* Function			: 1msec timer initialize. (Timer array unit0 channel1)
 *					: 
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void Timer1msec_init(void)
{
	/* Stop channel1 */
	TT0L_bit.no1 = 1;																			// Stop TM01

	TPS0 |= 0b0000000000110000;															// CKm1 : Clock=1MHz(8MHz/8)
			//		  |||+- PRSm10 : 1
			//		  ||+-- PRSm11 : 1
			//		  |+--- PRSm12 : 0
			//		  +---- PRSm13 : 0

	/* Mask channel1 interrupt */
	TMMK01 = 1;																			// disable INTTM01 interrupt
	TMIF01 = 0;																			// clear INTTM01 interrupt flag
	/* Set INTTM01 top priority */
	TMPR101 = 0;																		// TM01 Interrupt top priority
	TMPR001 = 0;
	/* Channel1 used as interval timer */
	TMR01 = 0x8000;																		// Select CKm1
	TDR01 = 1000;																		// 1MHz * 1000 = 1msec

	TMIF01 = 0;																			// clear INTTM01 interrupt flag
	TMMK01 = 0;																			// enable INTTM01 interrupt
	
} 
/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: SERIAL initialize Process
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void Serial_init(void)
 *--------------------------------------------------------------------------
 * Function			: Serial communication initialize.
 *					: (Unit1 channel1 : CSI21)
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void Serial_init(void)
{
	CS = 0;
	CS_DIR = 0;																			// PM7_2(CS) = OUTPUT
	
	PER2_bit.no7 = 1;																			// C2C enabled
	C2CCNT = 0x81;																		// C2C mode (4bit parallel communication)
																						// C2C communication enabled
	
}

/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: AFE initialize Process
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void AFE_init(void)
 *--------------------------------------------------------------------------
 * Function			: AFE initialize.
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void AFE_init(void)
{	
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	
	// AFE interrupt request flag clear
	AFE_WR(AFE_AFIF1,INT_WDTIR_W); 														// AFE WDT interrupt request flag clear
	
	/* AFE WDT stop */
	AFE_WR(AFE_AWDTCON,WDT_STOP);														// AFE WDT stop
	
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	
	LOCO2OCO();	                                                                        // Switch LOCO to OCO
                                                                                        // for countermeausre (Reset in Low Power Mode)
	
}

/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: 1sec interval timer initial processing for AFE timer
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void AFE_Timer1sec_init(void)
 *--------------------------------------------------------------------------
* Function			: 1sec timer initialize. (AFE timer)
 *					: 
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void AFE_Timer1sec_init(void)
{
	BYTE tmp;
	
	AFE_RD(AFE_AFMK3,tmp);
	AFE_WR(AFE_AFMK3,tmp & ~INT_AFETRAIE);												// AFE timer interrupt disable
	AFE_WR(AFE_AFIF3,INT_AFETRAIR_W);													// Clear INT_AFETRAIR flag(AFE timer)
	AFE_WR(AFE_ATMACR,AFETRASTART_SP | AFETRACLKSEL_16K);								// AFE timer stop / count source 16kHz / Timer mode
	AFE_WR(AFE_ATMAR,3);																// 0.25[s] * (3+1) = 1[s]
	AFE_WR(AFE_ATMACR,AFETRASTART_ST | AFETRACLKSEL_16K)								// AFE timer start
	
}

/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: AFE trimming setting Process
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void AFE_Trimming_setting(void)
 *--------------------------------------------------------------------------
 * Function			: AFE initialize.
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void AFE_Trimming_setting(void)
{
	/* AFE Window Change */
	AFE_WR(AFE_WINCHG,WINCHG_TR)														// AFE Window 2
	
	AFE_WR(AFE_AOCOT0,B_AFE_OCO0);
	AFE_WR(AFE_AOCOT1,B_AFE_OCO1);
	AFE_WR(AFE_AOCOT2,B_AFE_OCO2);
	AFE_WR(AFE_ALOCOT0,B_AFE_LOCO0);
	AFE_WR(AFE_ALOCOT1,B_AFE_LOCO1);
	AFE_WR(AFE_BT0,B_AFE_BT0);
	AFE_WR(AFE_BT1,B_AFE_BT1);
	AFE_WR(AFE_RT0,B_AFE_VREGAD);
	AFE_WR(AFE_RT1,B_AFE_VREGCC);
	AFE_WR(AFE_BUFT0,B_AFE_IVT0);
	AFE_WR(AFE_BUFT1,B_AFE_IVT1);
	//VREG2 = 3.3V(Selectable)
	AFE_WR(AFE_REG2T,B_AFE_VREG2_3V);
	// VREG2 = 5.0V(Selectable)
	//AFE_WR(AFE_REG2T,B_AFE_VREG2_5V);
	AFE_WR(AFE_FCPT0,B_AFE_FET0);
			
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
}

/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: AFE RESET Process
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void AFE_RESET(void)
 *--------------------------------------------------------------------------
 * Function			: AFE initialize.
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void AFE_RESET(void)
{
	
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	
	AFE_WR(AFE_AFESWRST,0xAA);
	AFE_WR(AFE_AFESWRST,0x55);
	AFE_WR(AFE_AFESWRST,0xFF);
	AFE_WR(AFE_AFESWRST,0x00);
								
	Waittimer_us(200);         															//¡¹ Add Wait 200us (it needs more than 130usec wait)
			
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
}

#ifdef	OCDCALIB_EN
/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: AFE Over current detection Calibration Process
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void AFE_OCDCalib_D(void)
 *--------------------------------------------------------------------------
* Function			: AFE Over current detection calibration(Discharge).
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void AFE_OCDCalib_D(void)
{
	BYTE atmp = 0;																		// OCDSTS register read
	BYTE atmp_1;
	BYTE awork = 0;																		// work for calibration value
	
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	AFE_WR(AFE_DOC1DCON,0x00);															// 0[mV] setting
	AFE_WR(AFE_DOC1DTIME1,0x00);														// detection time = 0.488[ms]*(n + 1) = 0.488[ms] (n = 0)
	
	AFE_WR(AFE_WINCHG,WINCHG_RM1);														// AFE Window 1
	AFE_WR(AFE_DOC1CAL,DOC1CAL_EN);														// DOC1 calibration enabled
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	AFE_WR(AFE_DOC1DCON,0x80);															// DOC1 detection curcuit enabled
	
	Waittimer_us(250);																	// wait 250us
		
	do{
	
		AFE_WR(AFE_WINCHG,WINCHG_RM1);														// AFE Window 1
		AFE_WR(AFE_DOC1CAL,DOC1CAL_EN | awork);												// DOC1 calibration enabled & calibration value
		AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	
		Waittimer_us(250);																	// wait 250us
		Waittimer_us(250);																	// wait 250us
		Waittimer_us(200);																	// wait 200us
		
		AFE_RD(AFE_OCDSTS,atmp);
		atmp_1 = atmp & DOC1FLG;
		
		if(awork < 0x18)
		{
			if(atmp_1 != DOC1FLG)
			{
				awork++;
			}else
			{
				break;
			}
		}else
		{
			awork = 0x18;
			break;
		}
	}while(1);	
	
	awork -= 1;
	AFE_WR(AFE_WINCHG,WINCHG_RM1);														// AFE Window 1
	AFE_WR(AFE_DOC1CAL,awork | DOC1CAL_DIS);											// DOC1CAL calibration value setting & DOC1 calibration disabled
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	AFE_RD(AFE_DOC1DCON,atmp);															// DOC1DCON register read
	AFE_WR(AFE_DOC1DCON,atmp | 0x00);													// DOC1 detection curcuit disabled
	AFE_WR(AFE_OCDRSTRT,DOC1RSTRT);														// Discharge detection flag clear
}

/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: AFE Over current detection Calibration Process
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void AFE_OCDCalib_C(void)
 *--------------------------------------------------------------------------
* Function			: AFE Over current detection calibration(Charge).
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void AFE_OCDCalib_C(void)
{
	BYTE atmp = 0;																		// OCDSTS register read
	BYTE atmp_1;
	BYTE awork = 0x18;																	// work for calibration value
	
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	AFE_WR(AFE_COCDCON,0x00);															// 0[mV] setting
	AFE_WR(AFE_COCDTIME,0x02);															// detectiom time 61[us] * 2 = 122[us]
	
	AFE_WR(AFE_WINCHG,WINCHG_RM1);														// AFE Window 1
	AFE_WR(AFE_COCCAL,COCCAL_EN | awork);												// COC calibration enabled
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	AFE_WR(AFE_COCDCON,0x80);															// COC detection curcuit enabled
	
	Waittimer_us(250);																	// wait 250us
	
	do{	
		AFE_WR(AFE_WINCHG,WINCHG_RM1);														// AFE Window 1
		AFE_WR(AFE_COCCAL,COCCAL_EN | awork);												// COC calibration enabled & calibration value
		AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
		
		Waittimer_us(200);																	// wait 200us
	
		AFE_RD(AFE_OCDSTS,atmp);
		atmp_1 = atmp & COCRFLG;
		
		
		if(awork > 0x00)
		{
			if(atmp_1 != COCRFLG)															
			{
				awork--;
			}else
			{
				break;
			}
		}else
		{
			awork = 0x00;
			break;
		}
	}while(1);	
	
	awork += 1;
	AFE_WR(AFE_WINCHG,WINCHG_RM1);														// AFE Window 1
	AFE_WR(AFE_COCCAL,awork | COCCAL_DIS);												// COCCAL calibration value setting & COC calibration disabled
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	AFE_RD(AFE_COCDCON,atmp);															// COCDCON register read
	AFE_WR(AFE_COCDCON,atmp | 0x00);													// COC detection curcuit disabled
	AFE_WR(AFE_OCDRSTRT,COCRSTRT);														// Charge detection flag clear
}


#endif

#ifdef	WUD_EN
/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: AFE Wakeup discharge current setting Process
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void AFE_DWUC(void)
 *--------------------------------------------------------------------------
* Function			: AFE wakeup current setting(Discharge).
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void AFE_DWUC_EN(void)
{
	AFE_WR(AFE_WUDRSTRT,DWUDRSTRT);														// DWU interrupt request flag clear
	
	AFE_WR(AFE_WUDTIME,0x0F);															// Wakeup current detection time = 3.91[ms]* 16 = 62.56[ms]
	AFE_WR(AFE_DWUDCON1,0x04);															// Wakeup current detection voltage:1.25[mV] * 4 = 5.0[mV]
	AFE_WR(AFE_DWUDCON2,DWUFETEN | DWUGAIN_10 | DWUACALEN | DWUPERIODIC |  DWUEN);		// FET enable, Gain:10 times, DWU auto calib:enabled, DWU detectiton enable
}

/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: AFE Wakeup charge current setting Process
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void AFE_CWUC(void)
 *--------------------------------------------------------------------------
* Function			: AFE wakeup current setting(Charge).
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void AFE_CWUC_EN(void)
{
	AFE_WR(AFE_WUDRSTRT,CWUDRSTRT);														// CWU interrupt request flag clear
	
	AFE_WR(AFE_WUDTIME,0x0F);															// Wakeup current detection time = 3.91[ms]* 16 = 62.56[ms]
	AFE_WR(AFE_CWUDCON1,0x04);															// Wakeup current detection voltage:1.25[mV] * 4 = 5.0[mV]
	AFE_WR(AFE_CWUDCON2,CWUFETEN | CWUGAIN_10 | CWUACALEN | CWUPERIODIC | CWUEN);		// FET enable, Gain:10 times, DWU auto calib:enabled, DWU detectiton enable
}


#endif

/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: Initial processing 
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void hdwinit(void)
 *--------------------------------------------------------------------------
 * Function			: Hardware initialize 
 *					: 
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: This function is called from startup routine.
 *					: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void hdwinit(void)
{
	volatile BYTE acnt;
	
	DI();																				// Interrupt disable


	PIOR0 = 0b00000010;																	// Peripheral I/O redirection register0
		//	  |||||||+- PIOR00 : 0
		//	  ||||||+-- PIOR01 : 1
		//	  |||||+--- PIOR02 : 0
		//	  ||||+---- PIOR03 : 0
		//	  |||+----- PIOR04 : 0
		//	  ||+------ PIOR05 : 0
		//	  |+------- Reserved bit : 0
		//	  +-------- Reserved bit : 0
	PM1 = 0b10111000;																	// P10-P11 output Low
		 //	|||||||+--- P10 : 0 OUTPUT
		 //	||||||+---- P11 : 0 OUTPUT
		 //	|||||+----- P12 : 0 INPUT(No function)
		 //	||||+------ P13 : 1 INPUT(No function)
		 //	|||+------- P14 : 1 INPUT(No function)
		 //	||+-------- P15 : 1 INPUT(No function)
		 //	|+--------- P16 : 0 OUTPUT
		 //	+---------- P17 : 1 INPUT

	PM2 = 0b11111100;
		 //	|||||||+--- P20 : 0 OUTPUT
		 //	||||||+---- P21 : 0 OUTPUT
		 //	|||||+----- P22 : 1 INPUT(No function)
		 //	||||+------ P23 : 1 INPUT(No function)
		 //	|||+------- P24 : 1 INPUT(No function)
		 //	||+-------- P25 : 1 INPUT(No function)
		 //	|+--------- P26 : 1 INPUT(No function)
		 //	+---------- P27 : 1 INPUT
	PM6 = 0b11110011;
	     //	|||||||+--- P60 : 1 INPUT
		 //	||||||+---- P61 : 1 INPUT
		 //	|||||+----- P62 : 0 OUTPUT(No function)
		 //	||||+------ P63 : 0 OUTPUT(No function)
		 //	|||+------- P64 : 1 INPUT(No function)
		 //	||+-------- P65 : 1 INPUT(No function)
		 //	|+--------- P66 : 1 INPUT(No function)
		 //	+---------- P67 : 1 INPUT

	CMC = 0b00000000;																	// CMC register
	
//		TIMER INITIALIZE
	TAU0EN = 1;																			// Timer array unit0 supplies input clock
	Timer1sec_init();
	Timer1msec_init();

//		IIC INITIALIZE
	SMBus_init();

//		SERIAL(for AFE) INITIALIZE
	Serial_init();
}



/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: main processing 
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void main(void)
 *--------------------------------------------------------------------------
 * Function			: Execute main processing 
 *					: 
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void main(void)
{
	BOOL	aoffs_flg = OFF;
	
	DI();																				// Interrupt disable

	aiic_sadr = ADDR_ERR;																// Set slave address initial value

	// Fixed Data Check
	if(InitFixed_Chk() == FALSE)
	{
			amode = M_FIXEDERR;															// Mode number set to Fixed data error
			EI();
			do
			{
				if( f_flashup == ON )													// Move to Flash update mode ?
				{
					DI();
					//__asm("br !!01CC00h");											// To Flash update mode
					FUD_Mode();
				}
				
			}while(1);
	}
	
	AFE_Trimming_setting();																// AFE trimming setting(this function calls before AFE reset)
	//AFE_RESET();																		// AFE Reset function(Default: Not execute this function)
	AFE_init();																			// AFE initialize

	AFE_Timer1sec_init();																// AFE timer initialize(1sec)
	
	// HVP(for SYS-connect) Setting
	if(F_SYSCONEN == ON)
	{
		AFE_WR(AFE_WINCHG,WINCHG_RM1);													// Change Window 1
		AFE_WR(AFE_HVEG,0x11);															// Both rising and falling edges(HVP0)	
		AFE_WR(AFE_WINCHG,WINCHG_RM0);													// Change Window 0
		AFE_WR(AFE_AFIF2,INT_HVP0IR_W); 												// HVP0 interrupt request flag clear
	}
	

#ifdef	OCDCALIB_EN
	AFE_OCDCalib_D();
	AFE_OCDCalib_C();
#endif
	
	//AD stop
	AFE_WR(AFE_ADCON0,ADC_DIS);
	
	// CC stop
	AFE_WR(AFE_CCCON,CC_STOP);

	// Discharge short current detection time
	AFE_WR(AFE_SCDTIME,SCITIM);															// detection time 61us * 5 = 305us 
	
	
	// Discharge short current detection 
	AFE_WR(AFE_SCDCON,SCEN | SCFETEN | SCCMPV);											// FET enabled, 150mV/1mohm = 150A setting

#ifdef	DOC2D_EN	
	// Discharge over current 2 detection time
	AFE_WR(AFE_DOC2DTIME1,DOC1ITIM);													// detection time 0.488ms* 100 = 48.8ms
	
	// Discharge over current 2 detection 
	AFE_WR(AFE_DOC2DCON,DOC2EN | DOC2FETEN | DOC2CMPV);									// FET enabled, 100mV/1mohm = 100A setting
#elif	defined DOC3D_EN
	
	AFE_WR(AFE_WINCHG,WINCHG_RM1);														// AFE Window 1
	// Discharge over current 3 detection time
	AFE_WR(AFE_DOC3DTIME1,DOC1ITIM);													// detection time 0.488ms* 100 = 48.8ms
	
	// Discharge over current 3 detection 
	AFE_WR(AFE_DOC3DCON,DOC3EN | DOC3FETEN | DOC3CMPV);									// FET enabled, 100mV/1mohm = 100A setting
	
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
#elif defined DOC4D_EN
	
	AFE_WR(AFE_WINCHG,WINCHG_RM1);														// AFE Window 1
	
	// Discharge over current 4 detection time
	AFE_WR(AFE_DOC4DTIME1,DOC1ITIM);													// detection time 0.488ms* 100 = 48.8ms
	
	// Discharge over current 4 detection 
	AFE_WR(AFE_DOC4DCON,DOC4EN | DOC4FETEN | DOC4CMPV);									// FET enabled, 100mV/1mohm = 100A setting
	
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// AFE Window 0
	
#else

	// Discharge over current 1 detection time
	AFE_WR(AFE_DOC1DTIME1,DOC1ITIM);													// detection time 0.488ms* 100 = 48.8ms
	
	// Discharge over current 1 detection 
	AFE_WR(AFE_DOC1DCON,DOC1EN | DOC1FETEN | DOC1CMPV);									// FET enabled, 100mV/1mohm = 100A setting
	
#endif
	// Charge over current detection time
	AFE_WR(AFE_COCDTIME,COCITIM);														// detectiom time 61[us] * 255 = 15564[us]
	
	// Charge over current detection 
	AFE_WR(AFE_COCDCON,COCFETEN | COCEN | COCCMPV);										// FET enabled, -60mV/1mohm = -60A setting
	
	// AFE interrupt request flag clear
	AFE_WR(AFE_AFIF0,INT_ADIR_W & INT_CCIR_W & INT_CCFSTFLG_W & INT_CHGIR_W ); 

	// AFE interrupt request flag clear
	AFE_WR(AFE_AFIF1,INT_SCDIR_W & INT_DOC1DIR_W & INT_COCDIR_W & INT_WDTIR_W); 
	
	// AFE interrupt request flag clear
	AFE_WR(AFE_AFIF3,INT_AFETRAIR_W); 

#ifdef	WUD_EN
	// AFE Interrupt control
	AFE_WR(AFE_AFMK0,INT_ADIE | INT_CCIE | INT_CHGIE | INT_DWUIE | INT_CWUIE);	
																						// AD,CC,CHG,DWU,CWU interrupt enabled
#else
	// AFE Interrupt control
	AFE_WR(AFE_AFMK0,INT_ADIE | INT_CCIE | INT_CHGIE );	
																						// AD,CC,CHG interrupt enabled
#endif
	// ANL Interrupt control 
	AFE_WR(AFE_AFMK1,INT_SCDIE | INT_DOC1DIE | INT_COCDIE | INT_WDTIE); 
																						// SCD,DOC,COC,WDT interrupt enabled
	
	// AFE timer Interrupt control
	AFE_WR(AFE_AFMK3,INT_AFETRAIE);														// AFE timer A interrupt enabled
	
	
    AFE_WR(AFE_COND0,COND_OFF);								                            // Cell Balancing OFF(Cell 1 - Cell 8) 
    AFE_WR(AFE_COND1,COND_OFF);							                                // Cell Balancing OFF(Cell 9 - Cell 10)
    
    
	// AFE timer interrupt enable(INTP13)
	EGP1_bit.no5 = 0;																			// Falling edge
	EGN1_bit.no5 = 1;
	PPR013 = 0;																			// Interrupt top priority
	PPR113 = 0;
	PIF13 = 0;																			// Interrupt flag clear
	PMK13 = 0;																			// Interrupt enable

	
	// ANL interrupt enable(INTP12)
	EGP1_bit.no4 = 0;																			// Falling edge
	EGN1_bit.no4 = 1;
	PPR012 = 0;																			// Interrupt top priority
	PPR112 = 0;
	PIF12 = 0;																			// Interrupt flag clear
	PMK12 = 0;																			// Interrupt enable
	
	// WU interrupt enable(INTP11)
	EGP1_bit.no3 = 0;																			// Falling edge
	EGN1_bit.no3 = 1;
	PPR011 = 0;																			// Interrupt top priority
	PPR111 = 0;
	PIF11 = 0;																			// Interrupt flag clear
	PMK11 = 0;																			// Interrupt enable

	// HVP interrupt enable(INTP10)
	
	EGP1_bit.no2 = 0;																	// Falling edge
	EGN1_bit.no2 = 1;
	PPR010 = 0;																			// Interrupt top priority
	PPR110 = 0;
	PIF10 = 0;																			// Interrupt flag clear
	if ( F_SYSCONEN == ON )
	{
		PMK10 = 0;																		// Interrupt enable
	}else{
		PMK10 = 1;																		// Interrupt disable
	}
	
	
	// AD interrupt enable(INTP9)
	EGP1_bit.no1 = 0;																			// Falling edge
	EGN1_bit.no1 = 1;
	PPR09 = 1;																			// Interrupt top priority
	PPR19 = 1;
	PIF9 = 0;																			// Interrupt flag clear
	PMK9 = 1;																			// Interrupt disable

	// CC interrupt enable(INTP8)
	EGP1_bit.no0 = 0;																			// Falling edge
	EGN1_bit.no0 = 1;
	PPR08 = 0;																			// Interrupt low priority
	PPR18 = 0;
	PIF8 = 0;
	PMK8 = 0;																			// Interrupt enable
	
	// C,D-FET ON
	AFE_WR(AFE_FCON,FET_CD);																
	// CC start
	// [Note] An accuracy error occurs on first current integration result when the current integrating circuit starts, 
	// because of the circuit's function.
	AFE_WR(AFE_CCCON,CC_START);
	
	EI();																				// Interrupt enabled
	
	
	
	// Initial calibration
	if( InitCalib_Chk() == FALSE )														// Check the initial caliblation data and Is an initial calibration necessary? 
	{
			
		f_calibmode = ON;																// Initial calibration mode flag ON 
		DI();																			// Interrupt disable 
			
		// Discharge short current detection 
		AFE_WR(AFE_SCDCON,0b00000000);													// Stop discharge short current ditection
		// Discharge over current detection 
		AFE_WR(AFE_DOC1DCON,0b00000000);												// Stop discharge over current ditection
			
		// Charge over current detection 
		AFE_WR(AFE_COCDCON,0b00000000);													// Stop charge over current ditection
		
		amode = M_CALIB;																// Mode number set to InitCalib
					
		AFE_WR(AFE_FCON,FET_CD);														// FET control [C=ON,D=ON]
			
		EI();																			// Interrupt enabled 
		KouseiMain();																	// To initial caliblation setting 
			
	}

	// Initial setting
	Init_CalibDat();
	
	// SYS-connect check
	if ( F_SYSCONEN == ON )						
	{ 
	   AFE_RD(AFE_PHV & 0x01,asys_in)
	}else{
	   asys_in = LOW;
	}	

	if( asys_in == HI )																	// Out of system ?
	{
		EGP1_bit.no2 = 0;																// Falling edge
		EGN1_bit.no2 = 1;
	} else {																			// In the system
		EGP1_bit.no2 = 0;																// Falling edge
		EGN1_bit.no2 = 1;
		f_sys_in = ON;																	// Set PackStatus:BI
	}

	t_com10c = INIT_FCC;																// set initial FCC
	lrc_w = (long)t_com10c * INIT_RSOC * 144;											// set initial RSOC
	Make_RC();
	t_com0d = INIT_RSOC;

	// Set FG mode
	amode = M_WAKE;																		// WakeUp mode
	
	Calc_Checksum();
	
	// Main loop 
	while(1)																			// mainloop start
	{
		if( f_curr_int == ON )															// Current integration interrupt available 
		{

			CVT_Func();																	// Current and voltage and temperature measurement 
			f_curr_int = OFF;															// Flag clear
			
			Calc_RC();
			Calc_RSOC();
			Alarm_Chk();
			Mode_Chk();
			if(F_CBEN == ON)
			{
				CB_Chk();
			}

			if((CLD0 == LOW) && (DAD0 == LOW))											// SCL/SDA = LOW ?
			{
				if( aoffs_flg == 0 )
				{
					aoffs_cnt++;														// Increment the counter

					if( aoffs_cnt >= 10 )												// 2.5sec has passed ?
					{
						aoffs_flg = 1;
						SMB_Timeout();													// Timeout function
					}
				}
			} else {
				aoffs_cnt = 0;															// Clear the counter
				aoffs_flg = 0;
			}
			
		}
		
		if( f_flashup == ON )															// Move to Flash update mode ?
		{
			DI();
			FUD_Mode();																	// To Flash update mode
		}
		
		
		if( f_fcslp == ON)
		{
			amode = M_ALON;																// StandAlone
			afet = FET_D;																// FET [C=OFF,D=ON]
			AFE_WR(AFE_FCON,afet);														// FET control
		
			if( TE0L_bit.no1 == OFF )													// No SMBus communication ?
			{
				SlowMain();																// To Slow main
			}
		}
		
		if(f_slow == ON)
		{
			if( TE0L_bit.no1 == OFF )													// No SMBus communication ?
			{
				SlowMain();																// To Slow main
			}
			
		}else {
			asleep_cnt1 = 0;															// Clear Sleep counter
			asleep_cnt2 = 0;
			
			if(amode == M_ALON)															// Sleep ?
			{
				amode = M_WAKE;															// move to WakeUp
			}
		}
		
		DI();																			// Interrupt disable
		// - Stop mode function -
		if( TE0L_bit.no1 == OFF )														// No SMBus communication(1msec timer stop) ?
		{
			if( (IICS0 & 0b10110010) == 0x00 )											// MSTS0 EXC0 COI0 STD == 0 ?
			{
				WUP0 = 1;																// Wakeup enable for stop mode
				TT0L_bit.no0 = 1;														// Stop TM00
				NOP();																	// wait 3clock
				NOP();
				NOP();
				EI();																	// Interrupt enable
				STOP();																	// Stop mode
				NOP();																	// wait 5clock
				NOP();
				NOP();
				NOP();
				NOP();
				WUP0 = 0;																// Wakeup disable
				TS0L_bit.no0 = 1;														// Start TM00
				if( TE0L_bit.no1 == OFF )												// Not during SMBus communication ?
				{
					PIF4 = 0;															// Interrupt flag clear
					PMK4 = 0;															// Interrupt enable(SCL/SDA)
				}
			}
		}
		EI();
	}																					// mainloop end
}

/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: Slow main function
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void SlowMain()
 *--------------------------------------------------------------------------
 * Function			: Main loop of slow mode.
 *					: 
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void SlowMain(void)
{
//	BYTE atmp;		
	
	AFE_WR(AFE_CCCON,CC_STOP);															// Stop current integration
	
	// OCO -> LOCO
#ifdef	Clock_SW
	OCO2LOCO();
#endif

#ifdef	WUD_EN
	AFE_DWUC_EN();
	AFE_CWUC_EN();
#endif

	
	EI();																				// Enable interrupt
	do
	{
		// No longer it isn't slow mode 
		if(f_slow == OFF)
		{
			if( f_fcslp == OFF )															// It isn't force sleep ?
			{
				if(amode == M_ALON)															// Sleep mode
				{
					amode = M_WAKE;
				}
			
				DI();																		// Disable interrupt
			// LOCO -> OCO
#ifdef	Clock_SW
				LOCO2OCO();
#endif

#ifdef	WUD_EN
				AFE_WR(AFE_CWUDCON2,CWUDIS);												// Wake-up Charge current detection disable 
				AFE_WR(AFE_DWUDCON2,DWUDIS);												// Wake-up Discharge current detection disable
#endif
				AFE_WR(AFE_CCCON,CC_START);													// Start current integration
				EI();																		// Enable interrupt
				return;
			}
		}

		if( f_slp2min == ON )															// 2min has passed ?
		{
			f_slp2min = OFF;															// Clear 2min passed flag
			// LOCO -> OCO
#ifdef		Clock_SW
			LOCO2OCO();
#endif
			CVT_Func();																	// Get Voltage and Temperature
			// OCO -> LOCO 
#ifdef	Clock_SW
			OCO2LOCO();
#endif
		}
		if( f_slpcalc == ON )															// Is there sleep calc req. ?
		{
			f_slpcalc = OFF;															// Clear sleep calc req. flag
					
			// LOCO -> OCO
#ifdef		Clock_SW
			LOCO2OCO();
#endif
			// Start current integration
			AFE_WR(AFE_CCCON,CC_START);													// Start current integration
			while( f_curr_int == OFF );													// Wait for current integ.
			f_curr_int = OFF;
			while( f_curr_int == OFF );													// Wait for current integ.
			f_curr_int = OFF;
			AFE_RD(AFE_CCRL,cc_curr.uc[0]);												// get CC value(L)
			AFE_RD(AFE_CCRM,cc_curr.uc[1]);												// get CC value(M)
			AFE_RD(AFE_CCRH,cc_curr.uc[2]);												// get CC value(H)
			
			AFE_WR(AFE_CCCON,CC_STOP);													// Stop current integration
#ifdef		Clock_SW
			OCO2LOCO();
#endif
			CalcCurr();																	// Calc current
					
			if( tabsc >= 250 )															// |Curr| >= Sleep current ?
			{
// Return to Normal mode
				f_fcslp = OFF;
				f_slow = OFF;															// Clear Slow condition flag
				acccv_cnt = 0;															// Clear CC/CV send counter
				anosmb_cnt = 0;															// Clear no SMBus counter
				f_nosmb = OFF;															// Clear no SMBus flag
				if( amode == M_ALON )													// Sleep mode ?
				{
					amode = M_WAKE;														// Move to WakeUp
				}
				DI();
#ifdef		Clock_SW
				LOCO2OCO();
#endif	
				AFE_WR(AFE_CCCON,CC_START);												// Start current integration
				EI();
				return;
			}
			if(F_CBEN == ON)
			{
				CB_Chk();
			}
			
		}
		
		DI();																			// Interrupt disable
		if( TE0L_bit.no1 == OFF)														// No SMBus communication ?
		{		
			if( (IICS0 & 0b10110010) == 0x00 )											// MSTS0 EXC0 COI0 STD == 0 ?
			{
				WUP0 = 1;																// Wakeup enable for stop mode
				NOP();																	// wait 3clock
				NOP();
				NOP();
				EI();																	// Interrupt enable
				STOP();																	// Stop mode
				NOP();																	// wait 5clock
				NOP();
				NOP();
				NOP();
				NOP();
				if( TE0L_bit.no1 == OFF )												// Not during SMBus comm. ?
				{
					PIF4 = 0;															// Interrupt flag clear
					PMK4 = 0;															// Interrupt enable(SCL/SDA)
				}
			}
		}
		EI();

	} while(1);
}

#ifdef	Clock_SW
/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: LOCO switch function
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void OCO2LOCO()
 *--------------------------------------------------------------------------
 * Function			: Clock switch(OCO -> LOCO).
 *					: 
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void OCO2LOCO(void)
{
	
	BYTE atmp;																			// AMOCON read
	
	DI();																				// Interrupt disable
	
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// Change WINDOW0
	AFE_WR(AFE_AMOCON,MCLKSEL_ALOCO);													// ALOCO select
	
	do{
	
		AFE_RD(AFE_AMOCON,atmp);														// MCLKSTS bit read
		atmp = atmp & 0x10;																// Bit Mask
		
	}while(atmp != MCLKSTS_ALOCO);														// ALOCO select wait
	
	EI();																				// Interrupt enable
	
}

/*""FUNC COMMENT""**********************************************************
 * ID				: 1.0
 * module outline	: OCO switch function
 *--------------------------------------------------------------------------
 * Include			: 
 *--------------------------------------------------------------------------
 * Declaration		: void LOCO2OCO()
 *--------------------------------------------------------------------------
 * Function			: Clock switch(LOCO -> OCO).
 *					: 
 *--------------------------------------------------------------------------
 * Argument			: None
 *--------------------------------------------------------------------------
 * Return			: None
 *--------------------------------------------------------------------------
 * Input			: None
 *					: 
 * Output			: None
 *					: 
 *--------------------------------------------------------------------------
 * Used function	: 
 *					: 
 *--------------------------------------------------------------------------
 * Caution			: 
 *--------------------------------------------------------------------------
 *""FUNC COMMENT END""*****************************************************/
void LOCO2OCO(void)
{
	
	BYTE atmp;																			// AMOCON read
	
	DI();																				// Interrupt disable
	
	
	AFE_WR(AFE_WINCHG,WINCHG_RM0);														// Change WINDOW0
	AFE_WR(AFE_AMOCON,MCLKSEL_AOCO);													// AOCO select
	
	do{
	
		AFE_RD(AFE_AMOCON,atmp);														// MCLKSTS bit read
		atmp = atmp & 0x10;																// Mask 4bit
		
	}while(atmp != MCLKSTS_AOCO);														// AOCO select wait
	
	EI();																				// Interrupt enable
}
#endif
