// MyLCD.cpp
#include "MyLCD.h"
#include <BufferedSerial.h>
#include "rtos.h"
// ---------------initialise static variables-----------------

//------------Temperature---------
int dh11_1_Temp;
int dh11_2_Temp;
int dh11_3_Temp;
int dh11_1_Humi;
int dh11_2_Humi;
int dh11_3_Humi;
int averageTemp;
int averageHumi;
int sliderTempValue=20;
int sliderRawLightValue1=5;
float sliderLightValue1; 
int sliderRawLightValue2=5;
float sliderLightValue2; 
int sliderRawLightValue3=5;
float sliderLightValue3; 
char menuTemperature[30];
char targeTempBuffer[65];
char tempSensor1Label[56];
char tempSensor2Label[56];
char tempSensor3Label[56];
char avgTempBuffer [35];

//----------Light--------------
bool swLightState1;
bool swLightState2;
bool swLightState3;
char lightSensor1 [30];
char lightSensor2 [30];
char lightSensor3 [30];
char lightMasterRoom1[10];
char lightMasterSensor2[10];
char lightMasterSensor3[10];
char menuLight[30];

//------------Lock------------
bool swLockState1=false;
bool swLockState2=false;
bool swLockState3=false;
char lockMasterLock1[10];
char lockMasterLock2[10];
char lockMasterLock3[10];
char menuLocks[30];

//---------Curtain----------
char curtainState[10];
char menuCurtain[30];

//----------------------initialise static objects--------------

//------------Temperature---------
static lv_obj_t* tempLabelButton;
static lv_obj_t *targetTemp;
static lv_obj_t *avgTemp;
static lv_obj_t *temperatureSensor1;
static lv_obj_t *temperatureSensor2;
static lv_obj_t *temperatureSensor3;

//----------Light--------------
static lv_obj_t *lightLabelLigh1;
static lv_obj_t *lightLabelLigh2;
static lv_obj_t *lightLabelLigh3;
static lv_obj_t *lightLabelButton;

//------------Lock------------
static lv_obj_t *lockLabelButton;

//---------Curtain----------
static lv_obj_t *curtainStateLabel;


//initialise static objects MyLCD.h -  Organized by PIN numbers
DigitalOut MyLCD::light1(D0);
DigitalOut MyLCD::light2(D1);
DigitalOut MyLCD::light3(D2);
DigitalOut MyLCD::temperatureHeater(D3);
Dht11 dh11sensor1(D4);
Dht11 dh11sensor2(D5);
Dht11 dh11sensor3(D6);
DigitalOut MyLCD::motorDir1(D7);
DigitalOut MyLCD::motorDir2(D8);
DigitalIn MyLCD::curtainsensor1(D9);
DigitalIn MyLCD::curtainsensor2(D10);
DigitalOut MyLCD::lock1(D11);
DigitalOut MyLCD::lock2(D12);
DigitalOut MyLCD::lock3(D13);
PwmOut MyLCD:: pwm(A0);
AnalogIn LDRsensor1(A1);
AnalogIn LDRsensor2(A2);
AnalogIn LDRsensor3(A3);
LCD_DISCO_F469NI MyLCD::lcd;
TS_DISCO_F469NI MyLCD::ts;
TS_StateTypeDef MyLCD::tsState;
BufferedSerial pc(USBTX, USBRX);
MyLCD::MyLCD(){};

//class to run
void MyLCD::run() {
//iniciallise LEDs as off
light1 = false;
light2 = false;
light3 = false;
//iniciallise temperature heater as off
temperatureHeater = 0;
//setting pwm for motor speed
pwm.period_ms(10);
pwm=0.4;
//First page screen
lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Room Conditionning Project", CENTER_MODE);
wait_us(100000);
uint8_t status = ts.Init(lcd.GetXSize(), lcd.GetYSize());

//checking and displaying the statud of the board and toutchscreen
if (status != TS_OK) {
lcd.Clear(LCD_COLOR_RED);
lcd.SetBackColor(LCD_COLOR_RED);
lcd.SetTextColor(LCD_COLOR_WHITE);
lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN INIT FAIL", CENTER_MODE);
}
else {
lcd.Clear(LCD_COLOR_GREEN);
lcd.SetBackColor(LCD_COLOR_GREEN);
lcd.SetTextColor(LCD_COLOR_WHITE);
lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN INIT OK", CENTER_MODE);
}
wait_us(100000);

//iniciallise lvgl 
lv_init();

//setting ticket
ticker.attach(callback(this, &MyLCD::every1ms), 0.0001f); // 1ms forever

// TFT LCD setup with LvGL
lv_disp_drv_t disp_drv;
lv_disp_drv_init(&disp_drv);
//Set up the functions to access to your display
disp_drv.disp_flush = ex_disp_flush;            /*Used in buffered mode (LV_VDB_SIZE != 0  in lv_conf.h)*/
disp_drv.disp_fill = ex_disp_fill;              /*Used in unbuffered mode (LV_VDB_SIZE == 0  in lv_conf.h)*/
disp_drv.disp_map = ex_disp_map;                /*Used in unbuffered mode (LV_VDB_SIZE == 0  in lv_conf.h)*/
lv_disp_drv_register(&disp_drv);                /*Finally register the driver*/

// Touchpad setup with LvGL
lv_indev_drv_t indev_drv;                       /*Descriptor of an input device driver*/
lv_indev_drv_init(&indev_drv);                  /*Basic initialization*/
indev_drv.type = LV_INDEV_TYPE_POINTER;         /*The touchpad is pointer type device*/
indev_drv.read = ex_tp_read;                    /*Library ready your touchpad via this function*/
lv_indev_drv_register(&indev_drv);              /*Finally register the driver*/

// Draw a necessary components on the GUI
lv_final_project_objects();

//set the client handler and task handler every 5 seconds
while (1) {
lv_task_handler();
wait_us(5000); // 5ms
}
}

// main menu temperature task label in order to update values
void tempTask(void* p){
sprintf(menuTemperature,SYMBOL_WARNING" Temperature\nAvg Temp: %d°C\nTarget temp: %d°C", averageTemp, sliderTempValue );
lv_label_set_text(tempLabelButton, menuTemperature);
}

// slider action for the target temperature by the user
lv_res_t MyLCD:: lv_slider_action_temperature(lv_obj_t * slider){
  sliderTempValue = lv_slider_get_value(slider);
  return LV_RES_OK;
}

//main temperature control monitor / curtain motor start/stop
void MyLCD:: temperature_monitor(void*p){
    //checking if curtain is closed and direction is closing and turn off motor
    if(curtainsensor2 == 1 && motorDir2 == 0){
      motorDir1 = 1;
      motorDir2 = 1;
    }
  //checking if curtain is openned and direction is openning and turn off motor
    if(curtainsensor1==1 && motorDir1 == 0){
      motorDir1 = 1;
      motorDir2 = 1;
    }

    //reading and storing temperature values
    dh11sensor1.read();
    dh11sensor2.read();
    dh11sensor3.read();
    dh11_1_Temp = dh11sensor1.getCelsius();
    dh11_2_Temp = dh11sensor2.getCelsius();
    dh11_3_Temp = dh11sensor3.getCelsius();

    dh11_1_Humi = dh11sensor1.getHumidity();
    dh11_2_Humi = dh11sensor2.getHumidity();
    dh11_3_Humi = dh11sensor3.getHumidity();

    //averaging the temperature and humidity
    averageTemp = (dh11_1_Temp + dh11_2_Temp + dh11_3_Temp)/3;
    averageHumi =(dh11_1_Humi + dh11_2_Humi + dh11_3_Humi)/3;

    //checking if the average temperature is lower than the targer temperature and turning on the heater
    if(averageTemp < sliderTempValue){
       temperatureHeater = true;
    }
    else{
      temperatureHeater = false;
    }
}

