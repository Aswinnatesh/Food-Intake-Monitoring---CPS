unsigned long old_time=0, new_time=0;
int check_1, check_2, threshold = 30, peak=0, i=0;
int window=0, trial=0; int peaks[100];

int t2 = 120; //Basic Hard Classifier
int t1 = 3;  //Basic Soft Classifier
int t_hard=0; //Adv Hard Classifier
int t_soft=0; // Adv Soft Classifier
int bitstream[6]={1, 0, 0, 0, 0, 0};

int noise_counter=0, hard_counter=0, soft_counter=0;
double ratio, chew;

void setup() {
  Serial.begin(9600);
}

void loop() 
{
  if(Serial.available() > 0)
    {
      trial = Serial.parseInt();
      Serial.print("TRIALS :"); Serial.println(trial);
    }
  if(trial>1)
    {
      for(int i=0; i<=trial; i++)
      {
        do   // 1.5sec // 
        {
          //Serial.println("Entered Millis");
          peaks[i] =t1;
          check_1 = analogRead(A0);
          delay(10);
          check_2 = analogRead(A0);
          if(check_2 > check_1)
            {
              //if (peak < check_2)
              //  {
                  peaks[i] = check_2;
              //  }
            }
          //else 
         // {
           //  /peaks[i] = 0;  
         // }
        } while(((millis() - old_time) <= 1000));
         old_time = millis();
         //Serial.print(peaks[i]); 
         //Serial.print("\t");
      }
      trial=0;
      //Serial.println(" ");
      basic_classifier();
      activity_detection();
      for(i=0;i<6;i++)
        {
        Serial.print(bitstream[i]);
        }
      Serial.print("\t");
      chew_rate();
      Serial.println("");
    }
}

    

void basic_classifier()
{
  for(int j=0; i<=trial; i++)
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
  //int bitstream[6]={1, 0, 0, 0, 0, 0};
  ratio = (soft_counter/hard_counter);
  if(noise_counter > 60)
    {
      bitstream[1] = 1;
      Serial.print("No Activity Detected \t");
    }
   else if((ratio < 0)&&(ratio >= t_hard))
    {
      bitstream[2] = 1;
      Serial.print("Hard Food is Being Consumed \t");
    }
   else if((ratio < 0)&&(ratio < t_hard))
    {
      bitstream[3] = 1;
      Serial.print("Extremely Hard Food is Being Consumed \t");
    }
   else if((ratio >= 0)&&(ratio <= t_soft))
    {
      bitstream[4] = 1;
      Serial.print("Soft Food is Being Consumed \t");
    }
   else if((ratio > 0)&&(ratio > t_soft))
    {
      bitstream[5] = 1;
      Serial.print("Extremely Soft Food is Being Consumed \t");
    }
   else 
   {
    Serial.print("Error: No Activivity Recognised \t");
   }
//  for(i=0;i<6;i++)
//  {
//    Serial.print(bitstream[i]);
//  }
//  Serial.print("\t");
}

void chew_rate()
{
  chew=(soft_counter+hard_counter)/60;
  Serial.print("Chew_rate :");
  Serial.print(chew);
  Serial.print("\t");
}





