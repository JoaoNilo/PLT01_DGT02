//==============================================================================
/**
 * @file Application.cpp
 * @title Digit Controller Module DGT-01 - Placar Tenis PLT-01
 * @author Joao Nilo Rodrigues  -  nilo@pobox.com
 */
//------------------------------------------------------------------------------
// NOTA: enm caso de necessidade de mais espaço na memória flash remover a
// variável BatteryVoltage (float) passando o cálculo final para o Android.
//------------------------------------------------------------------------------
// History:
// 2023-05-10: implemented delayed updates to FlipDisplay class, based on module
//             address to avoid "current surges" which caused "over current faults".
// 2023-05-11: delay constants increased to 500, 1000, 1500 and 2000ms.
//			   FlipDisplay was not counting "Delay" in "SetValue" and "DebugServo".
//             improved Value and Arrow updates.
//			   addressing code changed temporarily due to hardware bug. [V1.0.0-2]
//
//------------------------------------------------------------------------------
#include "Application.h"
#include "FlipDisplay.h"

//------------------------------------------------------------------------------
// NOTE: product ID, firmware version and publishing date
//------------------------------------------------------------------------------
const uint32_t productID 			= 1;
const uint8_t firmwareVersion[4] 	= {1,0,0,2};
const uint8_t publishingDate[4]  	= {11,05,20,23};

NLed* Led_Heartbeat;
NTimer* Timer1;


// Bus protocol components
NSerial* BusPort;
NTinyOutput* BusPort_DE;
NTinyOutput* BusPort_RE;
NDatagram* BUS_OutData;
NDataLink* BUS_Link;
NSerialProtocol* BUS_Interpret;
NSerialCommand* busGetVersion;
NSerialCommand* busGetStatus;
NSerialCommand* busSetData;
NSerialCommand* busSetServo;

FlipDisplay* Digit;
NTinyOutput* SegDrvH;
NTinyOutput* SegDrvV;
NTinyOutput* Segment_A;
NTinyOutput* Segment_B;
NTinyOutput* Segment_C;
NTinyOutput* Segment_D;
NTinyOutput* Segment_E;
NTinyOutput* Segment_F;
NTinyOutput* Segment_G;
NTinyOutput* Segment_H;

//------------------------------------------------------------------------------
// data section
#define SCORE_PARAMS_SIZE		14
#define PARAMS_PLAY1_TENS		0
#define PARAMS_PLAY1_UNITS		1
#define PARAMS_PLAY1_SET1		2
#define PARAMS_PLAY1_SET2		3
#define PARAMS_PLAY1_SET3		4
#define PARAMS_PLAY2_TENS		5
#define PARAMS_PLAY2_UNITS		6
#define PARAMS_PLAY2_SET1		7
#define PARAMS_PLAY2_SET2		8
#define PARAMS_PLAY2_SET3		9
#define PARAMS_FLAGS			10
#define PARAMS_SECONDS			11
#define PARAMS_MINUTES			12
#define PARAMS_HOURS			13
uint8_t ScoreParams[SCORE_PARAMS_SIZE];

#define PARAMS_FLAGS_SERV_MASK	((uint8_t) 0x03)
#define PARAMS_FLAGS_SERV_PLAY1	((uint8_t) 0x01)
#define PARAMS_FLAGS_SERV_PLAY2	((uint8_t) 0x02)

//------------------------------------------------------------------------------
uint8_t DebugParams[2];

uint8_t myBITE = 0;
uint8_t LocalAddress = 0;

//------------------------------------------------------------------------------
uint8_t LocalIndex = 255;

bool calibrating;
uint8_t fsm_bus = FSM_IDLE;
uint8_t fsm_counter = BUS_NODES;
uint8_t test_counter = 0;

float BatteryVoltage = 0.0;
//------------------------------------------------------------------------------
const uint8_t NodeAddresses[BUS_NODES] = {
		PLAY1_TENS, PLAY1_UNITS, PLAY1_SET1, PLAY1_SET2, PLAY1_SET3,
		PLAY2_TENS, PLAY2_UNITS, PLAY2_SET1, PLAY2_SET2, PLAY2_SET3
};

//------------------------------------------------------------------------------
const uint16_t NodeDelay[BUS_NODES] = {
		DELAY_TENS, DELAY_UNITS, DELAY_SET1, DELAY_SET2, DELAY_SET3,
		DELAY_TENS, DELAY_UNITS, DELAY_SET1, DELAY_SET2, DELAY_SET3
};

//------------------------------------------------------------------------------
void Timer1_OnTimer();
void BusPort_OnPacket(uint8_t* data, uint8_t size);
void BusPort_OnEnterTransmission();
void BusPort_OnLeaveTransmission();
void BusLink_OnPacketToSend(uint8_t*, uint8_t);
void busGetVersion_OnProcess(NDatagram*);
void busGetStatus_OnProcess(NDatagram*);
void busSetData_OnProcess(NDatagram*);
void busSetServo_OnProcess(NDatagram*);

