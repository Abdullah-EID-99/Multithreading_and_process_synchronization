#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

//****************structs****************tou*
typedef struct Philosopher
{
    /* data */
    int id;
    int thinking_time;
    int eating_time;
    int index_of_ph_in_tb;
    int table_id;
    void* status;
    int amount_of_rice_this_ph_eat;
    pthread_t dining_thread;
    pthread_mutex_t lock;
}ph;

typedef struct Table
{
    /* data */ 
    int id;
    int riceAmount;
    int eaten_rice_amount;
    int reorder_amount;
    //int empty_chair_count;
    int chair_index;
    int chairs[8];
    //ph philosophers[8];
    int finished_count;//yiyenlerin sayisi
    int is_opend;
    double receipt;
    pthread_mutex_t full_lock;
    pthread_mutex_t finish_lock;
    pthread_mutex_t reorder_lock;
    pthread_mutex_t eat_lock;
    pthread_mutex_t fork_lock;
    pthread_mutex_t print_lock;

}tb;
//****************structs*****************

//************global variables************
#define tb_count 10 //10 masa
#define chairs_count 8 //8 sandalya
#define rice_that_can_be_eaten_per_time 100 //100gr
#define rice_price 20
double open_new_table_price = 99.90;
double clear_table_price = 19.90;
int group_count;
int place_capacity;
int ph_count;
ph* ph_list;
tb* tb_list;
pthread_mutex_t find_table_lock;
pthread_mutex_t print_receipt_lock;
pthread_mutex_t enter_table_lock;
sem_t place_lock;
//************global variables************

//**************fuctions pre**************
ph ph_creat(int);
tb tb_creat(int);
void ph_print(ph*);
void tb_print(tb*);
void* ph_enter_DinoPhilo(int);
int ph_enter_tb(int);
void tb_prepare(tb*);
void* ph_eat(int,int);
void* ph_think(int);
int get_hungry_ph(tb*);
void* is_tb_open();
void tb_reorder(tb*);
void destroy_threads(tb*, int);
void tb_close(tb*);
//**************fuctions pre**************



/*main_main_main_main_main_main_main_main_main_main_main_main_main_main_main_main*/
int main(int argc, char *argv[], char** envp){
    printf("Please enter group count : ");
    scanf("%d",&group_count);
    ph_count = 8 * group_count;
    tb_list = (tb*)calloc(tb_count,sizeof(tb));
    ph_list = (ph*)calloc(ph_count, sizeof(ph));
    place_capacity = tb_count * chairs_count;
    
    void* status;
    int i;
    for(i=0;i<ph_count;i++){
        ph_list[i]=ph_creat(i);
    }
    for(i=0;i<tb_count;i++){
        tb_list[i]=tb_creat(i);
    }
    if( pthread_mutex_init(&enter_table_lock,NULL) != 0){
        printf("there is an error occurs in pthread_mutex_init(enter_table)\n"); 
        return 1;   
    }
    if(sem_init(&place_lock, 0,place_capacity) != 0){
        printf("there is an error occurs in sem_init()\n");    
        return 1;   
    }
    if( pthread_mutex_init(&print_receipt_lock,NULL) != 0){
        printf("there is an error occurs in pthread_mutex_init(print_receipt_lock)\n");  
    }
    
    for(i=0;i<ph_count;i++){
        pthread_create(&ph_list[i].dining_thread, NULL,ph_enter_DinoPhilo, i);
    }
    for(i=0;i<ph_count;i++){
        pthread_join(ph_list[i].dining_thread, &ph_list[i].status);
    }

    pthread_mutex_destroy(&enter_table_lock);
    sem_destroy(&place_lock);
    //for(i=0;i<tb_count;i++){
        //tb_print(&tb_list[i]);
    //}
    
    exit(0);
    return 0;

}
/*main_main_main_main_main_main_main_main_main_main_main_main_main_main_main_main*/

