/**
 * RP2040 FreeRTOS Template - App #3
 *
 * @copyright 2023, Tony Smith (@smittytone)
 * @version   1.4.2
 * @licence   MIT
 *
 */
#include "main.h"
#include <stdint.h>



/*
 * GLOBALS
 */

uint32_t start_time;

// Matrix
float a_matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS] = {4244.414350405894, 5386.011125484884, 209.27151037805984, 8320.633858162555, 6246.787365244573, 364.9568687340236, 4379.140825495792, 4239.739339063737, 1332.5864892241393, 6428.185470698703, 8275.31489946057, 4361.152446464416, 9997.324663646094, 3829.1363870631603, 4468.219803791731, 4847.224166623009, 4659.658753873815, 6029.631435831589, 8713.65656106301, 2977.203864675881, 137.47059570771677, 7601.802176426443, 3212.981130969402, 703.7599977848644, 7010.577989464238, 9727.002010306178, 9692.23770375902, 3025.135161237314, 408.3147451563641, 1252.3574162987122, 9936.807827926155, 6342.222264280734, 6006.698489510839, 5323.917755690009, 4186.801970407355, 3759.0382280510385, 4594.6335737781055, 7760.445862795432, 5818.354194675447, 2391.8765647717923, 316.195469085565, 3864.2008937817063, 92.6020331307634, 3098.251327350124, 2180.593113148481, 6910.716011414083, 8909.143774944805, 6999.64915005546, 3773.8421633976277, 7586.529047241985, 2275.3712431212903, 4806.341010845052, 9424.060842563313, 7226.860724966912, 7704.2996257261075, 8610.378748365669, 4249.206385305406, 6872.224245850335, 4885.346713298899, 2548.2575756377596, 6817.499606726475, 2954.9514347214313, 4811.364025533253, 7688.4476901840735};
float b_matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS] = {4572.641279498933, 8.18036431756791, 8331.90490424325, 4135.664619940735, 6693.260703226198, 390.29356497130766, 6753.196634729926, 5025.29504545816, 8643.691680160538, 1053.5940775421739, 685.621654203448, 5957.8980649292025, 7624.922057713783, 8472.496042694209, 9107.220024660664, 2966.7856087199807, 436.31513882638706, 5990.180013060972, 1509.3299065697363, 9787.421144383065, 3880.668947262626, 457.12859487265086, 3155.9908343953075, 23.571574014576775, 8220.290903423376, 2826.684628687179, 606.83857022858, 4866.280129069892, 3959.4833650658898, 572.9915699214148, 6018.920378207957, 4538.905717367602, 4912.555505190607, 4818.607785033592, 8047.306681840761, 9937.527075443257, 9713.419197855666, 2454.049889116547, 7066.097882870245, 6734.662110088401, 3715.5045825479488, 126.24572749384673, 8599.322429507007, 9381.23571671897, 9811.387153084463, 6252.459469172916, 4163.647911663322, 6100.864529277618, 6031.222075617114, 6023.570146152305, 8902.914629542895, 1695.047743579125, 2681.6251276077855, 1004.7779062662453, 5940.716276889315, 9081.906622350245, 8169.811586739108, 3576.261927292827, 9673.71969602524, 2958.4093572588463, 4198.927079095015, 851.5780782625499, 5403.391460875896, 2670.361323186919, 1863.0353886772514, 1455.8430461073108, 1221.7776969130455, 9909.848018237828, 6768.687207270927, 742.7891477663549, 7555.061140294844, 3849.993398176527, 297.9132700034249, 3355.734033771178, 4188.176454573078, 1127.7371182256609, 4838.361263965679, 5990.179665376204, 4912.511410362054, 4196.732633710848, 4408.1109074363885, 7798.517603678637, 4060.4212125665567, 9148.79779016715, 7897.448415857617, 2824.3669354007434, 3597.759314147984, 526.3408117360397, 5000.060710940836, 6750.319566600349, 5915.02323482885, 9808.474289722119, 3579.712793363508, 2822.330374281433, 9847.958790307412, 8700.421372071163, 181.93014729494575, 9128.836268518033, 1996.3044663836981, 7688.802715582583, 3137.1405485761848, 4569.911985930213, 2641.1677733827455, 3297.4175157589775, 8790.512341161406, 925.6980397509166, 2026.8082015241507, 6917.9683009559485, 1324.31138622342, 6736.227571892404, 9494.996529038834, 6202.455908559764, 4059.0914904975702, 637.9371712127597, 2632.3632551030487, 4365.219113617087, 8391.050397400588, 3448.4662766329957, 7347.308469330576, 1673.033938507273, 9476.987887610398, 3414.5201463405883, 9080.654958007573, 1441.4564955157045, 2879.09376141868, 5545.893289703927, 422.2831810508104, 1352.6875997554146, 1984.8745697271138, 9609.035651868702, 1706.0194322123891, 8563.930969937437, 3302.656508467116, 3460.174518870282, 3756.4110660368174, 1754.876614980667, 5918.808815527773, 7572.437706879628, 159.1175636113991, 6251.510712551017, 2242.484196714224, 5348.197359326709, 6719.17919661516, 1445.6050571612514, 6760.7499040018065, 2056.1253762549186, 77.29092788560128, 8979.380146730076, 9082.485157837367, 4769.253064697445, 1288.9122237569006, 7374.312554903791, 9777.246926288444, 542.4101839255562, 8258.271344618406, 6556.493314023635, 3124.880636519097, 2556.928413925972, 9139.293060231716, 5467.633649634611, 2694.1231004985384, 5474.821723669, 6748.665113990943, 4493.803507214838, 425.2434728378137, 3748.241070702191, 424.8662931014783, 3096.674267640271, 6051.159119656781, 6565.508877776821, 8637.948514457312, 3831.7211206245975, 2272.4485924021933, 4940.1985291958945, 9100.277909702489, 791.8115449155864, 1423.9445965536577, 8051.600262735728, 6204.770878551079, 5178.320806021114, 680.4948383092627, 8990.716931057, 6593.38751087975, 2402.48796089594, 3829.7372421468854, 2664.7638963633426, 6079.999869963815, 254.57960158328805, 5099.178957903915, 4740.501402196534, 5494.288153609383, 7314.501569163627, 4992.379416918124, 8482.31654892792, 1151.5017873129636, 5359.851654417237, 1943.7627374060442, 9717.984041237221, 8157.1761065363025, 7669.578989647811, 7436.971380693646, 557.9682428428927, 6206.764836663656, 66.4969533286982, 9396.323527893937, 4709.6840150708085, 4339.95571175665, 6872.7654487387545, 6661.604084607832, 9471.520983710423, 1890.5624941962935, 8865.087706125261, 7067.421509630767, 2757.5951502261046, 6961.463029579566, 8243.447132022611, 1102.8338038819893, 6022.311716006555, 4204.458160123923, 813.6546487343855, 4315.796459497669, 18.75824588050101, 629.8754356717806, 6857.325710116666, 4327.770864122436, 1648.9482219312763, 3681.2878330784292, 4740.627087466245, 5237.464402547813, 2267.382084345088, 8696.474036807613, 716.9534396209875, 1280.4897450643875, 6868.674748055055, 3395.291698760254, 2087.531736871806, 1340.9952422498686, 3267.2926633540424, 3564.1414536698167, 4806.421033975836, 5112.496261041656, 2188.8963459248703, 986.8116878762635, 7743.365880994239, 4569.801380339675, 3634.657384695782, 7728.277015552925, 6768.289351684366, 8039.473863223249, 3043.97517752829, 1366.7385889051411, 9970.557934409271, 601.4133760240228, 5088.566211715131, 3300.6692747281118, 4300.749707932765, 8276.55826128941, 1501.3107567041195, 4001.6889987965874, 3612.052317425966, 7498.811367224816, 5459.219669781638, 2085.969142521534, 9029.14875306537, 5160.396330494527, 8447.930238614881, 6001.810600247014, 1509.2010374400013, 6661.841326235457, 6230.18404430653, 8616.80515788306, 3632.306768232865, 371.80486037593124, 7098.375905471423, 9544.143927597559, 4167.934105838591, 2234.127448240763, 3563.9430333147398, 2632.578719711881, 3448.3131279467784, 5470.905189722554, 6110.457874402688, 504.28066561204986, 5743.494653831397, 5746.836185149126, 1853.2423497484317, 5028.545448540018, 6986.354071026756, 2061.968898331788, 7343.346428182251, 9720.14114992389, 4139.7163607056455, 4365.83640080932, 1704.360589535326, 4584.647344492374, 9785.832643361036, 8703.431185988155, 7106.959404969398, 9765.71710491411, 2382.944029895367, 8310.734599923473, 6585.9522907122955, 5439.289048481407, 582.2122407577245, 6520.103712974699, 2860.8594799736807, 3520.767349038576, 6252.239829204143, 638.5537797959637, 3113.794675326393, 5997.633378152925, 1714.5037737758846, 5340.600305240786, 2654.758613836655, 4668.84570821144, 3971.7548601318313, 8847.91513940805, 2446.9798377243715, 2107.011197971192, 8299.470389555283};