//temperature submenu labels
void MyLCD:: tempSensor1Task(void*p){
    sprintf(tempSensor1Label,"Sensor 1:   Humidity: %d %   Temperature: %d °C",dh11sensor1.getHumidity(), dh11sensor1.getCelsius());
    lv_label_set_text (temperatureSensor1, tempSensor1Label);  /*Set the text*/
    
}
void MyLCD:: tempSensor2Task(void*p){
sprintf(tempSensor2Label,"Sensor 2:   Humidity: %d %   Temperature: %d °C",dh11_2_Humi, dh11_2_Temp);
    lv_label_set_text (temperatureSensor2, tempSensor2Label);  /*Set the text*/
}

void MyLCD:: tempSensor3Task(void*p){
    sprintf(tempSensor3Label,"Sensor 3:   Humidity: %d %   Temperature: %d °C",dh11_3_Humi, dh11_3_Temp);
    lv_label_set_text (temperatureSensor3, tempSensor3Label);  /*Set the text*/
}
void MyLCD:: tempAverageTask(void*p){
  sprintf(avgTempBuffer,"Average Humidity: %d % | Average Temperature is: %d °C",averageHumi, averageTemp);
  lv_label_set_text (avgTemp, avgTempBuffer);  /*Set the text*/
}

void MyLCD:: tempSliderTemp(void*p){
  sprintf(targeTempBuffer,"The target temperature from the slider is: %d °C", sliderTempValue);
  lv_label_set_text (targetTemp, targeTempBuffer);  /*Set the text*/
 
}

//light sliders sensitivity
lv_res_t MyLCD:: lv_slider_action_Light1(lv_obj_t * sliderLightRoom1){
  sliderRawLightValue1 = lv_slider_get_value(sliderLightRoom1);
  return LV_RES_OK;
}
lv_res_t MyLCD:: lv_slider_action_Light2(lv_obj_t * sliderLightRoom2){
  sliderRawLightValue2 = lv_slider_get_value(sliderLightRoom2);
  return LV_RES_OK;
}
lv_res_t MyLCD:: lv_slider_action_Light3(lv_obj_t * sliderLightRoom3){
  sliderRawLightValue3 = lv_slider_get_value(sliderLightRoom3);
  return LV_RES_OK;
}

//light labels function
void MyLCD:: light_menu_monitor(void*p){
  if(light1 ==true){
    sprintf(lightMasterRoom1,"On");
  }
  else{
    sprintf(lightMasterRoom1,"Off");
  }
  if(light2==true){
  sprintf(lightMasterSensor2,"On");
    }
  else{
  sprintf(lightMasterSensor2,"Off");
  }
  if(light3==true){
  sprintf(lightMasterSensor3,"On");
  }
  else{
  sprintf(lightMasterSensor3,"Off");
  }
sprintf(menuLight,SYMBOL_CHARGE" Light\nLight 1: %s\nLight 2: %s\nLight 3: %s", lightMasterRoom1,lightMasterSensor2,lightMasterSensor3);
lv_label_set_text(lightLabelButton,menuLight);
}

//main light control task
void MyLCD:: light_monitor(void*p){

    //read light values 
    LDRsensor1.read();
    //set the value to decimals in order to display them
    sliderLightValue1 = (sliderRawLightValue1*0.1);
    LDRsensor2.read();
    sliderLightValue2 = (sliderRawLightValue2*0.1);
    LDRsensor3.read();
    sliderLightValue3 = (sliderRawLightValue3*0.1);
    
    //checking if the value is lower than the one set by the user or if the switch is on - turning the led on/off
    if(LDRsensor1.read() < sliderLightValue1 || swLightState1 ==true ){
      light1=true;
    }
    else{
      light1 = false;
    }

    if(LDRsensor2.read() < sliderLightValue2 || swLightState2 ==true ){
      light2=true;
    }
    else{
      light2 = false;
   }
    if(LDRsensor3.read() < sliderLightValue3 || swLightState3 ==true ){
      light3=true;
    }
    else{
      light3 = false;
    }

}

//setting the light labels
void MyLCD:: light_sensitivity(void*p){
    sprintf(lightSensor1,"Light 1 (%d) -", sliderRawLightValue1);
    lv_label_set_text(lightLabelLigh1, lightSensor1);  /*Set the text*/
    sprintf(lightSensor2,"Light 2 (%d) -", sliderRawLightValue2);
    lv_label_set_text(lightLabelLigh2,lightSensor2);  /*Set the text*/
    sprintf(lightSensor3,"Light 3 (%d) -", sliderRawLightValue3);
    lv_label_set_text(lightLabelLigh3, lightSensor3);  /*Set the text*/
}

//Lock labels submenu
void::MyLCD:: lockStateTask(void*p){
//conditions for the locks
if(swLockState1 == true){
  sprintf(lockMasterLock1,"Closed");
  }
else{
  sprintf(lockMasterLock1,"Open");
  }
if(swLockState2  == true){
  sprintf(lockMasterLock2,"Closed");
  }
else{
 sprintf(lockMasterLock2,"Open");
  }
if(swLockState3  == true){
  sprintf(lockMasterLock3,"Closed");
  }
else{
  sprintf(lockMasterLock3,"Open");
}
sprintf(menuLocks,SYMBOL_HOME" Locks\nLock 1: %s\nLock 2: %s\nLock 3: %s", lockMasterLock1 ,lockMasterLock2, lockMasterLock3);
lv_label_set_text(lockLabelButton,menuLocks);
}


//checking possible Curtain states
void MyLCD:: curtainStateTask(void*p){    
if(curtainsensor1 ==1){
sprintf(curtainState,"Open");
}
else if (curtainsensor2 ==1){
sprintf(curtainState,"Closed");
}
else{
sprintf(curtainState,"Operation");
}
sprintf(menuCurtain,SYMBOL_IMAGE" Curtain\nCurtain is: %s",curtainState);
lv_label_set_text(curtainStateLabel,menuCurtain);
}

