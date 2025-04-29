/*_____ I N C L U D E S ____________________________________________________*/
// #include <stdio.h>
#include <string.h>
#include "r_smc_entry.h"

#include "misc_config.h"
#include "custom_func.h"
#include "retarget.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

volatile struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_TIMER_PERIOD_SPECIFIC           	    (flag_PROJ_CTL.bit1)
#define FLAG_PROJ_REVERSE2                 	        	(flag_PROJ_CTL.bit2)
#define FLAG_PROJ_REVERSE3                    		    (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_REVERSE4                              (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_REVERSE5                              (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_REVERSE6                              (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_REVERSE7                              (flag_PROJ_CTL.bit7)


#define FLAG_PROJ_TRIG_1                                (flag_PROJ_CTL.bit8)
#define FLAG_PROJ_TRIG_2                                (flag_PROJ_CTL.bit9)
#define FLAG_PROJ_TRIG_3                                (flag_PROJ_CTL.bit10)
#define FLAG_PROJ_TRIG_4                                (flag_PROJ_CTL.bit11)
#define FLAG_PROJ_TRIG_5                                (flag_PROJ_CTL.bit12)
#define FLAG_PROJ_REVERSE13                             (flag_PROJ_CTL.bit13)
#define FLAG_PROJ_REVERSE14                             (flag_PROJ_CTL.bit14)
#define FLAG_PROJ_REVERSE15                             (flag_PROJ_CTL.bit15)

/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned int counter_tick = 0U;
volatile unsigned long ostmr_tick = 0U;

#define BTN_PRESSED_LONG                                (2500U)

#pragma section privateData

const unsigned char dummy_3 = 0x5AU;

volatile unsigned char dummy_2 = 0xFFU;

volatile unsigned char dummy_1;

#pragma section default


volatile unsigned long g_u32_counter = 0U;

unsigned char g_uart0rxbuf = 0U;                                 /* UART0 receive buffer */
unsigned char g_uart0rxerr = 0U;                                 /* UART0 receive error status */

unsigned char g_adcflag = 0U;
unsigned char g_adc2flag = 0U;
unsigned short g_adcbuf[5] = {0U};
unsigned short g_adcDiag = 0U;

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/



void ostmr_tick_counter(void)
{
	ostmr_tick++;
}

void ostmr_1ms_IRQ(void)
{
	ostmr_tick_counter();
}

void ostimer_dealyms(unsigned long ms)
{
    R_Config_OSTM0_Start();
    ostmr_tick = 0;

    while(ostmr_tick < ms);

    R_Config_OSTM0_Stop();

}

unsigned int get_tick(void)
{
	return (counter_tick);
}

void set_tick(unsigned int t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000U)
    {
        set_tick(0U);
    }
}

void delay_ms(unsigned long ms)
{
    unsigned long tickstart = get_tick();
    unsigned long wait = ms;
	unsigned long tmp = 0;
	
    while (1)
    {
		if (get_tick() > tickstart)	// tickstart = 59000 , tick_counter = 60000
		{
			tmp = get_tick() - tickstart;
		}
		else // tickstart = 59000 , tick_counter = 2048
		{
			tmp = 60000U -  tickstart + get_tick();
		}		
		
		if (tmp > wait)
			break;
    }
}

void adc_read_SELF_DIAG(void)
{       
    #if 1
    R_Config_ADCA0_ScanGroup2_Start();
    R_Config_ADCA0_ScanGroup2_OperationOn();
    while (( (ADCA0.DIR00) & (1U<<25U))==0)
    {
        /* Wait until value is stored */
    }
    g_adcDiag = ADCA0.DIR00 & 0xFFFFU;
    R_Config_ADCA0_ScanGroup2_OperationOff();
    #endif
}

void adc_read(unsigned char sel)
{
    unsigned char i = 0U;
    reset_buffer(g_adcbuf,0x00U,SIZEOF(g_adcbuf));

    tiny_printf("adc ref:0x%04X,",g_adcDiag);

    switch(sel)
    {
        case 0: // wait for adc interrupt
            R_Config_ADCA0_ScanGroup1_Start();
            R_Config_ADCA0_ScanGroup1_OperationOn();
            while(!g_adcflag);
            R_Config_ADCA0_ScanGroup1_GetResult(g_adcbuf,SIZEOF(g_adcbuf));
            R_Config_ADCA0_ScanGroup1_OperationOff();
            R_Config_ADCA0_Halt();

            tiny_printf("adc buffer:");
            for(i = 0; i < SIZEOF(g_adcbuf) ; i++)
            {
                tiny_printf("[%d]0x%04X,",i,g_adcbuf[i]);
            }
            tiny_printf("\r\n");     
                   
            g_adcflag = 0;
            break;

        case 1: // after flag trig in adc interrupt
            R_Config_ADCA0_ScanGroup1_GetResult(g_adcbuf,SIZEOF(g_adcbuf));
            R_Config_ADCA0_ScanGroup1_OperationOff();
            R_Config_ADCA0_Halt();

            tiny_printf("adc buffer:");
            for(i = 0; i < SIZEOF(g_adcbuf) ; i++)
            {
                tiny_printf("[%d]0x%04X,",i,g_adcbuf[i]);
            }
            tiny_printf("\r\n");

            R_Config_ADCA0_ScanGroup1_Start();
            R_Config_ADCA0_ScanGroup1_OperationOn();
            break;
    }
}

