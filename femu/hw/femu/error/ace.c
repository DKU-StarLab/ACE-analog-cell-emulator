/*ACE is a FEMU-based reliability measurement tool. Analog signals can be output 
using ACE. ACE is an emulator designed to generate errors similar to real SSDs.
 Gaussian distribution graph values ​​are assigned to each cell to simulate analog 
signal distribution in actual cells. It is necessary to model changes in the 
distribution graph to trigger errors. We modeled the parameters using polynomial 
regression, one of the machine learning techniques. The modeled parameters 
change the distribution graph according to the P/E cycle, write time, and read 
count, resulting in durability, retention, and disturbance error. These errors are 
similar to those that occur on real hardware. */
/*contack e-mail: gardenlee960828@dankook.ac.kr*/
/*GPL v2 license*/

#include "../nvme.h"

/*Analog TLC mean voltage*/
static float TLC_init_voltage[8]={-110,66,129,192,255,318,381,444};
/*Analog TLC reference voltage*/
static float TLC_reference_voltage[7]={23,97,161,225,282,353,413};

/*Analog MLC mean voltage*/
static float MLC_init_voltage[4]={-110,100,270,446.98};
/*Analog MLC reference voltage*/
static float MLC_reference_voltage[3]={30,200,375};

/*Analog TLC noise signal parameter*/
// static double TLC_PE_a1[8]={0.008244898,0.000836735,0.000403061,0.00055102,0.00030102,0.000255102,0.00022449,0.000341837};
// static double TLC_PE_b1[8]={1.133163265,-0.41377551,-0.344897959,-0.406632653,-0.306632653,-0.058163265,-0.434183674,-0.521938776};
static double TLC_PE_a2[8]={-0.0000001715 ,0.0000003351 ,0.0000000234 ,0.0000003365 ,0.00000029 ,0.00000012 ,0.00000052 ,0.00000047};
static double TLC_PE_b2[8]={0.00883,-0.00031,0.000323,-0.0006,-0.00068,-0.000147,-0.001541,-0.0013};

static double TLC_PE_c2[8]={0,0,0,0,0,0,0,0};
// static double TLC_T_a1[8]={0.001868291,0.000997014,0.000675825,0.00043405,0.000193759,-0.0000154891,-0.000205178,-0.000405251};
// static double TLC_T_b1[8]={8.215855299,1.861725058,0.575240292,-0.293293891,-1.24116222,-2.254740775,-3.07546869,-4.040857583};
static double TLC_T_a2[8]={-0.000000555 ,-0.00000025 ,-0.0000001643 ,-0.0000000813 ,-0.0000000153 ,0.00000005 ,0.0000000805 ,0.00000013};
static double TLC_T_b2[8]={0.007,0.0033,0.0022,0.0012,0.000335,-0.00047,-0.0001,-0.0016};
static double TLC_T_c2[8]={0,0,0,0,0,0,0,0};
// static double TLC_R_a1[8]={0.0005173590 ,0.9332516440 ,0.0000217294 ,0.0000064943 ,0.0000019238,0.0000118000,0.0000247764,0.0000404485};
// static double TLC_R_b1[8]={16.6513009027 ,0.0000470000 ,0.4253931299,0.0113956623,0.2475677000,0.7000490136,0.8527508884,1.2969489033};
static double TLC_R_a2[8]={0.0000000062,0.0000000003,0.0000000001,0.0000000000 ,0.0000000001 ,0.0000000001 ,0.0000000002 ,0.0000000005};
static double TLC_R_b2[8]={0.00114 ,0.00008 ,0.0000345478 ,0.0000107639 ,0.000008,0.000024,0.000044,0.000089};
static double TLC_R_c2[8]={0,0,0,0,0,0,0,0};


/*Analog MLC noise signal parameter*/
static double MLC_retention_value[4]={0.02,0.002,-0.004,-0.018};
static double MLC_read_disturbance_value[4]={1.8148,0.41997,0.30143,-0.3607};
static double MLC_PE_value[4]={0.001,0.0068,0.0079,0.0035};

/*Analog TLC std signal parameter*/
static double TLC_cell_cda[8]={0.026,0.005,0.0052,0.005,0.0049,0.005,0.0052,0.0048};
static double TLC_cycle_cda[8]={1,7,14,8,20,20,10,17};
static double TLC_time_cda[8]={-1.5,1.6,1.8,5,50,7,2.8,2.4};
static double TLC_read_cda[8]={-1.4,5.8,14.3,60,100,26,10.5,6.6};

