#include "exec.h"

//int main(){

// data
uint8_t isClicked = 0;
uint8_t status;
uint8_t accVal[6];
float x = 0;
float y = 0;
float z = 0;
char data[45] = {0x20};
uint8_t length = 0;
uint16_t endCycle = 0;

uint8_t isDetected = 0;
uint8_t isCatched = 0;

//while(1){

//function declaration for test
#if UNDER_TEST == 1
void execute(){
    if(!((PTB->PDIR)&(1 << 7))){
        isClicked = 1;
        I2C_WriteReg(ADDRESS, CTRL_REG1, 1 );
    }    

    I2C_ReadReg(ADDRESS, STATUS_REG, &status);

    if(status & (1 << 3)){
        I2C_ReadRegBlock(ADDRESS, ACC_DATA_REGS, 6, accVal);
    }

    x = (float)((int16_t)((accVal[0] << 8)|accVal[1]) >> 2)/4096;
    y = (float)((int16_t)((accVal[2] << 8)|accVal[3]) >> 2)/4096;
    z = (float)((int16_t)((accVal[4] << 8)|accVal[5]) >> 2)/4096;
    length = sprintf(data, "%d; %.2f; %.2f; %.2f\n", endCycle+1 ,x , y, z);

    if(isClicked){
        for(int i = 0; i < length; i++){
            while(!(UART0->S1 & UART0_S1_TDRE_MASK));
            //DELAY(35);
            UART0->D = data[i];     
        }
        DELAY(50)
        endCycle++;
    }
    // 500 in 20s
    if(endCycle == 1200) {
        isClicked = 0;
        endCycle = 0;
        if(((UART0->S1) & UART0_S1_TDRE_MASK)){
            UART0->D = '\n';
        }
        I2C_WriteReg(ADDRESS, CTRL_REG1, 0 );
    }

}

// function declaration for use
#else
void execute(){
    I2C_ReadReg(ADDRESS, STATUS_REG, &status);
    if(status & (1 << 3)){
        I2C_ReadRegBlock(ADDRESS, ACC_DATA_REGS, 6, accVal);
    } 
       
    x = (float)((int16_t)((accVal[0] << 8)|accVal[1]) >> 2)/4096;
    // interupt simulation
    if(x < ACTIVATION_TH && isDetected == 0) isDetected = 1;

    // after "interupt" execution
    while(isDetected){
        I2C_ReadReg(ADDRESS, STATUS_REG, &status);
        if(status & (1 << 3)){
            I2C_ReadRegBlock(ADDRESS, ACC_DATA_REGS, 6, accVal);
        } 
        
        x = (float)((int16_t)((accVal[0] << 8)|accVal[1]) >> 2)/4096;

        if(x > EXIT_TH && isCatched == 0) isCatched = 1;
        if(x < EXIT_TH && isCatched == 1){
            if(((UART0->S1) & UART0_S1_TDRE_MASK)){
            UART0->D = 'a';

            isCatched = isDetected = 0;
            break;
        }
        }
    }
}

#endif

//}}