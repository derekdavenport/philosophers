#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define THINKING 0
#define HUNGRY 1
#define EATING 2

void *philLogic(void *ptr);

pthread_mutex_t bloodyMutex = PTHREAD_MUTEX_INITIALIZER;

struct Chopstick
{
	int id;
	bool onTable;
	pthread_mutex_t mutex;
};

struct Philosopher
{
	int id;
	Chopstick *ChopLeft;
	Chopstick *ChopRight;
	bool chopInLeftHand, chopInRightHand;
	int state;
	int timesEaten;
	pthread_t thread;
	int ret;
};

int numEats;

int main(int argc, char *const argv[])
{
	numEats = 10; //defaults
	int numPhils = 5;
	if (argc >= 2)
	{
		sscanf(argv[1], "%d", &numEats); //parse arg1 as a number and put it in numEats
		if (argc >= 3)
		{
			sscanf(argv[2], "%d", &numPhils); //parse arg2 as a number and put it in numPhils
		}
	}

	if (numEats < 1)
	{
		numEats = 10;
		printf("Bad parameter for number of times to eat. Defaulting to 10\n");
	}

	if (numPhils < 2)
	{
		numPhils = 5;
		printf("Bad parameter for number of philosophers. Defaulting to 5\n");
	}

	struct timeval tp;			 //declare timeval tp for random number seeding
	gettimeofday(&tp, NULL); // set things up by seeding with the time of day
	srand(tp.tv_sec);

	Philosopher Phils[numPhils];
	Chopstick Chops[numPhils];

	//initialize all the philosophers
	for (int i = 0; i < numPhils; i++)
	{
		Phils[i].id = i;
		Phils[i].state = THINKING;			//start out thinking
		Phils[i].ChopRight = &Chops[i]; //initialize Right Chopstick
		Phils[i].ChopRight->id = i;
		Phils[i].ChopRight->onTable = true;
		Phils[i].ChopRight->mutex = bloodyMutex;
		Phils[i].timesEaten = 0;
		Phils[(i + 1) % numPhils].ChopLeft = Phils[i].ChopRight; //The next philosopher's left chopstick is this philosopher's right chop stick
		Phils[i].ret = pthread_create(&Phils[i].thread, NULL, philLogic, (void *)&Phils[i]); //create the thread and pass the Philosopher to it
	}

	for (int i = 0; i < numPhils; i++)
	{
		pthread_join(Phils[i].thread, NULL);
	}

	return 0;
}

void *philLogic(void *ptr)
{
	Philosopher *Phil;
	Phil = (Philosopher *)ptr;
	while (Phil->timesEaten < numEats)
	{
		if (Phil->state == HUNGRY)
		{
			if (Phil->id % 2) //if odd numbered Philosopher
			{
				if (!Phil->chopInLeftHand)
				{
					if (Phil->ChopLeft->onTable)
					{
						pthread_mutex_lock(&Phil->ChopLeft->mutex); //make sure picking up this chopstick doesn't interfere with other threads
						Phil->chopInLeftHand = true;
						Phil->ChopLeft->onTable = false;
						pthread_mutex_unlock(&Phil->ChopLeft->mutex); //clear!

						printf("Philosopher %d picked up left chopstick.\n", Phil->id);
					}
					else
					{
						printf("Philosopher %d could not find the left chopstick.\n", Phil->id);
					}
				}
				else if (!Phil->chopInRightHand) //chopstick in left hand, try to pick up right
				{
					if (Phil->ChopRight->onTable)
					{
						pthread_mutex_lock(&Phil->ChopRight->mutex); //make sure picking up this chopstick doesn't interfere with other threads
						Phil->chopInRightHand = true;
						Phil->ChopRight->onTable = false;
						pthread_mutex_unlock(&Phil->ChopRight->mutex); //clear!

						printf("Philosopher %d picked up right chopstick and now has both chopsticks.\n", Phil->id);
					}
					else
					{
						printf("Philosopher %d has the left chop stick, but could not find the right chopstick.\n", Phil->id);
					}
				}
				else //chopsticks in both hands, start eating
				{
					Phil->state = EATING;
					printf("Philosopher %d is eating now.\n", Phil->id);
				}
			}
			else
			{
				if (!Phil->chopInRightHand) //try to pick up right
				{
					if (Phil->ChopRight->onTable)
					{
						pthread_mutex_lock(&Phil->ChopRight->mutex); //make sure picking up this chopstick doesn't interfere with other threads
						Phil->chopInRightHand = true;
						Phil->ChopRight->onTable = false;
						pthread_mutex_unlock(&Phil->ChopRight->mutex); //clear!

						printf("Philosopher %d picked up right chopstick.\n", Phil->id);
					}
					else
					{
						printf("Philosopher %d could not find the right chopstick.\n", Phil->id);
					}
				}
				else if (!Phil->chopInLeftHand) //chopstick in right hand, try to pick up left
				{
					if (Phil->ChopLeft->onTable)
					{
						pthread_mutex_lock(&Phil->ChopLeft->mutex); //make sure picking up this chopstick doesn't interfere with other threads
						Phil->chopInLeftHand = true;
						Phil->ChopLeft->onTable = false;
						pthread_mutex_unlock(&Phil->ChopLeft->mutex); //clear!

						printf("Philosopher %d picked up left chopstick and now has both chopsticks.\n", Phil->id);
					}
					else
					{
						printf("Philosopher %d has the right chopstick, but could not find the left chopstick.\n", Phil->id);
					}
				}
				else //chopsticks in both hands, start eating
				{
					Phil->state = EATING;
					printf("Philosopher %d is eating now.\n", Phil->id);
				}
			}
		}
		else if (rand() % 2) //random true or false
		{
			if (Phil->state == THINKING) //done thinking, now we're hungry
			{
				Phil->state = HUNGRY;
				printf("Philosopher %d is hungry now.\n", Phil->id);
			}
			else // done eating, now it's time to think
			{
				Phil->chopInLeftHand = Phil->chopInRightHand = false;
				pthread_mutex_lock(&Phil->ChopLeft->mutex); //lock the chopstick
				pthread_mutex_lock(&Phil->ChopRight->mutex);
				Phil->ChopLeft->onTable = Phil->ChopRight->onTable = true;
				pthread_mutex_unlock(&Phil->ChopLeft->mutex);	//clear!
				pthread_mutex_unlock(&Phil->ChopRight->mutex); //clear!
				Phil->timesEaten++;
				Phil->state = THINKING;
				printf("Philosopher %d has eaten %d time%s, has set down both chopsticks, and is thinking now.\n", Phil->id, Phil->timesEaten, (Phil->timesEaten != 1) ? "s" : "");
			}
		}

		sleep((rand() % 10) + 1); // generate random number from 1-10
	}
	printf("Philosopher %d is full.\n", Phil->id);
}
