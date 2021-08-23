#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <lastlog.h>
#include <unistd.h>
#include <limits.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t signal = PTHREAD_COND_INITIALIZER;
struct timeval start_time, end_time;
long milli_time, seconds, useconds;
int scheduleType = 0; // DEFAULT = 0 FCFS, 1 = SJF, 2 = PRIO, 3= VRUNTIME//
int N, Bcount, minB, avgB, minA, avgA;
int bIndex = 0;
int fMode = 0;
int tVRUNTIME[] = {0,0,0,0,0,0,0,0,0,0,0,0};
char* filePath;
int data[300];
int waitingTime= 1;
int generatedBurst = 1;
typedef struct node {
    int burstIndex;
    int length;
    int wallClockTime;
    int threadIndex;
    struct node * next;
} queue;
queue * head = NULL;

void remove_data(int burstIndexToRemove) {

    if(head->burstIndex == burstIndexToRemove) { // IF INDEX = HEAD BURST
        printf("HEAD THiS REMOVE %d, %d, %d, %d REMOVED. treadInd-wclock-bIndx-lngth\n", head->threadIndex, head->wallClockTime, head->burstIndex, head->length);
        if(head->next != NULL) {
            struct node * cur = head;
            cur = head->next;
            free(head);
            head = cur;
            return;
        } else {
            free(head);
            head = NULL; //REMOVE HEAD
            return;
        }
    }
    struct node * prev = head;
    struct node * current = head;
    while(current->next != NULL && current->burstIndex != burstIndexToRemove) {
        prev = current;
        current = current->next;
    }
    if(current->burstIndex == burstIndexToRemove) {
        printf("THiS REMOVE %d, %d, %d, %d REMOVED. treadInd-wclock-bIndx-lngth\n", current->threadIndex, current->wallClockTime, current->burstIndex, current->length);
        prev->next = prev->next->next;
        free(current);
    } else
        printf("%d not found in the list.", burstIndexToRemove);
}

