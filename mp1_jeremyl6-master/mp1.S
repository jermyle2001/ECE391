
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64

	# Added in PREV for prev node so that we can remove nodes easier
	PREV 	= 12
	CITY_BASE = 20
	ROW_OFFSET = 160 	# 80 characters per row * 2 bytes per character
	COL_OFFSET = 2		# 2 bytes per character
	CITIES_ROW_OFFSET = 3840 # 24 rows * 2 bytes per character * 80 characters per row
	X_MAX = 79			# Max X value is 79
	X_MIN = 0			# Min X value is 0
	Y_MAX = 24 			# Max Y value is 24
	Y_MIN = 0 			# Min Y value is 0
	SIZE = 36 	# Size of struct is 36 bytes 

# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score

get_status_ptr: .long 0x0	# Will contain 32 bit integer for getStatus

# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
# Callee Subroutine Stack Setup
pushl 	%ebp 		
movl 	%esp, %ebp
pushl 	%edi 				
pushl 	%ebx
pushl 	%esi 

# Function Body
# Update Missiles
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 

	call update_missiles 

	# Caller Subroutine Stack Teardown
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 

# Redraw Cities
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 

	call tasklet_redraw_cities 

	# Caller Subroutine Stack Teardown
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 

# Redraw Crosshairs
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 

	call tasklet_redraw_crosshairs 

	# Caller Subroutine Stack Teardown
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 

# Callee Subroutine Stack Teardown
popl 	%esi 
popl 	%ebx 
popl 	%edi
leave 
ret

tasklet_redraw_cities:
	# Callee Subroutine Stack Setup
	pushl 	%ebp 		
	movl 	%esp, %ebp
	pushl 	%edi 				
	pushl 	%ebx
	pushl 	%esi 

