#include "DSK6713_AIC23.h"
#include "noise_gen.h"
#include <stdio.h>
#include "sine1500.h"
Uint32 fs=DSK6713_AIC23_FREQ_8KHZ;
#define N 240000
#define FILE_SIZE 64
FILE *fptr;
long i=0,j=1,k=0;
short buffer[N];
short buffer_echo[5000];
short am_buff[512];
short sine_buff[512];
short out_buff[20];
short square[16];
short square_out[256];
short ramp_out[1024];
short graph_out[5000];
short output_ramp;
#pragma DATA_SECTION(buffer,".EXT_RAM");
shift_reg sreg;
Uint32 input,output;
short fb;
short noise=0, frequency=1, echo=0, stereo=0, gain=1, delay=1, am=0, wavegen=0;

interrupt void c_int11()
{
	//노이즈 생성!
	short prnseq; 
	if(noise!=0){
		if(sreg.bt.b0)
			prnseq = -8000;
		else
			prnseq = 8000;
		fb=(sreg.bt.b0)^(sreg.bt.b1);
		fb^=(sreg.bt.b11)^(sreg.bt.b13);
		sreg.regval<<=1;
		sreg.bt.b0 = fb;
	}	
	input=input_sample();
	output_sample(input);

	//녹음
	if(DSK6713_DIP_get(3)==0)
	{	DSK6713_LED_on(3);
		buffer[i]=input_sample();
		i++;
		/*if(i==FILE_SIZE-1)
		{
			fptr=fopen("Record.dat","w");
			for(j=0;j<FILE_SIZE;j++)
				fprintf(fptr,"%d\n",buffer[j]);
			fclose(fptr);
			puts("done");
		}*/
		DSK6713_LED_off(3);
	}
	//재생
	else if(DSK6713_DIP_get(0)==0)
	{	DSK6713_LED_on(0);
		output=buffer[j]*0.5+0.1*echo*buffer_echo[i]+(prnseq*noise*0.1);
		buffer_echo[i]=output;
		graph_out[k]=output;
		if(k==5000)k=0;k++;
		i++;
		if(i>=1000*delay) i=0;
		if(stereo==1)
			output_sample((short)output*gain);	
		else if(stereo==0)
			output_left_sample(output*gain);
		else if(stereo==2)
			output_right_sample(output*gain);
		j+=frequency;
		DSK6713_LED_off(0);
	}
	else if (DSK6713_DIP_get(1)==0)
	{	DSK6713_LED_on(1);
		//AM 출력
		if(am==1)
		{
			out_buff[i]=carrier[i]*sine1500[i];	//+((15*sine1500[i]*carrier[i]/10)>>12);
			output_sample(gain*am_buff[i]);
			am_buff[j]=out_buff[i];

		}
		else	//sine정형파 출력
		{
			output_sample(sine1500[i]);
			sine_buff[j]=sine1500[i];
		}
		i++;
		j++;
		if(i==16) i=0;
		if(j>=512) j=0;
		DSK6713_LED_off(1);
	}
	else if (DSK6713_DIP_get(2)==0)
	{	DSK6713_LED_on(2);
		//사각파 출력
		if(wavegen==0)
		{
			output_sample(square[i]);
			square_out[j]=square[i];
			j++;
			if(j==256)j=0;
			if(++i>15)i=0;
		}
		else	//램프 출력
		{
			output_sample(output_ramp);
			output_ramp+=0x20;
			ramp_out[i]=output_ramp;
			i++;
			if(i==1024)i=0;
			if(output_ramp>=0x07FF)
				output_ramp=0;
		}
		DSK6713_LED_off(2);
	}
	else
	{	
		j=0;
		i=0;
		k=0;
		output_ramp=0;
	}
}
void main()
{	
	for(i=0;i<8;i++)	//사각파 초기화
		square[i]=0x0002;
	for(i=8; i<16;i++)
		square[i]=-0x0002;
	DSK6713_DIP_init();
	DSK6713_LED_init();	
	sreg.regval=0xffff;
	fb = 1;
	comm_intr();
	while(1){}
}