//------------------------------------------------------------------------------
void AddressResolution();

//------------------------------------------------------------------------------
void ApplicationCreate(){

 	Led_Heartbeat = new NLed(LED);
	Led_Heartbeat->Interval = 250;
	Led_Heartbeat->Status = ldBlinking;

    //--------------------------------------------------------------------------
    // Bus communication components
    BusPort = new NSerial(BUS_PORT); 			// Tx: PA9 Rx: PA10  DE: PA12  RE: PA11
    BusPort->OnPacket = BusPort_OnPacket;
    BusPort->OnEnterTransmission = BusPort_OnEnterTransmission;
    BusPort->OnLeaveTransmission = BusPort_OnLeaveTransmission;
    BusPort->Open();

    BUS_OutData = new NDatagram();
    BUS_OutData->Destination = PROSA_ADDR_BROADCAST;
    BUS_OutData->Source = PROSA_ADDR_IHM1;
    BUS_OutData->Command = PROSA_CMD_VERSION;

    BUS_Link = new NDataLink();
    BUS_Link->TimeReload = 100;
    BUS_Link->TimeDispatch = 40;
    BUS_Link->Timeout = 25;
    BUS_Link->BusPrivilege = dlSlave;
    BUS_Link->ServiceAddress = PROSA_ADDR_SERVICE;
    BUS_Link->BroadcastAddress = PROSA_ADDR_BROADCAST;
    BUS_Link->OnPacketToSend = BusLink_OnPacketToSend;

    BUS_Interpret = new NSerialProtocol(BUS_Link);

    busGetVersion = new NSerialCommand(BUS_Interpret);
    busGetVersion->ID = PROSA_CMD_VERSION;
    busGetVersion->OnProcess = busGetVersion_OnProcess;

    busGetStatus = new NSerialCommand(BUS_Interpret);
    busGetStatus->ID = PROSA_CMD_GETSTATUS;
    busGetStatus->OnProcess = busGetStatus_OnProcess;

    busSetData = new NSerialCommand(BUS_Interpret);
    busSetData->ID = PROSA_CMD_SETDATA;

    busSetServo = new NSerialCommand(BUS_Interpret);
    busSetServo->ID = PROSA_CMD_SETSERVO;

    BusPort_DE = new NTinyOutput(USART1_RTS);
    BusPort_RE = new NTinyOutput(USART1_CTS);
    BusPort_RE->Level = toLow; BusPort_DE->Level = toLow;

    //--------------------------------------------------------------------------
    calibrating = true;
    Timer1 = new NTimer();
    Timer1->OnTimer = Timer1_OnTimer;
    Timer1->Start(1000);

    SegDrvH = new NTinyOutput(DRV_HR);
    SegDrvV = new NTinyOutput(DRV_VR);

    // libera os pinos PB3 e PB4 (JTAG)
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;

    Segment_A  = new NTinyOutput(SEGMENT_A);
    Segment_B  = new NTinyOutput(SEGMENT_B);
    Segment_C  = new NTinyOutput(SEGMENT_C);
    Segment_D  = new NTinyOutput(SEGMENT_D);
    Segment_E  = new NTinyOutput(SEGMENT_E);
    Segment_F  = new NTinyOutput(SEGMENT_F);
    Segment_G  = new NTinyOutput(SEGMENT_G);

    Digit = new FlipDisplay(TIMEBASE);
    Digit->Driver_H = SegDrvH;
    Digit->Driver_V = SegDrvV;
    Digit->Segment[0] = Segment_A;
    Digit->Segment[1] = Segment_B;
    Digit->Segment[2] = Segment_C;
    Digit->Segment[3] = Segment_D;
    Digit->Segment[4] = Segment_E;
    Digit->Segment[5] = Segment_F;
    Digit->Segment[6] = Segment_G;

    //------------------------------------------
    AddressResolution();

    //------------------------------------------

    if((LocalAddress == PLAY1_TENS)||(LocalAddress == PLAY2_TENS)){
    	Segment_H  = new NTinyOutput(SEGMENT_H);
    	Digit->Segment[Seg_H] = Segment_H;
    }

    if(LocalIndex < BUS_NODES){
    	Digit->Delay = NodeDelay[LocalIndex];
    }

    BUS_Link->LocalAddress = LocalAddress;
    BUS_Link->Open();

    DebugParams[PARAM_DUTY] = PPM_SEG_CALIBRATE;
    DebugParams[PARAM_SEGMENTS] = SERVOS_DIGIT | SERVOS_ARROW;
    Digit->DebugServo(DebugParams);
}