float volatile result_matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS];


// Tick counters IDLE
uint64_t xTimeInPICO, xTimeOutPICO, xDifferencePICO, xTotalPICO;

// Capacity
int capacity_task_i_row = 0;
int CAPACITY = 250;

// Static allocation task_i_row

/* Dimensions of the buffer that the task being created will use as its stack.
NOTE:  This is the number of words the stack will hold, not the number of
bytes.  For example, if each stack item is 32-bits, and this is set to 100,
then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE_I_ROW 200
//#define REFERENCE
#ifndef REFERENCE
/* Structure that will hold the TCB of the task being created. */
StaticTask_t task_buffer_i_row[RESULT_MATRIX_ROWS];
StackType_t stack_i_row[RESULT_MATRIX_ROWS][ STACK_SIZE_I_ROW ];
#else
StaticTask_t task_buffer_reference;
StackType_t stack_reference[ STACK_SIZE_I_ROW ];
#endif
volatile uint32_t largest_stack = ~0;
uint32_t START_STACK = 0;

void tick() {
    uint32_t current = __get_MSP();
    if (current < largest_stack) {
        largest_stack = current;
    }
}

uint32_t task0start = 0;

#ifdef REFERENCE
void reference_task(void *parameters) {
    task0start = time_us();
    for (int i = 0; i < RESULT_MATRIX_ROWS; i++) {
        for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
            float tmp = 0.0;
            for (int k = 0; k < A_MATRIX_COLUMNS; k++) {
                tmp = tmp + (a_matrix[(i * A_MATRIX_COLUMNS) + k] * b_matrix[(k * B_MATRIX_COLUMNS) + j]);
            }
            result_matrix[(i * RESULT_MATRIX_COLUMNS) + j] = tmp;
        }

    }
    uint32_t end_time = time_us();
    //TickType_t end = xTaskGetTickCount();
    //printf_("End_time FreeRTOS: %u\n\r", end);
    //printf_("End_time: %u\n\r", end_time - start_time);
    //tick();
    //printf_("Stack usage: %u\r\n", START_STACK - largest_stack);
    printf_("%u\n", end_time - task0start);
    vTaskDelay(500);
    NVIC_SystemReset(); 
    //print_result_matrix(result_matrix);
    vTaskEndScheduler();
 
}

