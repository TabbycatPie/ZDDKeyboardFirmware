#include "ch552.H"                                                      
#include "Debug.H"
#include "DataFlash.h"
#include "usb.h"

//UINT8X MARCO_KEYCODE [50]= { 0x15,0x06,0x10,0x07,0x58,0x15,0x10,0x16,0x17,0x16,  //Win+r,c,m,d,Enter,Win+r,m,s,t,s,
//														 0x06,0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	 //c,Enter,,,,,,,,,
//														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//														 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
//													 };  //marco keycode

UINT8X MARCO_KEYCODE [34]= {0x00};  //marco keycode

UINT8X MARCO_SPLIT_INDX [11] = {0};  //split marco key position

UINT8X MARCO_SPE_KEYINDX [10] = {0x00};
UINT8X MARCO_SPE_KEYCODE [10] = {0x00};

UINT8X MARCO_DELAY_INDX  [10] = {0x00};
UINT8X MARCO_DELAY       [10] = {0x00};

UINT8X MEDIA_CODE [10] = {0x00};
UINT8X MOUSE_CODE [10] = {0x00};  //temporily used for mouse 
UINT8 KEY_CODE   [10] = {0x00};  //temporily used for key 
UINT8 SP_KEY_CODE[10] = {0x00};  //temporily used for sepcial key

//unpressed : 0x00
//  pressed :key_pressed[n] == 0xff
//if key(n) is a marco key 

UINT8 KEY_PRESS  [10] = {0x00};
UINT8 LST_PRESS  [10] = {0x00};
UINT8 KEY_MARCO  [10] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

UINT8X LAST_MEDIA_KEY = 0xff;
UINT8X CUR_MEDIA_KEY = 0xff;
UINT8X CUR_MEDIA_LAG = 0x0a;

UINT8X CUR_MOUSE_KEY = 0x00; //0xff mouse-key is pressed ,0xf0 send blank
UINT8X CUR_KEYBOARD = 0x00;  //0xff keboard-key is pressed,0xf0 send blank

UINT8X LAST_MARCO_KEY = 0xff;
UINT8X CUR_MARCO_KEY = 0xff;
UINT8X CUR_MARCO_LAG = 0x0a;

UINT8X KEY_CHANGE = 0x00;



//delay time*100ms
void MarcoDelay(UINT8 time){
	while(time--)
		mDelaymS(100);
}

void HIDMousesend(){
	if(CUR_MOUSE_KEY == 0xff && KEY_CHANGE == 0xff){
		FLAG = 0;
		Mouse_Send();    //if mouse key is set,then send mouse event
		while(FLAG == 0); 
	}
}

void HIDKeysend(){
	if(CUR_KEYBOARD == 0xff && KEY_CHANGE == 0xff){
		FLAG = 0;
		Keyboard_Send();      //send keyboard event
		while(FLAG == 0); /*�ȴ���һ���������*/
	}
}
//send message to computer
void HIDsendMessage(){
	HID_Busy = 0;
	HID_Send();    //send message
	while(HID_Busy == 0); 
}

void HIDmediasend(){
	if(CUR_MEDIA_KEY!= 0xff){
		//valid key triggered
		if(CUR_MEDIA_KEY!=LAST_MEDIA_KEY){
			FLAG = 0;
			//pressed frist time
			Multimedia_Send();   //send media event
			while(FLAG == 0);
			
			HIDMultimedia[0] = 0;
			FLAG = 0;
			Multimedia_Send();   //send media event
			while(FLAG == 0);
		}
		else if(CUR_MEDIA_LAG == 0){
			FLAG = 0;
			Multimedia_Send();   //send media event
			while(FLAG == 0);
			HIDMultimedia[0] = 0;
			FLAG = 0;
			Multimedia_Send();   //send media event
			while(FLAG == 0);
			CUR_MEDIA_LAG = 0x02;
		}
		else if(CUR_MEDIA_KEY!= 0xff && CUR_MEDIA_KEY==LAST_MEDIA_KEY){
			CUR_MEDIA_LAG--;
		}
		LAST_MEDIA_KEY = CUR_MEDIA_KEY;
	}
	else{
		CUR_MEDIA_LAG = 0x2f;
		LAST_MEDIA_KEY = 0xff;
	}
	
	
//	HIDMultimedia[0] = 0;
//	Enp4IntIn();   //send media event
//	while(FLAG == 0); 
}


