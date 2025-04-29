# Sample_Project_RH850_S1_ADC_MultiCh
Sample_Project_RH850_S1_ADC_MultiCh

update @ 2025/04/29

1. base on EVM : Y-ASK-RH850F1KM-S1-V3 , initial below function

- TAUJ0_0 : timer interval for 1ms interrupt

- UART : RLIN3 (TX > P10_10 , RX > P10_9) , for printf and receive from keyboard

- LED : LED18 > P0_14 , LED17 > P8_5 , toggle per 1000ms

- ADC : read adc value with group scan1 , per 25ms

	- CH11/AP0_11/ADCA0I11
            
	- CH12/AP0_12/ADCA0I12
            
	- CH13/AP0_13/ADCA0I13
           
	- CH14/AP0_14/ADCA0I14
            
	- CH15/AP0_15/ADCA0I15
 
 
2. below is log message :

![image](https://github.com/released/Sample_Project_RH850_S1_ADC_MultiCh/blob/main/log_adc.jpg)

