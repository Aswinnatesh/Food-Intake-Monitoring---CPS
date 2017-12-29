#include <SoftwareSerial.h>
SoftwareSerial espSerial =  SoftwareSerial(3,2);      

int t = 80; int h = 200, peak=0;
unsigned long old_time=0, new_time=0;
int threshold = 30, i=0, flag=0;
int window=0, trial=149; int peaks[400]; int count=0; 


float t2 = 16; //Basic Hard Classifier 50
float t1 = 7;  //Basic Soft Classifier 7
float t_hard=0.5; //Adv Hard Classifier
float t_soft=0.5; // Adv Soft Classifier
int bitstream[6]={1, 0, 0, 0, 0, 0};

int noise_counter2=0;

float noise_counter=0, hard_counter=0, soft_counter=0;
float ratio, chew, check_1, check_2;

String apiKey = "LAQA6TF3UFQX8JMR";// Think-Speak Channel

String ssid="WolfieNet-Open";    // Wifi network SSID
String password =" ";  // Wifi network password

boolean DEBUG=true;

void showResponse(int waitTime){
    long t=millis();
    char c;
    while (t+waitTime>millis()){
      if (espSerial.available()){
        c=espSerial.read();
        if (DEBUG) Serial.print(c);
      }
    }                  
}

//========================================================================
boolean thingSpeakWrite(String value1, int value2){
  String cmd = "AT+CIPSTART=\"TCP\",\"";                  // TCP connection
  cmd += "184.106.153.149";                               // api.thingspeak.com
  cmd += "\",80";
  espSerial.println(cmd);
  if (DEBUG) //Serial.println(cmd);
  if(espSerial.find("Error")){
    //if (DEBUG) { Serial.println("AT+CIPSTART error");}
    return false;
  }
  
  String getStr = "GET /apps/thinghttp/send_request?api_key=";   // prepare GET string
  
  getStr += apiKey;
  
  getStr +="&field1=";
  getStr += value1;
  getStr +="&field2=";
  getStr += String(value2);
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  espSerial.println(cmd);
  //if (DEBUG)  Serial.println(cmd);
  
  delay(100);
  if(espSerial.find(">")){
    espSerial.print(getStr);
    //if (DEBUG)  Serial.print(getStr);
  }
  else{
    espSerial.println("AT+CIPCLOSE");
    //if (DEBUG)   Serial.println("AT+CIPCLOSE");
    return false;
  }
  return true;
}


//================================================================================ setup
void setup() {                
  DEBUG=true;           // enable debug serial
  Serial.begin(9600); 
  
  espSerial.begin(9600);  // enable software serial
  
  espSerial.println("AT+CWMODE=1");   // set esp8266 as client
  showResponse(1000);

  espSerial.println("AT+CWJAP=\""+ssid+"\",\""+password+"\"");  // set your home router SSID and password
  showResponse(5000);

   if (DEBUG)  Serial.println("Setup completed");
}


// ====================================================================== loop
void loop()
{
  int trialno=0;
  if(Serial.available() > 0)
    {
      trialno = Serial.parseInt();
      trialno= trialno*2;
      //Serial.print("TRIALS :"); Serial.println(trial);
    }

  for (int k=0; k<trialno; k++)
  {

   //if(trial>1)
   // {

      for(int i=0; i<=trial; i++)
      {
         while(((millis()-old_time))<= 200) // sub window length
          {
            peak= 0;
             check_1 = analogRead(A0);               //Assing first reading 
             delay(20);                                //wait
             check_2 = analogRead (A0);              //Assing second reading.
             delay(20);  
             if (check_1 < check_2)
             {
                check_1 = check_2;                          //Write previous last reading as current first for comparison
                check_2 = analogRead(A0);  
                delay(20); 
             }

             else 
             {
              peak = check_1;
             }
          }
        peaks[i]=peak;
        //Serial.println(peaks[i]);
        old_time=millis();   
        }
        

    count=count+1;
      Serial.print(count);
      
      noise_counter2++; 
      basic_classifier();
      activity_detection();
      //noise_counter2++; 
      
      
      String getStr2 = "0";
      for(i=0;i<6;i++)
        {
          getStr2 += String(bitstream[i]);        
        }
      Serial.print(getStr2);  
      Serial.print("\t");
      
      chew_rate();
      Serial.println("");
      
      thingSpeakWrite(getStr2,chew);                                      // Write values to things
    }
}