void* scheduling(){
    struct node * cur = head;
    if(scheduleType == 0){ //FCFS
        while(1) {
            if(head != NULL){
                printf("DELETED NODE\n");
                gettimeofday(&end_time, NULL);
                seconds = end_time.tv_sec - start_time.tv_sec;
                useconds = end_time.tv_usec - start_time.tv_usec;
                milli_time = (int)((seconds) * 1000 + useconds/1000.0);
                usleep(1000*(head->length));
                int b_millitime = head->wallClockTime;
                waitingTime = waitingTime + (milli_time - b_millitime);
                float av = (waitingTime / generatedBurst);
                printf(" Avagarage waiting time , %f", av);
                cur = head->next;
                free(head);
                head = cur;
                generatedBurst++;

            }else{
                printf("STHREAD IS WAITING SIGNAL\n");
                pthread_cond_wait(&signal, &mutex); //WAITING FOR SIGNAL HEAD IS NULL
            }
            //printf("İçerdema");

        }

    }else if( scheduleType == 1) { // SJF
        while(1) {
            cur = head;
            int min = INT_MAX;
            int bIndex = -1;
            if(cur == NULL){
                printf("STHREAD IS WAITING SIGNAL\n");
                pthread_cond_wait(&signal, &mutex); //WAITING FOR SIGNAL HEAD IS NULL
            }else{
                int b_millitime = 0;
                while(cur!=NULL){
                    if(min > cur->length){
                        bIndex = cur->burstIndex;
                        min = cur->length;
                         b_millitime = cur->wallClockTime;
                    }
                    cur = cur->next;
                }
                gettimeofday(&end_time, NULL);
                seconds = end_time.tv_sec - start_time.tv_sec;
                useconds = end_time.tv_usec - start_time.tv_usec;
                milli_time = (int)((seconds) * 1000 + useconds/1000.0);
                waitingTime = waitingTime + (milli_time - b_millitime);
                float av = (waitingTime / generatedBurst);
                printf(" Avagarage waiting time , %f\n", av);
                remove_data(bIndex);
                generatedBurst++;
                usleep(1000*min);
            }
        }
    }else if( scheduleType == 2) { // PRIO
        while(1) {
            cur = head;
            int min = INT_MAX;
            int bIndex = -1;
            int burstC = 0;
            if(cur == NULL){
                printf("STHREAD IS WAITING SIGNAL\n");
                pthread_cond_wait(&signal, &mutex); //WAITING FOR SIGNAL HEAD IS NULL
            }else{
                int b_millitime = 0;
                while(cur!=NULL){
                    if(min > cur->threadIndex)
                    {
                        bIndex = cur->burstIndex;
                        min = cur->threadIndex;
                        burstC = cur->length;
                        b_millitime = cur->wallClockTime;

                    }
                    cur = cur->next;
                }
                gettimeofday(&end_time, NULL);
                seconds = end_time.tv_sec - start_time.tv_sec;
                useconds = end_time.tv_usec - start_time.tv_usec;
                milli_time = (int)((seconds) * 1000 + useconds/1000.0);
                waitingTime = waitingTime + (milli_time - b_millitime);
                float av = (waitingTime / generatedBurst);
                printf(" Avagarage waiting time , %f\n", av);
                remove_data(bIndex);
                generatedBurst++;
                usleep(1000* burstC);
            }
        }

    }else if( scheduleType == 3) { //VRUNTIME
        while(1) {
            cur = head;
            int min = INT_MAX;
            int vRuntime = INT_MAX;
            int bIndex = -1;
            int burstC = 0;
            int tIndex = -1;
            if(cur == NULL){
                printf("STHREAD IS WAITING SIGNAL\n");
                pthread_cond_wait(&signal, &mutex); //WAITING FOR SIGNAL HEAD IS NULL
            }else{
                int b_millitime = 0;

                while(cur!=NULL){
                    int vTrntme = ((cur->threadIndex * 3) * tVRUNTIME[cur->threadIndex]) + (tVRUNTIME[cur->threadIndex]* 7);
                    if(vRuntime > vTrntme)
                    {
                        bIndex = cur->burstIndex;
                        vRuntime = vTrntme;
                        burstC = cur->length;
                        tIndex = cur->threadIndex;
                        b_millitime = cur->wallClockTime;

                    }
                    cur = cur->next;
                }
                tVRUNTIME[tIndex] = tVRUNTIME[tIndex] + burstC;
                gettimeofday(&end_time, NULL);
                seconds = end_time.tv_sec - start_time.tv_sec;
                useconds = end_time.tv_usec - start_time.tv_usec;
                milli_time = (int)((seconds) * 1000 + useconds/1000.0);
                waitingTime = waitingTime + (milli_time - b_millitime);
                float av = (waitingTime / generatedBurst);
                printf(" Avagarage waiting time , %f\n", av);
                remove_data(bIndex);
                generatedBurst++;
                usleep(1000*burstC);
            }
        }
    }
    printf("\n");
}
int exponentialRandomGenerator(int avg, int min){

    double result;
    double lm;
    do{
        lm = (double)((double)1/ (double)avg);
        result = rand() / (RAND_MAX + 1.0);
        result = -log(1 - result) / lm;
    }while(result < min);
    return (int) result;
}

