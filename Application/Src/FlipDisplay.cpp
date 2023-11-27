//==============================================================================
#include "FlipDisplay.h"


//------------------------------------------------------------------------------
FlipDisplay::FlipDisplay(TIM_TypeDef* TIMn):NHardwareTimer(TIMn){

    OnValueUpdate = NULL;

    Value.setOwner(this);
    Value.set(&FlipDisplay::SetValue);
    Value.get(&FlipDisplay::GetValue);

    Arrow.setOwner(this);
    Arrow.set(&FlipDisplay::SetArrow);
    Arrow.get(&FlipDisplay::GetArrow);

    //---------------------------
    Enabled = true;
    next_state = fdIdle;
	period = PPM_PERIOD;
	previous = 0xFF;
	arrow = false;
	Delay = 0;

    for(int c=0; c<8; c++){ Segment[c] = NULL; segment[c] = PPM_SEG_SHOWN;}
    
    //---------------------------
    fsm_counter = 0;
}

//------------------------------------------------------------------------------
//FlipDisplay::~FlipDisplay(void){}

//------------------------------------------------------------------------------
uint8_t FlipDisplay::GetValue(){
    return(value);
}

//------------------------------------------------------------------------------
void FlipDisplay::SetValue(uint8_t new_value){
	if(Enabled == false){ return;}
    if(new_value != previous){
    	value = new_value;
    	if(next_state == fdIdle){
    		Convert(value);
    		next_state = fdServosWaiting; fsm_counter = Delay;
    		Start(PPM_TIMEBASE_100us);
    	}
    } else {
    	if(OnValueUpdate != NULL){ OnValueUpdate();}
    }
}

//------------------------------------------------------------------------------
bool FlipDisplay::GetArrow(void){ return arrow;}

//------------------------------------------------------------------------------
void FlipDisplay::SetArrow(bool status){

	if(Enabled == false){ return;}
	if(status == arrow){ return;}

	arrow = status;
	if(arrow){ segment[Seg_H] = PPM_SEG_SHOWN;}
	else { segment[Seg_H] = PPM_SEG_HIDDEN;}

	if(next_state == fdIdle){
		next_state = fdArrowOn; fsm_counter = FSM_SERVOS_ON;
		Start(PPM_TIMEBASE_100us);
	}
}

//------------------------------------------------------------------------------
void FlipDisplay::DebugServo(uint8_t* params){
	uint8_t segments = params[PARAM_SEGMENTS];
	uint8_t duty = params[PARAM_DUTY];
	uint8_t mask = ((uint8_t) 0x01);

	previous_g = segment[6];

	if(Enabled == false){ return;}
	if(next_state == fdIdle){
		for(int c=0; c<8; c++){
			//segment is updated
			if(segments & (mask<<c)){
				segment[c] = duty;
			}
		}

		group_to_move = segments;
		next_state = fdServosWaiting; fsm_counter = Delay;
		Start(PPM_TIMEBASE_100us);
	}
}

