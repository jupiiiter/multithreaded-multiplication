#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h> 
#include <sys/time.h> 

int i=0;
int j=0;
int k=0;
pthread_t threads[400];
struct timeval stop, start;

struct dimension {
    int row;
    int column;
};
struct dimension dimensiona, dimensionb, dimensionc;
int mata [20][20]={[0 ... 19][0 ... 19] = 989};
int matb [20][20]={[0 ... 19][0 ... 19] = 989};
int matc [20][20]={[0 ... 19][0 ... 19] = 989};

void get_input(int argc, char **argv){  
    FILE *file1, *file2;

    if (argc == 1){
        file1 = fopen("a.txt", "r"); //
        file2 = fopen("b.txt", "r");
    }
    else if (argc == 2){
        file1 = fopen((const char* restrict) argv[1], "r");
        file2 = fopen("b.txt", "r");
    }
    else if (argc >2){
        file1 = fopen((const char* restrict) argv[1], "r");
        file2 = fopen((const char* restrict) argv[2], "r");
    }
    
    if (file1 == NULL | file2 == NULL){
        printf("error : file not found \n");
        exit(1);
    }
    char linea[256];
    char lineb[256];

    fgets(linea, 256, file1); 
    fgets(lineb, 256, file2);
    char *number =  &linea[4];
    dimensiona.row = atoi(number);
    int digits = snprintf(0,0,"%+d", dimensiona.row)-1;
    number =  &linea[9+digits];
    dimensiona.column = atoi(number);
    number =  &lineb[4];
    dimensionb.row = atoi(number);
    digits = snprintf(0,0,"%+d", dimensionb.row)-1;
    number =  &lineb[9+digits];
    dimensionb.column = atoi(number);
    if (dimensiona.column != dimensionb.row){
        printf("error : a.column %i needs to be the same as b.row %i \n",dimensiona.column,dimensionb.row);
        exit(1);
    }

    char* element;
    for(i=0;i<dimensiona.row ;i++){
        fgets(linea, 256, file1);
        j = 0;
       element = strtok(linea, "	");
        while (element != NULL){
            
            mata[i][j] = atoi(element) ;
            element = strtok (NULL, "	");
            j++;
        }
     }


     for(i=0;i<dimensionb.row ;i++){
        fgets(lineb, 256, file2);
        j = 0;
       element = strtok(lineb, "	");
        while (element != NULL){
            matb[i][j] = atoi(element) ;
            element = strtok (NULL, "	");
            j++;
        }
     }
    dimensionc.row = dimensiona.row;
    dimensionc.column = dimensionb.column;
    fclose(file1);
    fclose(file2);
    printf("get_input finished , second elements : %i , %i \n output dimennsions : %ix%i \n\n",mata[0][1],matb[0][1],dimensionc.row,dimensionc.column);
}

void write_output(int argc, char* outputfile){
    FILE *file3;
    if (argc >= 4){
        file3 = fopen((const char* restrict) outputfile,"w");
    }
    else {
        file3 = fopen("c.txt","w");
    }
    
    fprintf(file3, "row=%d col=%d\n",dimensionc.row, dimensionc.column);
    for(i=0;i<dimensionc.row;i++){
        for(j=0;j<dimensionc.column;j++){
            fprintf(file3, "%d", matc[i][j]); 
            (j != dimensionc.column - 1) ? fprintf(file3, "  ") : (void)0;
        }
        (j != dimensionc.row - 1) ? fprintf(file3, "\n") : (void)0;
    }
    fclose(file3);
}

 void nothreads(){
    gettimeofday(&start, NULL); 
    for(i=0;i<dimensionc.row;i++){
        for(j=0;j<dimensionc.column;j++){
            matc[i][j] = 0;
            for(k=0;k<dimensionb.row;k++){
                matc[i][j] += mata[i][k] * matb[k][j];
                 }
        }
    }
    gettimeofday(&stop, NULL); 

    printf("Method 1 no threads \n");
    printf("    Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("    Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
}




void* t_perrow( void * irow){ 
    int i2 = (int *) irow; 
    int j2=0;
    int k2=0;
    for(j2=0;j2<dimensionc.column;j2++){
        matc[i2][j2] = 0; 
        for(k2=0;k2<dimensionb.row;k2++){
                matc[i2][j2] += mata[i2][k2] * matb[k2][j2]; 
        }
    }
    pthread_exit(NULL);
    return NULL;
}
void threadperrow(){
    
    gettimeofday(&start, NULL); 
    pthread_t rowthreads[dimensionc.row];
    int errorcode;
    for(i=0;i<dimensionc.row;i++){
       errorcode = pthread_create(&rowthreads[i], NULL, t_perrow,(void *) i);
       if (errorcode){
        printf("ERROR; return code from pthread_create() is %d\n", errorcode);
        exit(-1);
        }
    }

    for(i=0;i<dimensionc.row;i++){
        pthread_join(rowthreads[i], NULL); 
    }
    gettimeofday(&stop, NULL); 

    printf("Method 2 thread for each row  \n");
    printf("    Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("    Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("    number of threads %d \n",dimensionc.row);
} 

void* t_perelement( void * threadstruct){ 
    struct dimension *structure ;
    structure = (struct dimension *) threadstruct;
    int ie =  structure->row; 
    int je=  structure->column;
    int ke=0;
    
    matc[ie][je] = 0; 
    for(ke=0;ke<dimensionb.row;ke++){
         matc[ie][je] += mata[ie][ke] * matb[ke][je];
    }

    pthread_exit(NULL);
    return NULL;
}
void threadperelement(){
    struct dimension *threadstruct = malloc(sizeof(struct dimension)); 
    int errorcode;
    gettimeofday(&start, NULL); 
    pthread_t elementthreads[dimensionc.row][dimensionc.column];
    for(i=0;i<dimensionc.row;i++){
        threadstruct->row = i;
        for(j=0;j<dimensionc.column;j++){
            threadstruct->column = j;
            errorcode = pthread_create(&elementthreads[i][j], NULL, t_perelement,(void *) threadstruct );
            if (errorcode){
            printf("ERROR; return code from pthread_create() is %d\n", errorcode);
             exit(-1);
        }
        }
    }
    for(i=0;i<dimensionc.row;i++){
        for(j=0;j<dimensionc.column;j++){
            pthread_join(elementthreads[i][j], NULL); 
        }
    }
    gettimeofday(&stop, NULL); 

    printf("Method 3 thread for each element \n");
    printf("    Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("    Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("    number of threads %d \n",dimensionc.row*dimensionc.column); 
    }

int main(int argc, char *argv[]){
    printf("program started \n");
    get_input(argc, argv); 
    nothreads();
    write_output(argc, argv[3]);
    threadperrow();
    threadperelement();
    printf("program finished \n");
    return 0;
}
