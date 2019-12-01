/*
 * 沐阿里 3160300326
 *
 * ZJU OS 2019-2020 : Project 1 Part 1 Template
 *
 */

//INTERSECTION LOGIC
// When a single car passes through the intersection, there are several states  possible
/*

		A) when a vehicle from one direction comes, it will wait at the intersection. If it receives a signal, it will judge whether the vehicle is in the queue head. 
		If it does not receive a waiting signal, it will drive into the intersection. If it is not a queue head vehicle, continue to wait. 
		Note that in my algorithm, for example, if the first vehicle in the South leaves Zone A and enters zone B, the second vehicle in the South queue 
		(if no waiting signal is received) can immediately enter zone a, instead of waiting for the first vehicle to leave zone B and leave the intersection, 
		so as to reduce the waiting time of other vehicles and increase efficiency as much as possible.            
		
		B) when the vehicle arrives at the intersection in multiple directions, it shall be considered in different situations:
		I. In case of two vehicles, if the two vehicles are in the opposite direction (e.g. east-west, north-south), the two vehicles will enter the intersection at the same time.            
		II. In case of two vehicles, if the two vehicles are not in the opposite direction (e.g. northeast, Southeast, etc.), one vehicle shall enter one vehicle and wait in the principle of priority on the right.            
		III. when there are three vehicles, there must be a group of opposite vehicles and a single vehicle (such as north-south East, north-south opposite direction, East single). 
		At this time, two opposite vehicles go first and one-way vehicles wait, because two opposite vehicles can pass at one time.            
		IV. when there are four vehicles, there are two groups of opposite directions, then two vehicles in a north-south direction and two vehicles in an east-west direction, in turn.            
		My design principle is that if there are two opposite vehicles, let them go first, two vehicles can be taken at a time, which can reduce the waiting time of other vehicles and improve efficiency. 
		If there are no two opposite vehicles, the right side shall be given priority.
        
*/ 
	 
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define WEST 10
#define EAST 20
#define NORTH 30
#define SOUTH 40