#else
void task_i_row(void *parameter) {
    int i;
    i = (int) parameter;
    //printf_("Task %d\n\r", i);
    tick();
    if (i == 0) {
        task0start = time_us();
    }

    for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
        float tmp = 0.0;
        for (int k = 0; k < A_MATRIX_COLUMNS; k++) {
            tick();
            tmp = tmp + (a_matrix[(i * A_MATRIX_COLUMNS) + k] * b_matrix[(k * B_MATRIX_COLUMNS) + j]);
        }
        result_matrix[(i * RESULT_MATRIX_COLUMNS) + j] = tmp;
    }

    tick();

    capacity_task_i_row = capacity_task_i_row - 1;
    if (capacity_task_i_row == 0) {
        uint32_t end_time = time_us();
        //TickType_t end = xTaskGetTickCount();
        //printf_("End_time FreeRTOS: %u\n\r", end);
        //printf_("End_time: %u\n\r", end_time - start_time);

        printf_("%08x\n", largest_stack);
        //printf_("%u\n", end_time - task0start);
        //print_result_matrix(result_matrix);
        vTaskDelay(500);
        NVIC_SystemReset(); 

        vTaskEndScheduler();
    }

    vTaskSuspend(NULL);

    while(1){
        taskYIELD();
    }
}
#endif


/*
 * RUNTIME START
 */