# Function Body
	# Check if city is alive. If so, print as alive
	# Push local var i into stack for counting (print 3 cities)
	pushl 	$0 		# Effectively our local variable "i"
	cities_loop_start:
		# Check if all three cities have been printed. If so, teardown stack and leave. 
		movl 	(%esp), %ecx 		# Move i into ecx
		cmpl 	$3, %ecx 			# Set EFLAGS against #3
		jge 	finish_cities		# Finish printing cities if >= 3
		# Check if city is alive at locations 20, 40, and 60 (in order)
		movl 	$CITY_BASE, %ecx 	# Set ecx to 20 so we can multiply by i 
		movl 	(%esp), %eax 		# Set eax to i so that we can multiply by i
		mull 	%ecx 				# Multiply i * 20, we have found the offset from base position that we want to print at, stored in eax
		addl 	%eax, %ecx 			# Add base position offset to ecx
		# Check if city dead or alive, shift array i times and mask other values
		movl 	base_alive, %ebx 	# Put array into ebx
		movl 	(%esp), %eax 		# Load i into eax
		cmpl 	$0, (%esp) 			# Check if i == 0, check first byte in array
		je 		load_first_city 	# Load first city if so
		cmpl 	$1, (%esp) 			# Check if i == 1 , check second byte in array
		je 		load_second_city 	# Load second city if so
		jmp 	load_third_city 	# Otherwise, i == 2, load third city
		load_first_city:
			andl 	$0x00000001, %ebx 	# Check only first relevant byte
			jmp 	check_alive 		# Check if city alive
		load_second_city:
			andl 	$0x00000100, %ebx 	# Check only second relevant byte
			jmp 	check_alive 		# Check if city alive
		load_third_city:
			andl 	$0x00010001, %ebx 	# Check only third relevant byte
			jmp 	check_alive 		# Check if city alive

		check_alive:
		cmpl 	$0, %ebx 			# Check if base is alive
		je 		print_dead_city		# Check if base dead, print dead city.
		jmp		print_live_city		# Check if base alive, print live city.

		print_dead_city:
			# Dead city, print 'x' 5 times at location
			# Calculate offset, x value already stored in ecx, print at y = 24
			subl 	$2, %ecx 		# Subtract 2 from starting position, since that's the center of our city
			movl 	%ecx, %eax 		# We want to multiply by col offset, move into eax first to multiply
			movl 	$COL_OFFSET, %edx	# Load COL_OFFSET into edx so we can multiply 
			mull 	%edx			# Multiply by COL_OFFSET
			addl 	$CITIES_ROW_OFFSET, %eax 	# Add and store row offset into eax
			
			pushl 	%ecx 			# Save ecx, will get clobbered in mp1_poke
			movb 	$120, %cl 		# Set cl to 'x' as city base
			# Call poke
			call mp1_poke 			# Print city (1/5)
			popl 	%ecx 			# Pop ecx off stack

			addl 	$1, %ecx 		# Add 1 to position
			movl 	%ecx, %eax 		# We want to multiply by col offset, move into eax first to multiply
			movl 	$COL_OFFSET, %edx	# Load COL_OFFSET into edx so we can multiply 
			mull 	%edx			# Multiply by COL_OFFSET
			addl 	$CITIES_ROW_OFFSET, %eax 	# Add and store row offset into eax

			pushl 	%ecx 			# Save ecx, will get clobbered in mp1_poke
			movb 	$120, %cl 		# Set cl to 'x' as city base
			# Call poke
			call mp1_poke 			# Print city (2/5)
			popl 	%ecx 			# Pop ecx off stack

			addl 	$1, %ecx 		# Add 1 to position
			movl 	%ecx, %eax 		# We want to multiply by col offset, move into eax first to multiply
			movl 	$COL_OFFSET, %edx	# Load COL_OFFSET into edx so we can multiply 
			mull 	%edx			# Multiply by COL_OFFSET
			addl 	$CITIES_ROW_OFFSET, %eax 	# Add and store row offset into eax

			pushl 	%ecx 			# Save ecx, will get clobbered in mp1_poke
			movb 	$120, %cl 		# Set cl to 'x' as city base
			# Call poke
			call mp1_poke 			# Print city (3/5)
			popl 	%ecx 			# Pop ecx off stack

			addl 	$1, %ecx 		# Add 1 to position
			movl 	%ecx, %eax 		# We want to multiply by col offset, move into eax first to multiply
			movl 	$COL_OFFSET, %edx	# Load COL_OFFSET into edx so we can multiply 
			mull 	%edx			# Multiply by COL_OFFSET
			addl 	$CITIES_ROW_OFFSET, %eax 	# Add and store row offset into eax

			pushl 	%ecx 			# Save ecx, will get clobbered in mp1_poke
			movb 	$120, %cl 		# Set cl to 'x' as city base
			# Call poke
			call mp1_poke 			# Print city (4/5)
			popl 	%ecx 			# Pop ecx off stack

			addl 	$1, %ecx 		# Add 1 to position
			movl 	%ecx, %eax 		# We want to multiply by col offset, move into eax first to multiply
			movl 	$COL_OFFSET, %edx	# Load COL_OFFSET into edx so we can multiply 
			mull 	%edx			# Multiply by COL_OFFSET
			addl 	$CITIES_ROW_OFFSET, %eax 	# Add and store row offset into eax

			pushl 	%ecx 			# Save ecx, will get clobbered in mp1_poke
			movb 	$120, %cl 		# Set cl to 'x' as city base
			# Call poke
			call mp1_poke 			# Print city (5/5)
			popl 	%ecx 			# Pop ecx off stack

			# Finished drawing city, increment i and check if we need to draw another city
			addl 	$1, (%esp)		# Add 1 to i
			jmp 	cities_loop_start # Jump to beginning of loop

		print_live_city:
			# Live city, print /^^^\ at location using similar method
			# Calculate offset, x value already stored in ecx, print at y = 24
			subl 	$2, %ecx 		# Subtract 2 from starting position, since that's the center of our city
			movl 	%ecx, %eax 		# We want to multiply by col offset, move into eax first to multiply
			movl 	$COL_OFFSET, %edx	# Load COL_OFFSET into edx so we can multiply 
			mull 	%edx			# Multiply by COL_OFFSET
			addl 	$CITIES_ROW_OFFSET, %eax 	# Add and store row offset into eax
			
			pushl 	%ecx 			# Save ecx, will get clobbered in mp1_poke
			movb 	$47, %cl 		# Set cl to '/' as city base
			# Call poke
			call mp1_poke 			# Print city (1/5)
			popl 	%ecx 			# Pop value back into ecx 

			addl 	$1, %ecx 		# Add 1 to position
			movl 	%ecx, %eax 		# We want to multiply by col offset, move into eax first to multiply
			movl 	$COL_OFFSET, %edx	# Load COL_OFFSET into edx so we can multiply 
			mull 	%edx			# Multiply by COL_OFFSET
			addl 	$CITIES_ROW_OFFSET, %eax 	# Add and store row offset into eax
			
			pushl 	%ecx 			# Save ecx, will get clobbered in mp1_poke
			movb 	$94, %cl 		# Set cl to '^' as city base
			# Call poke
			call mp1_poke 			# Print city (1/5)
			popl 	%ecx 			# Pop value back into ecx 
			
			addl 	$1, %ecx 		# Add 1 to position
			movl 	%ecx, %eax 		# We want to multiply by col offset, move into eax first to multiply
			movl 	$COL_OFFSET, %edx	# Load COL_OFFSET into edx so we can multiply 
			mull 	%edx			# Multiply by COL_OFFSET
			addl 	$CITIES_ROW_OFFSET, %eax 	# Add and store row offset into eax
			
			pushl 	%ecx 			# Save ecx, will get clobbered in mp1_poke
			movb 	$94, %cl 		# Set cl to '^' as city base
			# Call poke
			call mp1_poke 			# Print city (1/5)
			popl 	%ecx 			# Pop value back into ecx 
			
			addl 	$1, %ecx 		# Add 1 to position
			movl 	%ecx, %eax 		# We want to multiply by col offset, move into eax first to multiply
			movl 	$COL_OFFSET, %edx	# Load COL_OFFSET into edx so we can multiply 
			mull 	%edx			# Multiply by COL_OFFSET
			addl 	$CITIES_ROW_OFFSET, %eax 	# Add and store row offset into eax
			
			pushl 	%ecx 			# Save ecx, will get clobbered in mp1_poke
			movb 	$94, %cl 		# Set cl to '^' as city base
			# Call poke
			call mp1_poke 			# Print city (1/5)
			popl 	%ecx 			# Pop value back into ecx 
			
			addl 	$1, %ecx 		# Add 1 to position
			movl 	%ecx, %eax 		# We want to multiply by col offset, move into eax first to multiply
			movl 	$COL_OFFSET, %edx	# Load COL_OFFSET into edx so we can multiply 
			mull 	%edx			# Multiply by COL_OFFSET
			addl 	$CITIES_ROW_OFFSET, %eax 	# Add and store row offset into eax
			
			pushl 	%ecx 			# Save ecx, will get clobbered in mp1_poke
			movb 	$92, %cl 		# Set cl to '\' as city base
			# Call poke
			call mp1_poke 			# Print city (1/5)
			popl 	%ecx 			# Pop value back into ecx 

			# Finished drawing city, increment i and check if we need to draw another city
			addl 	$1, (%esp)		# Add 1 to i
			jmp 	cities_loop_start # Jump to beginning of loop

	finish_cities:
	subl 	$4, %esp 	# Remove local variable i from stack
	# Tear down stack and return function
	popl 	%esi 
	popl 	%ebx 
	popl 	%edi 
	leave  
	ret



