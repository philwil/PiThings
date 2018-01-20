#if 0
void latch_tx(uint8_t latch_state)
{
   unsigned char i;

   gpioWrite(MOTORLATCH, PI_LOW);

   gpioWrite(MOTORDATA, PI_LOW);

   for (i=0; i<8; i++)
   {
      gpioDelay(10);  // 10 micros delay

      gpioWrite(MOTORCLK, PI_LOW);

      if (latch_state & BIT(7-i)) gpioWrite(MOTORDATA, PI_HIGH);
      else                        gpioWrite(MOTORDATA, PI_LOW);

      gpioDelay(10);  // 10 micros delay

      gpioWrite(MOTORCLK, PI_HIGH);
   }

   gpioWrite(MOTORLATCH, PI_HIGH);
}

void init(uint8_t latch_state)
{
  //latch_state = 0;

   latch_tx(latch_state);

   gpioWrite(MOTORENABLE, PI_LOW);
}

uint8_t DCMotorInit(uint8_t num, uint8_t latch_state)
{

   switch (num)
   {
      case 1: latch_state &= ~BIT(MOTOR1_A) & ~BIT(MOTOR1_B); break;
      case 2: latch_state &= ~BIT(MOTOR2_A) & ~BIT(MOTOR2_B); break;
      case 3: latch_state &= ~BIT(MOTOR3_A) & ~BIT(MOTOR3_B); break;
      case 4: latch_state &= ~BIT(MOTOR4_A) & ~BIT(MOTOR4_B); break;
      default: return latch_state;
   }

   latch_tx(latch_state);

   printf("Latch=%08X\n", latch_state);
   return latch_state;
}

uint8_t DCMotorRun(uint8_t motornum, uint8_t cmd, uint8_t latch_state)
{
   uint8_t a, b;

   switch (motornum)
   {
      case 1: a = MOTOR1_A; b = MOTOR1_B; break;
      case 2: a = MOTOR2_A; b = MOTOR2_B; break;
      case 3: a = MOTOR3_A; b = MOTOR3_B; break;
      case 4: a = MOTOR4_A; b = MOTOR4_B; break;
      default: return latch_state;
   }
 
   switch (cmd)
   {
      case FORWARD:  latch_state |=  BIT(a); latch_state &= ~BIT(b); break;
      case BACKWARD: latch_state &= ~BIT(a); latch_state |=  BIT(b); break;
      case RELEASE:  latch_state &= ~BIT(a); latch_state &= ~BIT(b); break;
      default: return latch_state;
   }

   latch_tx(latch_state);

   printf("Latch=%08X\n", latch_state);
   return latch_state;
}

#endif

int fwd_cmd(void *key, int n, char *data, int speed, int time)
{
    struct cmds *cmd = (struct cmds*)key;

    printf("running [%s] n %d data [%s]  speed %d time %d\n",
	   cmd->key, n, data, speed, time);
    return 0;
}

int init_g_spaces(void)
{
  int i;
  for (i=0 ; i< NUM_IDX; i++)
    g_spaces[i]=NULL;
  return 0;

}
#if 0
   if(0)
     {
       if (gpioInitialise()<0) return 1;

       gpioSetMode(MOTORLATCH,  PI_OUTPUT);
       gpioSetMode(MOTORENABLE, PI_OUTPUT);
       gpioSetMode(MOTORDATA,   PI_OUTPUT);
       gpioSetMode(MOTORCLK,    PI_OUTPUT);
       
       gpioSetMode(MOTOR_3_PWM, PI_OUTPUT);
       gpioSetMode(MOTOR_4_PWM, PI_OUTPUT);
       
       gpioPWM(MOTOR_3_PWM, 0);
       gpioPWM(MOTOR_4_PWM, 0);
       
       init(latch_st);
       
       for (i=60; i<160; i+=20)
	 {
	   gpioPWM(MOTOR_3_PWM, i);
	   gpioPWM(MOTOR_4_PWM, 220-i);
	   
	   latch_st = DCMotorRun(3, FORWARD, latch_st);
	   latch_st = DCMotorRun(4, BACKWARD, latch_st);
	   
	   sleep(2);
	   
	   latch_st = DCMotorRun(3, RELEASE, latch_st);
	   latch_st = DCMotorRun(4, RELEASE, latch_st);
	   
	   sleep(2);
	   
	   gpioPWM(MOTOR_4_PWM, i);
	   gpioPWM(MOTOR_3_PWM, 220-i);
	   
	   latch_st = DCMotorRun(3, BACKWARD, latch_st);
	   latch_st = DCMotorRun(4, FORWARD, latch_st);
	   
	   sleep(2);
	   
	   latch_st = DCMotorRun(3, RELEASE, latch_st);
	   latch_st = DCMotorRun(4, RELEASE, latch_st);
	   
	   sleep(2);
	 }
       
       gpioPWM(MOTOR_4_PWM, 0);
       gpioPWM(MOTOR_3_PWM, 0);
       
       latch_st = DCMotorRun(3, RELEASE, latch_st);
       latch_st = DCMotorRun(4, RELEASE, latch_st);
       
       gpioTerminate();
     }
#endif