//------------------------------------------------------------------------------
void Timer1_OnTimer(){
	if(calibrating){
		Timer1->Stop();
		calibrating = false;
	    busSetData->OnProcess = busSetData_OnProcess;
	    busSetServo->OnProcess = busSetServo_OnProcess;
	}
}

//------------------------------------------------------------------------------
void BusPort_OnEnterTransmission(){
	BusPort_RE->Level = toHigh; BusPort_DE->Level = toHigh;
}

//------------------------------------------------------------------------------
void BusPort_OnLeaveTransmission(){
	BusPort_DE->Level = toLow; BusPort_RE->Level = toLow;
}


//------------------------------------------------------------------------------
void BusPort_OnPacket(uint8_t* data, uint8_t size){
	BUS_Link->ProcessPacket(data, size);
}

//------------------------------------------------------------------------------
void BusLink_OnPacketToSend(uint8_t* data, uint8_t size){
	BusPort->Write(data, size);
}

//------------------------------------------------------------------------------
// BUS PROTOCOL
//------------------------------------------------------------------------------
void busGetVersion_OnProcess(NDatagram* iDt){
	iDt->SwapAddresses(); iDt->Size = 0;
	iDt->Append(*(uint32_t*) &productID);
	iDt->Append(*(uint32_t*) publishingDate);
	iDt->Append(*(uint32_t*) firmwareVersion);
	iDt->UpdateCrc();
}

//------------------------------------------------------------------------------
void busGetStatus_OnProcess(NDatagram* iDt){

	if((iDt->Length > 0) && (iDt->Extract() == 0x01)){
		AddressResolution();
	}

	iDt->SwapAddresses(); iDt->Size = 0;
	iDt->Append(LocalAddress);
	iDt->Append(myBITE);
	iDt->UpdateCrc();
}

//------------------------------------------------------------------------------
// Get the update values from the Control Unit
void busSetData_OnProcess(NDatagram* iDt){

	if(iDt->Length == SCORE_PARAMS_SIZE){
		iDt->Extract(ScoreParams, SCORE_PARAMS_SIZE);
		//result = true;
	}

	if(LocalIndex <= BUS_NODES){
		Digit->Value = ScoreParams[LocalIndex];

		if(LocalAddress == PLAY1_TENS){
			if(ScoreParams[PARAMS_FLAGS] & PARAMS_FLAGS_SERV_PLAY1){ Digit->Arrow = true;}
			else { Digit->Arrow = false;}
		} else if(LocalAddress == PLAY2_TENS){
			if(ScoreParams[PARAMS_FLAGS] & PARAMS_FLAGS_SERV_PLAY2){ Digit->Arrow = true;}
			else { Digit->Arrow = false;}
		}
	}

	iDt->SwapAddresses();
	iDt->Flush();
	iDt->Append(LocalAddress);
	iDt->Append(myBITE);
	iDt->UpdateCrc();
}

//------------------------------------------------------------------------------
// Set servo position (hardware debug purpose only)
// | dst | src | len | cmd |  segments| duty | crc | crc |
// sv_id: servo id: 1 = seg_A, 2= seg_B, ...
// duty: pulse width in miliseconds
void busSetServo_OnProcess(NDatagram* iDt){

	if(iDt->Length == 2){
		iDt->Extract(DebugParams, 2);
		//result = true;
		Digit->DebugServo(DebugParams);
	}

	iDt->SwapAddresses();
	iDt->Flush();
	iDt->Append(LocalAddress);
	iDt->Append(myBITE);
	iDt->UpdateCrc();
}

//------------------------------------------------------------------------------
void AddressResolution(){
	LocalAddress = 0;
	LocalIndex = -1;

	NInput* Addr0 = new NInput(ADDR0);
	NInput* Addr1 = new NInput(ADDR1);
	NInput* Addr2 = new NInput(ADDR2);
	NInput* Addr3 = new NInput(ADDR3);
	NInput* Addr4 = new NInput(ADDR4);

	Addr0->Bias = inPullUp;
	Addr1->Bias = inPullUp;
	Addr2->Bias = inPullUp;
	Addr3->Bias = inPullUp;
	Addr4->Bias = inPullUp;

	Addr0->Access = inImmediate;
	Addr1->Access = inImmediate;
	Addr2->Access = inImmediate;
	Addr3->Access = inImmediate;
	Addr4->Access = inImmediate;

	LocalAddress |= (Addr0->Level << 0);
	LocalAddress |= (Addr1->Level << 1);
	LocalAddress |= (Addr2->Level << 2);
	LocalAddress |= (Addr3->Level << 3);
	LocalAddress |= (Addr4->Level << 4);

	for(int i = 0; i < BUS_NODES; i++){
		if(NodeAddresses[i] == LocalAddress){ LocalIndex = i;}
	}

	delete Addr0;
	delete Addr1;
	delete Addr2;
	delete Addr3;
	delete Addr4;
}

//==============================================================================

