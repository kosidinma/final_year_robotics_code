#include "mbed.h"
#include "HMC5883L.h"
#include "MMA8451Q.h"
 
#define MMA8451_I2C_ADDRESS (0x1d<<1)
#define SDA      PTE0
#define SCL      PTE1
#define PI       3.14159265
 
Serial pc(USBTX, USBRX);
Serial myarduino(PTD3, PTD2); //tx,rx
Serial myandroid(PTC4, PTC3); //tx,rx
DigitalOut myled(LED1);
AnalogIn LDR(PTC2);
MMA8451Q acc(PTE25, PTE24, MMA8451_I2C_ADDRESS);

int val=0;
 
int main() 
{
    char data[100]; //from arduino
    char receive; //from android
    float x, y, z, heading;
    char direction[10];
    HMC5883L hmc5883l(SDA, SCL);
   // pc.baud(9600);
    myarduino.baud(9600);
    myandroid.baud(9600);
    
    while(1){
        x = hmc5883l.getMx(); 
        y = hmc5883l.getMy(); 
        z = hmc5883l.getMz(); 
        
        heading = atan2(y, x); 
        if(heading < 0) 
            heading += 2*PI; 
        if(heading > 2*PI) 
            heading -= 2*PI;        
        heading = heading * 180 / PI; 
        if (heading == 0 || heading == 360)
        strcpy(direction, "N");
        if (heading == 90)
        strcpy(direction, "E");
        if (heading == 180)
        strcpy(direction, "S");
        if (heading == 270)
        strcpy(direction, "W");
        if (heading > 0 && heading < 90)
        strcpy(direction, "NE");
        if (heading > 90 && heading < 180)
        strcpy(direction, "SE");
        if (heading > 180 && heading < 270)
        strcpy(direction, "SW");
        if (heading > 270 && heading < 360)
        strcpy(direction, "NW");
        receive = myandroid.getc(); //collect from android
        myarduino.putc(receive); //send to arduino
        myarduino.gets(data, 100); //receiving a string of 100 characters
        myandroid.printf("%f,%s,%f,%f,%f,%001.2f,%001.2f,%001.2f,%s,",
        LDR.read(),data,x,y,z,acc.getAccX()*10,acc.getAccY()*10,acc.getAccZ()*10
        ,direction);        
        wait(0.05); 
        }
}
 