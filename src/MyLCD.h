// MyLCD.h

#include <mbed.h>
#include <lvgl.h>
#include <TS_DISCO_F469NI.h>
#include <LCD_DISCO_F469NI.h>
#include "Dht11.h"
class MyLCD {

private:
    //------------Setting up MCU Components(LCD, PINS, Toutchscreen...)---------------------------------
    static LCD_DISCO_F469NI lcd;
    static TS_DISCO_F469NI ts;
    static TS_StateTypeDef tsState;
    Ticker ticker;
    static DigitalOut light1; 
    static DigitalOut light2; 
    static DigitalOut light3; 
    static DigitalOut temperatureHeater;
    static DigitalOut motorDir1;
    static DigitalOut motorDir2;
    static DigitalIn curtainsensor1;
    static DigitalIn curtainsensor2;
    static PwmOut pwm;
    static DigitalOut lock1;
    static DigitalOut lock2;
    static DigitalOut lock3;
public:
//---------------------------------------------Static Functions------------------------------------------------------------------------
    //--------------------------------------System Functions----------------------------------------------
    static void ex_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);
    static void ex_disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);
    static void ex_disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2,  lv_color_t color);
    static bool ex_tp_read(lv_indev_data_t *data);
    static lv_res_t ddlist_action(lv_obj_t * ddlist);
    //-----------------------------------Temperature Functions-------------------------------------------
    static lv_res_t btn_rel_actionTemperature(lv_obj_t * btn);
    static lv_res_t lv_slider_action_temperature (lv_obj_t * slider);

    static void temperature_monitor(void*p);
    static void tempSliderTemp(void*p);
    static void tempSensor1Task(void*p);
    static void tempSensor2Task(void*p);
    static void tempSensor3Task(void*p);
    static void tempAverageTask(void*p);
    //---------------------------------------Light Functions---------------------------------------------
    static lv_res_t btn_rel_actionLight(lv_obj_t * btn);
    static lv_res_t sw_light_1_action(lv_obj_t * btn);
    static lv_res_t sw_light_2_action(lv_obj_t * btn);
    static lv_res_t sw_light_3_action(lv_obj_t * btn);
    static lv_res_t lv_slider_action_Light1 (lv_obj_t * sliderLight1);
    static lv_res_t lv_slider_action_Light2 (lv_obj_t * sliderLight2);
    static lv_res_t lv_slider_action_Light3 (lv_obj_t * sliderLight3);

    static void light_monitor(void*p);
    static void light_menu_monitor(void*p);
    static void light_sensitivity(void*p);
    //----------------------------------------Lock Functions---------------------------------------------
    static lv_res_t btn_rel_actionLock(lv_obj_t * btn);
    static lv_res_t sw_lock_1_action(lv_obj_t * btn);
    static lv_res_t sw_lock_2_action(lv_obj_t * btn);
    static lv_res_t sw_lock_3_action(lv_obj_t * btn);

    static void lockStateTask(void*p);
    //-------------------------------------Curtain Functions---------------------------------------------
    static lv_res_t btn_rel_actionCurtain(lv_obj_t * btn);
    static lv_res_t btn_rel_actionOpenCurtain(lv_obj_t * btn);
    static lv_res_t btn_rel_actionCloseCurtain(lv_obj_t * btn);


    static void curtainStateTask(void*p);
    //--------------------------------Secondary Menu Functions-------------------------------------------
    static lv_res_t btn_rel_actionMainmenu(lv_obj_t * btn);

    
    //--------------------------------non-static functions-----------------------------------------------
    MyLCD();
    void run();
    void lv_final_project_objects(void);
    // Initialize a Ticker for 1 ms period and in its interrupt call lv_tick_inc(1);
    void every1ms() {
        lv_tick_inc(1);
    }

}
;