//send marco key
void HIDmarco(UINT8 key_num){
	UINT8 i,j,delay_n00ms;
	//get key_num position
	UINT8 key = 0x1E + key_num -1;
	UINT8 pos = 0;
	//init HIDKey[]
	HIDKey[0] = 0x00;  //special key
	HIDKey[1] = 0x00;  //reserved
	HIDKey[2] = 0x00;  //key
	HIDKey[3] = 0x00;  
	HIDKey[4] = 0x00;
	HIDKey[5] = 0x00;
	HIDKey[6] = 0x00;
	HIDKey[7] = 0x00;
	HIDKey[8] = 0x00;
	//HIDKeysend();
	for(i=0;i<8;i++){
		//prepare special key
		delay_n00ms = 0;
		HIDKey[0] = 0x00;

		//prepare normal key
		HIDKey [2] = key;
		Keyboard_Send();    //send keyboard event
		while(FLAG == 0); 
		//delay
		//wait 20ms to bounce up to simulate human clicking
		mDelaymS(20);
		
		//bounce up
		HIDKey[0] = 0x00;
		HIDKey[2] = 0x00;
		Keyboard_Send();    //send keyboard event
		while(FLAG == 0); 
		
		mDelaymS(5);
		
	}
	//clear buffer
//	HIDKey[0] = 0x00;
//	HIDKey[2] = 0x00;
//	Enp1IntIn();    //send keyboard event
//	while(FLAG == 0); 
	//wait marco-key bounce up
	mDelaymS(50);
}
void HIDmarcosend(){
	if(CUR_MARCO_KEY!= 0xff){
		//valid key triggered
		if(CUR_MARCO_KEY!=LAST_MARCO_KEY){
			//pressed frist time
			HIDmarco(CUR_MARCO_KEY);
		}
		else if(CUR_MARCO_LAG == 0){
			HIDmarco(CUR_MARCO_KEY);
			CUR_MARCO_LAG = 0x02;
		}
		else if(CUR_MARCO_KEY!= 0xff && CUR_MARCO_KEY==LAST_MARCO_KEY){
			CUR_MARCO_LAG--;
		}
		LAST_MARCO_KEY = CUR_MARCO_KEY;
	}
	else{
		CUR_MARCO_LAG = 0x2f;
		LAST_MARCO_KEY = 0xff;
	}
}
//report key code to computer
void HIDsend(){
	//HIDKeysend();
	//HIDMousesend();
	//HIDmediasend();
	HIDmarcosend();
}



