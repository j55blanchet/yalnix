// LedyardBridge.c
//
// Julien Blanchet and Jae Heon Lee.
//
// Tests Cvar Implementation by using Ledyard
// Bridge example. Instead of shared memory, we
// keep the status of the bridge always in a pipe,
// using our unique pipe semantics. Since pipes
// always contain strings, we need to have serialization
// and deserialization functions.

#include "programs/UserUtility.h"
#define BRIDGE_STATUS_BUFFER_SIZE 256

#define NUM_CARS 20
#define BRIDGE_CAPACITY 6

// synchronization globals
int lock;
int wantTravelNorwich;
int wantTravelHanover;
int status_pipe;


// =====================================
// Structure Definitions
// ======================================
// Direction of cars
typedef enum{
	TO_NORWICH = 0,
	TO_HANOVER = 1
} Direction;
// Holds info that describes the bridge
typedef struct{
	int carsWaitingToNorwich;
	int carsWaitingToHanover;
	Direction dirOfTravel;
	int carsOnBridge;
	int recordNumCarsOnBridge;
} LedyardBridge;
// Process-specific information
typedef struct{
	int carNum;
	int crossed;
	Direction dir;
} CarProcessInfo;
// ======= End Structure Definitions =======



// ===================================
// Utility Functions
// ===================================
void printCarMsg(CarProcessInfo* cpi, char* msg){
	char* dirName = (cpi->dir == TO_HANOVER ? "Han" : "Nor");
	putsArgs("Car #%d (%s, crossed:%d): %s\n", cpi->carNum, dirName, cpi->crossed, msg);
}
LedyardBridge makeLedyardBridge(){
    LedyardBridge br;
	br.carsWaitingToNorwich = 0;
	br.carsWaitingToHanover = 0;
	br.dirOfTravel = TO_HANOVER;
	br.carsOnBridge = 0;
	br.recordNumCarsOnBridge = 0;
	return br;
}
void writeBridgeStatus(int pipe_id, LedyardBridge* br){
    if (ERROR != PipeWrite(pipe_id, (char*) br, sizeof(LedyardBridge) ))
        puts("Wrote LedyardBridge Status to the Pipe\n");
    else
        puts("WARNING: could not write LedyardBridge status to the pipe\b");
}
LedyardBridge readBridgeStatus(int pipe_id){
    LedyardBridge br;
    if (ERROR != PipeRead(pipe_id, &br, sizeof(LedyardBridge))){
        puts("Read LedyardBridge Status to the Pipe\n");
    }else{
        puts("WARNING: could not read LedyardBridge status to the pipe\b");
    }
    return br;
}
void printBridge(LedyardBridge* br){
	char* directionName = (br->dirOfTravel == TO_HANOVER ? "Hanover" : "Norwich");

	putsArgs("\t%d car(s) on bridge\n", br->carsOnBridge);
	putsArgs("\ttraveling towards %s\n", directionName);
	putsArgs("\t%d waiting for Norwich\n", br->carsWaitingToNorwich);
	putsArgs("\t%d waiting for Hanover\n", br->carsWaitingToHanover);
	putsArgs("\t%d highest ever cars on bridge\n", br->recordNumCarsOnBridge);
}
// ======= End Utility Functions ==============



// ===================================
// Central Bridge Functions
// ===================================
void ArriveBridge(CarProcessInfo* ci);
int LaunchVehicleProcess(CarProcessInfo cpi);
void OnBridge(CarProcessInfo* cpi);
void ExitBridge(CarProcessInfo* cpi);

