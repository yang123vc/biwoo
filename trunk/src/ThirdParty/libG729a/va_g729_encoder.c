#include <stdio.h>
#include "va_g729.h"
//#include <dlfcn.h>

void main()
{
	// char char_stream1[160]={60,0,64,0,62,0,62,0,58,0,55,0,43,0,29,0,24,0,20,0,11,0,9,0,11,0,12,0,7,0,10,0,11,0,6,0,4,0,1,0,5,0,10,0,8,0,2,0,1,0,3,0,9,0,9,0,3,0,3,0,8,0,12,0,16,0,18,0,21,0,18,0,17,0,17,0,12,0,13,0,14,0,21,0,29,0,26,0,14,0,8,0,0,0,-12,-1,-15,-1,-10,-1,-12,-1,-21,-1,-16,-1,-8,-1,-8,-1,-10,-1,-6,-1,-10,-1,-15,-1,-14,-1,-8,-1,-8,-1,-8,-1,-5,-1,-2,-1,-4,-1,-13,-1,-14,-1,-21,-1,-27,-1,-33,-1,-36,-1,-38,-1,-36,-1,-29,-1,-23,-1,-18,-1,-19,-1,-17,-1,-11,-1};

	int i;
	/*   void *pdlhandle;
	char *pszerror;
	printf("open libg729encoder.so.\n");
	pdlhandle = dlopen("./libg729encoder.so",RTLD_LAZY);
	pszerror = dlerror();
	if (0!= pszerror)
	{
	printf("%s\n",pszerror);
	exit(1);
	}


	void (*va_g729a_init_encoder)();
	void (*va_g729a_encoder)(short *,unsigned char *);

	va_g729a_init_encoder=dlsym(pdlhandle,"va_g729a_init_encoder");
	va_g729a_encoder=dlsym(pdlhandle,"va_g729a_encoder");
	pszerror = dlerror();
	if(0!=pszerror){
	printf("%s\n",pszerror);
	exit(1);
	}*/



	//	short speech[L_FRAME];
	short input1[80]={18770,17990,28964,2,16727,17750,28006,8308,16,0,1,1,8000,0,16000,0,2,16,24932,24948,28928,2,60,64,62,62,58,55,43,29,24,20,11,9,11,12,7,10,11,6,4,1,5,10,8,2,1,3,9,9,3,3,8,12,16,18,21,18,17,17,12,13,14,21,29,26,14,8,0,-12,-15,-10,-12,-21,-16,-8,-8,-10,-6,-10};
	short input2[80]={-15,-14,-8,-8,-8,-5,-2,-4,-13,-14,-21,-27,-33,-36,-38,-36,-29,-23,-18,-19,-17,-11,-6,-4,-2,0,-6,-9,-10,-15,-17,-17,-13,-12,-13,-3,0,-3,-6,-4,-4,-6,-5,-6,-5,-6,-12,-11,-9,-17,-21,-18,-18,-23,-28,-30,-29,-27,-22,-19,-20,-16,-14,-20,-26,-24,-20,-23,-22,-18,-18,-17,-17,-19,-21,-22,-15,-13,-10,-1};
	short input3[80]={6,2,-10,-17,-16,-15,-12,-8,-1,-1,-5,-2,0,-1,1,1,-11,-19,-23,-27,-29,-30,-33,-38,-41,-30,-19,-9,-11,-11,-11,-4,0,-11,-19,-26,-35,-36,-34,-33,-26,-19,-17,-19,-19,-21,-19,-16,-17,-16,-19,-16,-20,-26,-27,-30,-32,-35,-30,-29,-33,-37,-40,-38,-37,-37,-36,-40,-40,-33,-27,-23,-20,-21,-28,-34,-41,-50,-51,-54};
	//  speech=char_stream1;
	//   printf("len of speech is:%d\n",sizeof(speech));
	//   for(i=0;i<80;i++)
	//  printf("input  is :%d\n",input[i]);
	unsigned char serial[L_FRAME_COMPRESSED];

	va_g729a_init_encoder();
	for(i=0;i<10;i++)
		printf("before serial is:%d\n",serial[i]);

	va_g729a_encoder(input1,serial);
	for(i=0;i<10;i++)
		printf("serial_1 is:%d\n",serial[i]); 

	va_g729a_encoder(input2,serial);
	for(i=0;i<10;i++)
		printf("serial_2 is:%d\n",serial[i]);

	va_g729a_encoder(input3,serial);
	for(i=0;i<10;i++)
		printf("serial_3 is:%d\n",serial[i]);



	//       printf("close libg729encoder.so\n");
	//      dlclose(pdlhandle);

} /* end of main() */