//First menu display
void MyLCD::lv_final_project_objects(void)
{
//-----------------------------------------SETTING STYLES----------------------------------------
static lv_style_t screenstyle; //creating screenstyle
lv_style_copy(&screenstyle, &lv_style_plain);//Copy a built-in style to initialize the new style
screenstyle.body.main_color = LV_COLOR_GRAY; //Main body color to Gray
screenstyle.body.radius = 0; //body radius to 0
screenstyle.body.border.color = LV_COLOR_BLACK; // border colour to black
screenstyle.body.border.width = 2; //border width to 2
screenstyle.body.border.opa = LV_OPA_COVER; // border opacity to cover 
screenstyle.body.padding.hor = 15;  //Horizontal padding, used by the bar indicator below
screenstyle.body.padding.ver = 10;  //Vertical padding, used by the bar indicator below
screenstyle.text.color = LV_COLOR_GRAY; // text colour to gray
screenstyle.text.font = &lv_font_dejavu_40; //text font to dejavu with size 40

static lv_style_t buttonstyle;  //creating button style
lv_style_copy(&buttonstyle, &lv_style_plain);   //Copy a built-in style to initialize the new style
buttonstyle.body.main_color = LV_COLOR_SILVER;  // main color to silver
buttonstyle.body.border.color = LV_COLOR_GRAY;  //border color to gray
buttonstyle.body.padding.hor = 0;  //horizontal padding
buttonstyle.body.padding.ver = 0;   //vertical padding
buttonstyle.body.padding.inner = 0; //inner padding 
buttonstyle.text.color = LV_COLOR_BLACK; //black text
buttonstyle.text.font = &lv_font_dejavu_40;//text font to dejavu with size 40
//-------------------------------------------------------------------------------------------------


//creating scr as main object parent
lv_obj_t *scr = lv_page_create(NULL, NULL);
lv_obj_set_style(scr, &screenstyle); 
lv_scr_load(scr);

//creating previous tasks for main temperature and light control
lv_task_create(temperature_monitor, 500, LV_TASK_PRIO_MID, NULL);
lv_task_create(light_monitor, 500, LV_TASK_PRIO_MID, NULL);


lv_obj_t *mainMenuLabel = lv_label_create(scr, NULL);  //first label in scr parent
lv_label_set_text(mainMenuLabel, "Choose from the available options: ");  //Set the text
lv_obj_set_style(mainMenuLabel, &screenstyle);  //Set style
lv_obj_align(mainMenuLabel, NULL, LV_ALIGN_IN_TOP_MID, 0, 20); //set the x and y coordinates


//---------------------------------------------------------BUTTONS CREATION------------------------------------------------------------
//create tempTask to set temperature label
lv_task_create(tempTask, 500, LV_TASK_PRIO_MID,NULL);

/*-------------------------------------------------Create Temperature button--------------------------------------------------------*/
static lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);  //Create a button on the currently loaded screen
lv_obj_set_style(btn1, &buttonstyle); //setting button with buttonstyle
lv_btnm_set_style(btn1, LV_BTNM_STYLE_BTN_TGL_PR, &buttonstyle);  //setting button with buttonstyle
lv_btn_set_action(btn1, LV_BTN_ACTION_CLICK, btn_rel_actionTemperature); //Set function to be called when the button is clicked*/
lv_obj_align(btn1, mainMenuLabel, LV_ALIGN_OUT_BOTTOM_LEFT, -70, 40);  //Align the button
//increasing button sizes
//Increase the button width - could also do it like this: lv_obj_set_size(btn2, 200, 100);Button size
lv_coord_t width = lv_obj_get_width(btn1);
lv_obj_set_width(btn1, width + 230);
lv_coord_t height = lv_obj_get_height(btn1);
lv_obj_set_height(btn1, height + 100);
tempLabelButton = lv_label_create(btn1, NULL);//creating and setting first temperature button label

/*-------------------------------------------------Create Light button--------------------------------------------------------*/
lv_task_create(light_menu_monitor,500,LV_TASK_PRIO_MID,NULL);//task for light labels
lv_obj_t *btn2 = lv_btn_create(lv_scr_act(), NULL); //Create a button on the currently loaded screen
lv_obj_set_style(btn2, &buttonstyle); 
lv_btnm_set_style(btn2, LV_BTNM_STYLE_BTN_TGL_PR, &buttonstyle);//Set function to be called when the button is togled and pressed
lv_btn_set_action(btn2, LV_BTN_ACTION_CLICK, btn_rel_actionLight);//Set function to be called when the button is released
lv_obj_align(btn2, btn1, LV_ALIGN_OUT_RIGHT_MID, 25, -50);   //Align next to the prev. button.*/
lv_obj_set_width(btn2, width + 230); //setting width
lv_obj_set_height(btn2, height + 100); //setting height
//lv_obj_set_size(btn2, 200, 100); //Button size
lv_obj_set_style(btn2, &buttonstyle);  //setting style
lightLabelButton = lv_label_create(btn2, NULL);

/*-------------------------------------------------Create Lock button--------------------------------------------------------*/
lv_task_create(lockStateTask, 500, LV_TASK_PRIO_MID, NULL);
lv_obj_t *btn3 = lv_btn_create(lv_scr_act(), NULL);//Create a button on the currently loaded screen
lv_obj_set_style(btn3, &buttonstyle);  //setting style
lv_btnm_set_style(btn3, LV_BTNM_STYLE_BTN_TGL_PR, &buttonstyle);//setting style
lv_btn_set_action(btn3, LV_BTN_ACTION_CLICK, btn_rel_actionLock); //Set function to be called when the button is released clicked
lv_obj_align(btn3, btn1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20); //Align next to the prev. button.
/*Increase the button width*/
lv_obj_set_width(btn3, width + 230);
lv_obj_set_height(btn3, height + 100);
/*Create a label on the button*/
lockLabelButton = lv_label_create(btn3, NULL);


/*-------------------------------------------------Create Curtain button--------------------------------------------------------*/
lv_obj_t *btn4 = lv_btn_create(lv_scr_act(), NULL); //Create a button on the currently loaded screen
lv_obj_set_style(btn4, &buttonstyle); //setting style
lv_btnm_set_style(btn4, LV_BTNM_STYLE_BTN_TGL_PR, &buttonstyle);//setting style
lv_btn_set_action(btn4, LV_BTN_ACTION_CLICK, btn_rel_actionCurtain); //Set function to be called when the button is clicked
lv_obj_align(btn4, btn3, LV_ALIGN_OUT_RIGHT_MID, 26, -50);     /*Align next to the prev. button.*/
/*Increase the button width*/
lv_obj_set_width(btn4, width + 230);
lv_obj_set_height(btn4, height + 100);
//creates label in curtainStateLabel 
curtainStateLabel = lv_label_create(btn4, NULL);
//starts the task for curtain labels
lv_task_create(curtainStateTask,500,LV_TASK_PRIO_LOW,NULL);
}