using namespace std;

    /*
     * Parse Command Line arguments
     */
	 
	/*
	* Approach intersection
	*/ 
    char input[100];	//Input data
    int westFirst = 0, eastFirst = 0, northFirst = 0, southFirst = 0; //Prevent starvation
    pthread_mutex_t a, b, c, d;	//Lock of 4 resources
    int empty = 4;      //At the intersection
	
    sem_t w_pass, e_pass, n_pass, s_pass; //Four directions
    sem_t haveCar;  //Car's signal coming towards intersection

    /*
     * Create Car Thread(s)
     * 
     */
	class queue{
	public:
		pthread_t q[100]; //Thread ID
		int id[100];  //Car ID
		int head;   //Queue header
		int rear;   //Queue tail
		int count;  //Number of car in the queue
		string queue_name;	//Queue name

    queue(string name){
        this->head = 0;
        this->rear = 0;
        this->count = 0;
        queue_name = name;
    }

    /*
     * Join to queue, towards intersection
     * 
     */
    void push(pthread_t car, int i){
        q[rear] = car;
        id[rear] = i;
        rear++;

        count = count + 1;
        sem_post(&haveCar);
        cout << "Car" << i << " from " << queue_name << " approach intersection." << endl;
    }
    //Leaving the queue/intersection
    void pop(){
        empty++;
        cout << "Car" << id[head] << " from " << queue_name << " leaves intersection." << endl;
        head++;
    }
	};

	queue n("north"), s("south"), w("west"), e("east");

	/*
     * Control thread CPU
     * 
     */
	void* cpu(void *none){
		int i;
		int whichGo;
		bool twoGo = false;

		int fourDir_whichGo = EAST;
    bool fourDir_haveCar = false;    //take turns

    cout << "Start->" << endl;

    for(i = 0; i < 1000; i++){
    	//Waiting for the signal of the incoming car
        sem_wait(&haveCar);

    /*
     * Prevent starvation
     */   
        if(southFirst >= 5||northFirst >= 5||eastFirst >= 5||westFirst >= 5){
            if(southFirst >= 5 && s.count > 0){
                cout << "STARVATION: Car(s) from South" << endl;
                sem_post(&s_pass);
                southFirst = 0;
                continue;
            }
            if(northFirst >= 5 && n.count > 0){
                cout << "STARVATION: Car(s) from North" << endl;
                sem_post(&n_pass);
                northFirst = 0;
                continue;
            }
            if(eastFirst >= 5 && e.count > 0){
                cout << "STARVATION: Car(s) from East" << endl;
                sem_post(&e_pass);
                eastFirst = 0;
                continue;
            }
            if(westFirst >= 5 && w.count > 0){
                cout << "STARVATION: Car(s) from West" << endl;
                sem_post(&w_pass);
                westFirst = 0;
                continue;
            }
        }

        // There's car going one direction
        if(n.count > 0 && s.count == 0 && w.count == 0 && e.count == 0){
            whichGo = NORTH;
            n.count--;
        }else if(n.count == 0 && s.count > 0 && w.count == 0 && e.count == 0){
            whichGo = SOUTH;
            s.count--;
        }else if(n.count == 0 && s.count == 0 && w.count > 0 && e.count == 0){
            whichGo = WEST;
            w.count--;
        }else if(n.count == 0 && s.count == 0 && w.count == 0 && e.count > 0){
            whichGo = EAST;
            e.count--;
        }
		
        //There are cars going two direction
		/*
		When the vehicle comes from the northeast, the North passes (right priority principle), the East waits, waits once, eastFirst++.           
		When the variable reaches 5 times in a certain direction, that is, the vehicle in that direction gives way and waits for 5 times, 
		the next scheduling unconditionally lets the vehicle in that direction pass (if there is a vehicle in that direction), and clears the variable. 
		In this way, we can ensure that the number of waiting times in each direction is limited, and we will prevent starvation.            
        */		
        else if(n.count > 0 && s.count == 0 && w.count == 0 && e.count > 0){
            whichGo = NORTH;
            n.count--;
            eastFirst++;
        }else if(n.count == 0 && s.count > 0 && w.count == 0 && e.count > 0){
            whichGo = EAST;
            e.count--;
            southFirst++;
        }else if(n.count == 0 && s.count == 0 && w.count > 0 && e.count > 0){
            whichGo = EAST;
            twoGo = true;
            w.count--;
            e.count--;
            //cout << "w e go" << endl;
        }else if(n.count > 0 && s.count == 0 && w.count > 0 && e.count == 0){
            whichGo = WEST;
            w.count--;
            northFirst++;
        }else if(n.count == 0 && s.count > 0 && w.count > 0 && e.count == 0){
            whichGo = SOUTH;
            s.count--;
            westFirst++;
        }else if(n.count > 0 && s.count > 0 && w.count == 0 && e.count == 0){
            whichGo = SOUTH;
            twoGo = true;
            n.count--;
            s.count--;
            //cout << "n s go" << endl;
        }
        //Three directions
        else if(n.count == 0 && s.count > 0 && w.count > 0 && e.count > 0){
            whichGo = EAST;
            twoGo = true;
            w.count--;
            e.count--;
            southFirst++;
            //cout << "3 go without north, car(s) from South have to wait" << endl;
        }else if(n.count > 0 && s.count == 0 && w.count > 0 && e.count > 0){
            whichGo = EAST;
            twoGo = true;
            w.count--;
            e.count--;
            northFirst++;
            //cout << "3 go without south, car(s) from North have to wait" << endl;
        }else if(n.count > 0 && s.count > 0 && w.count == 0 && e.count > 0){
            whichGo = SOUTH;
            twoGo = true;
            n.count--;
            s.count--;
            eastFirst++;
            //cout << "3 go without west, car(s) from East have to wait" << endl;
        }else if(n.count > 0 && s.count > 0 && w.count > 0 && e.count == 0){
            whichGo = SOUTH;
            twoGo = true;
            n.count--;
            s.count--;
            westFirst++;
            //cout << "3 go without east, car(s) from West have to wait" << endl;
        }
        //All 4 directions
        else if(n.count > 0 && s.count > 0 && w.count > 0 && e.count > 0){
            fourDir_haveCar = true;
            if(fourDir_whichGo == EAST){
                fourDir_whichGo = SOUTH;
                n.count--;
                s.count--;
            }
            else{
                fourDir_whichGo = EAST;
                w.count--;
                e.count--;
            }
            //cout << "4 go" << endl;
        }

    /*
     * Handling when there are vehicles in all four directions
     */
        if(fourDir_haveCar){
        	//East and West
            if(fourDir_whichGo == EAST){
                if(empty <= 2){
                    cout << "DEADLOCK: Car(s) from East&West should wait." << endl;
                    sleep(3);
                }
                sem_post(&e_pass);
                sem_post(&w_pass);
                empty--;empty--;
                sem_wait(&haveCar);
            }
            //North-South
            else{
                if(empty <= 2){
                    cout << "DEADLOCK: Car(s) from North&South should wait." << endl;
                    sleep(3);
                }
                sem_post(&n_pass);
                sem_post(&s_pass);
                empty--;empty--;
                sem_wait(&haveCar);
            }
            fourDir_haveCar = false;
            continue;
        }

        //Approach intersection, Handling on arrival
        switch (whichGo){
        	//Cars from the East (including cars from the East and the West)
            case EAST:{
                if(twoGo){
                    if(empty <= 2){
                        cout << "DEADLOCK: Car(s) from East&West should wait." << endl;
                        sleep(3);
                    }
                    sem_post(&w_pass);
                    sem_post(&e_pass);
                    empty--;empty--;
                    sem_wait(&haveCar);
                    twoGo = false;
                }else{
                    if(empty <= 1){
                        cout << "DEADLOCK: Car(s) from East should wait." << endl;
                        sleep(3);
                    }
                    sem_post(&e_pass);
                    empty--;
                }
            } break;
			
            //Cars from the South (including those from the north and the South)       	
            case SOUTH:{
                if(twoGo){
                    if(empty <= 2){
                        cout << "DEADLOCK: Car(s) from North&South should wait." << endl;
                        sleep(3);
                    }
                    sem_post(&n_pass);
                    sem_post(&s_pass);
                    empty--;empty--;
                    sem_wait(&haveCar);
                    twoGo = false;
                }else{
                    if(empty <= 1){
                        cout << "DEADLOCK: Car(s) from South should wait." << endl;
                        sleep(3);
                    }
                    sem_post(&s_pass);
                    empty--;
                }
            } break;
            //Cars from the West (excluding east-west)
            case WEST:{
                if(empty <= 1){
                    cout << "DEADLOCK: Car(s) from West should wait." << endl;
                    sleep(3);
                }
                sem_post(&w_pass);
                empty--;
            } break;
            //Cars from the North (excluding those from the north and the South)
            case NORTH:{
                if(empty <= 1){
                    cout << "DEADLOCK: Car(s) from North should wait." << endl;
                    sleep(3);
                }
                sem_post(&n_pass);
                empty--;
            } break;
        }
        //cout << "which go: " << whichGo << endl;
        //cout << "empty: " << empty << endl;
    }
}

    /*
     * thread processing
     */
	void* car(void* dir){
		int id;
		int i;
	
	 //INTERSECTION LOGIC 2
     // When a single car passes through the intersection, there are several states (taking the southern car as an example),
     // waiting in the queue, driving in area a, entering area B with the head and the tail still in area a, driving in area B, leaving the intersection.
     /*
        When the car drives into area a, the lock of area a is required. It is assumed that the car head will enter area B after driving for 1 second.

       At this time, you need to apply for the lock in area B (because you are going to drive in immediately, you can't drive in until you apply, otherwise it means a crash). 
	   If you don't, you can only park in area a and wait for the car in area B to drive before entering area B.
        
        Suppose that the process of crossing the AB area is completed instantaneously (if it is more realistic, "sleep" should be used to simulate the process of crossing the area, 
		but I didn't do it to make it easier to understand the completion). When the rear of the car leaves the a area, it contacts the lock of a. 
		Suppose it takes one second (sleep (1)) from the moment to leave area B, and one second later to leave area B to unlock  
     */
	
    switch ((int)dir){
		
		/*
		* The car comes from the North
		*/
		
        case NORTH: {
            while(1){
                sem_wait(&n_pass);

                //Find out the car's number on queue
                for(i = 0; pthread_self()!=n.q[i];i++);
                id = i;
            	//check if its 1st on wait line
                if((id-n.head)==0){
                    break;
                }
                sem_post(&n_pass);
            }
			
            //going through area C
            pthread_mutex_lock(&c);
            sleep(1);
            //enter area D
            pthread_mutex_lock(&d);
            //Leave area C
            pthread_mutex_unlock(&c);
            sleep(1);
            // Leave area D
            pthread_mutex_unlock(&d);
            n.pop();

        } break;
		
		/*
		* The car comes from the South
		*/

        case SOUTH: {
		/*
		The threads of each car are waiting for the signal of s_pass passing south. After getting the signal, 
		we can know whether the car is the first car in the queue through the thread ID sequence of the car in this direction stored in the queue. 
		If so, drive into the intersection. If not, release the pass signal and continue to wait for the next signal.
		*/			
            while(1){
                sem_wait(&s_pass);

                //Find out the car's number on queue
                for(i = 0; pthread_self()!=s.q[i];i++);
                id = i;
            	//check if its 1st on wait line
                if((id-s.head)==0){
                    break;
                }
                sem_post(&s_pass);
            }

            //going through area A
            pthread_mutex_lock(&a);
            sleep(1);
            //enter area B
            pthread_mutex_lock(&b);
            //Leave area A
            pthread_mutex_unlock(&a);
            sleep(1);
            //Leave area B
            pthread_mutex_unlock(&b);
            s.pop();

        } break;

		/*
		* The car comes from the West
		*/
        case WEST: {
            while(1){
                sem_wait(&w_pass);

                //Find out the car's number on queue
                for(i = 0; pthread_self()!=w.q[i];i++);
                id = i;
            	//check if its 1st on wait line
                if((id-w.head)==0){
                    break;
                }
                sem_post(&w_pass);
            }

            //same logic as above
            pthread_mutex_lock(&d);
            sleep(1);
            pthread_mutex_lock(&a);
            pthread_mutex_unlock(&d);
            sleep(1);
            pthread_mutex_unlock(&a);
            w.pop();

        } break;

		/*
		* The car comes from the East
		*/
        case EAST: {
            while(1){
                sem_wait(&e_pass);
                
                //Find out the car's number on queue
                for(i = 0; pthread_self()!=e.q[i];i++);
                id = i;
            	//check if its 1st on wait line
                if((id-e.head)==0){
                    break;
                }
                sem_post(&e_pass);
            }

			//same logic as above
            pthread_mutex_lock(&b);
            sleep(1);
            pthread_mutex_lock(&c);
            pthread_mutex_unlock(&b);
            sleep(1);
            pthread_mutex_unlock(&c);
            e.pop();

        } break;
    }
}

	int main(){
		pthread_t cpu_thread;
	
		/*
		* Initializing locks and semaphores
		*/
		sem_init(&w_pass,0,0);
		sem_init(&e_pass,0,0);
		sem_init(&n_pass,0,0);
		sem_init(&s_pass,0,0);
		sem_init(&haveCar,0,0);

		pthread_mutex_init(&a,NULL);
		pthread_mutex_init(&b,NULL);
		pthread_mutex_init(&c,NULL);
		pthread_mutex_init(&d,NULL);

		//Create scheduling
		if(pthread_create(&cpu_thread,NULL,cpu,NULL)){
			cout << "ERROR: CPU Thread creation!" << endl;
			exit(-1);
		}

		scanf("%s", input);

		//Generate according to the input
		for(int i = 0; i < strlen(input); i++){
			pthread_t id;
			switch (input[i]){
				case 'n':
					if(pthread_create(&id,NULL,car,(void*)NORTH)){
						cout << "ERROR: car pthread." << endl;
					}
					n.push(id,i);
					break;
				case 's':
					if(pthread_create(&id,NULL,car,(void*)SOUTH)){
						cout << "ERROR: car pthread." << endl;
					}
					s.push(id,i);
					break;
				case 'w':
					if(pthread_create(&id,NULL,car,(void*)WEST)){
						cout << "ERROR: car pthread." << endl;
					}
					w.push(id,i);
					break;
				case 'e':
					if(pthread_create(&id,NULL,car,(void*)EAST)){
						cout << "ERROR: car pthread." << endl;
					}
					e.push(id,i);
					break;
        }
    }
		//When finished, the main thread ends
		pthread_join(cpu_thread,NULL);
}