/*Analog MLC std signal parameter*/
static double MLC_cell_cda[4]={0.02,0.0093,0.0093,0.0095};
static double MLC_time_cda[4]={0.01,0.01,0.01,0.01};
static double MLC_cycle_cda[4]={0.01,0.05,0.04,0.02};
static double MLC_read_cda[4]={-0.04,0.018,0.01,0.01};

extern QemuLogFile *voltage_log;

uint64_t err=0;

/*MLC read retry pattern*/
float mlc_read_retry[21]={
    1,0,0,
    0,1,0,
    0,0,1,
    1,1,0,
    1,0,1,
    0,1,1,
    1,1,1
    };

/*TLC read retry pattern*/
// float tlc_read_retry[35]={
// 3,1,0.3,0.1,0.2,0.1,0.1,
// -1,-1,-1,-1,-1,-1,-1,
// -1,-1,-1,-1,1,1,1,
// 1,1,1,1,-1,-1,-1,
// 0,0,0,0,0,0,
// };

/*TLC read retry pattern*/
// float tlc_read_retry[35]={
// 3,2,1,-0.3,0.7,-0.2,-2.3,
// -1,-1,-1,-1,-1,-1,-1,
// 1,1,1,1,1,1,1,
// 1,1,1,1,-1,-1,-1,
// 0,0,0,0,0,0,
// };

float tlc_read_retry[35]={
1,1,1,1,1,1,1,
-1,-1,-1,-1,-1,-1,-1,
-0.1,-0.1,-0.1,-0.1,0.1,0.1,0.1,
0.1,0.1,0.1,0.1,-0.1,-0.1,-0.1,
0,0,0,0,0,0,0
};

// float tlc_read_retry[35]={
// 4.5,1.5,1,-0.8,0.2,3,0.8,
// -1,-1,-1,-1,-1,-1,-1,
// -0.1,-0.1,-0.1,-0.1,0.1,0.1,0.1,
// 0.1,0.1,0.1,0.1,-0.1,-0.1,-0.1,
// 0,0,0,0,0,0,0
// };


static double poly1(double a, double b, int x){
    return a*x+b;
}

static double poly2(double a, double b, double c ,int x){
    return a*x*x+b*x+c;
}


/*TLC read retry pattern*/
void init_TLC_state(struct state_bit *states)
{
    states->save = 0;
    states->er = 7;
    states->p1 = 3;
    states->p2 = 1;
    states->p3 = 5;
    states->p4 = 4;
    states->p5 = 0;
    states->p6 = 2;
    states->p7 = 6;
}

/*initialize MLC state*/
void init_MLC_state(struct state_bit *states)
{
    states->save = 0;
    states->er = 0;
    states->p1 = 1;
    states->p2 = 2;
    states->p3 = 3;
}

/*initialize TLC state*/
static int commit_TLC_state(struct state_bit *states, uint64_t bit)
{
    if(states->er == bit){
        return 0;
    }
    else if(states->p1 == bit){
        return 1;
    }
    else if(states->p2 == bit){
        return 2;
    }
    else if(states->p3 == bit){
        return 3;
    }
    else if(states->p4 == bit){
        return 4;
    }
    else if(states->p5 == bit){
        return 5;
    }
    else if(states->p6 == bit){
        return 6;
    }
    else if(states->p7 == bit){
        return 7;
    }
    else{
        ace_err("TLC bit error:%ld\n",bit);
        abort();
    }
}

/*initialize MLC state*/
static int commit_MLC_state(struct state_bit *states, uint64_t bit)
{
    if(states->er == bit){
        return 0;
    }
    else if(states->p1 == bit){
        return 1;
    }
    else if(states->p2 == bit){
        return 2;
    }
    else if(states->p3 == bit){
        return 3;
    }
    else{
        ace_err("MLC bit error:%ld\n",bit);
        abort();
    }
}


/*check TLC data*/
static uint64_t commit_TLC_data(struct state_bit *states, int state)
{
    if(state==0){
        return states->er;
    }
    else if(state==1){
        return states->p1;
    }
    else if(state==2){
        return states->p2;
    }
    else if(state==3){
        return states->p3;
    }
    else if(state==4){
        return states->p4;
    }
    else if(state==5){
        return states->p5;
    }
    else if(state==6){
        return states->p6;
    }
    else if(state==7){
       return states->p7;
    }
    else{
        ace_err("data error state: %d\n",state);
        abort(); 
    }
}