/*-------------------------------------------------Temperature Button Submenu--------------------------------------------------------*/
lv_res_t MyLCD::btn_rel_actionTemperature(lv_obj_t * btn)
{
  //create new Temperature Menu

  //------------------------------Style Parameters------------------------------------------------
  static lv_style_t style1; //set style
  lv_style_copy(&style1, &lv_style_plain);  
  style1.body.main_color = LV_COLOR_BLACK;
  style1.body.grad_color = LV_COLOR_RED;
  style1.body.radius = 0;
  style1.body.border.color = LV_COLOR_BLACK;
  style1.body.border.width = 2;
  style1.body.border.opa = LV_OPA_COVER;
  style1.body.padding.hor = 10;           
  style1.body.padding.ver = 15;           
  style1.text.color = LV_COLOR_BLACK;
  style1.text.font = &lv_font_dejavu_30;

  static lv_style_t buttonstyle;
  lv_style_copy(&buttonstyle, &lv_style_plain);    
  buttonstyle.body.main_color = LV_COLOR_SILVER;
  buttonstyle.body.border.color = LV_COLOR_GRAY;
  buttonstyle.body.padding.hor = 0;
  buttonstyle.body.padding.ver = 0;
  buttonstyle.body.padding.inner = 0;
  buttonstyle.text.color = LV_COLOR_BLACK;
  buttonstyle.text.font = &lv_font_dejavu_40;
  //--------------------------------------------------------------------------------------------

    //Creating Temperature Screen
    lv_obj_t *tempscreen = lv_page_create(NULL, NULL);
    lv_obj_set_style(tempscreen, &style1); 
    lv_scr_load(tempscreen);
    lv_obj_t *lightscreenLabel = lv_label_create(tempscreen, NULL);  
   
    //Go back button
    lv_obj_t *backbtn = lv_btn_create(lv_scr_act(), NULL);/*Create a button on the currently loaded screen*/
    lv_obj_set_style(backbtn, &buttonstyle); //sets style
    lv_obj_align(backbtn, lightscreenLabel, LV_ALIGN_CENTER, 30, 0);  /*Align below the label*/
    lv_obj_t * backButtonLabel = lv_label_create(backbtn, NULL); /*Create a label on the button*/
    lv_btn_set_action(backbtn, LV_BTN_ACTION_PR, btn_rel_actionMainmenu); /*Set function to be called when the button is pressed*/
    lv_obj_set_size(backbtn, 230, 100); //sets size
    lv_label_set_text(backButtonLabel, SYMBOL_LEFT" Back"); /*Set the text of the label*/

    //Refresh button (reloads the page)
    lv_obj_t *updateValues = lv_btn_create(lv_scr_act(), NULL);/*Create a button on the currently loaded screen*/
    lv_obj_set_style(updateValues, &buttonstyle); //sets style
    lv_obj_align(updateValues, backbtn, LV_ALIGN_IN_TOP_MID, 460, 0);  /*Align below the button*/
    lv_obj_t * refreshlabel = lv_label_create(updateValues, NULL); /*Create a label on the first button*/
    lv_btn_set_action(updateValues, LV_BTN_ACTION_PR, btn_rel_actionTemperature); /*Set function to be called when the button is pressed*/
    lv_obj_set_size(updateValues, 230, 100);//sets size
    lv_label_set_text(refreshlabel, SYMBOL_REFRESH" Refresh"); /*Set the text of the label*/

    //starts task for the sensor 1 label
    lv_task_create(tempSensor1Task,500,LV_TASK_PRIO_MID,NULL);
    temperatureSensor1 = lv_label_create(tempscreen, NULL);  
    lv_obj_align(temperatureSensor1, backbtn, LV_ALIGN_IN_BOTTOM_LEFT, 10, 40); 
    lv_obj_set_style(temperatureSensor1, &style1); 

    //starts task for the sensor 2 label
    lv_task_create(tempSensor2Task,500,LV_TASK_PRIO_MID,NULL);
    temperatureSensor2= lv_label_create(tempscreen, NULL); 
    lv_obj_align(temperatureSensor2, temperatureSensor1, LV_ALIGN_IN_BOTTOM_MID, -7, 35);  
    lv_obj_set_style(temperatureSensor2, &style1); 
    
    //starts task for the sensor 3 label
    lv_task_create(tempSensor3Task,500,LV_TASK_PRIO_MID,NULL);
    temperatureSensor3= lv_label_create(tempscreen, NULL);  /*First parameters (scr) is the parent*/
    lv_obj_align(temperatureSensor3, temperatureSensor1, LV_ALIGN_IN_BOTTOM_MID, -7, 80);  /*Align below the label*/
    lv_obj_set_style(temperatureSensor3, &style1); 
    
    //start task for average Temperature label
    lv_task_create(tempAverageTask, 500,LV_TASK_PRIO_MID,NULL);
    avgTemp= lv_label_create(tempscreen, NULL);  
    lv_obj_align(avgTemp, temperatureSensor1, LV_ALIGN_IN_BOTTOM_MID,-5, 130); 
    lv_obj_set_style(avgTemp, &style1); 


    //--------This method also works----------------
    //int humidity =sensor.getHumidity();
    //string humString = to_string(humidity); 
    //char const* humidConstChar = humString.c_str();
    //----------------------------------------------

    //start task for targer slider value
    lv_task_create(tempSliderTemp,500,LV_TASK_PRIO_MID,NULL);
    // Slider
    lv_obj_t *slider = lv_slider_create(tempscreen, NULL); /*Create a slider*/
    lv_obj_align(slider, avgTemp, LV_ALIGN_IN_BOTTOM_MID, 250, 110);  /*Align below the label*/
    lv_obj_set_size(slider, 380,50); //set size
    lv_slider_set_range(slider, 15, 25); // set range
    lv_slider_set_style(slider,LV_SLIDER_STYLE_INDIC,&style1); //set style
    lv_slider_set_action(slider,lv_slider_action_temperature); //set to be called when action happens before task
    lv_slider_set_value(slider, sliderTempValue); //set the latest value
    targetTemp= lv_label_create(tempscreen, NULL);  //set label on the screen
    lv_obj_align(targetTemp, avgTemp, LV_ALIGN_IN_BOTTOM_MID, -7, 40);  /*Align below the label*/
    lv_obj_set_style(targetTemp, &style1); //set styke
    lv_slider_set_action(slider,lv_slider_action_temperature); //set to be called when action happens after task
    return LV_RES_OK;
}