//10-key maping 
sbit Key1  = P3^2;
sbit Key2  = P1^4;
sbit Key3  = P1^5;
sbit Key4  = P1^6;
sbit Key5  = P1^7;
sbit Key6  = P3^1;
sbit Key7  = P3^0;
sbit Key8  = P1^1;
sbit Key9  = P3^3;
sbit Key10 = P3^4;
void scanKey(){
	UINT8 i = 0;
	UINT8 mouse_code = 0x00;
	UINT8 sp_key_code = 0x00;
	UINT8 media_code = 0x00;
	UINT8 temp_code=0x00;
	UINT8 key_count = 0;
	CUR_MARCO_KEY = 0xff;
	KEY_CHANGE = 0x00;
	if(!Key1){
		KEY_PRESS[0]=0xff;
	}
	else{
		KEY_PRESS[0]=0x00;
	}
	if(!Key2){
		KEY_PRESS[1]=0xff;
	}
	else{
		KEY_PRESS[1]=0x00;
	}
	if(!Key3){
		KEY_PRESS[2]=0xff;
	}
	else{
		KEY_PRESS[2]=0x00;
	}
	if(!Key4){
		KEY_PRESS[3]=0xff;
	}
	else{
		KEY_PRESS[3]=0x00;
	}
	if(!Key5){
		KEY_PRESS[4]=0xff;
	}
	else{
		KEY_PRESS[4]=0x00;
	}
	if(!Key6){
		KEY_PRESS[5]=0xff;
	}
	else{
		KEY_PRESS[5]=0x00;
	}
	if(!Key7){
		KEY_PRESS[6]=0xff;
	}
	else{
		KEY_PRESS[6]=0x00;
	}
	if(!Key8){
		KEY_PRESS[7]=0xff;
	}
	else{
		KEY_PRESS[7]=0x00;
	}
	if(!Key9){
		KEY_PRESS[8]=0xff;
	}
	else{
		KEY_PRESS[8]=0x00;
	}
	if(!Key10){
		KEY_PRESS[9]=0xff;
	}
	else{
		KEY_PRESS[9]=0x00;
	}
	mDelaymS(6); //avoid jitter
	if(!Key1){
		if(KEY_PRESS[0]!=0xff){
			KEY_PRESS[0] =0x00;
		}
		else{
			if(KEY_MARCO[0]==0xff)
				CUR_MARCO_KEY = 1;
		}
	}
	if(!Key2){
		if(KEY_PRESS[1]!=0xff){
			KEY_PRESS[1] =0x00;
		}
		else{
			if(KEY_MARCO[1]==0xff)
				CUR_MARCO_KEY = 2;
		}
	}
	if(!Key3){
		if(KEY_PRESS[2]!=0xff){
			KEY_PRESS[2] =0x00;
		}
		else{
			if(KEY_MARCO[2]==0xff)
				CUR_MARCO_KEY = 3;
		}
	}
	if(!Key4){
		if(KEY_PRESS[3]!=0xff){
			KEY_PRESS[3] =0x00;
		}
		else{
			if(KEY_MARCO[3]==0xff)
				CUR_MARCO_KEY = 4;
		}
	}
	if(!Key5){
		if(KEY_PRESS[4]!=0xff){
			KEY_PRESS[4] =0x00;
		}
		else{
			if(KEY_MARCO[4]==0xff)
				CUR_MARCO_KEY = 5;
		}
	}
	if(!Key6){
		if(KEY_PRESS[5]!=0xff){
			KEY_PRESS[5] =0x00;
		}
		else{
			if(KEY_MARCO[5]==0xff)
				CUR_MARCO_KEY = 6;
		}
	}
	if(!Key7){
		if(KEY_PRESS[6]!=0xff){
			KEY_PRESS[6] =0x00;
		}
		else{
			if(KEY_MARCO[6]==0xff)
				CUR_MARCO_KEY = 7;
		}
	}
	if(!Key8){
		if(KEY_PRESS[7]!=0xff){
			KEY_PRESS[7] =0x00;
		}
		else{
			if(KEY_MARCO[7]==0xff)
				CUR_MARCO_KEY = 8;
		}
	}
	if(!Key9){
		if(KEY_PRESS[8]!=0xff){
			KEY_PRESS[8] =0x00;
		}
		else{
			if(KEY_MARCO[8]==0xff)
				CUR_MARCO_KEY = 9;
		}
	}
	if(!Key10){
		if(KEY_PRESS[9]!=0xff){
			KEY_PRESS[9] =0x00;
		}
		else{
			if(KEY_MARCO[9]==0xff)
				CUR_MARCO_KEY = 10;
		}
	}
	
	CUR_MOUSE_KEY = 0x00;
	CUR_KEYBOARD = 0x00;
	CUR_MEDIA_KEY = 0xff;
	//get final result
	for(;i<10;i++){
		mouse_code  |= MOUSE_CODE [i]&KEY_PRESS[i]; //generate mouse_code
		sp_key_code |= SP_KEY_CODE[i]&KEY_PRESS[i]; //generate special_key_code
		temp_code  = MEDIA_CODE [i]&KEY_PRESS[i]; //generate meida_key_code
		if(temp_code!=0){
			media_code = temp_code;
			CUR_MEDIA_KEY=i;
		}
		if(mouse_code!=0x00){
			CUR_MOUSE_KEY = 0xff;
		}
		if(sp_key_code!=0x00){
			CUR_KEYBOARD = 0xff;
		}
		if(KEY_PRESS[i] != LST_PRESS[i]){
			KEY_CHANGE = 0xff;
			LST_PRESS[i] = KEY_PRESS[i];
		}
		//generate normal key_code
		if(key_count<6 && (KEY_PRESS[i]==0xff)){
			HIDKey[2+key_count]=KEY_CODE[i];
			key_count ++;
			CUR_KEYBOARD = 0xff;
		}
	}
	//FINAL ASSIGNING
	//mouse codes
	if(HIDMouse[0]!=0x00 && mouse_code==0x00)
		CUR_MOUSE_KEY = 0xff;
	HIDMouse[0] = mouse_code;
	//meida codes
	HIDMultimedia[0] = media_code;
	//key codes
	if(HIDKey[0]!=0x00 && sp_key_code==0x00)
		CUR_KEYBOARD = 0xff;
	HIDKey  [0] = sp_key_code; //special key Byte
	HIDKey  [1] = 0x00;        //reserved
	if(key_count<6){					 //fill blank
		for(i=7;i>=key_count+2;i--){
			if(HIDKey[i]!=0x00 && key_count == 0)
				CUR_KEYBOARD = 0xff;
			HIDKey[i]=0x00;
		}
	}
}

