#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdlib.h>

int main(){
	
	pid_t pid;
	int matrix_size ;
	int num_process=1;
	int id_child=1;
	int i;
	int shmid;
	int *ptr, *ptr2;
	int debug = 0;
	struct timeval start, end;

	/*  scanf the matrix_size*/
	printf("Input the matrix dimension: ");
	scanf("%d", &matrix_size);

	/* create shared memory*/
	shmid = shmget(IPC_PRIVATE, matrix_size*matrix_size*3*sizeof(unsigned int), IPC_CREAT|0666);
	
	/* set the matrix init value */
	ptr = shmat(shmid, NULL, 0);
	ptr2 = ptr;   /* save the ptr value and pass to ptr3; */
	for(i=0;i<2;i++){
		int j;
		for(j=0; j<(matrix_size*matrix_size); j++)
			*(ptr++) = j;
	} 
	
	/* 1~16 process loop */
	for(i=0;i<16;i++){
		int j;
		int *ptr3;
		ptr3 = ptr2+matrix_size*matrix_size*2;
		/* init and reset third matrix all 0 */
		for(j=0; j<(matrix_size*matrix_size); j++){
			*(ptr3+j) = 0;
		}
		gettimeofday(&start, 0);
		for(j=0;j<num_process;j++){
			fflush(stdout);  /* !!!! flush before fork ensure printf no buffered data!!!*/
			pid = fork();

			if(pid < 0){
				printf("fork(): error !!\n");
			}else if(pid ==0){    //child
				int *s, *e, *target;
				s = shmat(shmid, NULL, 0);
				/* set the base of the A, B matrix */
				e = s + matrix_size*matrix_size;
				s = s + ((matrix_size/num_process)*matrix_size*(id_child-1));
				target = s +  matrix_size*matrix_size*2;
				/* end of base setting*/
				
				/* count the result*/
				int k;
				int empty = (matrix_size)%num_process;
				int work = (matrix_size/num_process);  /* each process's work num */
				if((id_child == num_process) && empty){
//					printf("\nredundent : %d v.s %d ==> %d  ", work, work+empty, empty);
					work += empty;
				}
				for(k=0;k<work;k++){
					int l,m;
					for(l=0;l<matrix_size;l++){
						for(m=0;m<matrix_size;m++){
							*target += *(s+m) * *(e+m*matrix_size);
						}
						e++;   
						target++;
					}
					e -= matrix_size;
					s += matrix_size;
				}
				shmdt(s);
				exit(0);
			}else{		     //father
				id_child++;
			}	
		}
		for(j=0;j<num_process;j++){ 
			wait(NULL);
		}
		gettimeofday(&end, 0);
		
		num_process++;
		id_child = 1;
		

		/** show the result matrix */
		printf("Multiplying matrices using %d process\n", i+1);
		printf("Elapsed time: %f sec", ((end.tv_sec - start.tv_sec)*1000+((end.tv_usec - start.tv_usec)/1000.0))/1000);
		ptr = ptr2 + matrix_size*matrix_size*2;
		unsigned int checksum=0;
		for(j=0;j<matrix_size*matrix_size;j++)
			checksum += *(ptr+j);
		printf(", Checksum: %u\n", checksum);
//		ptr3 = ptr2+(matrix_size*matrix_size*2);
//		for(j=0;j<matrix_size*matrix_size;j++){
//			printf("%5d ",*(ptr3+j));
//			if((j+1)%matrix_size == 0)printf("\n");
//		}
			
	}/* end of loop*/



	/* delete shared memory*/
	shmctl(shmid, IPC_RMID, NULL);
}
