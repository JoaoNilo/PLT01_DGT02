//==============================================================================
/**
 * @file FlipDisplay.h
 * @brief Switch abstraction driver class\n
 * This class provides resources to setup and use general purpose IO pin as\n
 * a "Switch" (permanent state) or "Push-Button" (momentary state).
 * @version 1.0.0
 * @author Joao Nilo Rodrigues - nilo@pobox.com
 *
 *------------------------------------------------------------------------------
 *
 * <h2><center>&copy; Copyright (c) 2020 Joao Nilo Rodrigues
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by "Joao Nilo Rodrigues" under BSD 3-Clause
 * license, the "License".
 * You may not use this file except in compliance with the License.
 *               You may obtain a copy of the License at:
 *                 opensource.org/licenses/BSD-3-Clause
 *
 *///------------------------------------------------------------------------------
#ifndef FlipDisplay_H
    #define FlipDisplay_H

    #include "NHardwareTimer.h"
	#include "NTinyOutput.h"

    //-----------------------------------
	/**
	 * @enum fdStates
	 * @brief This enumeration defines the options for the @ref state variable.
	 */
    enum fdStates { fdServosWaiting,		//!< waiting servos start-up delay
    				fdServosOn,				//!< waiting servos power-on
    				fdServos_Start_Clear,
    	            fdServos_Start_H,   	//!< waiting 4 servos reach new position
					fdServos_Stop_H,    	//!< waiting 4 servos reach new position
    	            fdServos_Start_V,   	//!< waiting 4 servos reach new position
					fdServos_Stop_V,    	//!< waiting 4 servos reach new position
				    fdServosOff,			//!< waiting servos power-off
					fdIdle,
					fdArrowOn,				//!< waiting arrow servos power-on
					fdArrow_Move,   		//!< waiting arrow servos reach new position
					fdArrowOff,			//!< waiting arrow servos power-off
    			 };

    //-----------------------------------
    /** @brief Mechanical, servo driven, 7-segments display abstraction class\n
     */
    class FlipDisplay : private NHardwareTimer{

        private:
			#define PPM_TIMEBASE_100us	100
			#define PPM_PERIOD   		200
			#define PPM_SEG_SHOWN	  	 14
			#define PPM_SEG_HIDDEN		  4
			#define PPM_SEG_CALIBRATE	  4
			#define PPM_SEG_CLEAR	  	  11

			#define PARAM_SEGMENTS		  0
			#define PARAM_DUTY			  1

			#define FSM_JUMP	 	 	  2
			#define FSM_SERVOS_ON	 	 20
			#define FSM_SERVOS_OFF	 	 10
			#define FSM_SERVOS_CLEARING	 100
			#define FSM_SERVOS_MOVING_H	 200
			#define FSM_SERVOS_MOVING_V	 400
			#define FSM_ARROW_MOVING	 500


			#define SERVOS_DIGIT		 0b01111111
			#define SERVOS_ARROW		 0b10000000
    		#define SERVOS_HORIZONTAL	 0b01001001
			#define SERVOS_VERTICAL		 0b00110110
			#define SERVOS_CLEAR		 0b00100010
			#define SERVOS_NONE			 0b00000000

			#define Seg_A				0
			#define Seg_B				1
			#define Seg_C				2
			#define Seg_D				3
			#define Seg_E				4
			#define Seg_F				5
			#define Seg_G				6
			#define Seg_H				7

            //-------------------------
            fdStates next_state;
            fdStates current_state;
            uint8_t value;
            uint8_t previous;
            uint8_t period;
            uint8_t duty[8];
            uint8_t segment[8];
            uint8_t seg_B;
            uint8_t seg_F;
           //volatile bool G_Clear;
            uint8_t group_to_move;
            uint8_t previous_g;
            bool arrow;

            //-------------------------
            uint32_t fsm_counter;

            //-------------------------
            void SetValue(uint8_t);
            uint8_t GetValue();

            void SetArrow(bool);
            bool GetArrow(void);

            //-------------------------
            void Convert(uint8_t);
            void RunStateMachine();

        protected:
            bool ProcessEvent();

        //-------------------------------------------
        public:
            //-------------------------------------------
            // METHODS
            /**
             * @brief Constructor for this component.
             * @arg Port: GPIO to be used (GPIOA, GPIOB, etc.)
             * @arg Pin: pin number (0 to 15)
             * @note Make sure the specified GPIO and pin number are valid for
             * the chosen target MCU.
             */
            FlipDisplay(TIM_TypeDef*);


            /**
             * @brief This method is used as a system callback function for message dispatching.
             */
            virtual void Notify(NMESSAGE*);

            /**
             * @brief This method is used as a system callback function for time critical
             * events dispatching.
             */
            //virtual void InterruptCallBack(NMESSAGE*);


            /**
             * @brief PPM signal control (period / duty)
             */
            void Run(uint8_t);

            /**
             * @brief Servo segments positioning test command
             */
            void DebugServo(uint8_t*);

            //---------------------------------------
            // EVENTS
            /**
             * @brief This is the event handler for state changes.
             * - This event handler is called every time a state change is detected.
             * @note This event can be used when @ref swSwitch mode is selected.
             */
            void (*OnValueUpdate)(void);

            /**
             * @brief This is the event handler for "button press".
             * @note This event can be used when @ref swButton mode is selected.
             */
            //id (*OnPress)(void);

            /**
             * @brief This is the event handler for "button release".
             * @note This event can be used when @ref swButton mode is selected.
             */
            //void (*OnRelease)(void);

            //---------------------------------------
            // PROPERTIES
            using NComponent::Tag;

            bool Enabled;


            /**
             * @brief This property is used to assign new value to display.
             */
            property<FlipDisplay, uint8_t, propReadWrite> Value;

            /**
             * @brief This property is used to assign new arrow status (segment H) to display.
             */
            property<FlipDisplay, bool, propReadWrite> Arrow;

            /**
             * @brief This property is used to define the hardware output for horizontal segments servo power line.
             */
            NTinyOutput* Driver_H;

            /**
             * @brief This property is used to define the hardware output for vertical segments servo power line.
             */
            NTinyOutput* Driver_V;

            /**
             * @brief This property is used to define the hardware output for each segment.
             */
            NTinyOutput* Segment[8];

            /**
             * @brief This property is used to assign new "delay" to display.
             */
            uint16_t Delay;


    };

#endif
//==============================================================================