/*check MLC data*/
static uint64_t commit_MLC_data(struct state_bit *states, int state)
{
    if(state==0){
        return states->er;
    }
    else if(state==1){
        return states->p1;
    }
    else if(state==2){
        return states->p2;
    }
    else if(state==3){
        return states->p3;
    }
    else{
        ace_err("data error (state): %d\n",state);
        abort();
    }
}

/*initialize c_location*/
float set_c_location(float std, float mean)
{
    static float n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached) {
        float x, y, r;
        do {
            x = 2.0*rand()/RAND_MAX - 1;
            y = 2.0*rand()/RAND_MAX - 1;
            r = x*x + y*y;
        } while (r == 0.0 || r > 1.0);
        float d = sqrt(-2.0*log(r)/r);
        float n1 = x*d;
        n2 = y*d;
        float result = n1*std + mean;
        n2_cached = 1;
        return result;
    } else {
        n2_cached = 0;
        return n2*std + mean;
    }
}

/*trigger TLC error*/
static int TLC_check_error(uint64_t state, uint16_t wear_out, uint64_t retention_time, int erase_cnt, int read_cnt,float* vol, int index)
{
    double voltage = TLC_init_voltage[state];
    double moving_value;
    double cycle_cda;
    double time_cda;
    double read_cda;
    float wear_out_t;

    wear_out_t = ((float)wear_out)/100;

    /*set voltage*/

    voltage = voltage+(wear_out_t-100)*TLC_cell_cda[state]*TLC_init_voltage[3];

    /*P/E cycles shift*/
    moving_value =poly2(TLC_PE_a2[state],TLC_PE_b2[state],TLC_PE_c2[state],erase_cnt);
    cycle_cda = (wear_out_t*0.01)+(wear_out_t-100)*TLC_cycle_cda[state]*0.01;
    voltage = voltage+moving_value*cycle_cda;


    /*Retention error modeling[Unit's hour]*/
    moving_value = poly2(TLC_T_a2[state],TLC_T_b2[state],TLC_T_c2[state],retention_time);
    time_cda = (wear_out_t*0.01)+(wear_out_t-100)*TLC_time_cda[state]*0.01;

    if(moving_value < 0){
        voltage = voltage - moving_value*time_cda+2*moving_value;
    }
    else{
        voltage = voltage + moving_value*time_cda;
    }

 
    /*Read disturbnce modeling[Unit's read count]*/
    moving_value =poly2(TLC_R_a2[state],TLC_R_b2[state],TLC_R_c2[state],read_cnt);
    read_cda = (wear_out_t*0.01)+(wear_out_t-100)*TLC_read_cda[state]*0.01;
    if(moving_value < 0){
        voltage = voltage - moving_value*read_cda+2*moving_value;
    }
    else{
        voltage = voltage + read_cda*moving_value;
    }

    vol[index] = (voltage);
    // printf("%f %ld %d\n",vol[index],state,index);

    #if VOL_CHK
        fprintf(voltage_log->fd, "%lf\n",voltage);
    #endif

    /*RBER checker*/
    if(state == 0){
        if(voltage < TLC_reference_voltage[state])
            return 0;
        else{
            err++;
            return 1;
        }
    }
    else if(state == 7){
        if(voltage >= TLC_reference_voltage[state-1])
            return 0;
        else{
            err++;
            return -1;
        }
    }
    else{
        if(voltage >= TLC_reference_voltage[state-1] && voltage < TLC_reference_voltage[state])
            return 0;
        else if(voltage < TLC_reference_voltage[state-1]){
            err++;
            return -1;
        }
        else{
            err++;
            return 1;
        }
    }
}