/*-------------------------------------------------Light Button--------------------------------------------------------*/
lv_res_t MyLCD::btn_rel_actionLight(lv_obj_t * btn)
{
//------------------------------Style Parameters------------------------------------------------
  static lv_style_t style1;
  lv_style_copy(&style1, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
  style1.body.main_color = LV_COLOR_BLACK;
  style1.body.grad_color = LV_COLOR_RED;
  style1.body.radius = 0;
  style1.body.border.color = LV_COLOR_BLACK;
  style1.body.border.width = 2;
  style1.body.border.opa = LV_OPA_COVER;
  style1.body.padding.hor = 10;            /*Horizontal padding, used by the bar indicator below*/
  style1.body.padding.ver = 15;            /*Vertical padding, used by the bar indicator below*/
  style1.text.color = LV_COLOR_BLACK;
  style1.text.font = &lv_font_dejavu_40;

  static lv_style_t buttonstyle;
  lv_style_copy(&buttonstyle, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
  buttonstyle.body.main_color = LV_COLOR_SILVER;
  buttonstyle.body.border.color = LV_COLOR_GRAY;
  buttonstyle.body.padding.hor = 0;
  buttonstyle.body.padding.ver = 0;
  buttonstyle.body.padding.inner = 0;
  buttonstyle.text.color = LV_COLOR_BLACK;
  buttonstyle.text.font = &lv_font_dejavu_40;

  static lv_style_t sensorLabelStyle;
  lv_style_copy(&sensorLabelStyle, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
  sensorLabelStyle.body.main_color = LV_COLOR_SILVER;
  sensorLabelStyle.body.border.color = LV_COLOR_GRAY;
  sensorLabelStyle.body.padding.hor = 0;
  sensorLabelStyle.body.padding.ver = 0;
  sensorLabelStyle.body.padding.inner = 0;
  sensorLabelStyle.text.color = LV_COLOR_BLACK;
  sensorLabelStyle.text.font = &lv_font_dejavu_30;

  static lv_style_t sw_style;
  lv_style_copy(&sw_style, &lv_style_pretty);
  sw_style.body.radius = LV_RADIUS_CIRCLE;
  sw_style.body.main_color = LV_COLOR_BLACK;

  //---------------------------------------------------------------------------------------------------
    //back button settup
    lv_obj_t *lightscreen = lv_page_create(NULL, NULL);  //create light screen
    lv_obj_set_style(lightscreen, &style1);              //set style
    lv_scr_load(lightscreen);                            //lead screen
    lv_obj_t *lightscreenLabel = lv_label_create(lightscreen, NULL);//create label

    //Go back button
    lv_obj_t *backbtn = lv_btn_create(lv_scr_act(), NULL);  //Create a button on the currently loaded screen
    lv_obj_set_style(backbtn, &buttonstyle);                //set style
    lv_obj_align(backbtn, lightscreenLabel, LV_ALIGN_CENTER, 30, 0);  //Align below the label
    lv_obj_t * backButtonLabel = lv_label_create(backbtn, NULL); /*Create a label button*/
    lv_btn_set_action(backbtn, LV_BTN_ACTION_PR, btn_rel_actionMainmenu); /*Set function to be called when the button is pressed*/
    lv_obj_set_size(backbtn, 230, 100);                     //set size
    lv_label_set_text(backButtonLabel, SYMBOL_LEFT" Back"); /*Set the text of the label*/

    //Update values button
    lv_obj_t *updateValues = lv_btn_create(lv_scr_act(), NULL);  //Create a object on the currently loaded screen*/
    lv_obj_set_style(updateValues, &buttonstyle);                 //set style
    lv_obj_align(updateValues, backbtn, LV_ALIGN_IN_TOP_MID, 460, 0);  /*Align below the label*/
    lv_obj_t * refreshLabel = lv_label_create(updateValues, NULL); //Create a label button*/
    lv_btn_set_action(updateValues, LV_BTN_ACTION_PR, btn_rel_actionLight); /*Set function to be called when the button is pressed*/
    lv_obj_set_size(updateValues, 230, 100);                    //set size
    lv_label_set_text(refreshLabel, SYMBOL_REFRESH" Refresh"); //Set the text of the label*/

    //switches label
    lv_obj_t *switchLabel = lv_label_create(lightscreen, NULL);   //Create a object on the currently loaded screen*/
    lv_obj_align(switchLabel, updateValues, LV_ALIGN_IN_BOTTOM_MID, -400, 46);    //Align below the button
    lv_obj_set_style(switchLabel, &sensorLabelStyle);                 //set style
    lv_label_set_text (switchLabel, "Light Switches | Sensors Sensitivity(0-10)"); //put text on the label

    lv_task_create(light_sensitivity,500,LV_TASK_PRIO_MID,NULL);  //starts task for lights sensitivity
    

    //Switch Room Labels
    lightLabelLigh1 = lv_label_create(lightscreen, NULL);  //create room label
    lv_obj_align(lightLabelLigh1, backbtn, LV_ALIGN_OUT_BOTTOM_LEFT, 15, 75);   //Align next to the prev. button
    lv_obj_set_style(lightLabelLigh1, &buttonstyle); //set style
    
    //SWITCH light 1
    lv_obj_t *sw1 = lv_sw_create(lv_scr_act(), NULL); //create switch
    lv_sw_set_style(sw1, LV_SW_STYLE_INDIC, &sw_style); //set style
    lv_obj_align(sw1, lightLabelLigh1, LV_ALIGN_OUT_RIGHT_MID, 200, 0);  //Align next to the prev. button
    lv_obj_set_size(sw1, 100,40);   //set style

    // set condition for switch 
    if(swLightState1==true){
      lv_sw_on(sw1);
    }
    else{
      lv_sw_off(sw1);
    }


    // Slider light 1
    lv_obj_t *sliderLightRoom1 = lv_slider_create(lightscreen, NULL);  /*Create a slider*/
    lv_obj_align(sliderLightRoom1, sw1, LV_ALIGN_IN_BOTTOM_RIGHT, 240, -10);  /*Align wihth the swl*/
    lv_obj_set_size(sliderLightRoom1, 300,50); //set size
    lv_slider_set_range(sliderLightRoom1, 0, 10); //set range 0 - 10
    lv_slider_set_style(sliderLightRoom1,LV_SLIDER_STYLE_INDIC,&style1);  //set style
    lv_slider_set_value(sliderLightRoom1, sliderRawLightValue1); //set value in the slider


    //Switch Room Labels
    lightLabelLigh2 = lv_label_create(lightscreen, NULL);   //create room label
    lv_obj_align(lightLabelLigh2, lightLabelLigh1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 35); //Align next to the prev. label
    lv_obj_set_style(lightLabelLigh2, &buttonstyle); //set style
    
    //Switch light 2
    lv_obj_t *sw2 = lv_sw_create(lv_scr_act(), NULL);  //create sw2
    lv_sw_set_style(sw2, LV_SW_STYLE_INDIC, &sw_style);//set style
    lv_obj_align(sw2, sw1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);   //Align next to the prev. switch
    lv_obj_set_size(sw2, 100,40); //set object size

    // set condition for switch 
    if(swLightState2==true){
      lv_sw_on(sw2);
    }
    else{
      lv_sw_off(sw2);
    }

    // Slider light 2
    lv_obj_t *sliderLightRoom2 = lv_slider_create(lightscreen, NULL);  /*Create a slider*/
    lv_obj_align(sliderLightRoom2, sw2, LV_ALIGN_IN_BOTTOM_RIGHT, 240, -10); /*Align wihth the sw2*/
    lv_obj_set_size(sliderLightRoom2, 300,50); //set size
    lv_slider_set_range(sliderLightRoom2, 0, 10); //range for 0 - 10
    lv_slider_set_style(sliderLightRoom2,LV_SLIDER_STYLE_INDIC,&style1); // set style
    lv_slider_set_value(sliderLightRoom2, sliderRawLightValue2); //set value

    //Switch Room Labels
    lightLabelLigh3 = lv_label_create(lightscreen, NULL);   //create room label
    lv_obj_align(lightLabelLigh3, lightLabelLigh2, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);   //Align next to the prev. label
    lv_obj_set_style(lightLabelLigh3, &buttonstyle); //set style
    
    //Switch light 3
    lv_obj_t *sw3 = lv_sw_create(lv_scr_act(), NULL); //create switch
    lv_sw_set_style(sw3, LV_SW_STYLE_INDIC, &sw_style);//set style
    lv_obj_align(sw3, sw2, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 35);   //Align next to the prev. switch
    lv_obj_set_size(sw3, 100,40); //set switch size

    // set condition for switch 
    if(swLightState3==true){
      lv_sw_on(sw3);
    }
    else{
      lv_sw_off(sw3);
    }
    
    // Slider
    lv_obj_t *sliderLightRoom3 = lv_slider_create(lightscreen, NULL); /*Create a slider*/
    lv_obj_align(sliderLightRoom3, sw3, LV_ALIGN_IN_BOTTOM_RIGHT, 240, -10);  /*Align with sw3*/
    lv_obj_set_size(sliderLightRoom3, 300,50); //set slider size
    lv_slider_set_range(sliderLightRoom3, 0, 10); //set slider size 0 - 10
    lv_slider_set_style(sliderLightRoom3,LV_SLIDER_STYLE_INDIC,&style1);//set style
    lv_slider_set_value(sliderLightRoom3, sliderRawLightValue3); //set slider value

    lv_obj_t *targetTemp= lv_label_create(lightscreen, NULL);  //create target temp label
    lv_obj_align(targetTemp, sliderLightRoom3, LV_ALIGN_IN_BOTTOM_MID, -130, 35);  /*Align with slider light room*/
    lv_obj_set_style(targetTemp, &sensorLabelStyle); //set style
    lv_label_set_text (targetTemp, " Low  |  Medium   |  High");  /*Set the text*/


    lv_slider_set_action(sliderLightRoom1,lv_slider_action_Light1); //set slider 1 action when value changed
    lv_slider_set_action(sliderLightRoom2,lv_slider_action_Light2); //set slider 2 action when value changed
    lv_slider_set_action(sliderLightRoom3,lv_slider_action_Light3); //set slider 3 action when value changed

    lv_sw_set_action(sw1, sw_light_1_action); //set sw 1 action and calls object
    lv_sw_set_action(sw2, sw_light_2_action); //set sw 2 action and calls object
    lv_sw_set_action(sw3, sw_light_3_action); //set sw 3 action and calls object

    return LV_RES_OK;
}

//----------------Switch Light action 1---------------
lv_res_t MyLCD::sw_light_1_action(lv_obj_t * sw1){
  swLightState1 =lv_sw_get_state(sw1);//checking switch state
  //setting light with switch condition
  if(swLightState1==true){
    light1=true;
  }
  else{
    light1=false;
  }
  return LV_RES_OK;
}

//----------------Switch Light action 2---------------
lv_res_t MyLCD::sw_light_2_action(lv_obj_t * sw2){
  swLightState2 =lv_sw_get_state(sw2);
  if(swLightState2==true){
    light2=true;
  }
  else{
    light2=false;
  }   return LV_RES_OK;
}
//----------------Switch Light action 3---------------
lv_res_t MyLCD::sw_light_3_action(lv_obj_t * sw3){
  swLightState3 =lv_sw_get_state(sw3);
  if(swLightState3==true){
    light3=true;
  }
  else{
    light3=false;
  }
  return LV_RES_OK;
}





/*-------------------------------------------------Door Button--------------------------------------------------------*/
lv_res_t MyLCD::btn_rel_actionLock(lv_obj_t * btn)
{
  //------------------------------Style Parameters------------------------------------------------
 static lv_style_t style1;
  lv_style_copy(&style1, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
  style1.body.main_color = LV_COLOR_BLACK;
  style1.body.grad_color = LV_COLOR_RED;
  style1.body.radius = 0;
  style1.body.border.color = LV_COLOR_BLACK;
  style1.body.border.width = 2;
  style1.body.border.opa = LV_OPA_COVER;
  style1.body.padding.hor = 10;            /*Horizontal padding, used by the bar indicator below*/
  style1.body.padding.ver = 15;            /*Vertical padding, used by the bar indicator below*/
  style1.text.color = LV_COLOR_RED;
  style1.text.font = &lv_font_dejavu_40;

  static lv_style_t buttonstyle;
  lv_style_copy(&buttonstyle, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
  buttonstyle.body.main_color = LV_COLOR_SILVER;
  buttonstyle.body.border.color = LV_COLOR_GRAY;
  buttonstyle.body.padding.hor = 0;
  buttonstyle.body.padding.ver = 0;
  buttonstyle.body.padding.inner = 0;
  buttonstyle.text.color = LV_COLOR_BLACK;
  buttonstyle.text.font = &lv_font_dejavu_40;
  static lv_style_t sw_style;
  lv_style_copy(&sw_style, &lv_style_pretty);
  sw_style.body.radius = LV_RADIUS_CIRCLE;
  sw_style.body.main_color = LV_COLOR_BLACK;
  //----------------------------------------------------------------------------------------------------------


    //back button settup
    lv_obj_t *lockScreen = lv_page_create(NULL, NULL);//create Door screen
    lv_obj_set_style(lockScreen, &style1); //set style
    lv_scr_load(lockScreen);//load screen
    lv_obj_t *doorLabel = lv_label_create(lockScreen, NULL);  //create label

    //Go back button
    lv_obj_t *backbtn = lv_btn_create(lockScreen, NULL); //Create a button on the currently loaded screen
    lv_obj_set_style(backbtn, &buttonstyle); //set style
    lv_obj_align(backbtn, doorLabel, LV_ALIGN_CENTER, 30, 0);  /*Align with door label*/
    lv_obj_t * backctnLabel = lv_label_create(backbtn, NULL); /*Create a label on the first button*/
    lv_btn_set_action(backbtn, LV_BTN_ACTION_PR, btn_rel_actionMainmenu); //Set function to be called when the button is pressed
     lv_obj_set_size(backbtn, 230, 100);//set size
    lv_label_set_text(backctnLabel, SYMBOL_LEFT" Back"); /*Set the text of the label*/

    //Switch Room Labels
    lv_obj_t *lock1Label = lv_label_create(lockScreen, NULL);  //create lock 1 label
    lv_obj_align(lock1Label, backbtn, LV_ALIGN_OUT_BOTTOM_LEFT, 40, 20);   //Align next to the prev. button
    lv_obj_set_style(lock1Label, &buttonstyle); //set style
    lv_label_set_text(lock1Label, "Lock 1 - ");  //Set the text


    //SWITCH setup
    lv_obj_t *sw4 = lv_sw_create(lv_scr_act(), NULL); //create switch 4
    lv_sw_set_style(sw4, LV_SW_STYLE_INDIC, &sw_style); //set style
    lv_obj_align(sw4, lock1Label, LV_ALIGN_OUT_RIGHT_MID, 80, 0);//Align next to the prev. label and set position
    lv_obj_set_size(sw4, 100,40);// set size

    //condition for switch 4
    if(swLockState1==true){
      lv_sw_on(sw4);
    }
    else{
      lv_sw_off(sw4);
    }

    //Switch Room Labels
    lv_obj_t *lock2Label = lv_label_create(lockScreen, NULL);  //create lock 2 label
    lv_obj_align(lock2Label, lock1Label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 31);//Align next to the prev. label and set position
    lv_obj_set_style(lock2Label, &buttonstyle); //set style
    lv_label_set_text(lock2Label, "Lock 2 - ");  //Set the text

    //SWITCH setup
    lv_obj_t *sw5 = lv_sw_create(lv_scr_act(), NULL);//create switch 5
    lv_sw_set_style(sw5, LV_SW_STYLE_INDIC, &sw_style);//set style
    lv_obj_align(sw5, sw4, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 35); //Align next to the prev. label and set position
    lv_obj_set_size(sw5, 100,40);//set size

    //condition for switch 5
    if(swLockState2==true){
      lv_sw_on(sw5);
    }
    else{
      lv_sw_off(sw5);
    }

    lv_obj_t *lock3Label = lv_label_create(lockScreen, NULL);  //create lock 3 label
    lv_obj_align(lock3Label, lock2Label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);  //Align next to the prev. label and set position
    lv_obj_set_style(lock3Label, &buttonstyle); //set style
    lv_label_set_text(lock3Label, "Lock 3 - ");  //Set the text

    //SWITCH setup
    lv_obj_t *sw6 = lv_sw_create(lv_scr_act(), NULL);//create switch 5
    lv_sw_set_style(sw6, LV_SW_STYLE_INDIC, &sw_style);//set style
    lv_obj_align(sw6, sw5, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 35); //Align next to the prev. label and set position
    lv_obj_set_size(sw6, 100,40);//set size

    //condition for switch 6
    if(swLockState3==true){
      lv_sw_on(sw6);
    }
    else{
      lv_sw_off(sw6);
    }

    lv_sw_set_action(sw4, sw_lock_1_action); //set sw 4 action and calls object
    lv_sw_set_action(sw5, sw_lock_2_action); //set sw 5 action and calls object
    lv_sw_set_action(sw6, sw_lock_3_action); //set sw 6 action and calls object

return LV_RES_OK;
}

//----------------Switch Lock action 4---------------
lv_res_t MyLCD::sw_lock_1_action(lv_obj_t * sw4){
  swLockState1 =lv_sw_get_state(sw4);
  if(swLockState1 == true){
    lock1 =true;
  }
  else{
    lock1 =false;
  }
  return LV_RES_OK;
}

//----------------Switch Lock action 5---------------
lv_res_t MyLCD::sw_lock_2_action(lv_obj_t * sw5){
    swLockState2 =lv_sw_get_state(sw5);
  if(swLockState2 == true){
    lock2 =true;
  }
  else{
    lock2 =false;
  }
  return LV_RES_OK;
}

//----------------Switch Lock action 5---------------
lv_res_t MyLCD::sw_lock_3_action(lv_obj_t * sw6){
    swLockState3 =lv_sw_get_state(sw6);
  if(swLockState3 == true){
    lock3 =true;
  }
  else{
    lock3 =false;
  }
  return LV_RES_OK;
}




/*-------------------------------------------------Curtain Button--------------------------------------------------------*/
lv_res_t MyLCD::btn_rel_actionCurtain(lv_obj_t * btn)
{
  //------------------------------Style Parameters------------------------------------------------
   static lv_style_t style1;
  lv_style_copy(&style1, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
  style1.body.main_color = LV_COLOR_BLACK;
  style1.body.grad_color = LV_COLOR_RED;
  style1.body.radius = 0;
  style1.body.border.color = LV_COLOR_BLACK;
  style1.body.border.width = 2;
  style1.body.border.opa = LV_OPA_COVER;
  style1.body.padding.hor = 10;            /*Horizontal padding, used by the bar indicator below*/
  style1.body.padding.ver = 15;            /*Vertical padding, used by the bar indicator below*/
  style1.text.color = LV_COLOR_RED;
  style1.text.font = &lv_font_dejavu_40;

  static lv_style_t buttonstyle;
  lv_style_copy(&buttonstyle, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
  buttonstyle.body.main_color = LV_COLOR_SILVER;
  buttonstyle.body.border.color = LV_COLOR_GRAY;
  buttonstyle.body.padding.hor = 0;
  buttonstyle.body.padding.ver = 0;
  buttonstyle.body.padding.inner = 0;
  buttonstyle.text.color = LV_COLOR_BLACK;
  buttonstyle.text.font = &lv_font_dejavu_40;

  //-----------------------------------------------------------------------------------------------

    //back button settup
    lv_obj_t *curtainscreen = lv_page_create(NULL, NULL); //create curtain screen
    lv_obj_set_style(curtainscreen, &style1); //set style
    lv_scr_load(curtainscreen);//loead screen
    lv_obj_t *curtainLabel = lv_label_create(curtainscreen, NULL);  //create label in curtain screen

    //Go back button
    lv_obj_t *backbtn = lv_btn_create(lv_scr_act(), NULL);  //Create a button on the currently loaded screen
    lv_obj_set_style(backbtn, &buttonstyle); //set style
    lv_obj_align(backbtn, curtainLabel, LV_ALIGN_CENTER, 30, 0);  //Align below the label
    lv_obj_t * backButtonLabel = lv_label_create(backbtn, NULL); //create backbutton label
    lv_btn_set_action(backbtn, LV_BTN_ACTION_PR, btn_rel_actionMainmenu); //Set function to be called when the button is pressed
     lv_obj_set_size(backbtn, 230, 100); //set size
    lv_label_set_text(backButtonLabel, SYMBOL_LEFT" Back"); //Set the text of the label

    //Open button
    lv_obj_t *openCurtain = lv_btn_create(lv_scr_act(), NULL); //create open Curtain button
    lv_obj_set_style(openCurtain, &buttonstyle); //set style
    lv_btnm_set_style(openCurtain, LV_BTNM_STYLE_BTN_TGL_REL, &buttonstyle);//set object style
    lv_obj_set_size(openCurtain,350,300);//set size
    lv_obj_t * openCutainLabel = lv_label_create(openCurtain, NULL); //create open curtain label
    lv_btn_set_action(openCurtain, LV_BTN_ACTION_CLICK, btn_rel_actionOpenCurtain); //Set function to be called when the button is click
    lv_obj_align(openCurtain, backbtn, LV_ALIGN_OUT_BOTTOM_LEFT, 40, 32);   //Align next to the prev. button
    lv_label_set_text(openCutainLabel, "Open Curtain"); //Set the text of the label

    //Close button
    lv_obj_t *closeCurtain = lv_btn_create(lv_scr_act(), NULL); //create close Curtain button
    lv_obj_set_style(closeCurtain, &buttonstyle); //set style
    lv_btnm_set_style(closeCurtain, LV_BTNM_STYLE_BTN_TGL_REL, &buttonstyle);//set object style
    lv_obj_set_size(closeCurtain,350,300);//set size
    lv_obj_t * label2 = lv_label_create(closeCurtain, NULL); //create close curtain label
    lv_btn_set_action(closeCurtain, LV_BTN_ACTION_PR, btn_rel_actionCloseCurtain); //Set function to be called when the button is pressed
    lv_obj_align(closeCurtain, openCurtain, LV_ALIGN_OUT_RIGHT_MID, 20, 0);   //Align next to the prev. button
    lv_label_set_text(label2,"Close Curtain"); //Set the text of the label

    return LV_RES_OK;
}

//--------------------Action Open Curtain-------------------
lv_res_t MyLCD::btn_rel_actionOpenCurtain(lv_obj_t * btn)
{
  curtainsensor1.read();
  if(curtainsensor2 == 1){
    motorDir1=1;
    motorDir2=1;
    return LV_RES_OK;
  }
  else{
  motorDir1=1;
  motorDir2=0;
  }
  return LV_RES_OK;
}

//--------------------Action Close Curtain-------------------
lv_res_t MyLCD::btn_rel_actionCloseCurtain(lv_obj_t * btn)
{
  curtainsensor2.read();
  if(curtainsensor1 == 1){
    motorDir1=1;
    motorDir2=1;
    return LV_RES_OK;
  }
  else{
  motorDir1=0;
  motorDir2=1;
  }
  return LV_RES_OK;
}



/*-------------------------------------------------Secondary Main Menu--------------------------------------------------------*/
lv_res_t MyLCD::btn_rel_actionMainmenu(lv_obj_t * btn)
{
//----------------------------------Creating Styles-----------------------------------------------------------
  static lv_style_t screenstyle;
lv_style_copy(&screenstyle, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
screenstyle.body.main_color = LV_COLOR_GRAY;
//screenstyle.body.grad_color = LV_COLOR_GRAY;
screenstyle.body.radius = 0;
screenstyle.body.border.color = LV_COLOR_BLACK;
screenstyle.body.border.width = 2;
screenstyle.body.border.opa = LV_OPA_COVER;
screenstyle.body.padding.hor = 15;            /*Horizontal padding, used by the bar indicator below*/
screenstyle.body.padding.ver = 10;            /*Vertical padding, used by the bar indicator below*/
screenstyle.text.color = LV_COLOR_GRAY;
screenstyle.text.font = &lv_font_dejavu_40;

static lv_style_t buttonstyle;
lv_style_copy(&buttonstyle, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
buttonstyle.body.main_color = LV_COLOR_SILVER;
buttonstyle.body.border.color = LV_COLOR_GRAY;
buttonstyle.body.padding.hor = 0;
buttonstyle.body.padding.ver = 0;
buttonstyle.body.padding.inner = 0;
buttonstyle.text.color = LV_COLOR_BLACK;
buttonstyle.text.font = &lv_font_dejavu_40;
//------------------------------------------------------------------------------------------------------------
lv_obj_t *scr1 = lv_page_create(NULL, NULL);
lv_obj_set_style(scr1, &screenstyle); 
lv_scr_load(scr1);

//lv_obj_t * btn_rel_actionTemperature = lv_obj_create(scr, NULL);	    /*Create an object on the previously created parent object*/
lv_obj_t *optionsLabel1 = lv_label_create(scr1, NULL);  /*First parameters (scr) is the parent*/
lv_label_set_text(optionsLabel1, "Choose from the available options: ");  /*Set the text*/
lv_obj_set_style(optionsLabel1, &screenstyle); 
lv_obj_align(optionsLabel1, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);                        /*Set the x coordinate*/


/*-------------------------------------------------Create Temperature button--------------------------------------------------------*/
lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);          /*Create a button on the currently loaded screen*/
lv_obj_set_style(btn1, &buttonstyle); 
lv_btnm_set_style(btn1, LV_BTNM_STYLE_BTN_TGL_PR, &buttonstyle);
lv_btn_set_action(btn1, LV_BTN_ACTION_CLICK, btn_rel_actionTemperature); /*Set function to be called when the button is released*/
lv_obj_align(btn1, optionsLabel1, LV_ALIGN_OUT_BOTTOM_LEFT, -70, 40);  /*Align below the label*/
lv_coord_t width = lv_obj_get_width(btn1);
lv_obj_set_width(btn1, width + 230);
lv_coord_t height = lv_obj_get_height(btn1);
lv_obj_set_height(btn1, height + 100);
/*Setting label on the button */
tempLabelButton = lv_label_create(btn1, NULL);

/*-------------------------------------------------Create Light button--------------------------------------------------------*/
lv_obj_t *btn2 = lv_btn_create(lv_scr_act(), NULL); //Create a button on the currently loaded screen
lv_obj_set_style(btn2, &buttonstyle); 
lv_btnm_set_style(btn2, LV_BTNM_STYLE_BTN_TGL_PR, &buttonstyle);//Set function to be called when the button is togled and pressed
lv_btn_set_action(btn2, LV_BTN_ACTION_CLICK, btn_rel_actionLight);//Set function to be called when the button is released
lv_obj_align(btn2, btn1, LV_ALIGN_OUT_RIGHT_MID, 25, -50);   //Align next to the prev. button.*/
lv_obj_set_width(btn2, width + 230); //setting width
lv_obj_set_height(btn2, height + 100); //setting height
//lv_obj_set_size(btn2, 200, 100); //Button size
lv_obj_set_style(btn2, &buttonstyle);  //setting style
/*Setting label on the button */
lightLabelButton = lv_label_create(btn2, NULL);

/*-------------------------------------------------Create Door button--------------------------------------------------------*/
lv_obj_t *btn3 = lv_btn_create(lv_scr_act(), NULL);//Create a button on the currently loaded screen
lv_obj_set_style(btn3, &buttonstyle);  //setting style
lv_btnm_set_style(btn3, LV_BTNM_STYLE_BTN_TGL_PR, &buttonstyle);//setting style
lv_btn_set_action(btn3, LV_BTN_ACTION_CLICK, btn_rel_actionLock); //Set function to be called when the button is released clicked
lv_obj_align(btn3, btn1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20); //Align next to the prev. button.
/*Increase the button width*/
lv_obj_set_width(btn3, width + 230);
lv_obj_set_height(btn3, height + 100);
/*Setting label on the button */
lockLabelButton = lv_label_create(btn3, NULL);


/*-------------------------------------------------Create Curtain button--------------------------------------------------------*/
lv_obj_t *btn4 = lv_btn_create(lv_scr_act(), NULL);          /*Create a button on the currently loaded screen*/
lv_obj_set_style(btn4, &buttonstyle); 
lv_btnm_set_style(btn4, LV_BTNM_STYLE_BTN_TGL_PR, &buttonstyle);
lv_btn_set_action(btn4, LV_BTN_ACTION_CLICK, btn_rel_actionCurtain); /*Set function to be called when the button is released*/
lv_obj_align(btn4, btn3, LV_ALIGN_OUT_RIGHT_MID, 26, -50);     /*Align next to the prev. button.*/
/*Increase the button width*/
lv_obj_set_width(btn4, width + 230);
lv_obj_set_height(btn4, height + 100);
/*Setting label on the button */
curtainStateLabel = lv_label_create(btn4, NULL);

//-------------------deleting previous menus------------------
void lv_obj_del_async(lv_obj_t *btn_rel_actionTemperature);
void lv_obj_del_async(lv_obj_t *btn_rel_actionLight);
void lv_obj_del_async(lv_obj_t *btn_rel_actionLock);
void lv_obj_del_async(lv_obj_t *btn_rel_actionCurtain);

return 0;
}






//----------------------------------------------SYSTEM ESSENTIALS--------------------------------------------


/* Flush the content of the internal buffer the specific area on the display
* You can use DMA or any hardware acceleration to do this operation in the background but
* 'lv_flush_ready()' has to be called when finished
* This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
void MyLCD::ex_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

int32_t x;
int32_t y;
for (y = y1; y <= y2; y++) {
for (x = x1; x <= x2; x++) {
/* Put a pixel to the display. For example: */
/* put_px(x, y, *color_p)*/
uint32_t c = lv_color_to32(*color_p);
lcd.DrawPixel(x, y, c);
color_p++;
}
}
/* IMPORTANT!!!
* Inform the graphics library that you are ready with the flushing*/
lv_flush_ready();
}




/* Write a pixel array (called 'map') to the a specific area on the display
* This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
void MyLCD::ex_disp_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
int32_t x;
int32_t y;
for (y = y1; y <= y2; y++) {
for (x = x1; x <= x2; x++) {
/* Put a pixel to the display. For example: */
/* put_px(x, y, *color_p)*/
uint32_t c = lv_color_to32(*color_p);
lcd.DrawPixel(x, y, c);
color_p++;
}
}
}




/* Write a pixel array (called 'map') to the a specific area on the display
* This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
void MyLCD::ex_disp_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
/*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

int32_t x;
int32_t y;
for (y = y1; y <= y2; y++) {
for (x = x1; x <= x2; x++) {
/* Put a pixel to the display. For example: */
/* put_px(x, y, *color)*/
uint32_t c = lv_color_to32(color);
lcd.DrawPixel(x, y, c);
}
}
}





/* Read the touchpad and store it in 'data'
* Return false if no more data read; true for ready again */
bool MyLCD::ex_tp_read(lv_indev_data_t *data)
{
static int16_t last_x = 0;
static int16_t last_y = 0;

ts.GetState(&tsState);
if (tsState.touchDetected) {
data->point.x = tsState.touchX[0];
data->point.y = tsState.touchY[0];
last_x = data->point.x;
last_y = data->point.y;
data->state = LV_INDEV_STATE_PR;
}
else {
data->point.x = last_x;
data->point.y = last_y;
data->state = LV_INDEV_STATE_REL;
}
                     
return false;   /*false: no more data to read because we are no buffering*/
}