//------------------------------------------------------------------------------
void FlipDisplay::Run(uint8_t group){
	uint8_t mask = 0x01;
	if(period > 0){
		period--;
		for(int c=0; c<8; c++){
			if(group & (mask << c)){
				if(duty[c] > 0){ duty[c]--;}
				else {
					duty[c] = segment[c];
					// reset the "duty signal x" line back to "0"
					if(Segment[c] != NULL){ Segment[c]->Level = toLow;}
				}
			}
		}
	} else {
		period = PPM_PERIOD;
		for(int c=0; c<8; c++){
			if(group & (mask << c)){
				duty[c] = segment[c];
				if((current_state == fdServos_Start_Clear)||(current_state == fdServos_Start_H)||
						(current_state == fdServos_Start_V)||(current_state == fdArrow_Move)){
					// set the "duty signal x" high again
					if(Segment[c] != NULL){ Segment[c]->Level = toHigh;}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
bool FlipDisplay::ProcessEvent(){
	if(Enabled){
		Run(group_to_move);
	}
	//Segment[0]->Toggle();
	return(true);
}

//------------------------------------------------------------------------------
/*void FlipDisplay::InterruptCallBack(NMESSAGE* msg){
	NHardwareTimer::InterruptCallBack(msg);
}*/

//------------------------------------------------------------------------------
void FlipDisplay::Notify(NMESSAGE* msg){

    if(Enabled){    
        switch(msg->message){
            case NM_TIMETICK:
            	if(next_state != fdIdle){ RunStateMachine();}
                break;
            default:break;
        }
    }
    msg->message = NM_NULL;
}

//------------------------------------------------------------------------------
void FlipDisplay::RunStateMachine(){
	if( fsm_counter>0){ fsm_counter--; return;}

	// save current state before changing it
	current_state = next_state;

	switch(next_state){
		case fdServosWaiting:
			fsm_counter = FSM_SERVOS_ON;
			next_state = fdServosOn;
			break;

		case fdServosOn:
    		if(ServoPower != NULL) { ServoPower->Level = toHigh;}
    		fsm_counter = FSM_SERVOS_ON;
    		next_state = fdServos_Start_Clear; // next state
    		break;

		// start moving the
		case fdServos_Start_Clear:
			seg_B = segment[Seg_B]; seg_F = segment[Seg_F];
			segment[Seg_B] = PPM_SEG_CLEAR;
			segment[Seg_F] = PPM_SEG_CLEAR;
			group_to_move = SERVOS_CLEAR;
			fsm_counter = FSM_SERVOS_CLEARING;
			next_state = fdServos_Start_H;
			break;

		// start moving the horizontal segments
		case fdServos_Start_H:
			segment[Seg_B] = seg_B;
			segment[Seg_F] = seg_F;
			group_to_move = SERVOS_HORIZONTAL;
			fsm_counter = FSM_SERVOS_MOVING_H;
			next_state = fdServos_Stop_H;
			break;

		// finish horizontal segments move
		case fdServos_Stop_H: // fdServosMoving2
			fsm_counter = FSM_SERVOS_OFF;
			next_state = fdServos_Start_V;
			break;

		// start moving the vertical segments
		case fdServos_Start_V:
			fsm_counter = FSM_SERVOS_MOVING_V;
			//group_to_move = SERVOS_VERTICAL;
			group_to_move = SERVOS_DIGIT;
			next_state = fdServos_Stop_V; // next state
			break;

		// finish vertical segments move
		case fdServos_Stop_V:
			previous = value;
			fsm_counter = FSM_SERVOS_OFF;
			if(Delay == 0){
				next_state = fdArrowOn; // next state
			} else {
				next_state = fdServosOff; // next state
			}
			break;

		case fdServosOff:
			Stop();
			if(ServoPower != NULL){ ServoPower->Level = toLow;}
			next_state = fdIdle;
			if(OnValueUpdate != NULL){ OnValueUpdate();}
			break;

		case fdArrowOn:
    		if(ServoPower != NULL) { ServoPower->Level = toHigh;}
    		fsm_counter = FSM_SERVOS_ON;
    		next_state = fdArrow_Move; // next state
    		break;

		// keep moving
		case fdArrow_Move:
			group_to_move = SERVOS_ARROW;
			fsm_counter = FSM_ARROW_MOVING;
			next_state = fdArrowOff;
			break;

		case fdArrowOff:
			Stop();
			if(ServoPower != NULL){ ServoPower->Level = toLow;}
			next_state = fdIdle;
			break;

		default: break;
	}
}


//------------------------------------------------------------------------------
// Conversion table (bits):     6   5   4   3   2   1   0      Binary    Hex
// Segments:             		G   F   E   D   C   B   A
// Value: 0						-   x   x   x   x   x   x  = 0011 1111 = 0x3F
// Value: 1						-   -   -   -   x   x   -  = 0000 0110 = 0x06
// Value: 2						x   -   x   x   -   x   x  = 0101 1011 = 0x5B
// Value: 3						x   -   -   x   x   x   x  = 0100 1111 = 0x4F
// Value: 4						x   x   -   -   x   x   -  = 0110 0110 = 0x66
// Value: 5						x   x   -   x   x   -   x  = 0110 1101 = 0x6D
// Value: 6						x   x   x   x   x   -   x  = 0111 1101 = 0x7D
// Value: 7						-   -   -   -   x   x   x  = 0111 1101 = 0x07
// Value: 8						x   x   x   x   x   x   x  = 0111 1111 = 0x7F
// Value: 9						x   x   -   x   x   x   x  = 0110 1111 = 0x6F
// Value: A						x   x   x   -   x   x   x  = 0111 0111 = 0x77
// Value: b						x   x   x   x   x   -   -  = 0111 1100 = 0x7C
// Value: C						-   x   x   x   -   -   x  = 0011 1001 = 0x39
// Value: d						x   -   x   x   x   x   -  = 0101 1110 = 0x5E
// Value: E						x   x   x   x   -   -   x  = 0111 1001 = 0x79
// Value: F						x   x   x   -   -   -   x  = 0111 1001 = 0x71
//------------------------------------------------------------------------------
const uint8_t conversion_table[17] = {
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
  0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,
  0x00
};

//------------------------------------------------------------------------------
void FlipDisplay::Convert(uint8_t n){
	uint8_t converted_value; uint8_t mask=0x01;
	previous_g = segment[Seg_G];

	if(n > 0x0F){ n = 0x10;}
	converted_value = conversion_table[n];
	if(arrow){ converted_value |= SERVOS_ARROW;}
	for(int c=0; c<8; c++){

		if(converted_value & (mask<<c)){
			segment[c] = PPM_SEG_SHOWN;
		} else {
			segment[c] = PPM_SEG_HIDDEN;
		}
	}
}

//==============================================================================