tasklet_redraw_crosshairs:
# Callee Subroutine Stack Setup
	pushl 	%ebp 		
	movl 	%esp, %ebp
	pushl 	%edi 				
	pushl 	%ebx
	pushl 	%esi 

# Function Body
	# Calculate the row offset, store in ecx
	movl 	crosshairs_y, %eax 		# Store initial y value in eax
	movl 	$ROW_OFFSET, %edx 		# Store ROW_OFFSET into edx
	mull 	%edx					# Multiply by row offset
	movl 	%eax, %ecx 				# Store into ecx so that we can add it later
	# Calculate the col offset, store in eax
	movl 	crosshairs_x, %eax 		# Store initial x value in eax
	movl 	$COL_OFFSET, %edx 		# Store COL_OFFSET into edx
	mull 	%edx 					# Multiply by col offset
	# Add offsets together into eax so that we can print onto screen
	addl 	%ecx, %eax 				# Add offsets together
	movb 	$0x2B, %cl 				# Set cl to '+' as crosshair
	# Print crosshair onto screen with mp1_poke
	call 	mp1_poke

# Tear down stack and return function
popl 	%esi 
popl 	%ebx 
popl 	%edi 
leave  
ret




# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
	# Use argument "cmd" to determine which ioctl function to execute
		movl 	8(%esp), %ecx 				# Reference cmd at ebp, move into eax
		cmpl 	$0, %ecx
		jl		done_ioctl
		cmpl 	$5, %ecx 
		jg 		done_ioctl 
		jmp 	*jump_table(, %ecx, 4)
		ret


# ----------------- Functions private to this file -------------------

update_missiles:
# Save Callee Saved Registers
pushl 	%ebp 			# Push Base Pointer of previous frame onto stack
movl 	%esp, %ebp 		# Get new Base Pointer as current Stack Pointer
pushl 	%edi 				
pushl 	%ebx
pushl 	%esi 		
	