void tmr_1ms_IRQ(void)
{
    tick_counter();

    if ((get_tick() % 1000U) == 0U)
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 1U;
    }

    if ((get_tick() % 25U) == 0U)
    {
        FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 1U;
    }

    if ((get_tick() % 50U) == 0U)
    {

    }	

}

void LED_Toggle(void)
{
    static unsigned char flag_gpio = 0U;
		
    GPIO_TOGGLE(0,14);//PORT.PNOT0 |= 1u<<14;
	
	if (!flag_gpio)
	{
		flag_gpio = 1U;
        GPIO_HIGH(P8,5);//PORT.P8 |= 1u<<5;
	}
	else
	{
		flag_gpio = 0U;
		GPIO_LOW(P8,5);//PORT.P8 &= ~(1u<<5);
	}	
}

void loop(void)
{
	// static unsigned long LOG1 = 0U;

    if (FLAG_PROJ_TIMER_PERIOD_1000MS)
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 0U;

        g_u32_counter++;
        LED_Toggle();   
        // tiny_printf("timer:%4d\r\n",LOG1++);
    }

    if (FLAG_PROJ_TIMER_PERIOD_SPECIFIC)
    {
        FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 0U;
        
        adc_read_SELF_DIAG();

        #if 1
        adc_read(0);
        #else
        if (g_adcflag)
        {
            adc_read(1);
            g_adcflag = 0;
        }
        #endif
    }
}

void UARTx_ErrorCheckProcess(unsigned char err)
{
    if (err)          /* Check reception error */
    {   
        /* Reception error */
        switch(err)
        {
            case _UART_PARITY_ERROR_FLAG:   /* Parity error */
                tiny_printf("uart rx:Parity Error Flag\r\n");
                break;
            case _UART_FRAMING_ERROR_FLAG:  /* Framing error */
                tiny_printf("uart rx:Framing Error Flag\r\n");
                break;
            case _UART_OVERRUN_ERROR_FLAG:  /* Overrun error */
                tiny_printf("uart rx:Overrun Error Flag\r\n");
                break;
            case _UART_BIT_ERROR_FLAG:      /* Bit error */
                tiny_printf("uart rx:Bit Error Flag\r\n");
                break;
        }
        g_uart0rxerr = 0;
    }
}

void UARTx_Process(unsigned char rxbuf)
{    
    if (rxbuf == 0x00U)
    {
        return;
    }

    if (rxbuf > 0x7FU)
    {
        tiny_printf("invalid command\r\n");
    }
    else
    {
        tiny_printf("press:%c(0x%02X)\r\n" , rxbuf,rxbuf);   // %c :  C99 libraries.
        switch(rxbuf)
        {
            case '1':
                FLAG_PROJ_TRIG_1 = 1U;
                break;
            case '2':
                FLAG_PROJ_TRIG_2 = 1U;
                break;
            case '3':
                FLAG_PROJ_TRIG_3 = 1U;
                break;
            case '4':
                FLAG_PROJ_TRIG_4 = 1U;
                break;
            case '5':
                FLAG_PROJ_TRIG_5 = 1U;
                break;

            case 'X':
            case 'x':
                // RL78_soft_reset(7);
                break;
            case 'Z':
            case 'z':
                // RL78_soft_reset(1);
                break;

            default:       
                // exception
                break;                
        }
    }
}

void RLIN3_UART_SendChar(int c)
{
    /*
        UTS : 0 - transmission is not in progress    
    */
    while (((RLN30.LST & _UART_TRANSMISSION_OPERATED) != 0U));    
    RLN30.LUTDR.UINT16 = c;
    // RLN30.LUTDR.UINT8[L] = (unsigned char) c;  
}

void SendChar(int ch)
{
    RLIN3_UART_SendChar(ch);
}

void hardware_init(void)
{
    EI();

    R_Config_TAUJ0_0_Start();
    R_Config_OSTM0_Start();

    /*
        LED : 
            - LED18 > P0_14
            - LED17 > P8_5 
        UART : 
            - TX > P10_10
            - RX > P10_9    
    */
    R_Config_UART0_Receive(&g_uart0rxbuf, 1U);
    R_Config_UART0_Start();

    /*
        ADC :
            - CH11/AP0_11/ADCA0I11
            - CH12/AP0_12/ADCA0I12
            - CH13/AP0_13/ADCA0I13
            - CH14/AP0_14/ADCA0I14
            - CH15/AP0_15/ADCA0I15
    */
    R_Config_ADCA0_ScanGroup1_Start();
    R_Config_ADCA0_ScanGroup1_OperationOn();

    R_Config_ADCA0_SelfDiagnostic_VoltageCircuitOn();
    R_Config_ADCA0_ScanGroup2_Start();
    R_Config_ADCA0_ScanGroup2_OperationOn();
    
    tiny_printf("\r\nhardware_init rdy\r\n");

}