//======================================================================== showResponce


void chew_rate()
{
  chew=(soft_counter+hard_counter)/30;
  if((chew <= 0.08)||(flag==1)) {chew=0.00; flag=0;}
  Serial.print("Chew_rate :");
  Serial.print(chew);
  Serial.print("\t");
  Serial.print("Data Uploaded!");
  
}

void basic_classifier()
{

  noise_counter=0, hard_counter=0, soft_counter=0;
  for(int i=0; i<=trial; i++)
      {
        if(peaks[i] < t1) // 30
         {
          noise_counter+=1;
         }
         else if ((peaks[i] >= t1) && (peaks[i] < t2)) // Basic Soft Classifier
         {
          soft_counter+=1;
         }
         else if((peaks[i] >= t2)) //Basic Hard Classifier
         {
          hard_counter+=1;
         }
      }
}

void activity_detection()
{

ratio = (soft_counter/hard_counter);

for (int h= 1; h<=7; h++)
{
   bitstream[h] = 0;
}

  //Serial.print("\t");
  //Serial.print(noise_counter);
  //Serial.print("\t");
  //Serial.print(soft_counter);
  //Serial.print("\t");
  //Serial.print(hard_counter);  
  //Serial.print("\t");
  //Serial.print(ratio);

  Serial.print("\t");
  if((noise_counter2 == 1)||(noise_counter2 == 6))
    {
      bitstream[1] = 1;
      Serial.print("(No Activity Detected) \t");
    flag =1;
    }

   else if(noise_counter2 == 2)
    {
      bitstream[3] = 1;
      Serial.print("(Extremely Soft Food is Being Consumed) \t");
    }
 
   else if((noise_counter2 == 3)||(noise_counter2 == 6))  // &&(ratio >= t_hard)
    {
      bitstream[2] = 1;
      Serial.print("(Soft Food is Being Consumed) \t");
    }
  
   else if(noise_counter2 == 4)
    {
      bitstream[4] = 1;
      Serial.print("(Hard Food is Being Consumed) \t");
    }
   else if(noise_counter2 == 5)
    {
      bitstream[5] = 1;
      Serial.print("(Extremely Hard Food is Being Consumed) \t");
    }
   else 
   {
    Serial.print("Error: No Activivity Recognised \t");
   }
}




  // ______________
