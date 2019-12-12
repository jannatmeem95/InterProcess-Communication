#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include<semaphore.h>
#define STEPS_AB 100
#define STEPS_BC 5
#define STEPS_CD 50
#define STEPS_DD 300
sem_t *sem_bc;
sem_t *sem_cb;

sem_t *sem_cd;
sem_t *sem_dc;

sem_t sem_bridge;
pthread_mutex_t mtx_bc;
pthread_mutex_t mtx_cb;
pthread_mutex_t mtx_cd;
pthread_mutex_t mtx_cdperson;
pthread_mutex_t mtx_dc;
pthread_mutex_t mtx_dcperson;
int *bc_array;
int *cb_array;
int *cd_array;
int *dc_array;
int cd=0;
int dc=0;



void *thread_function(void *arg);

typedef struct object{
	int nid;
	int speed;	
}Person;


int main() {
	int N,line,number,temp;
	
	void *thread_result;
	int res;
	Person *p;
	pthread_t *thread_array;
	
	sem_bc=(sem_t*) malloc(STEPS_BC* sizeof(sem_t));
	bc_array=(int*) malloc(STEPS_BC* sizeof(int));
	
	sem_cb=(sem_t*) malloc(STEPS_BC* sizeof(sem_t));
	cb_array=(int*) malloc(STEPS_BC* sizeof(int));

	for(temp=0;temp<STEPS_BC;temp++){
		bc_array[temp]=0;
		sem_init(&sem_bc[temp],0,0);
		cb_array[temp]=0;
		sem_init(&sem_cb[temp],0,0);
	}

	
	cd_array=(int*) malloc(STEPS_CD* sizeof(int));
	sem_cd=(sem_t*) malloc(STEPS_CD* sizeof(sem_t));

	dc_array=(int*) malloc(STEPS_CD* sizeof(int));
	sem_dc=(sem_t*) malloc(STEPS_CD* sizeof(sem_t));

	for(temp=0;temp<STEPS_CD;temp++){
		cd_array[temp]=0;
		sem_init(&sem_cd[temp],0,0);
		dc_array[temp]=0;
		sem_init(&sem_dc[temp],0,0);
	}

	sem_init(&sem_bridge,0,1);

	line =1;
        FILE *fp;
    	fp = fopen("in.txt", "r");
	fscanf(fp, "%d", &N);
	printf("You entered: %d\n", N);
	p=(Person*) malloc(N * sizeof(Person));
	thread_array = (pthread_t*) malloc(N * sizeof(pthread_t));

	res = pthread_mutex_init(&mtx_bc, NULL);
	if (res != 0) {
			perror("MUTEX initialization failed");
			exit(EXIT_FAILURE);
	}

	res = pthread_mutex_init(&mtx_cb, NULL);
	if (res != 0) {
			perror("MUTEX initialization failed");
			exit(EXIT_FAILURE);
	}

	res = pthread_mutex_init(&mtx_cd, NULL);
	if (res != 0) {
			perror("MUTEX initialization failed");
			exit(EXIT_FAILURE);
	}
	
	res = pthread_mutex_init(&mtx_cdperson, NULL);
	if (res != 0) {
			perror("MUTEX initialization failed");
			exit(EXIT_FAILURE);
	}

	res = pthread_mutex_init(&mtx_dc, NULL);
	if (res != 0) {
			perror("MUTEX initialization failed");
			exit(EXIT_FAILURE);
	}
	
	res = pthread_mutex_init(&mtx_dcperson, NULL);
	if (res != 0) {
			perror("MUTEX initialization failed");
			exit(EXIT_FAILURE);
	}

	while (line<=N)
	{
	    	if (fscanf(fp, "%d", &number) != 1)
			break;        
	    	printf("You entered: %d\n", number);
		
		p[line].nid=line;
		p[line].speed=number;
		
		res = pthread_create(&thread_array[line], NULL, thread_function, (void *)&p[line]);
		if (res != 0) {
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}

	
		line++;
	}
	while(1)
	    {
		
	    }
	


}