/**********************************functions**********************************/
/**********************************functions**********************************/
/**********************************functions**********************************/
/**********************************functions**********************************/
void ph_print(ph* p){
    printf("<Philosopher: %d have eaten: %dgr of rice>\n",p->id,p->amount_of_rice_this_ph_eat);
}
void tb_print(tb* t){
    printf("****************** table %d *******************\n",t->id);
    printf("table | id:%d | seek:%d\n",t->id,t->chair_index);
    
    int i;
    for(i=0;i<chairs_count;i++){
        ph_print(&ph_list[t->chairs[i]]);    
    }
    printf("rice amount: %d\n",t->riceAmount);
    printf("eaten rice amount: %.2fkg\n",t->eaten_rice_amount/1000.0);
    printf("price: %.2f\n",t->receipt);
    printf("reorder_amount: %d\n",t->reorder_amount);
    printf("**************** table %d end *****************\n",t->id);
}
ph ph_creat(int id){
    ph p;
    p.id = id;
    p.amount_of_rice_this_ph_eat=0;
    p.eating_time=1;
     if( pthread_mutex_init(&p.lock,NULL) != 0){
        printf("there is an error occurs in pthread_mutex_init(p.lock)\n");  
    }
    pthread_mutex_lock(&p.lock);
    time_t t;
    srand((unsigned)time(&t));
    p.thinking_time=rand()%5;
    return p;
}
tb tb_creat(int id){
    tb t;
    t.id = id;
    t.is_opend=0;
    t.chair_index=-1;
    t.eaten_rice_amount=0;
    t.reorder_amount =0;
    if( pthread_mutex_init(&t.fork_lock,NULL) != 0){
        printf("there is an error occurs in pthread_mutex_init(fork_lock)\n");  
    }
    if( pthread_mutex_init(&t.reorder_lock,NULL) != 0){
        printf("there is an error occurs in pthread_mutex_init(reorder_lock)\n");  
    }
    if( pthread_mutex_init(&t.print_lock,NULL) != 0){
        printf("there is an error occurs in pthread_mutex_init(print_lock)\n");  
    }
    return t;
}
int get_hungry_ph(tb* t){
    int i;
    for(i=0;i<chairs_count;i++){
        if(ph_list[t->chairs[i]].amount_of_rice_this_ph_eat==0){
            return 1;
        } 
    }
    return 0;
}
void tb_prepare(tb* t){
    t->riceAmount=2000;
    t->receipt=open_new_table_price;
    t->receipt+=( t->riceAmount/1000.0)*rice_price;
    t->receipt+=clear_table_price;
}
void tb_reorder(tb* t){
    t->riceAmount=2000;
    t->receipt+=( t->riceAmount/1000.0)*rice_price;
    t->receipt+=clear_table_price;
    t->reorder_amount +=1;
}
void* ph_eat(int tb_id, int ph_id){
    
    ph_list[ph_id].amount_of_rice_this_ph_eat +=100;
    tb_list[tb_id].riceAmount -=100;
    tb_list[tb_id].eaten_rice_amount +=100;
      
}
void tb_close(tb* t){
    t->riceAmount=0;
    t->receipt=0;
    t->reorder_amount =0;
    t->eaten_rice_amount =0;
}
void* ph_think(int id){
    sleep(ph_list[id].thinking_time);
}
int ph_enter_tb(int id){
    //pthread_mutex_lock(&enter_table_lock);
    int i, table_id,k;
    for(i=0;i<tb_count;i++){
        if(!tb_list[i].is_opend){
            table_id=i;
            int index =tb_list[i].chair_index+1;
            ph_list[id].index_of_ph_in_tb=index;
            ph_list[id].table_id=table_id;
            tb_list[i].chairs[index]=id;
            //printf("%d. ph'losof enter table %d and setdown on %d chair\n", tb_list[i].chairs[index],i,index);
            if(index == chairs_count-1){
                 tb_list[i].chair_index=-1;
                 tb_list[i].is_opend=1;
                 printf("%d. table is full\n",i);
                 tb_prepare(&tb_list[i]);
                 //tb_print(&tb_list[i]);
                 for(k=0;k<chairs_count;k++){
                    int ph_id=tb_list[i].chairs[k];
                    pthread_mutex_unlock(&ph_list[ph_id].lock);
                    //printf("unlock ph %d\n",ph_id);
                }
                //pthread_mutex_unlock(&tb_list[table_id].print_lock);
                //sleep(1);
            }else{
                tb_list[i].chair_index++;
            }
            break;     
        }  
    }
    return table_id;
     //sleep(1);
    //pthread_mutex_unlock(&enter_table_lock);
}
void* ph_enter_DinoPhilo(int id){
    sem_wait(&place_lock);
    
    pthread_mutex_lock(&enter_table_lock);
    int table_id= ph_enter_tb(id);
    pthread_mutex_unlock(&enter_table_lock);

     if(!tb_list[table_id].is_opend){
        pthread_mutex_lock(&ph_list[id].lock);
    }
    //printf("sdssasdasdsdasdsa\n");
    if(tb_list[table_id].is_opend && table_id==ph_list[id].table_id){
        pthread_mutex_lock(&tb_list[table_id].eat_lock);
        pthread_mutex_unlock(&ph_list[id].lock);
        while(tb_list[table_id].riceAmount>0){
            pthread_mutex_lock(&ph_list[id].lock);
            pthread_mutex_lock(&tb_list[table_id].fork_lock);
            if(tb_list[table_id].riceAmount==0 && get_hungry_ph(&tb_list[table_id])==1){
                        tb_reorder(&tb_list[table_id]);
            }
             else if(tb_list[table_id].riceAmount==0 && get_hungry_ph(&tb_list[table_id])==0){
                destroy_threads(&tb_list[table_id], id);
                break;
             }
             //sleep(1);
     
             ph_eat(table_id ,id);
              if(tb_list[table_id].riceAmount>0 && get_hungry_ph(&tb_list[table_id])==0){
                        int k;                         
                        for(k=0;k<chairs_count;k++){
                            int ph_id=tb_list[table_id].chairs[k];
                            pthread_mutex_unlock(&ph_list[ph_id].lock);
                        }
            }
            pthread_mutex_unlock(&tb_list[table_id].fork_lock);
            pthread_mutex_unlock(&tb_list[table_id].eat_lock);
            ph_think(id);
            //sleep(0);  
        }
    }

    
    pthread_mutex_lock(&tb_list[table_id].print_lock);
    tb_print(&tb_list[table_id]);

    sem_post(&place_lock);
    //pthread_exit(0);
    return NULL;
}

void destroy_threads(tb* t, int id){
    int j;
    for(j=0;j<chairs_count;j++){
        int ph_id=t->chairs[j];
        if(ph_id !=id){
            pthread_detach(ph_list[ph_id].dining_thread);
        }
        
    }
}

/*
 

    
*/
/**********************************functions**********************************/
/**********************************functions**********************************/
/**********************************functions**********************************/
/**********************************functions**********************************/