void setMarco(unsigned char hi,unsigned char lo){
    unsigned char cur = 0x01;
    unsigned char i = 0;
    for(;cur!=0x00;cur<<=1){
        if((lo&cur)==cur){
            KEY_MARCO[i] = 0xff;
        }
				else{
					 KEY_MARCO[i] = 0x00;
				}
        i++;
    }
    for(cur=0x01;cur!=0x04;cur<<=1){
        if((hi&cur)==cur){
            KEY_MARCO[i] = 0xff;
        }
				else{
					KEY_MARCO[i] = 0x00;
				}
        i++;
    }
}




void initKeyValue(){
	//read from data flash
	UINT8 _temp[2];
	ReadDataFlash(0,10,KEY_CODE);
	ReadDataFlash(10,10,SP_KEY_CODE);
	ReadDataFlash(20,2,_temp);
	setMarco(_temp[0],_temp[1]);
	ReadDataFlash(22,11,MARCO_SPLIT_INDX);
	ReadDataFlash(33,10,MARCO_SPE_KEYCODE);
	ReadDataFlash(43,10,MARCO_SPE_KEYINDX);
	ReadDataFlash(53,34,MARCO_KEYCODE);
  //if mouse key is used send mouse every time
	ReadDataFlash(87,10,MOUSE_CODE);
	ReadDataFlash(97,10,MEDIA_CODE);
	ReadDataFlash(107,10,MARCO_DELAY);
	ReadDataFlash(117,10,MARCO_DELAY_INDX);
}

void main(){
  CfgFsys();                    //CH552ʱ��ѡ������
  mDelaymS(20);                 //�޸���Ƶ�ȴ��ڲ������ȶ�,�ؼ�

	/* ����P1��Ϊ׼˫��IO�� */
	P1_MOD_OC = 0xff;
	P1_DIR_PU = 0xff;
	
	P3_MOD_OC = 0xFF;
	P3_DIR_PU = 0xFF;
	
	UEP1_T_LEN = 0;      //Ԥʹ�÷��ͳ���һ��Ҫ���
	UEP2_T_LEN = 0;      //Ԥʹ�÷��ͳ���һ��Ҫ���
	FLAG = 0;
	Ready = 0;
	
	USBDeviceInit();              //USB�豸ģʽ��ʼ��
	//initKeyValue();              	//intialize key 
	
	EA = 1;                       //����Ƭ���ж�
	//�ȴ�USBö�ٳɹ�
	while(Ready == 0);
	while(1)
	{
		if(Ready)
		{
			scanKey();
			HIDsend();
			//handleReceive();
			FLAG = 0;
		}
	}
}