/* Extract TLC bit value from heap-storage*/
/*You can trigger an error using this function. for each parameter
- buf: data pointer of 512 bytes
- PE_cnt: program/erase count
- retention_time: how long the page was saved
- read_cnt: read count
- wear_out: c_location pointer
- idx_wear_out: c_location index
- states: cell encoding values
- voltage: Pointer to store the voltage value*/
int TLC_nand_sec_error(uint64_t* buf, int PE_cnt, uint64_t retention_time, int read_cnt, uint16_t *wear_out,uint64_t idx_wear_out, struct state_bit *states,float *voltage)
{
   uint64_t bit_mask = 0x7; 
   uint64_t data;
   uint64_t state;
   uint64_t cell_data=0;
   int i=0;
   int index=0;
    
    for(int j =0; j < 64; j++){
            bit_mask = 0x7;
        while(i < 22){
                data=0;
                memcpy(&data,buf,8);
                data = (data & bit_mask);
                data = (data >> (i*3));
                state=commit_TLC_state(states,data);
                state += TLC_check_error(state,wear_out[idx_wear_out],retention_time+1,PE_cnt+1,read_cnt+1,voltage,index);
                cell_data |= (commit_TLC_data(states, state)<< i*3);
                bit_mask = bit_mask << 3;
                i++;
                idx_wear_out++;
                index++;
        }

          #if VOL_CHK
                fflush(voltage_log->fd);
            #endif

        memset(buf,0,8);
        memcpy(buf,&cell_data,8);
        cell_data=0;
        buf++;
        i=0;
    }
   return err;
}

/*trigger MLC error*/
static int MLC_check_error(uint64_t state, uint16_t wear_out, uint64_t retention_time, int erase_cnt, int read_cnt, float* vol, int index)
{
    double voltage = MLC_init_voltage[state];
    double moving_value;
    double time_cda;
    double read_cda;
    float wear_out_t;

    wear_out_t = ((float)wear_out)/100;

    /*set voltage*/
    voltage = voltage+(wear_out_t-100)*MLC_cell_cda[state]*TLC_init_voltage[3];

    /*P/E cycles shift*/
    moving_value = MLC_PE_value[state]*(erase_cnt);
    voltage = voltage+moving_value*((wear_out_t*0.01)+(wear_out_t-100)*MLC_cycle_cda[state]);
 

    /*Retention error modeling[Unit's hour]*/
    moving_value = MLC_retention_value[state]*(retention_time);
    time_cda = (wear_out_t*0.01)+(wear_out_t-100)*MLC_time_cda[state];
    if(moving_value < 0){
        voltage = voltage - moving_value*time_cda+ 2*moving_value;
    }

    else{
        voltage = voltage + moving_value*time_cda;
    }
  
    /*Read disturbnce modeling[Unit's read count]*/
    moving_value = MLC_read_disturbance_value[state]*log(read_cnt);
    read_cda = (wear_out_t*0.01)+(wear_out_t-100)*MLC_read_cda[state];
    if(moving_value < 0){
        voltage = voltage - moving_value*read_cda +2*moving_value;
    }
    else{
        voltage = voltage + read_cda*moving_value;
    }
     vol[index]=voltage;
     if(voltage > 1000){
    ace_err("%ld %lf %lf\n",state,wear_out_t,voltage);
     }

     #if VOL_CHK
        fprintf(voltage_log->fd, "%lf\n",voltage);
    #endif

    /*RBER checker*/
    if(state == 0){
        if(voltage < MLC_reference_voltage[state])
            return state;
        else{
            return state+1;
        }
    }
    else if(state == 1){
        if(voltage < MLC_reference_voltage[state-1]){
            return state-1;
        }
        else if(voltage >= MLC_reference_voltage[state]){
            return state+1;
        }
        else{
            return state;
        }
    }
    else if(state == 2){
        if(voltage < MLC_reference_voltage[state-1]){
            return state-1;
        }
        else if(voltage >= MLC_reference_voltage[state]){
            return state+1;
        }
        else{
            return state;
        }
    }
    else if(state == 3){
        if(voltage >= MLC_reference_voltage[state-1]){
            return state;
        }
        else{
            return state-1;
        }
    }
    else{
        ace_err("ACE error state:%ld \n",state);
        return -1;
    }
}


/* Extract MLC bit value from heap-storage*/
/*You can trigger an error using this function. for each parameter
- buf: data pointer of 512 bytes
- PE_cnt: program/erase count
- retention_time: how long the page was saved
- read_cnt: read count
- wear_out: c_location pointer
- idx_wear_out: c_location index
- states: cell encoding values
- voltage: Pointer to store the voltage value*/
int MLC_nand_sec_error(uint64_t* buf, int PE_cnt, uint64_t retention_time, int read_cnt, uint16_t *wear_out, uint64_t idx_wear_out, struct state_bit *states,float* voltage)
{
   uint64_t bit_mask = 0x3; 
   uint64_t data;
   uint64_t state;
   int i=0;
   uint64_t cell_data;
   int index=0;

    for(int j =0; j < 64; j++){
            bit_mask = 0x3;
        while(i < 32){
                data=0;
                memcpy(&data,buf,8);
                data = (data & bit_mask);
                data = (data >> (i*2));
                state=commit_MLC_state(states,data);              
                state=MLC_check_error(state,wear_out[idx_wear_out],retention_time+1,PE_cnt+1,read_cnt+1,voltage,index);
                cell_data |= (commit_MLC_data(states, state)<< i*2);
                bit_mask = bit_mask << 2;
                i++;
                idx_wear_out++;
                index++;
        }

          #if VOL_CHK
                fflush(voltage_log->fd);
            #endif

        memcpy(buf,&cell_data,8);
        cell_data=0;
        buf++;
        i=0;
    }

   return err;
}