# Function Body
# Traverse down list of missiles, updating their position and status...
	missile_start_at_head:
	# First check if head is NULL. If so, return...
	# Check if missile exploding. If not, then update position. If so, update explosion.
	movl 	mp1_missile_list, %ecx 		# Store head missile into ecx
	
	missile_loop: 						# Check if not starting at head, missile already in ecx.
	cmpl 	$0, %ecx 					# Check if pointer in ecx is NULL. If so, finished updating missiles
	je 		update_missile_teardown		# Jump to teardown if finished
	cmpl 	$0, EXPLODED(%ecx) 			# Check if missile at EXPLODED is == 0
	jne 	update_explosion			# Check if EXPLODED != 0, then currently exploding, update as such
	jmp 	update_position				# Else, update position

	update_position:
	# Overwrite previous missile position first
	movl 	%ecx, %ebx 				# Move pointer to missile from ecx to ebx since ecx is needed for mp1_poke()
	# Calculate the row offset, store in ecx
	movl 	Y(%ebx), %eax 			# Store initial y value in eax
	sarl 	$16, %eax 				# Shift value to the right by 16 for video memory
	movl 	$ROW_OFFSET, %edx 		# Store ROW_OFFSET into edx
	mull 	%edx					# Multiply by row offset
	movl 	%eax, %ecx 				# Store into ecx so that we can add it later
	# Calculate the col offset, store in eax
	movl 	X(%ebx), %eax  			# Store initial x value in eax
	sarl 	$16, %eax 				# Shift value to the right by 16 for video memory
	movl 	$COL_OFFSET, %edx 		# Store COL_OFFSET into edx
	mull 	%edx 					# Multiply by col offset
	# Add offsets together into eax so that we can print onto screen
	addl 	%ecx, %eax 				# Add offsets together
	movb 	$32, %cl 				# Set cl to 'SPACE' to set blank
	# Overwrite old missile with mp1_poke
	call 	mp1_poke
	# Old missile overwritten, we are now free to update position
	# Update X
	movl 	%ebx, %ecx 				# Restore ecx as pointer for struct for organizational purposes (so I don't confuse myself)
	movl 	X(%ecx), %ebx 			# Store X value into ebx so that we can change it
	movl 	VX(%ecx), %edx 			# Store VX value into edx so that we can add it to X value
	addl 	%edx, %ebx 				# Add X and VX, updating its new value
	movl 	%ebx, X(%ecx)			# Store new value of X in corresponding location
	# Update Y
	movl 	Y(%ecx), %ebx 			# Store Y value into ebx so that we can change it
	movl 	VY(%ecx), %edx 			# Store VY value into edx so that we can add it to Y value
	addl 	%edx, %ebx 				# Add Y and VY, updating its new value
	movl 	%ebx, Y(%ecx) 			# Store new value of Y in corresponding location
	# Now check if missile stays within bounds. If not, remove missile.
	# ecx still holds the pointer to our missile
	jmp		check_destination 		# Jump to update_remove_missile if less than min value. Else, check if destination reached

	update_remove_missile:
	# Remove missile if out of bounds
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 

	call remove_missile 

	# Pointer to next missile returned, teardown stack and loop again
	movl 	%eax, %ebx 				# eax gets clobberd by stack teardown, move to ebx to save for now...
	# Caller Subroutine Stack Teardown
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 

	movl 	%ebx, %ecx 				# Move next missile into ecx, loop back to beginning...
	jmp 	missile_loop			

	check_destination:
	# DON'T CARE FOR NOW, SET VELOCITIES TO ZERO IF DESTINATION REACHED
	# Check if X destination reached 
	movl 	DEST_X(%ecx), %ebx 		# Move X destination into ebx for comparison
	movl 	X(%ecx), %esi 			# Shift value right by 16 for comparison
	shrl 	$16, %esi 				# ^^
	cmpl 	%esi, %ebx	 			# Set flags for comparison
	je 		check_y_dest			# Check Y destination if true. If not, we don't care and keep looping...
	jmp 	draw_missile			# WIP Draw missile regardless
		check_y_dest:				
		movl 	DEST_Y(%ecx), %ebx 		# Move Y destination into ebx for comparison
		movl 	Y(%ecx), %esi 			# Shift value right by 16 for comparison
		shrl 	$16, %esi 				# 
		cmpl 	%esi, %ebx 				# Set flags to check if == to destination
		je 		update_explosion 		# Explode missile if so
		jmp 	draw_missile 			# Otherwise, draw missile as normal

	update_explosion:
	movl 	$0, VX(%ecx)
	movl 	$0, VY(%ecx)
	# Call missile_explode
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 
	# Push current node as arg to missile_explode
	pushl 	%ecx 	

	call 	missile_explode 

	# Pointer to next missile returned, teardown stack and loop again
	movl 	%eax, %ebx 				# eax gets clobberd by stack teardown, move to ebx to save for now...
	# Caller Subroutine Stack Teardown
	addl 	$4, %esp 				# Pop arg off stack
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 

	# Check if notify_user needs to be called 
	cmpl 	$0, %ebx 				# Nonzero if game state has changed
	je 		dec_explosion			# Decrement explosion counter if zero, otherwise notify user
	jmp 	notify_user 			# Otherwise, notify the user

	notify_user:
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 

	call 	mp1_notify_user 

	# Pointer to next missile returned, teardown stack and loop again
	movl 	%eax, %ebx 				# eax gets clobberd by stack teardown, move to ebx to save for now...
	# Caller Subroutine Stack Teardown
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 
	jmp 	dec_explosion 			# Decrement explosion after notifying user

	dec_explosion:
	addl 	$-1, EXPLODED(%ecx) 	# ecx should still have missile pointer in it, dec EXPLODED
	movl 	EXPLODED(%ecx), %esi 	# Move EXPLODED value to esi so that we can compare it
	cmpl 	$0, %esi 				# Check if EXPLODED has ended (==0), finish explosion if so. Otherwise, print explosion.
	je 		finish_explosion		# Jump to finish_explosion if EXPLODED == 0
	jmp 	draw_explosion 			# Jump to draw_explosion if not finished

	finish_explosion:
	# Clear out the symbol at the current location on the screen, since the explosion has finished
	movl 	%ecx, %ebx 				# Move pointer to missile from ecx to ebx since ecx is needed for mp1_poke()
	# Calculate the row offset, store in ecx
	movl 	Y(%ebx), %eax 			# Store initial y value in eax
	sarl 	$16, %eax 				# Shift value to the right by 16 for video memory
	movl 	$ROW_OFFSET, %edx 		# Store ROW_OFFSET into edx
	mull 	%edx					# Multiply by row offset
	movl 	%eax, %ecx 				# Store into ecx so that we can add it later
	# Calculate the col offset, store in eax
	movl 	X(%ebx), %eax  			# Store initial x value in eax
	sarl 	$16, %eax 				# Shift value to the right by 16 for video memory
	movl 	$COL_OFFSET, %edx 		# Store COL_OFFSET into edx
	mull 	%edx 					# Multiply by col offset
	# Add offsets together into eax so that we can print onto screen
	addl 	%ecx, %eax 				# Add offsets together
	movb 	$32, %cl 				# Set character of explosion to "@"
	# Overwrite old missile with mp1_poke
	call 	mp1_poke
	# Missile printed, set next missile to check and loop back
	movl 	%ebx, %ecx 				# Current missile stored in ebx
	# Now we have to remove the missile
	jmp 	update_remove_missile 	# Jump to stack setup and teardown for remove missile, assumes ecx has the current node




	draw_explosion:
	movl 	%ecx, %ebx 				# Move pointer to missile from ecx to ebx since ecx is needed for mp1_poke()
	# Calculate the row offset, store in ecx
	movl 	Y(%ebx), %eax 			# Store initial y value in eax
	sarl 	$16, %eax 				# Shift value to the right by 16 for video memory
	movl 	$ROW_OFFSET, %edx 		# Store ROW_OFFSET into edx
	mull 	%edx					# Multiply by row offset
	movl 	%eax, %ecx 				# Store into ecx so that we can add it later
	# Calculate the col offset, store in eax
	movl 	X(%ebx), %eax  			# Store initial x value in eax
	sarl 	$16, %eax 				# Shift value to the right by 16 for video memory
	movl 	$COL_OFFSET, %edx 		# Store COL_OFFSET into edx
	mull 	%edx 					# Multiply by col offset
	# Add offsets together into eax so that we can print onto screen
	addl 	%ecx, %eax 				# Add offsets together
	movb 	$64, %cl 				# Set character of explosion to "@"
	# Overwrite old missile with mp1_poke
	call 	mp1_poke
	# Missile printed, set next missile to check and loop back
	movl 	NEXT(%ebx), %ecx 		# Current missile stored in ebx
	jmp 	missile_loop			# Jump back to loop


	draw_missile:
	movl 	%ecx, %ebx 				# Move pointer to missile from ecx to ebx since ecx is needed for mp1_poke()
	# Calculate the row offset, store in ecx
	movl 	Y(%ebx), %eax 			# Store initial y value in eax
	sarl 	$16, %eax 				# Shift value to the right by 16 for video memory
	movl 	$ROW_OFFSET, %edx 		# Store ROW_OFFSET into edx
	mull 	%edx					# Multiply by row offset
	movl 	%eax, %ecx 				# Store into ecx so that we can add it later
	# Calculate the col offset, store in eax
	movl 	X(%ebx), %eax  			# Store initial x value in eax
	sarl 	$16, %eax 				# Shift value to the right by 16 for video memory
	movl 	$COL_OFFSET, %edx 		# Store COL_OFFSET into edx
	mull 	%edx 					# Multiply by col offset
	# Add offsets together into eax so that we can print onto screen
	addl 	%ecx, %eax 				# Add offsets together
	movb 	C(%ebx), %cl 			# Set character of missile is stored in C(pointer)
	# Overwrite old missile with mp1_poke
	call 	mp1_poke
	# Missile printed, set next missile to check and loop back
	movl 	NEXT(%ebx), %ecx 		# Current missile stored in ebx
	jmp 	missile_loop			# Jump back to loop