//
// int Threshold = 100;
//int Hit;
//void setup(){
//
//  pinMode (13,OUTPUT);
//  BlinkLed (3,500);
//  Serial.begin(38400);
//
//}
//
//void loop(){
//  for (int i = 0;i<=5;i++){                //Cycle through ADC channels.
//    if (analogRead(i)>Threshold){          //If current channel reading exceeds threshold
//      Hit = AnalogMaxima (i,Threshold,2);  //find local maxima and return value.
//      if (Hit == 1025 || Hit == 0){        //if still ascending or below threshold,
//        break;                             //go to next ADC channel.
//      }
//      else{
//        Serial.println(Hit);               //Show maxima. or play midi note ;)
//      }
//    }
//  }
//}
//
//
////--------------------------------------------------------------------------------
///* AnalogMaxima compares succesive readings from ADC on channel AnalogCh
//and returns :
//1025 if voltage is decreasing,so that only the maxima is returned.
//0    if below threshold.
//or returns the highest value when voltage starts to drop (local maxima).
//
//AnalogCh  :  Channel on the ADC to check for maxima.
//Threshold :  Lowest voltage reading to begin maxima evaluation.
//Delay     :  Delay between succesive readings.
//*/
//
//int AnalogMaxima (int AnalogCh, int Threshold, int Delay){    
//  int check1;                                  //variable to store first reading.
//  int check2;                                  //variable to store second reading.
//
//  check1 = analogRead(AnalogCh);               //Assing first reading 
//  delay(Delay);                                //wait
//  check2 = analogRead (AnalogCh);              //Assing second reading.
//  if (check1>check2){                          //If voltage is DECREASING (no maxima)...
//    return 1025;                               //end loop and return 1025.
//  }
//  else{
//    while (analogRead(AnalogCh)>Threshold){     //While above threshold and RISING
//
//      check1 = check2;                          //Write previous last reading as current first for comparison
//      check2 = analogRead(AnalogCh);            //Assing second reading.
//      delay(Delay/2);                           //wait,and loop unless...                
//       
//      if (check1>check2){                       //voltage drop is observed
//        return check1;                          //if so return highest value :)
//
//      }
//    }
//  }
//}                            // end of AnalogMaxima //
////--------------------------------------------------------------------------------
//
//
////--------------------------------------------------------------------------------
///* BlinkLed ensures there is enough time in the start of the sketch to download a
//new sketch if desired,in case that the serial tx is constantly sending data.
//Times  :  times to blink.
//Delay  :  delay between blinks.
//*/
//
//void BlinkLed (int Times, int Delay){
//  for (int i = 1; i<=Times; i++){
//    digitalWrite (13,HIGH);
//    delay(Delay);
//    digitalWrite (13,LOW);
//    delay(Delay);
//  }
//}                             // end of BlinkLed //
////--------------------------------------------------------------------------------
//
//
//
//
//
//
//
//
//
//  check1 = analogRead(AnalogCh);               //Assing first reading 
//  delay(10);                                //wait
//  check2 = analogRead (AnalogCh);              //Assing second reading.
//  if (check1<check2){                          //If voltage is DECREASING (no maxima)...
//    while (analogRead(AnalogCh)>Threshold){     //While above threshold and RISING
//
//      check1 = check2;                          //Write previous last reading as current first for comparison
//      check2 = analogRead(AnalogCh);            //Assing second reading.
//      delay(Delay/2);                           //wait,and loop unless...                
//       
//      if (check1>check2){                       //voltage drop is observed
//        peaks[i]= check1;                          //if so return highest value :)
//
//      }
//    }
//  }




// our Old codec


//
//
//
//
//
//        
//          peak= 0;
//          check_1 = analogRead(A0);               //Assing first reading 
//          delay(10);                                //wait
//          check_2 = analogRead (A0);              //Assing second reading.
//          if (check_1<check_2){                          //If voltage is DECREASING (no maxima)...
//          while (analogRead(A0)>t1){     //While above threshold and RISING
//
//          check_1 = check_2;                          //Write previous last reading as current first for comparison
//          check_2 = analogRead(A0);            //Assing second reading.
//          delay(10);                           //wait,and loop unless...                
//       Serial.print(check_1);
//       Serial.print("\t"); Serial.println(check_2);
//          if (check_1>check_2)                       //voltage drop is observed
//          {
//            peak= check_1;                          //if so return highest value :)
//         //Serial.println(peak);
//          } 
//          } 
//        } 
//
//       peaks[i]= 0;
//      // while ((millis() - old_time) <= 1000) 
//       // {
//          peaks[i] = peak;  
//       // }
//        //Serial.println(peaks[i]);
//        old_time = millis();
//    }

        
//        if ((millis() - old_time) <= 1000)   // 1.5sec // 
//        {
//          //Serial.println("Entered Millis");
//          peaks[i] =t1;
//          check_1 = analogRead(A0);
//          delay(10);
//          check_2 = analogRead(A0);
//          if(check_2 > check_1)
//            {
//              if (peaks[i] < check_2)
//                {
//                  peaks[i] = check_2;
//                }
//            }
//          //else 
//         // {
//         //    peaks[i] = 0;  
//         // }
//
//        }
//      //  } while(((millis() - old_time) <= 1000));
//         old_time = millis();
//         //Serial.print(peaks[i]); 
//         //Serial.print("\t");
     
      //trial=0;