int LaunchVehicleProcess(CarProcessInfo cpi){
    int pid = Fork();
    if (pid == ERROR){
        puts("FORK FAILED!\n");
        Exit(-1);
    }
    
    else if (pid != 0)
        return pid;
        
    printCarMsg(&cpi, "New Car");
    
	//Arrived at bridge. Will block
	//until bridge is safe for travel
	ArriveBridge(&cpi);
	
	// Traveling accross bridge. Will print the state
	// of the bridge. Returns when the car has crossed the
	// bridge
	OnBridge(&cpi);

	// Runs when the car has passed the bridge, opening
	// the bridge up to the next vehicle
	ExitBridge(&cpi);

	Exit(cpi.crossed);
}
void ArriveBridge(CarProcessInfo* ci){

	// Before checking if we can travel across bridge,
	// we must aquire lock in bridge to ensure we view it in
	// a good state
	int rc = Acquire(lock);
	if (rc == ERROR){
		panic("failed to aquire bridge mutex\n");
	}
	printCarMsg(ci, "arrived at bridge\n");
	
	
	LedyardBridge br = readBridgeStatus(status_pipe);

	// Mark us as waiting
	if (ci->dir == TO_NORWICH)
		br.carsWaitingToNorwich ++;
	else
		br.carsWaitingToHanover ++;
		
		
    writeBridgeStatus(status_pipe, &br);

	// Now, we wait for when it is safe to enter the bridge.
	// This happens when there are either no cars on the bridge, 
	// or when there are less than the maximum number of cars on
	// bridge, and they are all traveling in the same direction
	while(	!(br.carsOnBridge == 0 ||
		  	 (br.carsOnBridge < BRIDGE_CAPACITY && br.dirOfTravel == ci->dir))){

		int cvar;
		if (ci->dir == TO_NORWICH)
			cvar = wantTravelNorwich;
		else
			cvar = wantTravelHanover;

		printCarMsg(ci, "waiting to cross bridge\n");

		if (CvarWait(cvar, lock) == ERROR){
		    panic("WARNING: Could not perform Cvar Wait\n");
		}
		
		// reload bridge before checking new conditions
		br = readBridgeStatus(status_pipe);
	}

	// At this point, it is safe to enter the bride. So, we do, and
    // update the bridge variable appropiately. Then, release lock
	br.carsOnBridge ++;
	br.dirOfTravel = ci->dir;

	if (br.carsOnBridge > br.recordNumCarsOnBridge)
		br.recordNumCarsOnBridge = br.carsOnBridge;

	// We're done waiting
	if (ci->dir == TO_NORWICH)
		br.carsWaitingToNorwich --;
	else
		br.carsWaitingToHanover --;

    // update bridge in the pipe
    writeBridgeStatus(status_pipe, &br);

	// Release lock before returning
	rc = Release(lock);
	if (rc == ERROR){
	    panic("failed to unlock bridge mutex!\n");
	}
}
void OnBridge(CarProcessInfo* cpi){
	// First, we aquire a lock to make sure what we
	// print out is valid information
	int rc = Acquire(lock);
	
	LedyardBridge br = readBridgeStatus(status_pipe);
	
	if (rc == ERROR){
		panic("failed to aquire bridge mutex\n");
	}

	printCarMsg(cpi, "crossing brige\n");
	
	printBridge(&br);

	/* Release the lock before exiting*/
	rc = Release(lock);
	
	if (rc== ERROR){
		panic("failed to unlock bridge mutex!\n");
	}
	
	Delay(1); // let it take one cycle to cross the bridge
}
void ExitBridge(CarProcessInfo* cpi){
	int rc = Acquire(lock);
	if (rc == ERROR){
		panic("failed to aquire bridge mutex\n");
	}
	
	LedyardBridge br = readBridgeStatus(status_pipe);

	printCarMsg(cpi, "leaving bridge\n");
	
	cpi->crossed = 1;

	// Now, update the status of the bridge, 
	// and signal appropiate CVARs
	br.carsOnBridge--;
    writeBridgeStatus(status_pipe, &br);
    
	// only signal one car waiting to
	// go in the same direction
	if (br.carsOnBridge > 0){
		if (cpi->dir == TO_NORWICH){
			CvarBroadcast(wantTravelNorwich);
		}else{
			CvarBroadcast(wantTravelHanover);
		}
	}
	// if 0 cars, signal to both directions
	else{
	    CvarBroadcast(wantTravelHanover);
	    CvarBroadcast(wantTravelNorwich);
	}
	
	// Release the lock before exiting
	rc = Release(lock);
	if (rc == ERROR){
		panic("failed to unlock bridge mutex!\n");
	}
}
// ======= End Central Bridge Functions ==============


// ===== START MAIN =====
int main(void) {
  
    if (LockInit(&lock) == ERROR){
        puts("LedyardBridge: I failed to create a lock.\n");
        Exit(-1);
    }
    if (CvarInit(&wantTravelNorwich) == ERROR || 
        CvarInit(&wantTravelHanover) == ERROR)  
    {
      puts("LedyardBridge: I failed to create cvars.\n");
      Exit(-1);
    }

    // our special semantics for pipe will
    // allow us to basically share the pipe's
    // buffer
    if( SUCCESS != PipeInit(&status_pipe) ) {
    puts("LedyardBridge: I failed to create a pipe.\n");
    Exit(-2);
    }

    // Test bridge status sharing mechanism
    puts("==== Phase 1: Test Pipe as method of bridge status saving===\n");
    LedyardBridge br = makeLedyardBridge();
    puts("Created Ledyard Bridge\n");
    puts("Modifying the bridge\n");
    br.carsOnBridge = 21;
    printBridge(&br);
    writeBridgeStatus(status_pipe, &br);
    puts("Wrote initial brige status\n");
    br = readBridgeStatus(status_pipe);
    printBridge(&br);

    // Phase 2: actually do the bridge simulation
    puts("==== Phase 2: Performing Bridge Simulation===\n");
    br = makeLedyardBridge();

    // Wait until are car processes are created before releasing the lock
    Acquire(lock);
    writeBridgeStatus(status_pipe, &br);

    int carid;
    for(carid = 1; carid <= NUM_CARS; carid ++){
      CarProcessInfo cpi;
      {
      	cpi.carNum = carid;
        cpi.crossed = 0;
        cpi.dir = (carid % 2 == 0 ? TO_NORWICH : TO_HANOVER);
      }
      int cpid = LaunchVehicleProcess(cpi);
      putsArgs("Car #%d has pid #%d\n", carid, cpid);
    }

    putsArgs("Global nums: lock:%d, waitNor:%d, waitHan:%d, pipe:%d\n",
            lock, wantTravelNorwich, wantTravelHanover, status_pipe);


    Release(lock);
    

    // wait for all car processes to have completed
    int hasCrossed;
    int numCrossed = 0;
    int numNotCrossed = 0;
    while(Wait(&hasCrossed) != ERROR){
        if (hasCrossed){
            numCrossed++;
        }else{
            numNotCrossed++;
        }
    }
    
    puts("Reclaiming Interop Reasources\n");
    Reclaim(lock);
    Reclaim(wantTravelNorwich);
    Reclaim(wantTravelHanover);
    Reclaim(status_pipe);

    putsArgs("\n\tCars Crossed: %d\n\tNot Crossed: %d\n", numCrossed, numNotCrossed);
    
    if (numCrossed != NUM_CARS || numNotCrossed != 0){
        puts("Test Completed: FAILURE!\n");
        return ERROR;
    } 
    
    puts("Test Completed: SUCCESS!\n");
    return SUCCESS;
}
/// ===== END MAIN =====

// End of LedyardBridge.c