update_missile_teardown:
# Callee Teardown Registers
popl 	%esi 
popl 	%ebx 
popl 	%edi 
leave 
ret

# INPUTS: Pointer to missile to be removed (presumably was in ecx on caller saved registers, will be referenced accordingly)
# OUTPUTS: Pointer to next missile (in eax)
remove_missile:
# Save Callee Saved Registers
pushl 	%ebp 			# Push Base Pointer of previous frame onto stack
movl 	%esp, %ebp 		# Get new Base Pointer as current Stack Pointer
pushl 	%edi 				
pushl 	%ebx
pushl 	%esi 

# Function Body
# Returns pointer to next missile, also resets head if appropriate - change NEXT BEFORE we free memory, NOT after
	# Check if removing head
	movl 	12(%ebp), %ecx 				# Store pointer to missile we are removing into ecx
	movl 	NEXT(%ecx), %eax 			# Store next missile into eax so that we can return it
	cmpl 	mp1_missile_list, %ecx 		# Compare missile to head
	je 		rm_reset_head				# Reset head if missile is head. If not, we need to find prev node.
	jmp 	rm_find_prev				# Find previous node if not head

	rm_reset_head:
	# Reset head, set new head first
	movl 	%eax, mp1_missile_list 		# New head is NEXT of current missile (already stored into eax). Could be zero.
	jmp 	missile_free_memory			# Head reset, proceed to free memory

	rm_find_prev:
	# Need to find previous missile so we can remove current and fix hole in b/w nodes
	movl 	mp1_missile_list, %ebx 		# Set ebx to head so that we can compare its "next" to current missile, we already know we are NOT removing the head
		rm_find_prev_loop:				
		cmpl 	NEXT(%ebx), %ecx 		# Check if NEXT(ebx) is ecx then we have found our prev node. Set its next to ecx's next and free memory
		je 		rm_set_prev_next		# Set previous node's next to current's next. Else, keep looping
		movl 	NEXT(%ebx), %ebx 		# Set ebx to next node
		jmp 	rm_find_prev_loop		# Jump back to beginning of loop
		rm_set_prev_next:
		movl 	%eax, NEXT(%ebx)		# Next node already stored in eax, move into NEXT of previous node
		jmp 	missile_free_memory		# Now free memory at current missile
	
	missile_free_memory:
	# Set up for call to mp1_free
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 
	# Push argument to stack, stored in ecx
	pushl	%ecx
	
	call mp1_free 

	# Nothing returned, add stack pointer to release arg, also save return value in another register
	addl 	$4, %esp 		# add stack pointer to release arg
	# Caller Subroutine Stack Teardown
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 
	jmp 	remove_missile_teardown

