#ifndef KEYPAD_ADC_H
#define KEYPAD_ADC_H

#define KEY_ACTION_DELAY 10 //1000ms delay after key

class STM32_IO
{
  private:
	uint8_t ex_key=0;

  public:

	NAVI_BIT navi_state;
	uint8_t beep=0;
	uint8_t cancel_key=0;
	uint8_t on_item_drawing=0;
	uint8_t on_KeyEvent = 0;
	uint8_t off_KeyEvent = 0;
	uint16_t ENCODER_CNT = 0;
	vector<uint8_t> vKEY_BUFFER;
	uint16_t buzzer_cnt;
    STM32_IO();
    virtual ~STM32_IO();
    void STM32_IO_init();
    void BEEP_ON(void);
    void BEEP_OFF(void);

    void kpad_init(void);
    void Get_AdcData();
    void Push_Key();
    uint8_t Get_Key_Buffer();
    //void RelayEventVector_Process();
    //void PutRelayEventVector(RELAY relay, uint8_t on, uint16_t count);

////++
    void Clear_Key_Buffer();
    void SYSTEM_MENU(void);
    void SYSTEM_SELJUNG(void);
    void LEFT_RIGHT_KeyProcess(uint8_t left, uint8_t plus);
    void UPDN_KeyProcess(uint8_t up, uint8_t plus);
////--
    void FACTORY_INIT();
    void Main_IRQ_KEY_Process();
};

#endif