void *thread_function(void *p)
{
	int steps,temp,search,res,mode,x;
	Person *pp = (Person *) p;
	
	steps=0;
	temp=0;

	//AB path
	while(1){
		if(steps==STEPS_AB-1) break;
		temp=steps;
		steps+=pp->speed;
		//printf("steps %d\n",steps);
		if(steps>=STEPS_AB){
			steps=STEPS_AB-1;
			printf("Person %d moved along AB from %d to %d\n",pp->nid,temp,steps);
			break;
		}
		printf("Person %d moved along AB from %d to %d\n",pp->nid,temp,steps);		
	}
	printf("Person %d has completed his walk along AB\n",pp->nid);

	steps=0;
	temp=0;
	search=0;
	mode=0;
	
	//BC path
	while(1){ 
		pthread_mutex_lock(&mtx_bc);
		//printf("Person %d has locked mutex\n",pp->nid);
		if(steps+pp->speed>=STEPS_BC) search=STEPS_BC-1;
		else search=steps+pp->speed;

		
		for(temp=steps+1;temp<=search;temp++){
			if(bc_array[temp]==1){
				
				printf("Person %d moved along BC from %d to %d\n",pp->nid,steps,temp-1);
				bc_array[steps]=0;
				bc_array[temp-1]=1;
				x=steps;
				steps=temp-1;
				mode=1;
				//if((steps!=0) && (bc_array[steps-1]==1)) sem_post(&sem_bc[steps-1]);
				//search=steps;
				//printf("Steps of %d : %d\n",pp->nid,steps); 
				break;
			}
		}
		if(temp==search+1){
			bc_array[steps]=0;
			bc_array[search]=1; 
			x=steps;
			printf("Person %d moved along BC from %d to %d\n",pp->nid,steps,search);	
			steps=search;
		}
		
		if(steps==STEPS_BC-1){
			printf("Person %d has completed his walk along BC\n",pp->nid);	
			bc_array[steps]=0;//printf("Person %d has unlocked mutex\n",pp->nid);
			if(bc_array[steps-1]==1) sem_post(&sem_bc[steps-1]);
			pthread_mutex_unlock(&mtx_bc);		
			break;
		}//printf("Person %d has unlocked mutex\n",pp->nid);
		
		if((x!=0) && (bc_array[x-1]==1)) sem_post(&sem_bc[x-1]);
		pthread_mutex_unlock(&mtx_bc);
		if(mode==1) sem_wait(&sem_bc[steps]);
		mode=0;		

	}

	steps=0;
	temp=0;
	search=0;
	mode=0;
	x=0;
	//CD path
	
	pthread_mutex_lock(&mtx_cd);
	//printf("Person %d has locked mutex cd\n",pp->nid);
		cd+=1;
		if(cd==1) sem_wait(&sem_bridge);
	//printf("Person %d has unlocked mutex cd\n",pp->nid);
	pthread_mutex_unlock(&mtx_cd);
	
	while(1){ 
		pthread_mutex_lock(&mtx_cdperson);
		//printf("Person %d has locked mutex\n",pp->nid);
		if(steps+pp->speed>=STEPS_CD) search=STEPS_CD-1;
		else search=steps+pp->speed;

	
		
		for(temp=steps+1;temp<=search;temp++){
			if(cd_array[temp]==1){				
				printf("Person %d moved along CD from %d to %d\n",pp->nid,steps,temp-1);
				cd_array[steps]=0;
				cd_array[temp-1]=1;
				x=steps;
				steps=temp-1;
				mode=1;
				break;
			}
		}
		if(temp==search+1){
			cd_array[steps]=0;
			cd_array[search]=1; 
			x=steps;
			printf("Person %d moved along CD from %d to %d\n",pp->nid,steps,search);	
			steps=search;
		}
		
		if(steps==STEPS_CD-1){
			printf("Person %d has completed his walk along CD\n",pp->nid);	
			cd_array[steps]=0;//printf("Person %d has unlocked mutex\n",pp->nid);
			if(cd_array[steps-1]==1) sem_post(&sem_cd[steps-1]);
			pthread_mutex_unlock(&mtx_cdperson);		
			break;
		}
		
		if((x!=0) && (cd_array[x-1]==1)) sem_post(&sem_cd[x-1]);
		//printf("Person %d has unlocked mutex\n",pp->nid);
		pthread_mutex_unlock(&mtx_cdperson);
		if(mode==1) sem_wait(&sem_cd[steps]);
		mode=0;		

	}
	
	
	pthread_mutex_lock(&mtx_cd);
	//printf("Person %d has locked mutex cd\n",pp->nid);
		cd-=1;
		if(cd==0) sem_post(&sem_bridge);
	//printf("Person %d has unlocked mutex cd\n",pp->nid);
	pthread_mutex_unlock(&mtx_cd);

	//DD path
	steps=0;
	temp=0;

	while(1){
		if(steps==STEPS_DD-1) break;
		temp=steps;
		steps+=pp->speed;
		//printf("steps %d\n",steps);
		if(steps>=STEPS_DD){
			steps=STEPS_DD-1;
			printf("Person %d moved along DD from %d to %d\n",pp->nid,temp,steps);
			break;
		}
		printf("Person %d moved along DD from %d to %d\n",pp->nid,temp,steps);		
	}
	printf("Person %d has completed his walk along DD\n",pp->nid);
	
	//DC path
	steps=STEPS_CD-1;
	temp=0;
	search=0;
	mode=0;
	x=0;

	pthread_mutex_lock(&mtx_dc);
	//printf("Person %d has locked mutex dc\n",pp->nid);
		dc+=1;
		if(dc==1) sem_wait(&sem_bridge);
	//printf("Person %d has unlocked mutex dc\n",pp->nid);
	pthread_mutex_unlock(&mtx_dc);
	
	while(1){ 
		pthread_mutex_lock(&mtx_dcperson);
		//printf("Person %d has locked mutex\n",pp->nid);
		if(steps-pp->speed<0) search=0;
		else search=steps-pp->speed;

		
		for(temp=steps-1;temp>=search;temp--){
			if(dc_array[temp]==1){				
				printf("Person %d moved along DC from %d to %d\n",pp->nid,steps,temp+1);
				dc_array[steps]=0;
				dc_array[temp+1]=1;
				x=steps;
				steps=temp+1;
				mode=1;
				break;
			}
		}
		if(temp==search-1){
			dc_array[steps]=0;
			dc_array[search]=1; 
			x=steps;
			printf("Person %d moved along DC from %d to %d\n",pp->nid,steps,search);	
			steps=search;
		}
		
		if(steps==0){
			printf("Person %d has completed his walk along DC\n",pp->nid);	
			dc_array[steps]=0;//printf("Person %d has unlocked mutex\n",pp->nid);
			if(dc_array[steps+1]==1) sem_post(&sem_dc[steps+1]);
			pthread_mutex_unlock(&mtx_dcperson);		
			break;
		}
		
		if((x!=STEPS_CD-1) && (dc_array[x+1]==1)) sem_post(&sem_dc[x+1]);
		//printf("Person %d has unlocked mutex\n",pp->nid);
		pthread_mutex_unlock(&mtx_dcperson);
		if(mode==1) sem_wait(&sem_dc[steps]);
		mode=0;		

	}
	
	printf("Person %d has left loop DC\n",pp->nid);

	pthread_mutex_lock(&mtx_dc);
	//printf("Person %d has locked mutex dc\n",pp->nid);
		dc-=1;
		if(dc==0) sem_post(&sem_bridge);
	//printf("Person %d has unlocked mutex dc\n",pp->nid);
	pthread_mutex_unlock(&mtx_dc);

	//CB path
	
	steps=STEPS_BC-1;
	temp=0;
	search=0;
	mode=0;
	x=0;
	while(1){ 
		pthread_mutex_lock(&mtx_cb);
		//printf("Person %d has locked mutex\n",pp->nid);
		if(steps-pp->speed<0) search=0;
		else search=steps-pp->speed;

		
		for(temp=steps-1;temp>=search;temp--){
			if(cb_array[temp]==1){
				
				printf("Person %d moved along CB from %d to %d\n",pp->nid,steps,temp+1);
				cb_array[steps]=0;
				cb_array[temp+1]=1;
				x=steps;
				steps=temp+1;
				mode=1;
				
				break;
			}
		}
		if(temp==search-1){
			cb_array[steps]=0;
			cb_array[search]=1; 
			x=steps;
			printf("Person %d moved along CB from %d to %d\n",pp->nid,steps,search);	
			steps=search;
		}
		
		if(steps==0){
			printf("Person %d has completed his walk along CB\n",pp->nid);	
			cb_array[steps]=0;//printf("Person %d has unlocked mutex\n",pp->nid);
			if(cb_array[steps+1]==1) sem_post(&sem_cb[steps+1]);
			pthread_mutex_unlock(&mtx_cb);		
			break;
		}//printf("Person %d has unlocked mutex\n",pp->nid);
		
		if((x!=STEPS_BC-1) && (cb_array[x+1]==1)) sem_post(&sem_cb[x+1]);
		pthread_mutex_unlock(&mtx_cb);
		if(mode==1) sem_wait(&sem_cb[steps]);
		mode=0;		

	}

	steps=STEPS_AB-1;
	temp=0;

	while(1){
		if(steps==0) break;
		temp=steps;
		steps-=pp->speed;
		//printf("steps %d\n",steps);
		if(steps<=0){
			steps=0;
			printf("Person %d moved along BA from %d to %d\n",pp->nid,temp,steps);
			break;
		}
		printf("Person %d moved along BA from %d to %d\n",pp->nid,temp,steps);		
	}
	printf("Person %d has completed his walk along BA\n",pp->nid);

	printf("Person %d has completed his walk.\n",pp->nid);



	



}