remove_missile_teardown:
# Callee Teardown Registers
popl 	%esi 
popl 	%ebx 
popl 	%edi 
leave 
ret


mp1_ioctl_startgame:
	pushl 	%ebp 					# Push Base Pointer of previous frame onto stack
	movl 	%esp, %ebp 				# Get new Base Pointer as current Stack Pointer
	movl 	$0, mp1_missile_list 	# Initialize missile list to be NULL, currently no missiles at start
	movl 	$0x00010101, base_alive # Initialize bases to be alive, two MSBytes are padding so set to zero at start
	movl 	$0, mp1_score			# Initialize score to be zero upon at start
	movl 	$40, crosshairs_x		# Initialize crosshair x to 40 at start
	movl 	$12, crosshairs_y		# Initialize crosshair y to be 12 at start
	leave
	ret

mp1_ioctl_addmissile:
# Adds missile to head of list, sets its next as well
# Save Callee Saved Registers
pushl 	%ebp 			# Push Base Pointer of previous frame onto stack
movl 	%esp, %ebp 		# Get new Base Pointer as current Stack Pointer
pushl 	%edi 				
pushl 	%ebx
pushl 	%esi 	

# Function Body
# We want to call malloc to allocate new space for registers, save Caller Saved Registers...
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 
	# Push args to stack before calling malloc
	pushl 	$SIZE			# Push size of struct to stack as arg (36 bytes)
	# Call malloc
	call mp1_malloc			# Allocate new memory, never fails, returns in eax
	popl 	%ebx 			# Pop argument off stack, throw it away
	movl 	%eax, %ebx 		# Move return value out of eax before we teardown stack
	# Caller Subroutine Stack Teardown
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 
	# Check if malloc was successful (!= -1). If not, free associated memory...
	cmpl 	$-1, %ebx 		# Pointer is stored in ebx, compare to see if it failed...
	je 		malloc_fail 	# Jump to malloc_fail if returned value is -1, else continue and copy to kernel...
# Copy new missile into kernel
	movl 	8(%ebp), %ecx 			# Get pointer to user location as argument to dispatcher in stack
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 
	# Push args to stack before calling unsigned long mp1_copy_from_user(void *to, const void *from, unsigned long n)
	pushl 	$SIZE 					# Push arg n to stack
	pushl 	%ecx 					# Push arg *from to stack
	pushl 	%ebx 					# Push arg *to to stack
	# Call subroutine
	call mp1_copy_from_user			# Copies data from user to kernel
	# Caller Subroutine Stack Teardown
	movl 	%eax, %esi 				# Move return value out before we teardown
	subl 	$12, %esp 				# Subtract 12 from esp to effectively release values from stack
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 
	# Check if copy was successful - if not, free memory and return
	cmpl 	$0, %esi 				# Should be zero if successful. If anything else, copy failed...
	jne 	copy_fail				# Jump if copy failed
	movl 	$0, %eax 				# Return 0 if nothing failed
	# New struct pointer stored in ebx, set its NEXT value to the current head and set new head
	movl 	mp1_missile_list, %eax 		# Move data at label to eax so that we can set its new value
	movl 	%eax, NEXT(%ebx) 			# Move value of current head into NEXT of new missile
	movl 	%ebx, mp1_missile_list 		# Set new head to be new missile
	jmp 	addmissile_teardown		# Else, we are finished with function, teardown stack


	malloc_fail:
		# Return -1 if malloc fails
		movl 	$-1, %eax 		# Malloc failed, return -1...
		jmp addmissile_teardown	# Jump to teardown...
	copy_fail:
		# Free memory if copy fails
		movl 	mp1_missile_list, %ecx 	# Set ecx to head, since we already set it
		movl 	NEXT(%ecx), %eax 		# Set eax to our new functional head
		# Caller Subroutine Stack Setup
		pushl 	%eax 
		pushl 	%ecx 
		pushl 	%edx 
		# Push arg to stack (pointer to which we free)
		pushl 	mp1_missile_list 		# Our head that we set is the memory we allocated, 
		# Free memory
		call 	mp1_free 				# Frees memory at current head, which is our new missile
		# Caller Suroutine Stack Teardown
		popl 	%edx 
		popl 	%ecx 
		popl 	%eax 					# eax contains our new head, reset head to such
		movl 	%eax, mp1_missile_list 	# Set head to old head
		movl 	$-1, %eax 				# Return -1 if failed
		jmp 	addmissile_teardown 	# Finished, jump to teardown