void* genBurst(void* tIndex){
    int threadIndex = (int)tIndex;
    int ctr = 0;
    if(fMode == 1){
        printf("DENEDIM OLMADI \n");
        printf("%s", filePath);

        char str[3];
        sprintf(str, "%d", threadIndex);
        ctr = 0;
        printf("MERHABA GENBURST2 \n");

        char* file_name = strcpy(filePath, str);
        file_name = strcpy(file_name,".txt");
        printf("%s", file_name);
        printf("MERHABA GENBURST3 \n");

        FILE* file = fopen (file_name, "r");
        int i = 0;
        fscanf (file, "%d", &i);
        while (!feof (file))
        {
            data[ctr] = i;
            printf("%d, value \n", i);
            ctr++;
            fscanf (file, "%d", &i);
        }
        fclose (file);
     Bcount = (ctr+1) / 2;
    }

    for(int i = 0; i < Bcount; i++){
        pthread_mutex_lock(&mutex);
        int threadIndex = (int)tIndex;


        //printf("Burst entered other locked thread Index: %d\n", threadIndex);
        if(head == NULL){
            //printf("\nHEAD NULLDU VE BIZ GIRDIK\n");
            head = (queue* ) malloc(sizeof(queue));

            double timePass = 10;

            gettimeofday(&end_time, NULL);
            seconds = end_time.tv_sec - start_time.tv_sec;
            useconds = end_time.tv_usec - start_time.tv_usec;
            milli_time = ((seconds) * 1000 + useconds/1000.0);

            if(fMode == 1){
                head->length = data[2 * i];
            }else{
                head->length = exponentialRandomGenerator(avgB, minB);
            }
            head->burstIndex = bIndex++;
            if(threadIndex < 0 ){
                threadIndex = -threadIndex;
            }
            head->threadIndex = threadIndex;
            head->wallClockTime = (int)milli_time;
            head->next = NULL;
            printf("A new burst with %d, %d, %d, %d created. treadInd-wclock-bIndx-lngth\n", head->threadIndex, head->wallClockTime, head->burstIndex, head->length);

        }else{

            struct node * cur = head;
            while(cur->next != NULL){
                cur = cur->next;
            }
            double timePass = 10;

            gettimeofday(&end_time, NULL);
            seconds = end_time.tv_sec - start_time.tv_sec;
            useconds = end_time.tv_usec - start_time.tv_usec;
            milli_time = ((seconds) * 1000 + useconds/1000.0);

            cur->next = (struct node*) malloc(sizeof(struct node));
            cur = cur->next;
            cur->burstIndex = bIndex++;
            if(threadIndex < 0 ){
                threadIndex = -threadIndex;
            }
            cur->threadIndex = threadIndex;

            if(fMode == 1){
                head->length = data[2 * i];
            }else{
                cur->length = exponentialRandomGenerator(avgB, minB);
            }
            //printf("Time in Mill: %f \n",time_in_mill); GÜZEL ÇALIŞIYO
            cur->wallClockTime = (int)milli_time;
            //printf("%d Time.", tv.tv_usec);
            cur->next = NULL;
            printf("A new burst with %d, %d, %d, %d created. treadInd-wclock-bIndx-lngth\n", cur->threadIndex, cur->wallClockTime, cur->burstIndex, cur->length);

        }
        int await;
        if(fMode == 1){
            await = data[(2*i)+1]; //TODO
        }else{
           await = exponentialRandomGenerator(avgA, minA);
        }
        usleep(1000*await);
        pthread_cond_signal(&signal);
        pthread_mutex_unlock(&mutex);


    }
}
int main(int argc, char *argv[]){
    char* ALG;
    gettimeofday(&start_time, NULL);

    if(argc > 6){
        N = atoi(argv[1]);
        Bcount  = atoi(argv[2]);
        minB = atoi(argv[3]);
        avgB = atoi(argv[4]);
        minA = atoi(argv[5]);
        avgA = atoi(argv[6]);
        ALG = argv[7];
    }else {
        N = atoi(argv[1]);
        ALG = argv[2];
        fMode = 1;
        filePath = argv[4];
    }
    if(N > 10 || N < 1){
        printf("Number of W Threads count should be in the range between 1 and 10. It is assigned 10 automatically.\n");
        N = 10;
    }
    if(strcmp(ALG, "FCFS") == 0){
        scheduleType = 0;
    }
    if(strcmp(ALG, "SJF") == 0){
        scheduleType = 1;
    }
    if(strcmp(ALG, "PRIO") == 0){
        scheduleType = 2;
    }
    if(strcmp(ALG, "VRUNTIME") == 0){
        scheduleType = 3;
    }
    pthread_t sThread;
    pthread_create(&sThread, NULL, &scheduling, NULL);
    printf("%s , %d,\n", ALG, scheduleType);
    pthread_t wThreads[N];
    for(int i = 0; i < N; i++){
        int thIndex = i + 1;
        pthread_create(&wThreads[i], NULL, genBurst, (void *)thIndex);
        //printf("INDEX:%d", i);
    }
    for(int i = 0; i < N; i++){
        pthread_join (wThreads[i], NULL);
    }
    int i = 0;
    sleep(5);
    struct node * cur = head;
    if(cur == NULL){
        printf("Cur De null");
    }else{
        while(cur != NULL){
            i++;
            printf("\n%d th node: %d, %d, %d, %d", i, cur->threadIndex, cur->wallClockTime, cur->burstIndex, cur->length);
            cur = cur->next;
        }
    }
    pthread_join(sThread, NULL);
    return 0;
}