/*MLC read retry*/
static int MLC_read_retry(float* voltage, int index, struct state_bit *states,float retry_count,int ref)
{


    if(voltage[index] < (MLC_reference_voltage[0]+mlc_read_retry[ref*3]*retry_count))
        {
           return states->er; 
        }        
    else if(voltage[index] < (MLC_reference_voltage[1]+mlc_read_retry[ref*3+1]*retry_count))
        {
            return states->p2;
        }
    else if(voltage[index] < (MLC_reference_voltage[2]+mlc_read_retry[ref*3+2]*retry_count))
        {
            return states->p2;
        }
    else
        {
            return states->p3;
        }
}

/*TLC read retry*/
static int TLC_read_retry(float* voltage, int index, struct state_bit *states,float retry_count,int ref)
{
    if(voltage[index] < (TLC_reference_voltage[0]+tlc_read_retry[ref*7]*retry_count))
        {
            // printf("%lf\n",TLC_reference_voltage[0]+tlc_read_retry[ref*7]*retry_count);
           return states->er; 
        }        
    else if(voltage[index] < (TLC_reference_voltage[1]+tlc_read_retry[ref*7+1]*retry_count))
        {
                // printf("%lf\n",TLC_reference_voltage[1]+tlc_read_retry[ref*7+1]*retry_count);
            return states->p1;
        }
    else if(voltage[index] < (TLC_reference_voltage[2]+tlc_read_retry[ref*7+2]*retry_count))
        {
                // printf("%lf\n",TLC_reference_voltage[2]+tlc_read_retry[ref*7+2]*retry_count);
            return states->p2;
        }

    else if(voltage[index] < (TLC_reference_voltage[3]+tlc_read_retry[ref*7+3]*retry_count))
        {
                // printf("%lf\n",TLC_reference_voltage[3]+tlc_read_retry[ref*7+3]*retry_count);
            return states->p3;
        }

    else if(voltage[index] < (TLC_reference_voltage[4]+tlc_read_retry[ref*7+4]*retry_count))
        {
                // printf("%lf\n",TLC_reference_voltage[4]+tlc_read_retry[ref*7+4]*retry_count);
            return states->p4;
        }

    else if(voltage[index] < (TLC_reference_voltage[5]+tlc_read_retry[ref*7+5]*retry_count))
        {
                // printf("%lf\n",TLC_reference_voltage[5]+tlc_read_retry[ref*7+5]*retry_count);
            return states->p5;
        }

    else if(voltage[index] < (TLC_reference_voltage[6]+tlc_read_retry[ref*7+6]*retry_count))
        {
                // printf("%lf\n",TLC_reference_voltage[6]+tlc_read_retry[ref*7+6]*retry_count);
            return states->p6;
        }
    else
        {
                // printf("else\n");
            return states->p7;   
        }
}

/*read retry method by using retry mapping table*/
void read_retry(uint64_t* buf,float* voltage, struct state_bit* states,int retry_count,int ref)
{
    uint64_t cell_data=0;
    int index = 0;
    int retry_data;
     for(int j =0; j < 64; j++){
        #if TLC_ERROR
            for(int i=0; i < 22; i++){
                retry_data = ((TLC_read_retry(voltage,index,states,(float)retry_count,ref))<< i*3);
                cell_data |= retry_data;
                index++;
            }
        #endif

        #if MLC_ERROR
            for(int i=0; i < 32; i++){
                cell_data |= (MLC_read_retry(voltage,index,states,(double)retry_count,ref)<< i*3);
                index++;
            }
        #endif

        memcpy(buf,&cell_data,8);
        memset(&cell_data,0,8);
        buf++;
        cell_data=0;
    }
    
}