addmissile_teardown:
# Callee Teardown Registers
popl 	%esi 
popl 	%ebx 
popl 	%edi 
leave 
ret

mp1_ioctl_movexhairs:
# Save Callee Saved Registers
pushl 	%ebp 			# Push Base Pointer of previous frame onto stack
movl 	%esp, %ebp 		# Get new Base Pointer as current Stack Pointer
pushl 	%edi 				
pushl 	%ebx
pushl 	%esi 

# Function Body
	# Print over previous crosshair...
	# Calculate the row offset, store in ecx
	movl 	crosshairs_y, %eax 		# Store initial y value in eax
	movl 	$ROW_OFFSET, %edx 		# Store ROW_OFFSET into edx
	mull 	%edx					# Multiply by row offset
	movl 	%eax, %ecx 				# Store into ecx so that we can add it later
	# Calculate the col offset, store in eax
	movl 	crosshairs_x, %eax 		# Store initial x value in eax
	movl 	$COL_OFFSET, %edx 		# Store COL_OFFSET into edx
	mull 	%edx 					# Multiply by col offset
	# Add offsets together into eax so that we can print onto screen
	addl 	%ecx, %eax 				# Add offsets together
	movb 	$32, %cl 				# Set cl to 'SPACE' to set blank
	# Overwite old crosshair with mp1_poke
	call 	mp1_poke

	change_crosshair_x:
	# Low 16 bits contain x change in crosshair position
	movl 	8(%ebp), %ecx 			# Get new value of crosshairs in exc
	andl 	$0x0000FFFF, %ecx 		# Mask lower bits, push into ecx
	movl 	crosshairs_x, %edx 		# Move value of crosshairs_x to edx so we can decrease it later
	cmpw 	$0, %cx 				# Compare lower half of register to see if we need to change x value
	je 		change_crosshair_y		# Check if zero, nothing to do here, change y component of crosshair
	jl 		sub_crosshair_x			# Check if less than zero, decrement value of x component by 1
	jg 		add_crosshair_x			# Check if greater than zero, increment value of x component by 1
	sub_crosshair_x:
		decl 	%edx 				# Decrement edx by 1
		movl 	%edx, crosshairs_x 	# Store new value of crosshairs_x into crosshairs_x
		jmp 	change_crosshair_y 	# x crosshair changed, now see if y changes too
	add_crosshair_x:
		incl 	%edx 				# Increment edx by 1
		movl 	%edx, crosshairs_x 	# Store new value of crosshairs_x into crosshairs_x
		jmp 	change_crosshair_y 	# x crosshair changed, now see if y changes too

	change_crosshair_y:
	# High 16 bits contain y change in crosshair position
	movl 	8(%ebp), %ecx 			# Get new value of crosshairs in exc
	andl 	$0xFFFF0000, %ecx 		# Mask higher bits, push into ecx
	shrl 	$16, %ecx 				# Shift higher bits into lower half of register
	movl 	crosshairs_y, %edx 		# Move value of crosshairs_x to edx so we can decrease it later
	cmpw 	$0, %cx 				# Compare lower half of register to see if we need to change the y value
	je 		limit_crosshairs 		# Check if zero, nothing to do here, see if crosshair out of bounds
	jl 		sub_crosshair_y			# Check if less than zero, decrement value of y component by 1
	jg 		add_crosshair_y			# Check if greater than zero, increment value of y component by 1
	sub_crosshair_y:
		decl 	%edx 				# Decrement edx by 1
		movl 	%edx, crosshairs_y 	# Store new value of crosshairs_y into crosshairs_y
		jmp 	limit_crosshairs 	# Finished changing crosshair, check if in bounds
	add_crosshair_y:
		incl 	%edx 				# Decrement edx by 1
		movl 	%edx, crosshairs_y 	# Store new value of crosshairs_y into crosshairs_y
		jmp 	limit_crosshairs 	# Finished changing crosshair, check if in bounds

	limit_crosshairs:
	# Now limit the crosshairs to bounds of screen
	# x limited to between 0-79
	# Check if x is less than zero
	cmpl 	$0, crosshairs_x 		# Check if crosshairs_x is less than zero
	jge 	check_crosshair_x_max 	# Check if x >= 0, check if greater than 79 (max bounds)
	movl 	$0, crosshairs_x 		# Set x value to zero to prevent out of bounds
	check_crosshair_x_max:
	cmpl 	$X_MAX, crosshairs_x 	# Check if crosshairs_x is greater than 79
	jle 	check_crosshair_y_min 	# Check if x <= 79, check if y values in bounds
	movl 	$X_MAX, crosshairs_x 	# Set x value to 79 to prevent out of bounds
	check_crosshair_y_min:			
	cmpl 	$0, crosshairs_y 		# Check if crosshairs_x is less than zero
	jge 	check_crosshair_y_max 	# Check if y >= 0, check if greater than 24 (max bounds)
	movl 	$0, crosshairs_y 		# Set y value to zero to prevent out of bounds
	check_crosshair_y_max:
	cmpl 	$Y_MAX, crosshairs_y 	# Check if crosshairs_x is greater than 24
	jle 	finish_movexhairs	 	# Check if y <= 24, check if y values in bounds
	movl 	$Y_MAX, crosshairs_y 	# Set y value to 24 to prevent out of bounds
	finish_movexhairs:
		
