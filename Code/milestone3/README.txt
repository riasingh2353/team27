README for drive_code_dfs.ino

in order to control the robot's movement logic with DFS (as opposed to wall following) we need to change a lot of things.
i'm doing my best to list these changes here. please let me know if you have questions/need clarification about anything
here + please make improvements to this code if you think of any. 

THINGS ADDED:
	FUNCTIONS:
	dfs(int calling_dir); 
		this function essentially replaces intersection() as the main controller of the robot's movement logic.
		it's a recursive dfs algorithm which is called at each intersection. 
		the algorithim basically consists of 3(ish) stages, which are as follows:
			stage 0: mark space as visited
				set visited(position) = 1. 
				(for an m x n maze, visited is a m x n entry array which stores 1 if the corresponding space has been visited
				and 0 otherwise) 
			stage 1: figure out movement options
				in this stage, the robot gets the wall values and uses these to determine which directions it can move in.
				the cardinal direction of these movement options (e.g. N, E, S, W) is recorded in the array 'options'
			stage 2: explore movement options
				for each movement option, the robot will determine which way it needs to turn (if at all) to travel in that direction
				it will then move in that direction and call dfs recursively.
			stage 3: backtrack
				the robot orients itself in the direction opposite to the direction it was facing when dfs was called (i.e., calling_dir). 
				it then drives back to the intersection that dfs was called from.

		this code should allow the robot to traverse the entire maze. however, i'm pretty sure that it'll break 
		the transmission of the robot's position to the base station.

	line_follow_until_intersection();
		this function basically just copies code used in the loop section of previous iterations of our drive code
		it line follows, and then returns when it reaches an intersection.
		
		this has to be implemented as a helper function (and taken out of the loop section) because dfs() will be continuously executing 
		until the entire maze has been traversed

	update_position(int facing, int turn_dir);
		this function updates the (new!) global position variable based on the direction the robot is facing upon reaching an intersection
		and the direction of the turn it will make at that intersection (if any). 
			turn_dir = 0 --> right turn
			turn_dir = 1 --> left turn
			turn_dir = 2 --> no turn (i.e. the robot drives straight at the intersection)

		NOTE 1: the global position variable is an integer that stores the raster coordinates of the space the robot is currently in.
		for a 3x3 maze, the associated raster coordinates are as follows:
			  _ _ _       
			 |0|1|2|    
			  - - -        
			 |3|4|5|      
			  - - -       
			 |6|7|8|    
			  _ _ _      
		
		NOTE 2: this function must be called before update_direction is called to update global direction

		NOTE 3: MAKE SURE THE WIDTH VARIABLE IN SETUP IS SET TO PROPERLY REFLECT THE WIDTH OF THE MAZE. IF NOT THEN 
		EVERYTHING WILL BE VERY CONFUSING.
			the robot always starts in the northwest corner facing east. so width will be the number of intersections that exist in the direction
			the robot is facing when it starts.

	VARIABLES:
		width 
			represents the width of the maze. 
			from above: the robot always starts in the northwest corner facing east. so width will be the number of intersections that exist in the direction
			the robot is facing when it starts.
		height
			represents the height of the maze.

		visited
			a width*height entry array. an index stores 1 if the space with the corresponding raster coordinate has been visited. otherwise it stores 0.

		pos
			stores the raster coordinate associated with the robot's current position. 