int main() {
    START_STACK = __get_MSP();

    enable_timer();

    //printf_("A_ROW %d A_COL %d\n\r",A_MATRIX_ROWS, A_MATRIX_COLUMNS);
    //printf_("B_ROW %d B_COL %d\n\r",B_MATRIX_ROWS, B_MATRIX_COLUMNS);
    //printf_("RESULT_ROW %d RESULT_COL %d\n\r",RESULT_MATRIX_ROWS, RESULT_MATRIX_COLUMNS);
    //print_a_matrix(a_matrix);
    //print_b_matrix(b_matrix);

    #ifndef REFERENCE
    char buf[5];
    for (int i = 0; i < RESULT_MATRIX_ROWS; i++) {
        // Convert 123 to string [buf]
        capacity_task_i_row++;
        sprintf_(buf,"%i", i);
        xTaskCreateStatic(task_i_row,
                        buf,
                        128,
                        (void *) i,
                        1,
                        stack_i_row[i],
                        &task_buffer_i_row[i]);
        //printf_("Created task %d\n\r", i);
    }
    #else
    xTaskCreateStatic(reference_task,
        "reference",
        128,
        NULL,
        1,
        stack_reference,
        &task_buffer_reference
    );
    #endif

            
    // Start the FreeRTOS scheduler if any of the tasks are good
    start_time = time_us();
    // Start the scheduler
    vTaskStartScheduler();

    // We should never get here, but just in case...
    while(1) {
        // NOP
    };
}

/* 
###############################################
    HELPER FUNCTIONS
###############################################
*/

/*
* PRINTS
*/

void print_result_matrix(float matrix[RESULT_MATRIX_ROWS * RESULT_MATRIX_COLUMNS]) {
    printf_("Matrix: \n\r");

    for (int i = 0; i < RESULT_MATRIX_ROWS; i++) {
        printf_("[");
        for (int j = 0; j < RESULT_MATRIX_COLUMNS; j++) {
            if (j == (RESULT_MATRIX_COLUMNS - 1)) {
                printf_("%f", matrix[(i * RESULT_MATRIX_COLUMNS) + j]);
            } else {
                printf_("%f, ", matrix[(i * RESULT_MATRIX_COLUMNS) + j]);
            }
        }
        printf_("]\n\r");
    }
}

void print_a_matrix(float matrix[A_MATRIX_ROWS * A_MATRIX_COLUMNS]) {
    printf_("Matrix A: \n\r");

    for (int i = 0; i < A_MATRIX_ROWS; i++) {
        printf_("[");
        for (int j = 0; j < A_MATRIX_COLUMNS; j++) {
            printf_("%f , ", matrix[(i * A_MATRIX_COLUMNS) + j]);
        }
        printf_("]\n\r");
    }
}

void print_b_matrix(float matrix[B_MATRIX_ROWS * B_MATRIX_COLUMNS]) {
    printf_("Matrix B: \n\r");

    for (int i = 0; i < B_MATRIX_ROWS; i++) {
        printf_("[");
        for (int j = 0; j < B_MATRIX_COLUMNS; j++) {
            printf_("%f , ", matrix[(i * B_MATRIX_COLUMNS) + j]);
        }
        printf_("]\n\r");
    }
}


/*
* PERFORMANCE FUNCTIONS 
*/
/**
 * @brief Handler for when tasks switch in.
 */
void handle_switched_in(int* pxCurrentTCB) {
    TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
     if (handle == xTaskGetIdleTaskHandle()) {
        xTimeInPICO = time_us();
        //printf_("Switched in to IDLE");
     }
}

/**
 * @brief Handler for when tasks switch out.
 */
void handle_switched_out(int* pxCurrentTCB) {
    TaskHandle_t handle = (TaskHandle_t)*pxCurrentTCB;
     if (handle == xTaskGetIdleTaskHandle()) {
        char str[64];
        xTimeOutPICO = time_us();
        xDifferencePICO = xTimeOutPICO - xTimeInPICO;
        xTotalPICO = xTotalPICO + xDifferencePICO;
        sprintf_(str, "IDLE time: %lu", xDifferencePICO);
        //printf_(str);
        xTimeInPICO = xTimeOutPICO = xDifferencePICO = 0;
     }
}

/**
 * @brief Callback actioned when CPU usage timer fires.
 *
 * @param timer: The triggering timer.
 */
void task_cpu_usage(TimerHandle_t timer) {

    char str[64];
    int average_time = (AVERAGE_USAGE_INTERVAL_MS*1000);
    float usage = ( (float)average_time - (float)xTotalPICO) / (float)average_time;

    xTotalPICO = 0;

    sprintf_(str, "CPU USAGE: %f, BACKGROUND TASKS: %u", usage,capacity_task_i_row);
    printf_(str);    
}

/*
 * FREERTOS 
 */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
 
    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
 
    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
 
    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