# Callee Teardown Registers
popl 	%esi 
popl 	%ebx 
popl 	%edi 
leave 
ret

mp1_ioctl_getstatus:
# Save Callee Saved Registers
pushl 	%ebp 			# Push Base Pointer of previous frame onto stack
movl 	%esp, %ebp 		# Get new Base Pointer as current Stack Pointer
pushl 	%edi 				
pushl 	%ebx
pushl 	%esi 

# Function Body
movl 	8(%ebp), %ecx 			# Get pointer to user 32-bit integer in user space as argument
movl 	mp1_score, %ebx 		# Move score to ebx so that we can put it into ecx
# Check bases
movl 	base_alive, %eax 		# Move base_alive array to eax to check first city
andl 	$0x00010000, %eax 		# Check if first base alive
# shll 	$2, %eax 				# Shift left twice into corresponding spot (move from bit 16 to bit 18)
movl 	base_alive, %esi 		# Move base_alive array into esi to check second city
andl 	$0x00000100, %esi 		# Check if second base alive
shll 	$9, %esi 				# Shift left nine times into corresponding spot (move from bit 8 to bit 17)
orl 	%esi, %eax 				# OR with eax to store into same array
movl 	base_alive, %esi 		# Move base_alive array into esi to check third city
andl 	$0x00000001, %esi 		# Check if third base alive
shll 	$18, %esi 				# Shift left sixteen times into corresponding spot (move from bit 0 to bit 16)
orl 	%esi, %eax 				# OR with eax to store into same array
orl 	%eax, %ebx 				# OR with ebx to store into full array
# Copy to user from kernel
movl 	%ebx, get_status_ptr	# Store into get_status_ptr so that we have an address to copy to userspace
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 
	# Push args to stack, right to left 
	pushl 	$4					# 32 bits, or 4 bytes
	pushl 	$get_status_ptr 	# Push the ADDRESS of get_status_ptr, and not its value
	pushl 	%ecx 				# Pointer to userspace 32-bit integer already stored in ecx
	call 	mp1_copy_to_user
	# Pop args off stack
	addl 	$12, %esp 
	movl 	%eax, %ebx 			# Move return value out of eax, since it gets clobbered by teardown
	# Caller Subroutine Stack Teardown
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 
# Check if copy was successful. Set return value accordingly...
cmpl 	$0, %ebx 				# Set flags, compare to 0...
jne 	status_return_else 		# Return -1 if nonzero (unsuccessful copy)
jmp 	status_return_zero 		# Return 0 if successful copy
status_return_zero:
movl 	$0, %eax 				# Set return value to 0 after successful copy
jmp 	get_status_teardown 	# Jump to teardown after setting return value
status_return_else:
movl 	$-1, %eax 				# Set return value to -1 after unsuccessful copy
jmp 	get_status_teardown 	# Jump to teardown after setting return value

get_status_teardown:
# Callee Teardown Registers
popl 	%esi 
popl 	%ebx 
popl 	%edi 
leave 
ret

mp1_ioctl_endgame:
# Save Callee Saved Registers
pushl 	%ebp 			# Push Base Pointer of previous frame onto stack
movl 	%esp, %ebp 		# Get new Base Pointer as current Stack Pointer
pushl 	%edi 				
pushl 	%ebx
pushl 	%esi 

# Free all associated memory, starting at HEAD
movl 	mp1_missile_list, %ecx 		# Store HEAD into ecx, we'll use it as our current node
	endgame_free_memory_loop:
	cmpl 	$0, %ecx 				# Check if current node is NULL. If so, teardown stack and finish
	je 		endgame_teardown		# Jump if NULL. Otherwise, free memory...
	# Caller Subroutine Stack Setup
	pushl 	%eax 
	pushl 	%ecx 
	pushl 	%edx 
	call 	remove_missile	# Call remove_missile
	movl 	%eax, %ebx 		# Next missile returned in eax, move out of eax before it gets clobbered by stack teardown
	# Caller Subroutine Stack Teardown
	popl 	%edx 	
	popl	%ecx
	popl 	%eax 
	
	movl 	%ebx, %ecx 		# Move next missile back into ecx, then loop
	jmp 	endgame_free_memory_loop 	# Loop back to start of loop

endgame_teardown:
# Callee Teardown Registers
popl 	%esi 
popl 	%ebx 
popl 	%edi 
leave 
ret

jump_table:
	.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame 

done_ioctl:
	ret


